#include <timer.h>
#include <rcc.h>
#include <systick.h>
#include <gpio.h>
#include <spi.h>
#include <i2c.h>
#include <gt911.h>
#include <nested-vector.h>
#include <sys-ctrl-blk.h>
#include <usb-core.h>
#include <stddef.h>


void crt0_init(void){
    extern volatile uint8_t  _sbss[], _ebss [], _sidata[];
    extern volatile uint8_t  _sdata[], _edata[];
    extern void (* const g_pfnVectors[])(void);

    volatile uint32_t *src, *dst;
    volatile uint8_t *byte_src, *byte_dst;

    SCB->VTOR = (uint32_t)g_pfnVectors;

    // Copy initialized data from Flash to RAM (optimize for large chunks)
    src = (volatile uint32_t *)&_sidata;
    dst = (volatile uint32_t *) &_sdata;
    while (dst + 1 <= (volatile uint32_t *) &_edata) {  *dst++ = *src++;  }
    // Handle remaining bytes (if the section size isn't a multiple of word size)
    byte_src = (volatile uint8_t *) src;
    byte_dst = (volatile uint8_t *) dst;
    while (byte_dst < (volatile uint8_t *) &_edata) { *byte_dst++ = *byte_src++;  }
    // Clear the .bss section (optimize for large chunks)
    dst = (volatile uint32_t *) &_sbss;
    while (dst + 1 <= (volatile uint32_t *) &_ebss) { *dst++ = 0; }
    // Handle remaining bytes in .bss
    byte_dst = (volatile uint8_t *) dst;
    while (byte_dst < (volatile uint8_t *) &_ebss) { *byte_dst++ = 0; }
}

void *memset(void *dst, int c, size_t n)
{
    uint8_t *p = (uint8_t *)dst;
    while (n--) {
        *p++ = (uint8_t)c;
    }
    return dst;
}

uint32_t irq_count = 0; // debug

extern uint8_t  bulk_tx_busy;

/* tiny unsigned->decimal-ASCII, no libc dependency */
static unsigned u16_to_dec(unsigned val, char *out)
{
    char tmp[6];
    unsigned n = 0;
    if (val == 0U) { out[0] = '0'; return 1U; }
    while (val > 0U && n < sizeof(tmp)) {
        tmp[n++] = (char)('0' + (val % 10U));
        val /= 10U;
    }
    for (unsigned i = 0; i < n; i++) out[i] = tmp[n - 1U - i];
    return n;
}

/* Builds "id,x,y;id,x,y;...\n" for up to GT911_MAX_POINTS points.
   Bails out early if it would overflow max_len (keeps us under 64B/packet). */
static unsigned format_multi_touch(char *line, unsigned max_len, const struct gt911_point_t *pts, unsigned n)
{
    unsigned p = 0;

    if (max_len == 0)
        return 0;

    for (unsigned i = 0; i < n; i++) {

        unsigned id = pts[i].id;
        unsigned x  = pts[i].x;
        unsigned y  = pts[i].y;

        /* Reserve space for newline */
        if (p + 14U >= max_len)
            break;

        p += u16_to_dec(id, line + p);
        line[p++] = ',';

        p += u16_to_dec(x, line + p);
        line[p++] = ',';

        p += u16_to_dec(y, line + p);
        line[p++] = ';';
    }

    line[p++] = '\n';

    return p;
}


#define MOUSE_MOVE_DIV          1
#define SCROLL_DIV              18
#define PINCH_DIV               24

#define TAP_MOVE_LIMIT          12U
#define DOUBLE_TAP_MS           350U

#define HID_QUEUE_LEN           16U
#define HID_QUEUE_REPORT_MAX    HID_KEYBOARD_REPORT_SIZE

enum mouse_gesture_state_t {
    MOUSE_GESTURE_NONE  = 0,
    MOUSE_GESTURE_ONE   = 1,
    MOUSE_GESTURE_TWO   = 2,
    MOUSE_GESTURE_MULTI = 3
};

struct hid_queue_entry_t {
    uint8_t len;
    uint8_t data[HID_QUEUE_REPORT_MAX];
};

static enum mouse_gesture_state_t mouse_gesture_state;

static uint8_t mouse_primary_id;
static uint16_t mouse_last_x;
static uint16_t mouse_last_y;
static uint16_t mouse_start_x;
static uint16_t mouse_start_y;

static uint8_t mouse_moved;
static uint8_t mouse_drag_active;

/*
 * A normal single tap sends left click immediately.
 *
 * If another one-finger touch begins within DOUBLE_TAP_MS, it becomes
 * a second click. We send left-button-down immediately.
 *
 * If that second touch is held and moved, it becomes a drag.
 */
static uint8_t mouse_tap_pending;
static uint32_t mouse_last_tap_time;

static uint16_t two_last_center_x;
static uint16_t two_last_center_y;
static int32_t two_last_span;

static int32_t scroll_accum_x;
static int32_t scroll_accum_y;
static int32_t pinch_accum;

static struct hid_queue_entry_t hid_queue[HID_QUEUE_LEN];
static uint8_t hid_queue_head;
static uint8_t hid_queue_tail;
static uint8_t hid_queue_count;


/* absolute signed 32-bit value without libc */
static uint32_t abs_i32(int32_t v)
{
    return (v < 0) ? (uint32_t)(-v) : (uint32_t)v;
}

/* Limit signed HID relative values to range -127..127. */
static int8_t clamp_mouse_delta(int32_t v)
{
    if (v > 127) return 127;
    if (v < -127) return -127;
    return (int8_t)v;
}

/*
 * Queue arbitrary HID report.
 *
 * Mouse reports are 6 bytes.
 * Keyboard reports are 9 bytes.
 */
static void hid_queue_raw(const uint8_t *data, uint8_t len)
{
    uint8_t i;
    struct hid_queue_entry_t *q;

    if (len == 0U || len > HID_QUEUE_REPORT_MAX) return;
    if (hid_queue_count >= HID_QUEUE_LEN) return;

    q = &hid_queue[hid_queue_head];
    q->len = len;

    for (i = 0; i < len; i++) {
        q->data[i] = data[i];
    }

    hid_queue_head++;
    if (hid_queue_head >= HID_QUEUE_LEN) hid_queue_head = 0;

    hid_queue_count++;
}

/* Queue standard mouse report: buttons, X, Y, wheel, horizontal pan. */
static void hid_queue_mouse(uint8_t buttons, int8_t x, int8_t y, int8_t wheel, int8_t pan)
{
    uint8_t report[HID_MOUSE_REPORT_SIZE];

    report[0] = HID_MOUSE_REPORT_ID;
    report[1] = buttons;
    report[2] = (uint8_t)x;
    report[3] = (uint8_t)y;
    report[4] = (uint8_t)wheel;
    report[5] = (uint8_t)pan;

    hid_queue_raw(report, HID_MOUSE_REPORT_SIZE);
}

/* Queue keyboard modifier report. Used for Ctrl during pinch zoom. */
static void hid_queue_keyboard(uint8_t modifiers)
{
    uint8_t report[HID_KEYBOARD_REPORT_SIZE];
    uint8_t i;

    for (i = 0; i < HID_KEYBOARD_REPORT_SIZE; i++) {
        report[i] = 0;
    }

    report[0] = HID_KEYBOARD_REPORT_ID;
    report[1] = modifiers;

    hid_queue_raw(report, HID_KEYBOARD_REPORT_SIZE);
}

/*
 * Ctrl + wheel is interpreted as zoom by most desktop applications:
 *
 * Browser: zoom page
 * PDF viewer: zoom document
 * Image viewer: zoom image
 * File manager: often change icon size
 *
 * Positive wheel usually means zoom in.
 * Negative wheel usually means zoom out.
 */
static void hid_queue_ctrl_wheel(int8_t wheel)
{
    if (wheel == 0) return;

    hid_queue_keyboard(HID_KEY_MOD_LEFT_CTRL);
    hid_queue_mouse(0, 0, 0, wheel, 0);
    hid_queue_keyboard(0);
}

/*
 * Send next queued HID report only after previous EP1 transmission finishes.
 *
 * HID EP1 is interrupt IN, and bulk_tx_busy is your existing endpoint busy
 * flag name from the old CDC implementation.
 */
static void hid_queue_service(void)
{
    struct hid_queue_entry_t *q;

    if (bulk_tx_busy || hid_queue_count == 0U) return;

    q = &hid_queue[hid_queue_tail];

    if (hid_send_report(q->data, q->len) == 0) {
        hid_queue_tail++;
        if (hid_queue_tail >= HID_QUEUE_LEN) hid_queue_tail = 0;

        hid_queue_count--;
    }
}

static void mouse_left_down(void)
{
    hid_queue_mouse(HID_MOUSE_BUTTON_LEFT, 0, 0, 0, 0);
}

static void mouse_left_up(void)
{
    hid_queue_mouse(0, 0, 0, 0, 0);
}

static void mouse_left_click(void)
{
    mouse_left_down();
    mouse_left_up();
}

static void mouse_reset_gesture(void)
{
    mouse_gesture_state = MOUSE_GESTURE_NONE;
    mouse_moved = 0;
    mouse_drag_active = 0;

    scroll_accum_x = 0;
    scroll_accum_y = 0;
    pinch_accum = 0;
}

/*
 * GT911 contacts have IDs.
 *
 * Do not assume points[0] remains the same finger across every read.
 * Find the continuing finger by GT911 contact ID.
 */
static const struct gt911_point_t *find_gt911_point(const struct gt911_point_t *points, uint8_t count, uint8_t id)
{
    uint8_t i;

    for (i = 0; i < count; i++) {
        if (points[i].id == id) return &points[i];
    }

    return 0;
}

/*
 * Span is a cheap pinch-distance estimate:
 *
 * abs(x1 - x0) + abs(y1 - y0)
 *
 * It avoids sqrt() and floating point.
 *
 * Wider span  = fingers farther apart
 * Narrow span = fingers closer together
 */
static int32_t two_finger_span(const struct gt911_point_t *a, const struct gt911_point_t *b)
{
    int32_t dx = (int32_t)a->x - b->x;
    int32_t dy = (int32_t)a->y - b->y;

    if (dx < 0) dx = -dx;
    if (dy < 0) dy = -dy;

    return dx + dy;
}

static void mouse_process_one_finger(const struct gt911_point_t *points)
{
    const struct gt911_point_t *p;
    int32_t dx, dy;
    uint32_t move_x, move_y;

    if (mouse_gesture_state != MOUSE_GESTURE_ONE) {
        mouse_gesture_state = MOUSE_GESTURE_ONE;

        mouse_primary_id = points[0].id;
        mouse_last_x = points[0].x;
        mouse_last_y = points[0].y;
        mouse_start_x = points[0].x;
        mouse_start_y = points[0].y;

        mouse_moved = 0;
        mouse_drag_active = 0;

        /*
         * Second touch within the double-click time:
         *
         * First tap already did left down + left up.
         * This second touch begins another click by holding left button down.
         *
         * If the finger moves: drag.
         * If the finger releases immediately: normal double click.
         */
        if (mouse_tap_pending &&
            (uint32_t)(ms_ticks - mouse_last_tap_time) <= DOUBLE_TAP_MS) {
            mouse_drag_active = 1;
            mouse_tap_pending = 0;
            mouse_left_down();
        } else {
            mouse_tap_pending = 0;
        }

        return;
    }

    p = find_gt911_point(points, 1U, mouse_primary_id);

    /* GT911 normally retains its contact ID. Rebase safely if necessary. */
    if (!p) {
        mouse_primary_id = points[0].id;
        mouse_last_x = points[0].x;
        mouse_last_y = points[0].y;
        return;
    }

    dx = (int32_t)p->x - mouse_last_x;
    dy = (int32_t)p->y - mouse_last_y;

    mouse_last_x = p->x;
    mouse_last_y = p->y;

    move_x = (p->x > mouse_start_x) ? p->x - mouse_start_x : mouse_start_x - p->x;
    move_y = (p->y > mouse_start_y) ? p->y - mouse_start_y : mouse_start_y - p->y;

    if (move_x > TAP_MOVE_LIMIT || move_y > TAP_MOVE_LIMIT) {
        mouse_moved = 1;
    }

    dx /= MOUSE_MOVE_DIV;
    dy /= MOUSE_MOVE_DIV;

    if (dx != 0 || dy != 0) {
        hid_queue_mouse(mouse_drag_active ? HID_MOUSE_BUTTON_LEFT : 0,
                        clamp_mouse_delta(dx),
                        clamp_mouse_delta(dy),
                        0,
                        0);
    }
}

static void mouse_process_two_fingers(const struct gt911_point_t *points)
{
    uint16_t center_x, center_y;
    int32_t center_dx, center_dy;
    int32_t span;
    int32_t span_delta;
    uint32_t center_motion;
    uint32_t pinch_motion;
    int32_t wheel, pan, zoom;

    center_x = (uint16_t)(((uint32_t)points[0].x + points[1].x) / 2U);
    center_y = (uint16_t)(((uint32_t)points[0].y + points[1].y) / 2U);
    span = two_finger_span(&points[0], &points[1]);

    if (mouse_gesture_state != MOUSE_GESTURE_TWO) {
        mouse_gesture_state = MOUSE_GESTURE_TWO;

        two_last_center_x = center_x;
        two_last_center_y = center_y;
        two_last_span = span;

        scroll_accum_x = 0;
        scroll_accum_y = 0;
        pinch_accum = 0;

        /*
         * Two fingers cancel pending tap/drag logic.
         *
         * If a left button happened to be held during a drag, release it.
         */
        if (mouse_drag_active) {
            mouse_left_up();
            mouse_drag_active = 0;
        }

        mouse_tap_pending = 0;
        return;
    }

    center_dx = (int32_t)center_x - two_last_center_x;
    center_dy = (int32_t)center_y - two_last_center_y;
    span_delta = span - two_last_span;

    two_last_center_x = center_x;
    two_last_center_y = center_y;
    two_last_span = span;

    center_motion = abs_i32(center_dx) + abs_i32(center_dy);
    pinch_motion = abs_i32(span_delta);

    /*
     * Distinguish slide from pinch:
     *
     * If both fingers move together, center movement is dominant -> scroll.
     * If fingers separate/approach, span change is dominant -> zoom.
     *
     * This is especially useful for diagonal gestures.
     */
    if (pinch_motion > center_motion) {
        pinch_accum += span_delta;

        if (abs_i32(pinch_accum) >= PINCH_DIV) {
            zoom = pinch_accum / PINCH_DIV;
            pinch_accum %= PINCH_DIV;

            /*
             * Fingers farther apart: positive Ctrl+wheel -> zoom in.
             * Fingers closer together: negative Ctrl+wheel -> zoom out.
             */
            hid_queue_ctrl_wheel(clamp_mouse_delta(zoom));
        }
    } else {
        scroll_accum_x += center_dx;
        scroll_accum_y += center_dy;

        pan = 0;
        wheel = 0;

        if (abs_i32(scroll_accum_x) >= SCROLL_DIV) {
            pan = scroll_accum_x / SCROLL_DIV;
            scroll_accum_x %= SCROLL_DIV;
        }

        if (abs_i32(scroll_accum_y) >= SCROLL_DIV) {
            /*
             * The minus sign gives conventional touchpad direction:
             * move fingers upward -> scroll page upward.
             *
             * If it feels reversed on your OS, remove the minus sign.
             */
            wheel = -scroll_accum_y / SCROLL_DIV;
            scroll_accum_y %= SCROLL_DIV;
        }

        if (wheel != 0 || pan != 0) {
            hid_queue_mouse(0, 0, 0,
                            clamp_mouse_delta(wheel),
                            clamp_mouse_delta(pan));
        }
    }
}

static void mouse_process_points(const struct gt911_point_t *points, uint8_t count)
{
    if (count == 0U) {
        if (mouse_gesture_state == MOUSE_GESTURE_ONE) {
            if (mouse_drag_active) {
                /*
                 * Second tap was held, possibly moved:
                 * release button and finish drag / second click.
                 */
                mouse_left_up();
            } else if (!mouse_moved) {
                /*
                 * First quick tap:
                 * send click now, then remember it briefly in case this
                 * becomes a double-tap-and-drag gesture.
                 */
                mouse_left_click();
                mouse_tap_pending = 1;
                mouse_last_tap_time = ms_ticks;
            }
        }

        mouse_reset_gesture();
        return;
    }

    if (count == 1U) {
        mouse_process_one_finger(points);
        return;
    }

    if (count == 2U) {
        mouse_process_two_fingers(points);
        return;
    }

    /*
     * GT911 can still process up to five fingers.
     *
     * For now, 3..5 fingers are intentionally ignored. They cancel a
     * running one/two finger gesture so accidental extra contact is safe.
     */
    if (mouse_drag_active) {
        mouse_left_up();
    }

    mouse_tap_pending = 0;
    mouse_gesture_state = MOUSE_GESTURE_MULTI;
    mouse_drag_active = 0;
}



int main(void)
{
    /* PC13 LED */
    RCC->APB2ENR |= (1U << 4);

    GPIOC->CRH = (GPIOC->CRH & ~0x00F00000U) | 0x00200000U;
    GPIOC->BSRR = (1U << 13);

    /* USB clock: 72 MHz SYSCLK, 48 MHz USB */
    usb_clock_init();

    /* I2C1 */
    i2c_init(I2C1, FREQ_400KHZ);

    /* GT911 initialization */
    if (gt911_init(I2C1) != 0) {
        while (1);
    }

    /* Enable interrupts */
    __asm__ volatile ("cpsie i");

    /* USB device initialization */
    usb_init();

    struct gt911_point_t points[GT911_MAX_TOUCHES] = {0};
    uint8_t count = 0;
    // char line[64];
    unsigned len;


    while (1)
    {
        if (gt911_irq_flag)
        {
            gt911_irq_flag = 0;

            if (gt911_read_points(points, &count) == 0)
            {
                mouse_process_points(points, count);
            }
        }

        /*
         * Send queued mouse, keyboard, click, scroll, pan, and zoom reports.
         *
         * The USB ISR clears bulk_tx_busy after EP1 IN completion.
         */
        hid_queue_service();

        /* Wait for GT911 interrupt, USB interrupt, or SysTick interrupt */
        __asm__ volatile ("wfi");
    }

    return 0;
}
import serial
import threading

PORT = 'COM9'
BAUD = 115200

def parse_line(raw):
    """raw like '0,123,456;1,45,67;\n' -> list of (id,x,y)"""
    raw = raw.strip()
    points = []
    for chunk in raw.split(';'):
        chunk = chunk.strip()
        if not chunk:
            continue
        parts = chunk.split(',')
        if len(parts) == 3:
            try:
                tid, x, y = (int(p) for p in parts)
                points.append((tid, x, y))
            except ValueError:
                pass
    return points

def reader(ser):
    while True:
        raw = ser.readline()
        if not raw:
            continue
        try:
            text = raw.decode(errors='replace')
        except Exception:
            continue
        pts = parse_line(text)
        if pts:
            print("touches:", pts)

def main():
    with serial.Serial(PORT, BAUD, timeout=1) as ser:
        t = threading.Thread(target=reader, args=(ser,), daemon=True)
        t.start()
        print("Multi-touch points will print automatically.")
        print("Type 1/0 + Enter to control LED on PC13.")
        while True:
            value = input()
            if value in ("0", "1"):
                ser.write(bytes([int(value)]))

if __name__ == "__main__":
    main()

"""
/* ===================== GT911 integration ===================== */
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
    char line[64];
    unsigned len;



    while (1)
    {
        if (gt911_irq_flag)
        {
            gt911_irq_flag = 0;
            if (gt911_read_points(points, &count) == 0)
            {
                if (count > 0)
                {
                    len = format_multi_touch(line, sizeof(line), points, count);
                    if (!bulk_tx_busy && len > 0 && len <= 64)
                    {
                        cdc_send_data((const uint8_t *)line, len);
                    }
                }
            }
        }

        /* Wait for touch interrupt or USB interrupt */
        __asm__ volatile ("wfi");
    }

    return 0;
}
"""
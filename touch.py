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
extern unsigned char bulk_tx_busy;
#define GT911_REG_PID     0x8140u
#define GT911_REG_STATUS  0x814Eu
#define GT911_REG_POINT1  0x8150u
#define GT911_MAX_POINTS  5u

static int i2c_probe(volatile struct i2c_t *i2c, unsigned char addr)
{
    unsigned timeout = I2C_TIMEOUT_CYCLES;

    i2c->CR1 |= I2C_CR1_START;
    while (!(i2c->SR1 & I2C_SR1_SB)) {
        if (--timeout == 0U) return I2C_ERR_TIMEOUT;
    }

    i2c->DR = (unsigned)(addr << 1);

    timeout = I2C_TIMEOUT_CYCLES;
    while (!(i2c->SR1 & (I2C_SR1_ADDR | I2C_SR1_AF))) {
        if (--timeout == 0U) {
            i2c->CR1 |= I2C_CR1_STOP;
            return I2C_ERR_TIMEOUT;
        }
    }

    if (i2c->SR1 & I2C_SR1_AF) {
        i2c->SR1 &= ~I2C_SR1_AF;
        i2c->CR1 |= I2C_CR1_STOP;
        return I2C_ERR_NACK;
    }

    (void)i2c->SR1;
    (void)i2c->SR2;
    i2c->CR1 |= I2C_CR1_STOP;
    return I2C_OK;
}

static int gt911_read_reg(volatile struct i2c_t *i2c, unsigned char addr,
                           unsigned short reg, unsigned char *data, unsigned short len)
{
    unsigned char reg_buf[2] = { (unsigned char)(reg >> 8), (unsigned char)(reg & 0xFF) };
    int ret = i2c_write(i2c, addr, reg_buf, 2);
    if (ret != I2C_OK) return ret;
    return i2c_read(i2c, addr, data, len);
}

static int gt911_write_reg(volatile struct i2c_t *i2c, unsigned char addr,
                            unsigned short reg, const unsigned char *data, unsigned short len)
{
    unsigned char buf[2 + 4];
    if (len > 4U) return I2C_ERR_BUS;
    buf[0] = (unsigned char)(reg >> 8);
    buf[1] = (unsigned char)(reg & 0xFF);
    for (unsigned short i = 0; i < len; i++) buf[2 + i] = data[i];
    return i2c_write(i2c, addr, buf, (unsigned short)(len + 2U));
}

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
static unsigned format_multi_touch(char *line, unsigned max_len,
                                    const unsigned char *pts, unsigned n)
{
    unsigned p = 0;
    for (unsigned i = 0; i < n; i++) {
        unsigned id = pts[i * 8 + 0];
        unsigned x  = pts[i * 8 + 1] | ((unsigned)pts[i * 8 + 2] << 8);
        unsigned y  = pts[i * 8 + 3] | ((unsigned)pts[i * 8 + 4] << 8);

        /* worst case per point: "4,9999,9999;" = 12 bytes, reserve margin */
        if (p + 14U >= max_len) break;

        p += u16_to_dec(id, line + p);
        line[p++] = ',';
        p += u16_to_dec(x, line + p);
        line[p++] = ',';
        p += u16_to_dec(y, line + p);
        line[p++] = ';';
    }
    if (p < max_len) line[p++] = '\n';
    return p;
}

/* ===================== main ===================== */
int main(void)
{
    /* LED on PC13 */
    RCC->APB2ENR |= (1U << 4);  /* IOPCEN */
    GPIOC->CRH = (GPIOC->CRH & ~0x00F00000U) | 0x00200000U;
    GPIOC->BSRR = (1U << 13);   /* LED OFF (active low) */

    usb_clock_init();
    i2c_init(I2C1, 400000U);

    rcc_gpiob_enable();
    gpio_pin_mode(GPIOB, PIN_5, GPIO_INPUT_FLOATING);

    unsigned char touch_addr = 0;
    if (i2c_probe(I2C1, 0x5D) == I2C_OK)      touch_addr = 0x5D;
    else if (i2c_probe(I2C1, 0x14) == I2C_OK) touch_addr = 0x14;

    __asm__("cpsie i");
    usb_init();

    unsigned char status;
    unsigned char pts[GT911_MAX_POINTS * 8];
    char line[64];

    while (1) {
        if (touch_addr) {
            if (gt911_read_reg(I2C1, touch_addr, GT911_REG_STATUS, &status, 1) == I2C_OK) {
                if (status & 0x80U) {
                    unsigned n = status & 0x0FU;
                    if (n > GT911_MAX_POINTS) n = GT911_MAX_POINTS;

                    if (n > 0U) {
                        /* single I2C transaction pulls all active points */
                        gt911_read_reg(I2C1, touch_addr, GT911_REG_POINT1,
                                       pts, (unsigned short)(n * 8U));

                        unsigned len = format_multi_touch(line, sizeof(line), pts, n);
                        if (!bulk_tx_busy && len > 0U) {
                            cdc_send_data((unsigned char *)line, len);
                        }
                    }

                    unsigned char clear = 0;
                    gt911_write_reg(I2C1, touch_addr, GT911_REG_STATUS, &clear, 1);
                }
            }
        }
        for (volatile int d = 0x8000; d; --d) { }
    }

    return 0;
}
"""
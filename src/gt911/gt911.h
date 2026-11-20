#ifndef GT911_H
#define GT911_H

#include <i2c.h>

#define GT911_ADDR_14               0x14U
#define GT911_ADDR_5D               0x5DU

#define GT911_REG_PRODUCT_ID        0x8140U
#define GT911_REG_FIRMWARE_VERSION  0x8144U
#define GT911_REG_X_RESOLUTION      0x8146U
#define GT911_REG_Y_RESOLUTION      0x8148U
#define GT911_REG_POINT_STATUS      0x814EU
#define GT911_REG_POINT_DATA        0x814FU

#define GT911_MAX_TOUCHES           5U

struct gt911_point_t {
    unsigned char id;
    unsigned short x;
    unsigned short y;
    unsigned short size;
};

extern volatile uint8_t gt911_irq_flag;

int gt911_init(volatile struct i2c_t *i2c);
int gt911_read_points(struct gt911_point_t *points, uint8_t *count);

#endif
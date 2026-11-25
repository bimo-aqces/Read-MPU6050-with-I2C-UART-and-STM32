#ifndef _I2CDEV_H_
#define _I2CDEV_H_

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <math.h>
#include "main.h"
#include "i2c.h"

#ifndef I2CDEV_DEFAULT_READ_TIMEOUT
#define I2CDEV_DEFAULT_READ_TIMEOUT 100
#endif

#ifndef I2CDEVLIB_WIRE_BUFFER_LENGTH
#define I2CDEVLIB_WIRE_BUFFER_LENGTH 32
#endif

#ifndef PI
#define PI 3.14159265358979323846f
#endif

#ifndef TWO_PI
#define TWO_PI 6.28318530717958647693f
#endif

#ifndef HALF_PI
#define HALF_PI 1.57079632679489661923f
#endif

#ifndef DEG_TO_RAD
#define DEG_TO_RAD 0.01745329251994329577f
#endif

#ifndef RAD_TO_DEG
#define RAD_TO_DEG 57.29577951308232087680f
#endif

/* Shim minimal agar kode bergaya Arduino bisa dikompilasi di STM32 HAL */
#ifndef PROGMEM
#define PROGMEM
#endif

#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif

#ifndef pgm_read_word
#define pgm_read_word(addr) (*(const uint16_t *)(addr))
#endif

#ifndef pgm_read_dword
#define pgm_read_dword(addr) (*(const uint32_t *)(addr))
#endif

#ifndef delay
#define delay(ms) HAL_Delay((uint32_t)(ms))
#endif

#ifndef millis
#define millis() HAL_GetTick()
#endif

#ifndef micros
static inline uint32_t hal_micros_shim(void) {
    return HAL_GetTick() * 1000U;
}
#define micros() hal_micros_shim()
#endif

#ifndef map
static inline long arduino_map_shim(long x, long in_min, long in_max, long out_min, long out_max) {
    if (in_max == in_min) return out_min;
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
#define map(x, in_min, in_max, out_min, out_max) arduino_map_shim((x), (in_min), (in_max), (out_min), (out_max))
#endif

#ifndef Serial
struct HalSerialShim {
    void begin(uint32_t) {}
    void print(...) {}
    void println(...) {}
    void write(uint8_t) {}
    int available() { return 0; }
    int read() { return -1; }
    operator bool() const { return true; }
};
static HalSerialShim Serial;
#endif

class I2Cdev {
public:
    static int8_t readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);
    static int8_t readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, uint16_t timeout = I2CDEV_DEFAULT_READ_TIMEOUT, void *wireObj = nullptr);

    static bool writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data, void *wireObj = nullptr);
    static bool writeBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data, void *wireObj = nullptr);
    static bool writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data, void *wireObj = nullptr);
    static bool writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data, void *wireObj = nullptr);
    static bool writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data, void *wireObj = nullptr);
    static bool writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data, void *wireObj = nullptr);
    static bool writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, void *wireObj = nullptr);
    static bool writeWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, void *wireObj = nullptr);

    static uint16_t readTimeout;
};

#endif /* _I2CDEV_H_ */
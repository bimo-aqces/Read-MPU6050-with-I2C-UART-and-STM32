#include "I2Cdev.h"

uint16_t I2Cdev::readTimeout = I2CDEV_DEFAULT_READ_TIMEOUT;

static uint16_t halAddr8(uint8_t devAddr7) {
    return (uint16_t)(devAddr7 << 1);
}

int8_t I2Cdev::readBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    uint8_t b = 0;
    int8_t count = readByte(devAddr, regAddr, &b, timeout);
    if (count > 0) {
        *data = (uint8_t)(b & (1U << bitNum));
    }
    return count;
}

int8_t I2Cdev::readBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    uint16_t w = 0;
    int8_t count = readWord(devAddr, regAddr, &w, timeout);
    if (count > 0) {
        *data = (uint16_t)(w & (1U << bitNum));
    }
    return count;
}

int8_t I2Cdev::readBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    uint8_t b = 0;
    int8_t count = readByte(devAddr, regAddr, &b, timeout);
    if (count > 0) {
        uint8_t mask = (uint8_t)(((1U << length) - 1U) << (bitStart - length + 1U));
        b &= mask;
        b >>= (bitStart - length + 1U);
        *data = b;
    }
    return count;
}

int8_t I2Cdev::readBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    uint16_t w = 0;
    int8_t count = readWord(devAddr, regAddr, &w, timeout);
    if (count > 0) {
        uint16_t mask = (uint16_t)(((1U << length) - 1U) << (bitStart - length + 1U));
        w &= mask;
        w >>= (bitStart - length + 1U);
        *data = w;
    }
    return count;
}

int8_t I2Cdev::readByte(uint8_t devAddr, uint8_t regAddr, uint8_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    return readBytes(devAddr, regAddr, 1, data, timeout);
}

int8_t I2Cdev::readWord(uint8_t devAddr, uint8_t regAddr, uint16_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    uint8_t buf[2] = {0, 0};
    int8_t count = readBytes(devAddr, regAddr, 2, buf, timeout);
    if (count == 2) {
        *data = (uint16_t)(((uint16_t)buf[0] << 8) | buf[1]);
        return 1;
    }
    return count;
}

int8_t I2Cdev::readBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    if (length == 0 || data == nullptr) {
        return 0;
    }
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &hi2c1,
        halAddr8(devAddr),
        regAddr,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        (uint32_t)timeout
    );
    return (status == HAL_OK) ? (int8_t)length : -1;
}

int8_t I2Cdev::readWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, uint16_t timeout, void *wireObj) {
    (void)wireObj;
    if (length == 0 || data == nullptr) {
        return 0;
    }
    uint8_t buf[128];
    uint8_t totalBytes = (uint8_t)(length * 2U);
    if (totalBytes > sizeof(buf)) {
        return -1;
    }
    HAL_StatusTypeDef status = HAL_I2C_Mem_Read(
        &hi2c1,
        halAddr8(devAddr),
        regAddr,
        I2C_MEMADD_SIZE_8BIT,
        buf,
        totalBytes,
        (uint32_t)timeout
    );
    if (status != HAL_OK) {
        return -1;
    }
    for (uint8_t i = 0; i < length; i++) {
        data[i] = (uint16_t)(((uint16_t)buf[i * 2U] << 8) | buf[i * 2U + 1U]);
    }
    return (int8_t)length;
}

bool I2Cdev::writeBit(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint8_t data, void *wireObj) {
    (void)wireObj;
    uint8_t b = 0;
    if (readByte(devAddr, regAddr, &b) < 0) {
        return false;
    }
    b = (data != 0) ? (uint8_t)(b | (1U << bitNum)) : (uint8_t)(b & ~(1U << bitNum));
    return writeByte(devAddr, regAddr, b);
}

bool I2Cdev::writeBitW(uint8_t devAddr, uint8_t regAddr, uint8_t bitNum, uint16_t data, void *wireObj) {
    (void)wireObj;
    uint16_t w = 0;
    if (readWord(devAddr, regAddr, &w) < 0) {
        return false;
    }
    w = (data != 0) ? (uint16_t)(w | (1U << bitNum)) : (uint16_t)(w & ~(1U << bitNum));
    return writeWord(devAddr, regAddr, w);
}

bool I2Cdev::writeBits(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint8_t data, void *wireObj) {
    (void)wireObj;
    uint8_t b = 0;
    if (readByte(devAddr, regAddr, &b) < 0) {
        return false;
    }
    uint8_t mask = (uint8_t)(((1U << length) - 1U) << (bitStart - length + 1U));
    data <<= (bitStart - length + 1U);
    data &= mask;
    b &= (uint8_t)(~mask);
    b |= data;
    return writeByte(devAddr, regAddr, b);
}

bool I2Cdev::writeBitsW(uint8_t devAddr, uint8_t regAddr, uint8_t bitStart, uint8_t length, uint16_t data, void *wireObj) {
    (void)wireObj;
    uint16_t w = 0;
    if (readWord(devAddr, regAddr, &w) < 0) {
        return false;
    }
    uint16_t mask = (uint16_t)(((1U << length) - 1U) << (bitStart - length + 1U));
    data <<= (bitStart - length + 1U);
    data &= mask;
    w &= (uint16_t)(~mask);
    w |= data;
    return writeWord(devAddr, regAddr, w);
}

bool I2Cdev::writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data, void *wireObj) {
    (void)wireObj;
    return writeBytes(devAddr, regAddr, 1, &data);
}

bool I2Cdev::writeWord(uint8_t devAddr, uint8_t regAddr, uint16_t data, void *wireObj) {
    (void)wireObj;
    uint8_t buf[2];
    buf[0] = (uint8_t)((data >> 8) & 0xFFU);
    buf[1] = (uint8_t)(data & 0xFFU);
    return writeBytes(devAddr, regAddr, 2, buf);
}

bool I2Cdev::writeBytes(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint8_t *data, void *wireObj) {
    (void)wireObj;
    if (length == 0 || data == nullptr) {
        return false;
    }
    HAL_StatusTypeDef status = HAL_I2C_Mem_Write(
        &hi2c1,
        halAddr8(devAddr),
        regAddr,
        I2C_MEMADD_SIZE_8BIT,
        data,
        length,
        (uint32_t)readTimeout
    );
    return (status == HAL_OK);
}

bool I2Cdev::writeWords(uint8_t devAddr, uint8_t regAddr, uint8_t length, uint16_t *data, void *wireObj) {
    (void)wireObj;
    if (length == 0 || data == nullptr) {
        return false;
    }
    uint8_t buf[128];
    uint8_t totalBytes = (uint8_t)(length * 2U);
    if (totalBytes > sizeof(buf)) {
        return false;
    }
    for (uint8_t i = 0; i < length; i++) {
        buf[i * 2U] = (uint8_t)((data[i] >> 8) & 0xFFU);
        buf[i * 2U + 1U] = (uint8_t)(data[i] & 0xFFU);
    }
    return writeBytes(devAddr, regAddr, totalBytes, buf);
}
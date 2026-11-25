#include "dmp_app.h"
#include "MPU6050_6Axis_MotionApps612.h"
#include "helper_3dmath.h"
#include "main.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

extern volatile uint8_t mpu_int_flag;

static MPU6050 mpu;
static bool dmpReady = false;
static uint8_t devStatus = 0;
static uint16_t packetSize = 0;
static uint8_t fifoBuffer[64];

static Quaternion q;
static VectorFloat gravity;
static float ypr[3];

static void dmp_uart_log(const char *msg) {
    if (msg != nullptr) {
        HAL_UART_Transmit(&huart1, (uint8_t *)msg, (uint16_t)strlen(msg), 100);
    }
}

bool DMP_App_Init(void) {
    char buf[96];
    dmpReady = false;

    dmp_uart_log("Initializing MPU6050...\r\n");
    mpu.initialize();

    uint8_t devId = mpu.getDeviceID();
    int len = snprintf(buf, sizeof(buf), "MPU6050 DeviceID (6-bit) = 0x%02X\r\n", devId);
    dmp_uart_log(buf);

    /* Catatan: pada modul Anda WHO_AM_I terbaca 0x72, jadi tidak diblokir meski testConnection() berbeda */
    if (!mpu.testConnection()) {
        dmp_uart_log("Warning: testConnection() mismatch, continuing DMP init...\r\n");
    }

    dmp_uart_log("Initializing DMP...\r\n");
    devStatus = mpu.dmpInitialize();

    /* Offset awal (bisa dikalibrasi ulang sesuai posisi sensor) */
    mpu.setXGyroOffset(0);
    mpu.setYGyroOffset(0);
    mpu.setZGyroOffset(0);
    mpu.setXAccelOffset(0);
    mpu.setYAccelOffset(0);
    mpu.setZAccelOffset(0);

    if (devStatus == 0) {
        dmp_uart_log("Enabling DMP...\r\n");
        mpu.setDMPEnabled(true);

        mpu_int_flag = 0;
        packetSize = mpu.dmpGetFIFOPacketSize();
        dmpReady = true;

        len = snprintf(buf, sizeof(buf), "DMP ready! FIFO packet size = %u bytes\r\n", (unsigned)packetSize);
        dmp_uart_log(buf);
        return true;
    } else {
        len = snprintf(buf, sizeof(buf), "DMP Initialization failed (code %u)\r\n", (unsigned)devStatus);
        dmp_uart_log(buf);
        return false;
    }
}

bool DMP_App_ReadYPR(float *yaw_deg, float *pitch_deg, float *roll_deg) {
    if (!dmpReady || yaw_deg == nullptr || pitch_deg == nullptr || roll_deg == nullptr) {
        return false;
    }

    /* Reset FIFO jika overflow agar data tidak bergeser */
    uint16_t fifoCount = mpu.getFIFOCount();
    if (fifoCount >= 1024U) {
        mpu.resetFIFO();
        mpu_int_flag = 0;
        return false;
    }

    /* Tunggu sampai minimal 1 paket penuh tersedia (via flag INT EXTI0 atau FIFO count) */
    if (!mpu_int_flag && fifoCount < packetSize) {
        return false;
    }

    mpu_int_flag = 0;

    if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        mpu.dmpGetYawPitchRoll(ypr, &q, &gravity);

        *yaw_deg   = ypr[0] * (180.0f / (float)PI);
        *pitch_deg = ypr[1] * (180.0f / (float)PI);
        *roll_deg  = ypr[2] * (180.0f / (float)PI);
        return true;
    }

    return false;
}
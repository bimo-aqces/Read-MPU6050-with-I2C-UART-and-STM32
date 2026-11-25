#ifndef DMP_APP_H
#define DMP_APP_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Menginisialisasi MPU6050 dan DMP 6-axis.
 * Mengembalikan true jika DMP berhasil aktif, false jika gagal.
 */
bool DMP_App_Init(void);

/**
 * Mengecek dan membaca paket terbaru dari FIFO DMP, lalu menghitung Yaw, Pitch, Roll (dalam derajat).
 * Mengembalikan true jika ada data baru yang berhasil diproses.
 */
bool DMP_App_ReadYPR(float *yaw_deg, float *pitch_deg, float *roll_deg);

#ifdef __cplusplus
}
#endif

#endif /* DMP_APP_H */
#pragma once
#include <stdint.h>
constexpr int QMI8658_L_SLAVE_ADDRESS = 0x6b, QMI8658_H_SLAVE_ADDRESS = 0x6a;
enum class AccelFullScaleRange { FS_8G };
namespace ImuBase { enum class DataReadyMask { ACCEL }; }
struct AccelerometerData { struct { float x, y, z; } mps2; };
inline AccelerometerData testAccel{{0, 0, 9.80665f}};
inline bool testSampleReady = false;
struct SensorQMI8658 {
    enum class LpfMode { MODE_0 };
    bool begin(int, int, int, int) { return true; }
    void configAccel(AccelFullScaleRange, float, LpfMode) {}
    void enableAccel() {}
    bool isDataReady(uint8_t) { return testSampleReady; }
    void readAccel(AccelerometerData &a) { a = testAccel; testSampleReady = false; }
};

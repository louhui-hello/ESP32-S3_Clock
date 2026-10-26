/**
 * ESP32-S3 时钟板 —— RTC 探针
 *
 * 目标: 搞清楚 0x68 上那颗到底是 DS3231 还是 DS1307（还是别的芯片）
 *
 * 判断依据:
 *   DS3231  寄存器 0x11/0x12 是片内温度（有符号 10 位，0.25°C 分辨率）
 *   DS1307  没有温度寄存器，0x11 读出来通常是 0x00 或 0xFF
 *   MPU6050 寄存器 0x75 (WHO_AM_I) 返回 0x68
 *
 * 同时验证走时: 每 2 秒读一次秒寄存器，看是否在递增
 */

#include <Arduino.h>
#include <Wire.h>

#define PIN_SDA 9
#define PIN_SCL 8
#define RTC_ADDR 0x68

static uint8_t bcd2dec(uint8_t v) { return (v >> 4) * 10 + (v & 0x0F); }

static uint8_t read_reg(uint8_t reg) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return 0xFF;
    if (Wire.requestFrom(RTC_ADDR, (uint8_t)1) != 1) return 0xFF;
    return Wire.read();
}

static bool read_block(uint8_t reg, uint8_t *buf, uint8_t len) {
    Wire.beginTransmission(RTC_ADDR);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(RTC_ADDR, len) != len) return false;
    for (uint8_t i = 0; i < len; i++) buf[i] = Wire.read();
    return true;
}

static void probe() {
    Serial.println();
    Serial.println("---- RTC probe ----");

    uint8_t t[7];
    if (!read_block(0x00, t, 7)) {
        Serial.println("read 0x00..0x06 failed");
        return;
    }

    bool stop_bit = (t[0] & 0x80) != 0;
    uint8_t sec  = bcd2dec(t[0] & 0x7F);
    uint8_t min  = bcd2dec(t[1]);
    uint8_t hour = bcd2dec(t[2] & 0x3F);   // 24h 模式
    uint8_t wday = t[3] & 0x07;
    uint8_t date = bcd2dec(t[4] & 0x3F);
    uint8_t mon  = bcd2dec(t[5] & 0x1F);
    uint8_t year = bcd2dec(t[6]);

    Serial.printf("raw: %02X %02X %02X %02X %02X %02X %02X\n",
                  t[0], t[1], t[2], t[3], t[4], t[5], t[6]);
    Serial.printf("time: 20%02u-%02u-%02u %02u:%02u:%02u (wday=%u)%s\n",
                  year, mon, date, hour, min, sec, wday,
                  stop_bit ? "  [CH=1 振荡器停]" : "");

    // DS3231 温度: 0x11 整数部分(有符号), 0x12 高两位是 0.25 小数
    uint8_t temp[2];
    bool has_temp = read_block(0x11, temp, 2);
    float tval = 0;
    if (has_temp) {
        tval = (int8_t)temp[0] + ((temp[1] >> 6) * 0.25f);
    }

    // 0x75 WHO_AM_I（MPU6050 会返回 0x68）
    uint8_t who = read_reg(0x75);

    Serial.printf("temp regs: 0x11=%02X 0x12=%02X -> %.2f C\n",
                  has_temp ? temp[0] : 0, has_temp ? temp[1] : 0, tval);
    Serial.printf("whoami(0x75): 0x%02X\n", who);

    const char *chip = "unknown";
    if (who == 0x68) {
        chip = "MPU6050 (6-axis IMU)";
    } else if (has_temp && tval > -40.0f && tval < 85.0f && temp[0] != 0xFF && temp[0] != 0x00) {
        chip = "DS3231 (has temp sensor)";
    } else if (year >= 0 && year <= 99 && mon >= 1 && mon <= 12 && date >= 1 && date <= 31) {
        chip = "DS1307 (no temp sensor)";
    }
    Serial.printf("=> likely: %s\n", chip);

    // 控制/状态寄存器
    Serial.printf("status 0x0F=%02X  control 0x0E=%02X\n", read_reg(0x0F), read_reg(0x0E));
}

void setup() {
    Serial.begin(115200);
    delay(1500);
    Serial.println();
    Serial.println("ESP32-S3 RTC probe (SDA=9 SCL=8)");
    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(100000);
    delay(100);
}

void loop() {
    probe();

    // 看秒是否在走
    uint8_t s1 = read_reg(0x00);
    delay(2200);
    uint8_t s2 = read_reg(0x00);
    Serial.printf("seconds: %02X -> %02X  %s\n", s1, s2,
                  (s1 != s2) ? "RUNNING" : "STOPPED / 读数不变");
    Serial.println();
    delay(3000);
}

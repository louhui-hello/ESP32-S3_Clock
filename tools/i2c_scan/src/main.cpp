/**
 * ESP32-S3 时钟板 —— I2C 总线扫描
 *
 * 引脚与主固件一致: SDA=GPIO9, SCL=GPIO8
 *
 * 预期设备:
 *   0x50  AT24C02 EEPROM
 *   0x3C  SSD1306 OLED（部分模块是 0x3D）
 *   0x57  DS3231（如果焊了）
 *
 * 每 3 秒扫一次，能发现插拔/焊接状态变化。
 */

#include <Arduino.h>
#include <Wire.h>

#define PIN_SDA 9
#define PIN_SCL 8

struct Known {
    uint8_t addr;
    const char *name;
};

static const Known KNOWN[] = {
    {0x50, "AT24C02 EEPROM"},
    {0x51, "AT24C02 (alt addr)"},
    {0x3C, "SSD1306 OLED"},
    {0x3D, "SSD1306 OLED (alt)"},
    {0x57, "DS3231 RTC"},
    {0x68, "DS1307 RTC"},
    {0x48, "LM75 / ADS1115"},
    {0x23, "BH1750 光照"},
};

static const char *known_name(uint8_t addr) {
    for (const auto &k : KNOWN) {
        if (k.addr == addr) return k.name;
    }
    return "";
}

/** 检测总线是否被拉死（SDA/SCL 一直为低） */
static void check_bus_stuck() {
    pinMode(PIN_SDA, INPUT_PULLUP);
    pinMode(PIN_SCL, INPUT_PULLUP);
    delay(2);
    int sda = digitalRead(PIN_SDA);
    int scl = digitalRead(PIN_SCL);
    if (sda == 0 || scl == 0) {
        Serial.printf("[警告] 总线电平异常: SDA=%d SCL=%d —— 可能短路或被拉低\n", sda, scl);
        Serial.println("       检查: 传感器是否反贴、上拉是否短路、GND 是否连通");
    }
}

static void scan_once() {
    Serial.println();
    Serial.println("========================================");

    int found = 0;
    for (uint8_t addr = 0x01; addr < 0x7F; addr++) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();

        if (err == 0) {
            const char *n = known_name(addr);
            if (n[0]) {
                Serial.printf("  0x%02X  ✅  %s\n", addr, n);
            } else {
                Serial.printf("  0x%02X  ✅  (未知设备)\n", addr);
            }
            found++;
        } else if (err == 4) {
            Serial.printf("  0x%02X  ⚠️  总线错误（err=4）\n", addr);
            break;
        }
    }

    if (found == 0) {
        Serial.println("  未发现任何设备");
        Serial.println("  排查顺序: 供电 → 上拉电阻 → SDA/SCL 是否接反 → 焊点");
    } else {
        Serial.printf("共发现 %d 个设备\n", found);
    }
    Serial.println("========================================");
}

void setup() {
    Serial.begin(115200);
    delay(1500);

    Serial.println();
    Serial.println("ESP32-S3 时钟板 · I2C 扫描");
    Serial.printf("芯片: %s  核心: %d  闪存: %u MB\n",
                  ESP.getChipModel(), ESP.getChipCores(),
                  (unsigned)(ESP.getFlashChipSize() / (1024 * 1024)));
    Serial.printf("SDA=GPIO%d  SCL=GPIO%d\n", PIN_SDA, PIN_SCL);

    check_bus_stuck();

    Wire.begin(PIN_SDA, PIN_SCL);
    Wire.setClock(100000);   // 先用 100kHz，排除速率问题
}

void loop() {
    scan_once();
    delay(3000);
}

# 引脚定义表 · Pinout

> 板子：ESPRESSIF32-S3_CLOCK_BAT V3.0（ESP32-S3-WROOM-1-N16R8）
> 更新：2026-10-24
> 说明：以 **GPIO 编号** 为准，括号内为模组物理引脚号。

## I2C 总线

| 信号 | GPIO | 模组引脚 | 说明 |
|---|---|---|---|
| SCL | GPIO8 | 12 | 400 kHz，OLED + EEPROM 共用 |
| SDA | GPIO9 | 17 | |

总线上设备（实测扫描结果）：

| 地址 | 器件 | 状态 |
|---|---|---|
| 0x3C | SSD1306 OLED 0.91" 128×32 | ✅ 正常 |
| 0x50 | AT24C02 EEPROM | ✅ 正常 |
| 0x68 | DS3231 RTC | ❌ 已拆除，当前用 ESP32 内部 RTC |

## 蜂鸣器

| 信号 | GPIO | 模组引脚 | 说明 |
|---|---|---|---|
| BUZZER | GPIO13 | 21 | QMB-09B-03 **无源**蜂鸣器，需 PWM（2.7 kHz） |

> ⚠️ 注意：模组物理第 13 脚是 **GPIO19**（USB D-），不要混淆。
> 本板使用原生 USB-JTAG（GPIO19/20），这两脚不可挪作他用。

## 已被占用、不可用

| GPIO | 用途 |
|---|---|
| GPIO19 / GPIO20 | USB D- / D+（原生 USB-JTAG 控制台） |
| GPIO43 / GPIO44 | UART0 TX / RX |
| GPIO26–GPIO37 | SPI Flash / PSRAM（N16R8 内部占用） |
| GPIO0 / GPIO45 / GPIO46 | 启动 strapping，谨慎使用 |

## 当前固件

`tools/oled_rtc/`（PlatformIO + Arduino + U8g2）

- 显示：U8g2，`SSD1306_128X32_UNIVISION_F_HW_I2C`，`U8G2_R2`（180°）
- 走时：ESP32 内部 RTC，上电以编译时刻为初值
- 时区：`CST-8`
- 蜂鸣：GPIO13，`tone(13, 2730, ms)`

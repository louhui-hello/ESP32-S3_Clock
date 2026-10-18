# ESP32-S3 桌面闹钟 (esp32-clock)

> 一块自己画的 **ESP32-S3** 桌面闹钟：DS3231 走时、OLED 显示、蜂鸣器闹铃、EEPROM 存配置、Micro-USB 供电与烧录。

```
 ⏰  ESP32-S3 + DS3231 + OLED + Buzzer + EEPROM
```

## ✨ 功能

| 功能 | 实现 |
|---|---|
| 走时 | **DS3231**（±2ppm，内置 TCXO，掉电由 CR1220 保持） |
| 闹钟 | DS3231 的 Alarm → **INT 中断唤醒** ESP32-S3（深睡省电） |
| 显示 | **OLED 128×64**（I2C，SSD1306） |
| 声音 | **9mm 无源蜂鸣器**（S8050 驱动 + 1N4148 续流，PWM 变调） |
| 配置存储 | **AT24C02** EEPROM（存闹钟、亮度、音量等） |
| 操作 | **3 个按键**（菜单 / 设置 / 闹钟） |
| 供电 | Micro-USB 5V → **AMS1117-3.3** |

## 🔧 硬件参数

| 项目 | 参数 |
|---|---|
| 主控 | **ESP32-S3-WROOM-1-N16R8**（16MB Flash + 8MB PSRAM） |
| 板层 | **4 层** |
| 板框 | 约 **64.1 × 71.1 mm** |
| 供电 | Micro-USB 5V → AMS1117-3.3 |
| RTC 备份 | CR1220（不可充） |
| 差分对 | USB D+/D−，50 mil 间距，长度偏差 <0.7 mm |

## 📐 引脚分配

| 功能 | GPIO |
|---|---|
| USB D− / D+ | 19 / 20 |
| I2C SCL / SDA | 8 / 9 |
| DS3231 INT#（闹钟唤醒） | 12 |
| 蜂鸣器（PWM） | 13 |
| 按键：菜单 / 设置 / 闹钟 | 21 / 47 / 48 |
| EN（复位） | EN + 10k↑ + 100nF + 按键 |
| BOOT | GPIO0 + 10k↑ + 100nF + 按键 |
| I2C 地址 | OLED `0x3C`、EEPROM `0x50`、DS3231 `0x68` |

## 📁 仓库结构

```
.
├── hardware/            # 嘉立创EDA专业版工程（原理图 + PCB）
├── firmware/            # 固件（ESP-IDF / Arduino）
├── docs/                # 图片、说明
│   └── images/
├── tools/               # 可选：辅助脚本（差分对检查等）
├── README.md
└── LICENSE
```

## 🚀 使用

### 硬件
1. 用**嘉立创EDA专业版**打开 `hardware/*.epro2`；
2. 或直接看 `docs/` 里的原理图/PCB 截图；
3. 打样：**4 层板**，OSP 工艺即可。

### 固件
```
（待补充：ESP-IDF / Arduino 工程）
```

## 📸 预览

```
docs/images/   ← 放原理图、PCB 3D、实物照片
```

## ⚠️ 注意

- **RTC 备份电池只能用不可充的 CR1220**，DS3231 的涓流充电寄存器保持关闭；
- **I2C 上拉由 OLED 模块自带**（4.7k），如果要用不带模块的 OLED，需在板上补两颗 4.7k；
- USB 接口是 **Micro-USB**（ID 悬空）。

## 📄 License

见 [LICENSE](LICENSE)。

## 🙏 致谢

- 嘉立创EDA专业版（原理图/PCB）
- Microchip / Maxim（DS3231）、Espressif（ESP32-S3）

# ESP32-S3 桌面闹钟

> 一块自己画的 **ESP32-S3** 桌面闹钟：DS3231 走时、OLED 显示、蜂鸣器闹铃、EEPROM 存配置、Micro-USB 供电与烧录。

**作者：louhui** ｜ GitHub: [@louhui-hello](https://github.com/louhui-hello)

## 功能

| 功能 | 实现 |
|---|---|
| 走时 | **DS3231**（内置 TCXO，掉电由 CR1220 保持走时） |
| 闹钟 | DS3231 Alarm → **INT 中断唤醒** ESP32-S3（深睡省电） |
| 显示 | **OLED 128×64**（I2C，SSD1306） |
| 声音 | **9mm 无源蜂鸣器**（S8050 驱动 + 1N4148 续流，PWM 变调） |
| 配置存储 | **AT24C02** EEPROM（闹钟 / 亮度 / 音量等） |
| 操作 | **3 个按键**（菜单 / 设置 / 闹钟） |
| 供电 | Micro-USB 5V → **AMS1117-3.3** |

## 硬件参数

| 项目 | 参数 |
|---|---|
| 主控 | **ESP32-S3-WROOM-1-N16R8**（16MB Flash + 8MB PSRAM） |
| 板层 | **4 层** |
| 板框 | 约 **64.1 × 71.1 mm** |
| 供电 | Micro-USB 5V → AMS1117-3.3 |
| RTC 备份 | **CR1220**（不可充） |
| USB | Micro-USB，D+/D− 差分对 |
| EDA | 嘉立创EDA 专业版 |

## 引脚分配

| 功能 | GPIO |
|---|---|
| USB D− / D+ | 19 / 20 |
| I2C SCL / SDA | 8 / 9 |
| DS3231 INT#（闹钟唤醒） | 12 |
| 蜂鸣器（PWM） | 13 |
| 按键：菜单 / 设置 / 闹钟 | 21 / 47 / 48 |
| EN（复位） | EN + 10k 上拉 + 100nF + 按键 |
| BOOT | GPIO0 + 10k 上拉 + 100nF + 按键 |

**I2C 地址**：OLED `0x3C`、EEPROM `0x50`、DS3231 `0x68`

## 仓库结构

```
.
├── hardware/     # 嘉立创EDA 专业版工程（原理图 + PCB）
├── firmware/     # 固件（ESP-IDF / Arduino）
├── docs/         # 文档与图片
│   └── images/
├── LICENSE       # GPL-3.0
└── README.md
```

## 打样说明

- **4 层板**，OSP 工艺即可；
- 板厚 1.6mm；
- 天线区域（S3 模组左侧）**所有层禁止铺铜**。

## 注意

- **RTC 备份电池只能用不可充的 CR1220**，DS3231 的涓流充电寄存器保持关闭；
- **I2C 上拉由 OLED 模块自带**（4.7k）；若使用不带模块的 OLED，需在板上补两颗 4.7k 到 3.3V；
- 电池座极性以实物实测为准（丝印已标 "+"）。

## 许可证

本项目采用 **GNU General Public License v3.0**，详见 [LICENSE](LICENSE)。

> 简单说：**你可以自由使用、修改、分发，但修改后的作品必须同样以 GPL-3.0 开源**，且需保留原作者署名。商业使用也需遵守同样条款（即不能闭源）。

## 致谢

- [嘉立创EDA 专业版](https://pro.lceda.cn/)（原理图 / PCB）
- Espressif（ESP32-S3）、Maxim / ADI（DS3231）

# ESP32-S3 桌面闹钟

> 一块自己画的 **ESP32-S3** 桌面闹钟：DS3231 走时、OLED 显示、蜂鸣器闹铃、EEPROM 存配置、**锂电池 + USB 双供电自动切换**、Micro-USB 烧录。

**作者：louhui** ｜ GitHub: [@louhui-hello](https://github.com/louhui-hello)

仓库：[louhui-hello/ESP32-S3_Clock](https://github.com/louhui-hello/ESP32-S3_Clock)

## 功能

| 功能 | 实现 |
|---|---|
| 走时 | **DS3231**（内置 TCXO，掉电由 CR1220 保持走时） |
| 闹钟 | DS3231 Alarm → **INT 中断唤醒** ESP32-S3（深睡省电） |
| 显示 | **0.91 寸 OLED 128×32**（I2C，SSD1306）—— 板上有"带鱼屏"之称 |
| 声音 | **9mm 无源蜂鸣器**（S8050 驱动 + 1N4148 续流，PWM 变调） |
| 配置存储 | **AT24C02** EEPROM（闹钟 / 亮度 / 音量等） |
| 操作 | **3 个按键**（菜单 / 设置 / 闹钟） |
| 供电 | **锂电池 + Micro-USB 5V 自动切换**（TP4056 充电 + TLV62568 降压） |
| 充电指示 | 双色 LED：红 = 充电中，绿 = 满电 |

## 硬件参数

| 项目 | 参数 |
|---|---|
| 硬件版本 | **V3.0**（板上丝印） |
| 主控 | **ESP32-S3-WROOM-1-N16R8**（16MB Flash + 8MB PSRAM） |
| 板层 | **4 层** |
| 板框 | **81.91 × 72.26 mm**（四角圆角 R3） |
| 供电 | 锂电池 3.0~4.2V 或 USB 5V → **TLV62568 buck** → 3.3V |
| 充电 | **TP4056**（1A 线性，PROG 电阻设定电流） |
| 电源路径 | **SS34 + AO3401 P-MOS** 负载分配（充电时电池隔离） |
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
├── LICENSE       # CERN-OHL-S-2.0
└── README.md
```

## 打样说明

- **4 层板**，OSP 工艺即可；
- 板厚 1.6mm；
- 天线区域（S3 模组**上方**）**所有层禁止铺铜**（用「多层禁止区域」实现）；
- 叠层顺序：**信号 / GND / 3.3V / 信号**（4 层全用上）；
- 天线净空区在 L1~L4 **全部无铜**。

## 注意

- **RTC 备份电池只能用不可充的 CR1220**，DS3231 的涓流充电寄存器保持关闭；
- **主电池用带保护板的软包锂电**（MX1.25 插头），首次试机建议先只插 USB；
- **I2C 上拉已在板上**（R7/R8 = 4.7kΩ 到 3.3V）；用自带上拉的 OLED 模块也没问题（并联后约 2.35k，仍在 I2C 规范内）；
- 电池座极性以实物实测为准（丝印已标 "+"）。

## 许可证

本项目采用 **CERN Open Hardware Licence Version 2 - Strongly Reciprocal**。

SPDX-License-Identifier: `CERN-OHL-S-2.0`

> 官方全文：https://ohwr.org/cern_ohl_s_v2.txt

Copyright (C) 2026 **louhui** (https://github.com/louhui-hello)

全文见 [LICENSE](LICENSE)。这是**硬件圈推荐的强互惠（strongly reciprocal）协议**：

- ✅ 你可以自由**使用、制造、修改、分发**本设计（包括商业用途）；
- ✅ 但**分发修改版硬件时，必须提供完整的设计源文件**（原理图、PCB 等 "Complete Source"）；
- ✅ 且**衍生作品必须使用相同的 CERN-OHL-S 协议**；
- ✅ 必须**保留原作者署名与版权声明**。

> 也就是说：**改了我的板子再发出去，你必须同样开源你的 PCB 源文件**——不能闭源。

文档与图片（`docs/`）如需注明，可另按 **CC-BY-SA-4.0** 处理。

### 用于你自己的设计文件

若你基于本设计做了修改，请在你的设计文件（原理图/PCB 页）中保留如下声明：

```
Copyright (C) 2026 louhui (https://github.com/louhui-hello)
This source describes Open Hardware and is licensed under the CERN-OHL-S v2.
You may redistribute and modify this source and make products using it under
the terms of the CERN-OHL-S v2 (https://ohwr.org/cern_ohl_s_v2.txt).
```

## 致谢

- [嘉立创EDA 专业版](https://pro.lceda.cn/)（原理图 / PCB）
- Espressif（ESP32-S3）、Maxim / ADI（DS3231）

# ESP32-S3 开源电池版时钟

一张 81.9 × 72.3 mm 的四层板桌面时钟：DS3231 高精度走时 + 0.91" OLED + 蜂鸣器闹钟 + 21700 电池。

An 81.9 × 72.3 mm four-layer desktop clock: high-accuracy DS3231 RTC, 0.91" OLED, buzzer alarm, and a 21700 battery.

---

## 特性 · Features

| 项目 / Item | 说明 / Details |
|---|---|
| 主控 / MCU | ESP32-S3-WROOM-1-N16R8（16MB Flash · 8MB PSRAM） |
| 实时时钟 / RTC | DS3231MZ（±2 ppm，CR1220 备电） |
| 显示 / Display | 0.91" OLED 128×32（I2C） |
| 提示 / Alert | 无源蜂鸣器（PWM 驱动） |
| 存储 / Storage | AT24C02 EEPROM（I2C） |
| 充电 / Charging | TP4056（带散热焊盘 + 6 个散热过孔） |
| 降压 / Buck | TLV62568（效率 90%+，静态电流 ~15µA） |
| 电池 / Battery | 21700 锂电池 + 外置保护板 |
| 按键 / Buttons | 4 键 + RST + BOOT |
| 调试 / Debug | UART（TX/RX/GND） |

---

## 硬件 · Hardware

| 项目 / Item | 参数 / Value |
|---|---|
| 尺寸 / Size | 81.915 × 72.263 mm |
| 层数 / Layers | 4（Signal / **GND** / **3V3** / Signal） |
| 板厚 / Thickness | 1.6 mm |
| 表面处理 / Finish | OSP（无铅 / lead-free） |
| 阻焊颜色 / Solder mask | 蓝色 / Blue |

**层叠说明 / Stackup**

- **L1** Top — 信号 + 元件，局部补地
- **L2** Inner1 — **GND 完整地平面**
- **L3** Inner2 — **3.3V 电源平面**
- **L4** Bottom — 信号

---

## 目录结构 · Repository Layout

```
├── README.md              本文件 / this file
├── LICENSE                CERN-OHL-S-2.0
├── docs/
│   ├── versions.md        版本演进 / version history
│   ├── soldering.md       焊接指南 / soldering guide
│   └── images/            图片 / images
├── hardware/
│   ├── PCB/               立创EDA 工程 / EasyEDA Pro project
│   ├── Gerber/            打样文件 / fabrication files
│   ├── BOM/               物料清单 / bill of materials
│   ├── Schematic/         原理图 / schematic
│   └── STEP/              3D 模型 / 3D model
├── firmware/              ESP-IDF 固件 / firmware
├── legacy/                旧版本存档 / archived revisions
└── tools/                 辅助脚本 / helper scripts
```

---

## 快速开始 · Quick Start

1. **打样 / Fabricate** — 见 [hardware/README.md](hardware/README.md)
2. **买料 / Source parts** — `hardware/BOM/`
3. **焊接 / Solder** — 见 [docs/soldering.md](docs/soldering.md)
4. **烧录 / Flash** — 见 `firmware/`

---

## 许可证 · License

本项目采用 **CERN-OHL-S-2.0**（强互惠开源硬件协议）。
This project is licensed under **CERN-OHL-S-2.0** (Strongly Reciprocal).

---

## 作者 · Author

**@学 ARM Keil 的初中生**

> 开源是种浪漫。
> Open Source is Love.

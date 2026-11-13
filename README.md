# ESP32-S3 Clock

基于 ESP32-S3 的桌面时钟开源项目，整理硬件工程、原理图、PCB 制造文件、物料清单及配套烧录工具。

作者：**学 ARM Keil 的初中生 / louhui-hello**。

## 项目状态

当前以 **Mini V4.0 硬件资料**和工具归档为主，完整时钟固件仍在开发。固件源码后续单独整理，当前不提供已完成板级适配的固件。

| 版本 | 仓库内容 | 使用说明 |
| --- | --- | --- |
| Mini V4.0 | 原理图、Gerber、BOM、引脚说明 | 当前整理版本；尚未完成整机实板验证 |
| V3.0 | 原理图、Gerber、BOM | 历史版本，Gerber 已标注“不推荐打样” |

## 硬件资料

| 资料 | Mini V4.0 | V3.0 |
| --- | --- | --- |
| 原理图 PDF | [查看原理图](hardware/Schematic/Mini_v4.0/SCH_Schematic2_2026-11-11.pdf) | [查看原理图](hardware/Schematic/v3.0/SCH_Schematic1_2026-11-11.pdf) |
| PCB 制造文件 | [下载 Gerber](hardware/Gerber/Mini_v4.0/esp32s3clock_Mini_v4.0.zip) | [历史 Gerber，不推荐打样](hardware/Gerber/v3/v3.0不推荐打样.zip) |
| 物料清单 | [V4.0 BOM](hardware/BOM/V4.0/v4.0_bom_list.xlsx) | [V3.0 BOM](hardware/BOM/V3.0/v3.0_bom_list.xlsx) |

嘉立创 EDA 专业版原始工程：

- [`.epro` 工程](hardware/EasyEDA_Project_File/epro/esp32s3clock.epro)
- [`.epro2` 工程](hardware/EasyEDA_Project_File/epro2/esp32s3_clock.epro2)

导入工程后，先核对板名和版本，再对照原理图、BOM 与 Gerber 使用。工程格式与板卡版本不应仅凭扩展名判断。

## Mini V4.0 引脚分配

下表整理自[硬件引脚说明](hardware/引脚分配/README.md)，对应 `esp32s3clock_Mini_v4.0`。

| GPIO / 引脚 | 信号 | 用途 |
| --- | --- | --- |
| GPIO4 | TFT_BL | TFT 背光 |
| GPIO5 | TFT_CS | TFT 片选 |
| GPIO6 | TFT_DC | TFT 数据 / 命令选择 |
| GPIO7 | TFT_RES | TFT 复位 |
| GPIO15 | TFT_MOSI | TFT SPI 数据 |
| GPIO16 | TFT_CLK | TFT SPI 时钟 |
| GPIO8 | SCL | I²C 时钟 |
| GPIO9 | SDA | I²C 数据 |
| GPIO19 | USB D− | USB 数据负端 |
| GPIO20 | USB D+ | USB 数据正端 |
| GPIO12 | DS3231 WAKE_UP | RTC 唤醒信号 |
| GPIO13 | BUZZER_PWM | 蜂鸣器控制 |
| GPIO21 | KEY_LEFT | 左键 |
| GPIO47 | KEY_RIGHT | 右键 |
| GPIO48 | KEY_ENTER | 确认键 |
| GPIO0 | BOOT | 下载启动按键 |
| EN | ENABLE | 芯片使能 / 复位 |

根据板级引脚说明，I²C 已配置 **4.7 kΩ 上拉**，BOOT 和 EN 均通过 **10 kΩ 电阻上拉至 3.3 V**。

不同板卡和测试示例的 GPIO 配置可能不同。接线与固件配置应按对应版本的原理图核对，Mini V4.0 的 SDA 为 GPIO9。

## 配套烧录工具

[tools/esp-flasher/](tools/esp-flasher/) 提供 Windows x64 烧录器：

- [esp32flasher.exe](tools/esp-flasher/esp32flasher.exe)：图形界面。
- [esptool.exe](tools/esp-flasher/esptool.exe)：独立烧录后端。
- [使用说明](tools/esp-flasher/使用说明.txt)：运行与打包说明。

请将整个工具目录一起保留。Qt 已静态链接，esptool 已打包 Python，运行时无需另行安装 Qt、Python 或激活 venv。选择正确的串口、芯片、固件和烧录偏移后使用。

本目录保存的是可运行工具包；其源码、构建脚本与发布版本位于独立的 [esp32-flasher 仓库](https://github.com/louhui-hello/esp32-flasher)。本仓库暂未提供已完成板级验证的整机固件。

## 固件状态

[firmware/README.md](firmware/README.md) 保留固件开发说明。固件应用计划采用 GPL-3.0-only，自研驱动采用 MIT；源码与板级适配将在后续单独整理。

## 目录结构

```text
esp_clock/
├── README.md
├── LICENSE                   # 许可范围索引
├── LICENSES/                 # GPL-3.0、MIT、CERN-OHL-S-2.0 全文
├── hardware/
│   ├── EasyEDA_Project_File/   # .epro / .epro2 原始工程
│   ├── Schematic/             # V3.0 / Mini V4.0 原理图
│   ├── Gerber/                # PCB 制造文件
│   ├── BOM/                   # V3.0 / V4.0 物料清单
│   └── 引脚分配/               # Mini V4.0 引脚说明
├── tools/
│   └── esp-flasher/           # 烧录器、后端及第三方许可证
└── firmware/
    ├── README.md              # 开发状态说明
    └── LICENSE                # 固件应用许可，驱动另按 MIT
```

## 许可证

本仓库按内容分别授权，根目录 [LICENSE](LICENSE) 是许可范围索引，完整条款收录在 [LICENSES/](LICENSES/)。

| 内容 | 许可证 | 完整条款与范围 |
| --- | --- | --- |
| 固件应用、整机逻辑及示例应用 | **GPL-3.0-only** | [firmware/LICENSE](firmware/LICENSE)；自研驱动除外 |
| 自研驱动；包括后续单独整理的 `ds3231.c`、`ds3231.h` | **MIT** | [MIT 许可证](LICENSES/MIT.txt) |
| 原创硬件设计、原理图、PCB 工程、Gerber、BOM 与硬件说明 | **CERN-OHL-S-2.0** | [hardware/LICENSE](hardware/LICENSE) |
| ESP32 Flasher 图形工具 | **GPL-3.0-only** | [工具许可证](tools/esp-flasher/LICENSE) |
| esptool 后端 | **GPL-2.0-or-later** | [后端许可证](tools/esp-flasher/licenses/esptool-LICENSE.txt) |
| Qt、Python、PyInstaller 及其他依赖 | 各自原有许可证 | [第三方声明](tools/esp-flasher/THIRD_PARTY_NOTICES.md)与[随包许可证](tools/esp-flasher/licenses/) |

自研驱动作为独立文件保持 MIT 授权；包含这些驱动的固件应用按 GPL-3.0-only 发布。第三方代码、库、器件资料和已有版权声明保留各自许可。

工具包第三方声明中出现的 `packaging/` 路径指向烧录器源码仓库；本目录随包许可证存放在 `tools/esp-flasher/licenses/`。

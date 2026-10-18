# 硬件 · Hardware

ESP32-S3 开源电池版时钟的硬件资料。
Hardware resources for the ESP32-S3 open-source battery clock.

---

## 板子参数 · Board Specs

| 项目 / Item | 参数 / Value |
|---|---|
| 尺寸 / Size | 81.915 × 72.263 mm |
| 层数 / Layers | 4（Signal / GND / 3V3 / Signal） |
| 板厚 / Thickness | 1.6 mm |
| 表面处理 / Finish | OSP（无铅 / lead-free） |
| 阻焊 / Solder mask | 蓝色 / Blue |
| 主控 / MCU | ESP32-S3-WROOM-1-N16R8 |
| RTC | DS3231MZ（±2 ppm） |
| 显示 / Display | 0.91" OLED 128×32 |
| 电源 / Power | TP4056 + TLV62568 |
| 电池 / Battery | 21700 + 外置保护板 |

---

## 目录 · Contents

```
hardware/
├── Project_Files/            源工程 / source projects
│   ├── Project_EasyEDA/
│   │   ├── v2/  *.epro       立创EDA 标准版格式
│   │   └── v3/  *.epro2      立创EDA Pro 格式  ← 推荐
│   └── EasyEDA_URL/          一键打开链接
├── PCB/
│   ├── Gerber/               打样文件（zip）
│   ├── BOM/                  物料清单（xlsx）
│   └── Schematic/            原理图（PDF zip）
├── STEP/                     3D 模型（待补）
└── README.md                 本文件
```

---

## 两个源工程的区别 · Why Two Project Files

```
v2/ProPrj_*.epro    408 KB   立创EDA 标准版
v3/ProPrj_*.epro2   498 KB   立创EDA Pro
```

**内容完全一样** —— **只是文件格式不同**。

| | `.epro` | `.epro2` |
|---|---|---|
| 客户端 | 立创EDA 标准版（网页内核） | **立创EDA Pro**（桌面） |
| 能否互相打开 | ❌ | ⚠️ Pro 可以导入 .epro 转换 |
| 是否推荐 | 留档用 | **✅ 推荐** |

> 建议使用 `v3/*.epro2`。标准版正在逐步退役。
> Use the `.epro2` file — the Standard edition is being phased out.

---

## 打开方式 · How to Open

| 格式 / Format | 工具 / Tool | 说明 / Note |
|---|---|---|
| `.epro2` | **立创EDA Pro**（免费） | 源工程，可编辑 |
| `.epro` | 立创EDA 标准版，或 Pro 导入 | 旧格式，留档 |
| `Gerber/*.zip` | 任意 EDA / 在线查看器 | 打样用 |
| `Schematic/*.zip` | 解压后用 PDF 阅读器 | 只看 |
| `STEP/*.step` | Fusion 360 / FreeCAD / SolidWorks | 外壳设计 |

> 立创EDA Pro 下载：<https://lceda.cn/editor>
> 免费、跨平台（Windows / macOS / Linux），打开工程不需要注册。

---

## PCB 打样建议从嘉立创下单

* 每月免费打样（OSP，1~6 层板）
* 速度快、质量高
* 作者本人就是从嘉立创免费打样的，肥美

## PCB Fabrication — We Recommend Ordering from JLCPCB

* **Free prototype fabrication every month** (OSP finish, 1–6 layer boards)
* Fast turnaround and high quality
* The author's own boards were fabricated for free at JLCPCB — *chef's kiss*

**URL:** <https://www.jlc.com/newOrder/#/pcb/newOnlinePlaceOrder>

---

## 建议使用 LCSC 采购 BOM

* BOM 里的元件几乎都能在 LCSC 买到
* 每个元件都标了 **LCSC 编号（Cxxxxx）**，直接搜编号即可
* 与嘉立创 SMT 贴片直接打通
* 常见阻容比其它分销商便宜

## BOM — We Recommend Sourcing from LCSC

* Nearly every part in the BOM is in stock at LCSC
* Each part carries an **LCSC part number (Cxxxxx)** — just paste it into the search box
* Orders sync directly with JLCPCB for SMT assembly
* Cheaper than most other distributors for common components

**URL:** <https://www.lcsc.com>

---

## 层叠说明 · Stackup

```
L1  Top Layer      信号 + 元件，局部补地
L2  Inner1         GND 完整地平面
L3  Inner2         3.3V 电源平面
L4  Bottom Layer   信号
```

> 铺铜实测（从 EDA 工程读取）：
> * `GND`  → Inner1（整片）
> * `3.3V` → Inner2（整片）
> * `GND`  → TopLayer（局部）

---

## 已知问题 · Known Issues (V3.0)

| 问题 / Issue | 说明 / Note | 计划 / Plan |
|---|---|---|
| 无安装孔 | 外壳靠卡槽或双面胶 | V4.0 加 M2 孔 |
| 调试口丝印写成 JTAG | 实际是 UART（TX/RX/GND） | V4.0 改成 DEBUG |
| `BAT+` 与 RTC 电池同名易混 | `BAT+` 是 RTC 备电；主板电池接 TP4056 旁的 `+ / −` | V4.0 标成 `RTC+` |

---

## 许可证 · License

CERN-OHL-S-2.0

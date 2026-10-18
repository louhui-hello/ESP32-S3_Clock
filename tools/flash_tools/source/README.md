# ESP32 烧录器

基于 **Qt 6 + QML** 的 ESP32 烧录工具，底层调用 `esptool`。
A Qt 6 / QML GUI for flashing ESP32 series chips, powered by `esptool`.

![screenshot](docs/screenshot.png)

---

## 特性 · Features

- 🔌 **串口扫描** — 自动列出本机 COM 口
- ⬇️ **一键烧录** — 调用 esptool，实时解析进度
- 🗑️ **擦除 Flash** — `erase-flash`
- 📟 **读芯片信息** — `chip-id`
- 🖥️ **串口监视器** — 收发数据，支持 Hex 显示
- 🖱️ **拖拽固件** — 把 .bin 拖进窗口即可
- 🎨 **明色主题** — 简洁、不刺眼

---

## 支持的芯片 · Supported Chips

`esp32s3` · `esp32s2` · `esp32c3` · `esp32` · `esp8266`

---

## 快速开始 · Quick Start

### 1. 准备 esptool

```bash
pip install esptool
```

然后把 `esptool.exe` 放到程序同目录，或在界面上 esptool 一栏填写绝对路径。

> ⚠️ **esptool v5.x 的命令是横杠**：`write-flash` / `erase-flash` / `chip-id`
> 本项目按 v5.x 编写，v4.x 不兼容。

### 2. 选择固件

固件 → 浏览... 或直接把 `.bin` 拖进窗口。

### 3. 开始烧录

选择 COM 口 → 波特率 460800 → 点【开始烧录】。

**烧录时串口监视器会自动关闭**（esptool 需要独占串口）。

---

## 从源码构建 · Build from Source

### 依赖

| 组件 | 版本 |
|---|---|
| Qt | 6.5+（含 Quick / QuickControls2 / SerialPort） |
| CMake | 3.21+ |
| 编译器 | MinGW 13 / MSVC 2022 |

### 构建

```bash
cmake -B build -G Ninja \
  -DCMAKE_PREFIX_PATH=/path/to/Qt/6.12.0/mingw_64
cmake --build build
```

### 打包

```bash
cd build
windeployqt --release --qmldir ../qml esp32flasher.exe
```

---

## 项目结构 · Layout

```
esp32-flasher/
├── CMakeLists.txt
├── main.cpp
├── qml/
│   └── Main.qml             界面
├── src/
│   ├── esptoolrunner.h/.cpp  esptool 进程封装
│   ├── serialmonitor.h/.cpp  串口收发
│   └── portlister.h/.cpp     串口枚举
└── docs/
    └── screenshot.png
```

---

## 已知限制 · Known Limits

- 只按 **esptool v5.x** 的命令格式写，v4.x 不兼容
- 烧录时串口监视器会被强制关闭
- 不做固件校验（不 verify flash）

---


---

## 交付物 · Deliverables

| 路径 | 体积 | 说明 |
|---|---|---|
| `dist/ESP32Flasher_OneFile.exe` | 31.5 MB | 单文件，双击即用；首次启动解压约 5 秒 |
| `release/` | 135 MB | 文件夹版，启动瞬间；日常使用推荐 |
| `sourcecode/` | 极小 | 完整源码，可自行修改重编 |

### 单文件版的注意点

`dist/ESP32Flasher_OneFile.exe` 是 **7-Zip SFX**（`7zSD.sfx` + `config.txt` + 压缩包）拼出来的。

- 双击后解压到 `%TEMP%\7zS<随机数>\`，然后启动 `esp32flasher.exe`
- ⚠️ **7zSD 不会自动清理这个临时目录**，每次启动会留约 135 MB
- 定期清理：`Get-ChildItem $env:TEMP -Directory -Filter "7zS*" | Remove-Item -Recurse -Force`

自己长期用请用 `release/` 目录；把单文件版发给别人。

---

## 许可证 · License

MIT

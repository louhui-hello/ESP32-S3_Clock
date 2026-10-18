# 烧录工具 · Flashing Tools

ESP32-S3 时钟的烧录与调试工具。
Flashing and debugging helpers for the ESP32-S3 clock.

---

## 目录 · Contents

```
flash_tools/
├── ESP32Flasher_OneFile.exe   单文件烧录器（31.5 MB，双击即用）
├── merge_bin.ps1              把 IDF 的 build 产物合并成一个 bin
├── i2c_scan_merged.bin        示例固件（I2C 扫描，250 KB，烧到 0x0）
├── source/                    烧录器源码（Qt 6 + QML）
└── README.md                  本文件
```

---

## 图形化烧录器 · GUI Flasher

**双击 `ESP32Flasher_OneFile.exe` 即可启动**，不需要装 Qt、不需要装 Python。

首次启动会解压约 5 秒（到 `%TEMP%\7zS*`），之后正常使用。

### 功能

- 串口扫描，自动列出本机 COM 口
- 一键烧录，实时显示进度百分比
- 擦除 Flash、读芯片信息
- 串口监视器（收发、Hex 显示）
- 拖拽 `.bin` 到窗口即可选择固件

### 用法

```
① 选 COM 口（插上板子后点【刷新】）
② 波特率 460800
③ 芯片 esp32s3
④ 固件：浏览... 或把 .bin 拖进窗口
⑤ 偏移 0x0（合并后的固件）或 0x10000（仅 app）
⑥ 点【开始烧录】
```

> ⚠️ **烧录时串口监视器会自动关闭** —— esptool 要独占 COM 口。
> ⚠️ 板子上没有自动下载电路时，先按住 BOOT 再点一下 RST。

---

## 命令行 · Command Line

不想用 GUI 的话，直接调 esptool：

```powershell
# 查看芯片信息
esptool --chip esp32s3 --port COM4 chip-id

# 擦除整片
esptool --chip esp32s3 --port COM4 erase-flash

# 烧录合并固件
esptool --chip esp32s3 --port COM4 --baud 460800 write-flash 0x0 merged-binary.bin

# ESP-IDF 原生
idf.py -p COM4 flash monitor
```

> ⚠️ **esptool v5.x 的命令用横杠**：`write-flash` / `erase-flash` / `chip-id`
> v4.x 用的是下划线 `write_flash`，两套不通用。

---

## 固件从哪来 · Where the Firmware Comes From

```powershell
cd firmware\ESP-IDF
idf.py build

# 三份分开烧：
#   0x0      build\bootloader\bootloader.bin
#   0x8000   build\partition_table\partition-table.bin
#   0x10000  build\<project>.bin

# 或者合成一份，从 0x0 一次烧：
idf.py merge-bin
# -> build\merged-binary.bin
```

**烧录器选偏移 `0x0` 时，要给 `merged-binary.bin`**；
**选 `0x10000` 时，给单独的 app bin。**

### 一键合并脚本

`merge_bin.ps1` 不用进 IDF 环境，直接调 esptool 按 `build/flash_args` 合并：

```powershell
cd tools\flash_tools
.\merge_bin.ps1 -ProjectDir ..\i2c_scan\i2c_scan
# -> i2c_scan_merged.bin，烧录时偏移填 0x0
```

**原理**：ESP-IDF 在 `build/flash_args` 里写好了每个分段的偏移，脚本读它，
再交给 `esptool merge-bin` 拼起来。偏移永远跟着工程走，不用手抄。

---

## 重新编译烧录器 · Rebuild the Flasher

`source/` 里是完整源码，依赖：

| 组件 | 版本 |
|---|---|
| Qt | 6.5+（Quick / QuickControls2 / SerialPort） |
| CMake | 3.21+ |
| 编译器 | MinGW 13 / MSVC 2022 |

```powershell
cd source
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH=G:/lang/Qt/6.12.0/mingw_64
cmake --build build
cd build
windeployqt --release --qmldir ../qml esp32flasher.exe
```

---

## 注意 · Notes

- ⚠️ **单文件版每次启动会在 `%TEMP%` 留约 135 MB**（7zSD 不自动清理）
  定期清一下：`Get-ChildItem $env:TEMP -Directory -Filter "7zS*" | Remove-Item -Recurse -Force`
- ⚠️ 烧录波特率从 115200 起试，稳了再加到 460800
- ⚠️ 没有自动下载电路时，GPIO0 拉低 + 复位才能进下载模式

# 旧版本存档 · Legacy Archive

这里存放**历史版本**的资料。保留是为了可追溯，**不再维护**。
Archived material from earlier revisions — kept for traceability, **no longer maintained**.

---

## 内容 · Contents

| 目录 / Dir | 内容 / Content |
|---|---|
| `old_repo/` | **旧仓库 `open_source` 的完整快照**（V3.0 重构前的结构） |
| `v1.5_esp8266/` | V1.5 板子（ESP8266）的 Gerber、仿真图、照片（待补） |
| `boms/` | 历次 BOM 快照（待补） |
| `project_files/` | 旧格式工程文件（待补） |

---

## `old_repo/` 是什么 · What Is `old_repo`

```
旧仓库 open_source 的结构：
├── README.md                        4.4 KB
├── LICENSE                          13.2 KB
├── docs/
│   ├── NOTES.md                     2.5 KB
│   └── images/
├── firmware/README.md               0.5 KB
└── hardware/
    ├── BOM_立创商城_ESP32-S3_Clock.xlsx   9.8 KB
    ├── BOM.csv                      2.9 KB
    ├── BOM.md                       7.3 KB
    ├── esp32s3_clock.epro2          371.4 KB
    └── README.md                    0.5 KB
```

**它和新结构的关系 · Relation to the new layout**

| 旧仓库 | 新仓库 |
|---|---|
| `hardware/BOM.csv` / `BOM.md` | `hardware/PCB/BOM/` |
| `hardware/esp32s3_clock.epro2` | `hardware/Project_Files/Project_EasyEDA/v3/` |
| `docs/NOTES.md` | 内容已并入 `docs/versions.md` |
| 根目录的 BOM 文件 | 已按目录归类 |

> `old_repo/.gitignore` 已改名为 `.gitignore.old`，避免影响上级仓库。
> The archived `.gitignore` was renamed so it doesn't apply to this repository.

---

## 为什么保留 · Why Keep It

- 对比演进（参考 [../docs/versions.md](../docs/versions.md)）
- 有人想复刻旧版
- 记录踩过的坑

---

## 原则 · Principle

> 存档**冻结**，新东西进 `hardware/`。
> Archived content is **frozen** — new work goes into `hardware/`.

---

## 版本索引 · Version Index

| 版本 / Ver | 主控 / MCU | 状态 / Status |
|---|---|---|
| V1 | STC15 | ❌ 文件已丢失 |
| V1.5 | ESP8266（ESP-12F） | 🔒 待补入 `v1.5_esp8266/` |
| V2 | ? | ❓ 待补 |
| V3.0 | ESP32-S3 | ✅ 当前版本，见 `hardware/` |

---

## TODO

- [ ] 把 V1.5 的 Gerber / 仿真图放进 `v1.5_esp8266/`
- [ ] 把历次 BOM 快照放进 `boms/`
- [ ] 如果有 V1 的照片，也放进来

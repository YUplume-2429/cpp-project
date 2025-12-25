# 数独游戏（Qt 5.14.2）

## 功能概述
- 主菜单包含：继续游戏、新的开始、清空记录、退出游戏。
- 继续游戏：弹出最近10次游玩记录窗口；若不足则显示“没有游玩记录”。点击记录可继续游戏（若记录缺失提示）。
- 新的开始：弹出难度选择（简单/普通/困难）。根据难度生成随机9x9完整数独，并在 `records/时间戳_难度/` 保存：
  - `grid_ply.txt`：完整版（解）
  - `grid_init.txt`：初始题面（含空格为0）
  - `grid_current.txt`：当前进度（对局中实时更新）
  - `steps.log`：每次操作的时间日志
- 清空记录：删除 `records/` 下所有目录与文件。
- 退出游戏：关闭程序。

## 关键文件
- [main.cpp](main.cpp)：程序入口。
- [mainwindow.ui](mainwindow.ui)：主菜单界面。
- [mainwindow.h](mainwindow.h), [mainwindow.cpp](mainwindow.cpp)：主菜单逻辑与按钮响应。
- [recordsdialog.h](recordsdialog.h), [recordsdialog.cpp](recordsdialog.cpp)：最近记录列表对话框。
- [difficultydialog.h](difficultydialog.h), [difficultydialog.cpp](difficultydialog.cpp)：难度选择对话框。
- [sudokulogic.h](sudokulogic.h), [sudokulogic.cpp](sudokulogic.cpp)：数独生成与规则校验、序列化。
- [gamewindow.h](gamewindow.h), [gamewindow.cpp](gamewindow.cpp)：棋盘界面（9x9表格），输入校验、进度保存、完成检测。
- [sudoku.pro](sudoku.pro)：qmake项目配置。

## 运行与构建
1. 使用 Qt 5.14.2 + qmake + MSVC/MinGW 在 Windows 构建：
```bash
qmake sudoku.pro
make   # 或 nmake / mingw32-make
./sudoku.exe
```
2. 程序会在可执行文件同级目录创建 `records/` 保存记录文件。

## 设计与实现细节
- 难度控制通过空格数量：简单≈30，普通≈40，困难≈55（可微调）。
- 完整数独生成：基于合法基准模式并随机打乱行列与分区，保证生成一个有效完成盘。
- 题目生成：基于完成盘，移除数字并每次校验解计数，保证唯一解；支持对称移除确保题面美观。
- 输入逻辑：对玩家输入进行行/列/宫冲突校验，不合法则拒绝；合法则写入 `grid_current.txt` 并追加一步日志。
- 最近10项：记录窗口显示最新的10个 `records/` 子目录，超过10的不显示但目录保留。

## 可改进建议
- 增加“撤销/重做”功能（基于 `steps.log`）。
- 保证唯一解题目生成与更精细的难度设计（按线索数控制：简单≈44、普通≈36、困难≈28，支持对称）。
- 高亮行/列/宫与冲突提示、候选数笔记模式。
- 记录中展示用时、完成状态等更多元数据。

## 约束与取舍
- 为简化逻辑，目前不保证题目唯一解，但规则校验与完成检查有效。
- 记录路径使用可执行目录下的 `records/`，如需用户文档目录可改为 `QStandardPaths`。

## 变更摘要
- 新增 UI：主菜单四按钮；记录列表对话框；难度选择对话框；游戏棋盘窗口。
- 新增逻辑：数独生成、题面生成、输入校验、文件持久化（解/初始/当前/步骤）。
- 更新 `sudoku.pro`：包含所有新增源文件与表单。

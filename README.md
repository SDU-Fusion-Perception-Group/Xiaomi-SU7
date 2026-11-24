# 工创地面站 · 地图与只读联调
Tauri v2 + Vue 3 TypeScript。驾驶舱支持模拟位姿及独立的只读 Pi 遥测通道；启动、急停只改变模拟器，不发送底盘指令。

## 目录与交付

- 本仓库只跟踪源码。Windows x64 EXE 和 ZIP 不纳入 Git；构建方式及验证记录见 [Windows 发布记录](docs/windows-release-check.md)。
- `src/`：Vue 界面与状态；`src-tauri/`：Rust 桌面壳；`docs/`：接口与验收资料。
- `tools/`：联调和校验工具；`tests/`：自动回归；`node_modules/`、`dist/`、`src-tauri/target/` 为本机依赖或构建产物，不进入交付包。

## 日常联调

1. 点左侧 **操作台** 看会话日志；启动模拟，再急停。按来源或级别筛选，或在 CLI 输入 `status`。此处不是 SSH 终端。
2. 点 **Pi 接入** 填实际网关地址。用无效地址（如 https://localhost/）试连接，在操作台错误列表应看到链路诊断。合法地址需形如 ws://主机:端口/ws。
3. 收到有效扫描才绘制点云。无数据不是零个障碍。现场网关部署、时钟与坐标对照见 [Pi 端联调手册](docs/Pi端联调手册.md)。
4. JSONL 是可携带的诊断快照；当前会话只保留最近 300 条，刷新会丢失，尚非 SQLite 历史库。
5. 在工程目录运行 `node --test tests/*.test.mjs` 验证本地协议与模拟控制。

## 本机预览
在本目录运行 npm run dev，打开终端显示的本机地址。npm run build 执行 TypeScript 检查并生成前端 dist；这不是 Windows EXE。正式桌面需使用 Rust/MSVC 和 npm run tauri build。

## 验收顺序
1. 对照 docs/rules-fig1.jpg（赛规第 3 页图 1）验收 2.4 m 方形场地、两个启停区、四块禁入区和原料／粗加工／暂存装置。虚线仅为启停区 1 出发的模拟行车示例，不是官方指定路径。摆位取规则标称值，尚未模拟现场抽签或判分。
2. 启动后检查位置读数连续更新。滚轮或 + / − 改变比例；重置恢复默认。切换网格、点云不应遮住状态栏。
3. 点急停或按空格，检查车停、状态条锁定、屏幕红框和急停文字。解锁后应回到待命，不自动运动。
4. 缩窄窗口，再检查状态栏、急停操作和底部地图工具。视觉通过后再开发 ECharts 遥测、YAML 参数、SQLite 会话等后续模块。

## 文件与工具位置
前端依赖归项目 node_modules，版本清单由 package-lock.json 固定。此次安装使用 C:\Sdk\Node\npm-cache，不写全局 npm 包、不改用户 PATH。Node 是已装工具。本轮不安装 Rust 或其他系统 SDK。

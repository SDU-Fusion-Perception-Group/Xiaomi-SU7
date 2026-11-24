# Jetson → Windows 地面站联调

此分支只提供 API 和联调文档，不含 Jetson 发布端、ROS 驱动或 Windows 程序源码。文档沿用旧文件名；其中 Pi 指车端发布者，在本分支对应 Jetson。Windows 工程与只读抽检 CLI 在 [`win` 分支](https://github.com/SDU-Fusion-Perception-Group/Xiaomi-SU7/tree/win)。

- [上报 API v1](docs/Pi上报接口-v1.md)：`task_state`、`event`、`telemetry`、`scan_lite` 的字段、单位与校验规则。
- [联调手册](docs/Pi端联调手册.md)：局域网、时钟、坐标、抽检及验收。手册中的 CLI 与测试命令在 `win` 源码目录执行。

## 第一次联调

1. Jetson 启动只读 WebSocket 发布服务，监听局域网地址的 `/ws`，握手后主动推送 `schema_version: 1` 的 JSON 文本消息。
2. 先用模拟数据交错上报两个不同 `run_id` 的任务快照和一条事件。同一连接的任务 `seq` 全局递增。
3. Windows 连接 `ws://<Jetson局域网IP>:8765/ws`（示例端口）。确认两个进度独立更新，隐藏后重新勾选能看到最新进度，并核对事件日志。通过后再接真实遥测和点云。

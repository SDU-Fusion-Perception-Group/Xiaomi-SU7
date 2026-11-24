# Pi 5 → 地面站：只读联调手册

先在 Pi 上确认采集与坐标，再在 Windows 上抽检同一条 WebSocket 流，最后检查地图。**本工程提供的是接收端，不是 ROS 网关。** 外部 CLI 已可直接运行，不需要 npm 依赖；浏览器接入使用同一份线协议。Mock 是界面演示源，不能证明已连接 Pi。测试用回环服务不是车端程序。CLI 不订阅 ROS topic、不监听端口，也不下发控制或急停命令。

## 1. 先确认网络和发布端

例：Pi 有线地址 `192.168.10.5/24`，上位机 `192.168.10.20/24`，网关监听 `192.168.10.5:8765`。地址与端口均为示例，不是自动发现值。用 Pi 的 `ip -br addr` 与 Windows 的 `ipconfig` 核对接口；避免 Wi-Fi 客户端隔离或重复 IP。只在受信任的联调网开放该 TCP 端口，限制来源为上位机，不要直接暴露到公网。

1. Pi 开发者需自行提供发布端：读取传感器/ROS → 获取采样时刻 TF → 打包下述 JSON → 对成功升级到 `/ws` 的连接主动推送。不要求客户端先发订阅帧。既有 rosbridge 协议不能直接替代此协议。
2. Pi 用 `ss -ltn` 检查实际监听地址和端口。只绑定 `127.0.0.1` 时，Windows 无法访问。在 Windows PowerShell 执行 `Test-NetConnection 192.168.10.5 -Port 8765`；TCP 成功不是 WebSocket 握手成功。
3. 私网示例入口：`ws://192.168.10.5:8765/ws`。HTTPS 页面应使用证书受信的 `wss://…/ws`；经过反向代理时须转发 WebSocket Upgrade，允许预期浏览器 Origin。不要以关闭全局安全检查来解决握手问题。

## 2. 线协议 v1

实现 Pi 发布端时，以 [Pi 上报 API v1](./Pi上报接口-v1.md) 为完整字段参考；本节保留现场速查和示例。

每条 WebSocket **文本消息**只装一个 UTF-8 JSON 对象；不用数组、二进制、ROS 信封或跨消息拼接。下面是两条独立消息（时间仅示意）：

~~~jsonl
{"schema_version":1,"type":"event","level":"info","source":"pi5.lidar","message":"雷达已就绪","ts_ms":1795000000000}
{"schema_version":1,"type":"scan_lite","frame_id":"map","points":[[0.35,0.72],[0.36,0.73]],"ts_ms":1795000000010}
~~~

- `schema_version` 是数字 1。`ts_ms` 为采样/事件发生时的 Unix 毫秒，非负安全整数；不要发送字符串或发送时刻代替采样时刻。
- `event`：`level` 只允许 `info|warn|error`；`source`、`message` 是非空文本。TF 缺失等故障可发限频 warn，不要把旧数据改时间戳重新发送。
- `scan_lite`：只接受 `frame_id:"map"`；每点恰好两个有限数值，单位米，不是 `{x,y}`。每帧最多 4096 点，超过时在发布端按角度均匀降采样。空数组表示完成一次扫描但无有效回波；不要以空数组掩盖断流。所有帧均不携带控制指令。
- `telemetry`：每帧一个标量实测值。字段 `channel`、`unit`、`value` 必填，value 是有限数字（不是字符串）。Pi 先转换到约定单位，不要用速度指令或模拟值充当反馈。

| channel | 测量及方向约定 | unit（精确匹配） |
|---|---|---|
| gyro | 机体 Z 轴角速度，从上往下看逆时针为正 | °/s |
| motor | 约定驱动电机的有符号实测转速，前进为正；联调记录中注明电机编号 | rpm |
| fused | 融合航向角，map +x 为零，逆时针为正 | ° |

~~~jsonl
{"schema_version":1,"type":"telemetry","channel":"gyro","unit":"°/s","value":-2.5,"ts_ms":1795000000011}
{"schema_version":1,"type":"telemetry","channel":"motor","unit":"rpm","value":240,"ts_ms":1795000000012}
{"schema_version":1,"type":"telemetry","channel":"fused","unit":"°","value":87.2,"ts_ms":1795000000013}
~~~

上位机按通道分别接收，时间戳应严格递增；重复或倒序帧不覆盖新读数。接收时若采样时间与本机相差超过 1500 ms，或单位错误，会记录协议错误并断开。连接仍在但某通道停更超过 1500 ms，面板提示“测量断流”，不补零；当前读数和历史分开显示。窗口可选 5/10/30 s，每通道至多 2048 点，重连清空旧会话。

融合位置通道 `fused_x`、`fused_y` 使用 `map` 坐标系，单位均为 `m`，每轴一条 telemetry 消息；与模拟车图标的数据源分开。

### 任务状态与事件

沿用同一个 `/ws`。Pi 负责运行状态机、计算任务总进度；地面站不推断状态，不下发控制。任务栏里的「Pi 任务」与「模拟任务流程」分别显示。

~~~jsonl
{"schema_version":1,"type":"task_state","ts_ms":1795000000100,"seq":42,"run_id":"run-20261120-01","state":"running","progress_pct":35,"stage_id":"pickup","stages":[{"id":"pickup","label":"取料"},{"id":"deliver","label":"放料"}],"detail":"已到达原料区"}
{"schema_version":1,"type":"event","ts_ms":1795000000110,"level":"info","source":"mission","message":"已到达原料区"}
~~~

- `task_state` 每帧是单个 `run_id` 的完整快照；多任务可交错上报，使用同一连接全局递增的 `seq`。新版前端在「任务 → 显示任务」勾选任务，隐藏时仍更新快照；每个任务独立过期。完整时序和上限见 [Pi 上报 API v1](./Pi上报接口-v1.md)。
- `state`：`idle | running | paused | estop | succeeded | failed | cancelled`。进度为整数。空闲用 `run_id:null, stage_id:null, progress_pct:0, stages:[]`；成功用 `stage_id:null, progress_pct:100`。其他状态保留当前或最后阶段及 0–99 的实际进度。
- 每次执行使用新的 `run_id`；任务内阶段按显示顺序给出，1–32 项，阶段 ID 唯一，`stage_id` 对应列表中的一项。ID 只能用 ASCII 字母、数字、点、下划线、冒号、短横线，以字母或数字开头，最长 64。阶段名称最长 80 个 UTF-16 码元；可选 `detail` 最长 256，均须非空白。
- `event` 进入操作台日志，保留车端 `ts_ms`。事件不改进度；需要同时记日志和改变当前状态时，分别推两帧。实车急停状态只是 Pi 的报告，界面「模拟急停」不影响实车。
- 联调时推送上述两帧，检查阶段高亮、35% 和日志；再递增 `seq` 并切换为 `paused`，核对画面。停更 4 秒应显示过期；断开、重连应重新接收完整快照。

## 3. 从雷达坐标变换到 map

场地标称 2.4 m × 2.4 m；正视地图，原点在左下，+x 向右、+y 向上。屏幕 y 轴翻转由显示端负责，Pi 不翻转。若定位系统的 map 原点/轴与场地不同，先标定两者的固定变换。

对 LaserScan 第 i 束，令 θ = angle_min + i × angle_increment（弧度），剔除非有限值及 range_min/range_max 以外的 r，取雷达点 (r cosθ, r sinθ, 0)。查采样时刻的 `T_map←base × T_base←laser`，先将点乘雷达安装外参，再乘车体位姿；最后发 map 中的 (x,y)。不可只把 laser 的 frame_id 改成 map。车辆运动明显时按束时刻补偿；若 TF 不可用，丢弃该帧并报原因。

**手算抽检**：雷达与车体同轴同原点，车在 (1.0,0.5)，逆时针转 90°，正前方 0.4 m 回波应为 (1.0,0.9)。现场选两个不共线方向的已知标志点，静止、换朝向各采一次；目标应仍落在原位置。若整体平移查外参/地图原点；绕车镜像查角度正负；放大千倍查毫米与米。

## 4. 对时与新鲜度

Pi 用 `timedatectl status`，Windows 用 `w32tm /query /status` 核对时钟同步源。真实采集须使用同步后的系统时间，不能用启动时长或 ROS 仿真时钟。`sec × 1000 + floor(nanosec / 1000000)` 是 ROS 墙钟时间戳的毫秒转换。局域网建议两端偏差小于 100 ms，给缓存和传输留出余量。抽查接收时本机 `Date.now() - ts_ms`：连续很大时查队列积压或时钟，持续负值查 Pi 快钟。地图对与当前时间相差超过 1500 ms 的扫描不作有效检测；CLI 保留原时间便于诊断，退出 0 不代表数据新鲜。

## 5. 部署与十秒抽检

在有原生 WebSocket 的 Node 22+ 机器执行；可以只复制 `tools/station-cli.mjs` 和 `src/task-protocol.mjs` 到现场机器（保留相对目录），不需复制 node_modules 或启动桌面壳。在工程目录运行：

~~~sh
node -p "typeof WebSocket"   # 应为 function
node tools/station-cli.mjs --help
node --test tests/cli.test.mjs tests/scope.test.mjs  # 源码测试需项目依赖和 Node 24
node tools/station-cli.mjs ws://192.168.10.5:8765/ws --timeout-ms 10000 > sample.jsonl
~~~

最后一条从连接开始计时（默认 10 秒，不是空闲超时），stdout 每个有效帧一行，诊断走 stderr。采样窗口到期或服务端正常关闭时，已收到有效帧才算成功。遇到坏帧立即退出；此前的行仍在文件中，**必须检查退出码**。Bash 用 `echo $?`，PowerShell 用 `$LASTEXITCODE`；Windows 如需原样保存 UTF-8，可在 cmd.exe 下重定向并用 `echo %ERRORLEVEL%`。

| 退出码 | 含义与下一步 |
|---|---|
| 0 | 帮助或成功采样。检查所需消息类型及三个测量通道是否齐全；单个 event 也会成功。 |
| 1 | 运行时、握手、异常断连、空流提前关闭或输出失败。先查 Node 和输出路径，再核对监听、TCP、路径和证书。 |
| 2 | 参数错误。网址须为 ws/wss 且路径为 /ws；超时以正整数毫秒指定。 |
| 3 | 协议错误。对照 stderr 检查文本 JSON、版本、类型、时间、点对形状及坐标系。 |
| 4 | 总窗口内未收到帧。核对握手及推送日志，再查传感器、TF 和发布频率；不要把无样本写成零障碍。 |

现场部署时把发布端版本、端点、标定外参和时钟源一起登记。开机顺序：网络和对时 → 传感器和定位 → 自备网关 → CLI 抽检 → 地面站。若部署为服务，使用普通账户并限制日志大小；重启后重复抽检。本仓库不提供 Pi 镜像、ROS 包或服务单元。

## 6. 验收记录（须用实际 Pi 填写）

- [ ] 记录两端 IP、端口、Node 版本、采样起止、退出码和原始 JSONL；CLI 与浏览器指向同一发布端。浏览器不可停留在 Mock 源。
- [ ] 在十秒内由发布端推送一次带可辨识标记的 event 和连续 scan_lite；逐行核对版本、来源、级别、ts_ms、点数与首末点。检查连续帧时间不倒退，计算相邻时间差与期望扫描周期比对。
- [ ] 两个实测地标在两个车体朝向下均对齐，记录最大误差并与现场标定容差比较；目标不应随车转动。CLI 有点但地图不显示时先核新鲜度、图层开关及 map 对齐。
- [ ] 打开三个测量标签，对照 CLI 的 channel、unit、value 和 ts_ms 与右栏最近采样；暂停单个通道 2 秒，确认该通道读数变为 —，其余通道正常。切换历史窗口、关闭并重新添加标签。
- [ ] 停止发布验证无帧超时；再断开网关验证显示断连而非假实时数据。恢复后重新连接、重新采样。不要把回环测试通过或 Mock 显示正常当作实车验收。

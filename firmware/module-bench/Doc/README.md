# DM-MC02 模块联调模板

本目录是 CMake + `arm-none-eabi-gcc` 工程，不使用 Keil。它复用官方 `CtrBoard-H7_ALL` 的 CubeMX 初始化，仅用于模块联调，不包含物流搬运任务流程。

## 构建

官方源码路径包含中文时，GCC 15 无法创建目标文件。构建前建立两个英文目录联接：

```powershell
New-Item -ItemType Junction C:\Users\S\dmc-official (Resolve-Path dm-mc02\例程\CtrBoard-H7_ALL)
New-Item -ItemType Junction C:\Users\S\dmc-build (Resolve-Path firmware\module-bench)
cmake -S C:\Users\S\dmc-build -B C:\Users\S\dmc-out -G Ninja `
  -DCMAKE_TOOLCHAIN_FILE=C:/Users/S/dmc-build/cmake/arm-none-eabi-gcc.cmake `
  -DOFFICIAL_ROOT=C:/Users/S/dmc-official
cmake --build C:\Users\S\dmc-out
```

产物为 `C:\Users\S\dmc-out\module-bench` 和 `module-bench.hex`。

## 模块

- USART3 RS485：PD8 TX、PD9 RX、PB14 DE，115200 8N1，空闲中断组帧。
- LYX Modbus RTU：地址 1，读取报警、位置和速度；USB 命令行执行速度、距离和角度动作。
- FDCAN3：PD13 TX、PD12 RX，经典 CAN 1 Mbps，周期发送 ID `0x123`。
- BMI088 + Mahony：内部 1000 Hz 更新，USB 姿态 10 Hz 上报。
- TIM12/PB15 蜂鸣器和 SPI6 WS2812：状态灯语与声音提示，见下文。

## 状态机

状态机分为三层，当前没有比赛任务状态。

### 启动状态

`BOOT` 蓝色，完成外设和协议初始化。`SELF_TEST` 黄色，静止采集 400 个陀螺样本。成功后进入运行状态并响 500 ms；BMI088 或协议记录失败则进入 `IMU_ERROR` 红灯快闪，不进入运动控制。

### 链路状态

由 `BOARD_RS485_ATTACHED` 和 `BOARD_CAN_ATTACHED` 决定：两者接入为绿色，仅 RS485 缺失为青色，仅 CAN 缺失为紫色，两者都缺失为橙色。值为 0 的链路任务立即退出，不发送测试数据。

### 运动状态

`IDLE` 接受命令。`DRIVE` 按轮径执行距离，`ROTATE` 按轮距执行角度，`RUN` 为持续转速，`STOPPING` 向电机写停止。动作期间拒绝新动作并返回 `ERR busy`，完成后自动回 `IDLE`。

机械宏在 `board_config.h`：`BOARD_WHEEL_DIAMETER_MM=75`，`BOARD_WHEEL_TRACK_MM=200`，`BOARD_WHEEL_PULSES_PER_REV=51200`。

USB 文本命令：

| 命令 | 动作 |
| --- | --- |
| `drive 10` / `drive -10` | 前进或后退 10 cm |
| `cw 90` / `ccw 90` | 顺时针或逆时针旋转 90° |
| `fw [rpm]` / `rv [rpm]` | 持续正转或反转 |
| `stop` | 停止 |
| `status` | 电机地址、报警、位置、转速 |
| `help` | 命令列表 |

当前只连接地址 1 的电机，左右轮接口暂时指向它。第二台电机分配独立地址后再分别控制。

## 灯语与蜂鸣器

灯语由 `status_indicator.c` 每 100 ms 刷新，异常状态以 4 Hz 闪烁。

| 状态 | RGB 灯 | 蜂鸣器 |
| --- | --- | --- |
| 启动初始化 | 蓝色常亮 | 静音 |
| 陀螺仪零偏校准 | 黄色常亮 | 静音 |
| 板载芯片自检失败 | 红色快闪 | 静音 |
| RS485 和 CAN 都已接入 | 绿色常亮 | 自检结束响 500 ms |
| 仅 RS485 未接入 | 青色常亮 | 自检结束响 500 ms |
| 仅 CAN 未接入 | 紫色常亮 | 自检结束响 500 ms |
| RS485 和 CAN 都未接入 | 橙色常亮 | 自检结束响 500 ms |

`BOARD_RS485_ATTACHED` 和 `BOARD_CAN_ATTACHED` 都在 `board_config.h`。值为 0 时，对应任务在启动后立即退出，不会发送轮询或测试帧。当前两者均为 0。

| 收到 USB `ping` | 保持当前灯语 | 短响 30 ms |
| 收到运动命令 | 保持当前灯语 | 静音，进入对应运动状态 |

- USB HS CDC：承载 MAVLink v2 和文本命令行。

## MAVLink

- 上行遥测：`ATTITUDE (30)`，单位为弧度。
- 扩展量：`NAMED_VALUE_FLOAT (251)`，名称包括 `gx gy gz ax ay az temp motor alarm pos rpm rsf rse crx ctx cerr`。
- 下行命令：`COMMAND_LONG (76)`；`command=1` 触发蜂鸣，`command=2` 触发电机点动。
- System ID 和 Component ID 均为 1。

浏览器使用 Web Serial 读取 USB CDC 字节流后，可直接使用 MAVLink v2 解析器解包，不需要 VOFA 的 JustFloat 帧尾。

# Xiaomi SU7 下位机工程

STM32H723VGT6 / DM-MC02 物流车下位机工程。当前仓库只保存自研固件、独立 CAN 测试和协议文档。

## 目录

- `firmware/module-bench`：CMake/GCC 模块联调固件，包含 IMU、USB CLI、RS485 和 DJI CAN。
- `firmware/can1-fdcan-test`：基于官方 `CtrBoard-H7_CAN` 的独立 FDCAN1 测试工程。
- `DOC`：LYX 步进驱动与 DJI C620/M3508 CAN 协议说明。

官方 CubeMX 例程、HAL 源码、原理图和三维模型体积较大，不纳入本仓库。构建时通过 `OFFICIAL_ROOT` 或 `OFFICIAL_CAN_ROOT` 指向本地官方例程目录。

## CAN 状态

C620/M3508 使用经典 CAN、1 Mbps、标准帧：

- `0x200`：电调 ID 1–4 电流控制帧。
- `0x1FF`：电调 ID 5–8 电流控制帧。
- `0x201`–`0x208`：对应电调反馈帧。

当前实机测试使用官方 CAN 位时序，运行寄存器为 `NBTP=0x26003A13`。在 80 MHz FDCAN 内核时钟下，总时间片为 80，波特率为 1 Mbps。MCU 已发送控制帧，但尚未收到电调 ACK 或 `0x201` 反馈。

## 构建

在已配置 Arm GNU Toolchain、CMake 和 Ninja 的环境中：

```powershell
cmake -S firmware/module-bench -B build/module-bench -G Ninja
cmake --build build/module-bench --parallel
```

独立 CAN 测试：

```powershell
cmake -S firmware/can1-fdcan-test -B build/can1-fdcan-test -G Ninja
cmake --build build/can1-fdcan-test --parallel
```

# Jetson 重装前比赛资料全量备份

备份日期：2026-11-28（Windows 时间）。设备：NVIDIA Jetson Orin Nano Developer Kit；USB SSH 192.168.55.1，用户 b0weny。Jetson 时钟为 1970，不能据此解释本次备份时间。

## 范围
完整复制 /home/b0weny 下六个比赛目录：hwt905_serial_lab、robot_px4_core、rplidar_serial_lab、rplidar_sdk-verified、wifi_bandwidth_lab、Xiaomi-SU7。包含日志、raw、源码、配置、测试、第三方 SDK、构建缓存、驱动 ko 与官方源码包，未只挑文档。原始清单 577 文件，575098912 字节；本地复制已逐文件 SHA-256 验证一致。三个嵌套 Git 目录（含构建依赖 SDK）另存 archives 中，无损保留历史。已从提交内容独立重建，577 个原始文件全部校验通过。

这是比赛项目全量备份，不是 NVMe 整盘镜像。未包含 OS 本体、用户浏览器缓存、SSH 私钥、认证文件、Wi-Fi 密码、shell 历史、未完成下载 .part。system-snapshot.json 补充系统版本、包清单、驱动模块、设备信息和可读取的 udev/modprobe 等配置，不能替代整盘恢复。原始 Jetson 文件未删除或修改。

## 大文件与恢复
GitHub 单文件限制要求将 public_sources.tbz2 与 kernel_src.tbz2 无损分成 40 MiB 卷，映射、哈希见 archive-map.json。完整原件仍留在 Windows 本地备份，Git 中仅忽略重复原件。不需要 Git LFS 或安装新软件。

在新 Linux 上克隆 jetson 分支后，使用 Python 3.12+ 执行本目录 restore.py：拼接两份大文件，解包 Git 元数据，逐文件核对 source-manifest.json，恢复执行权限与软链接。验证成功后人工审查并复制 home/* 到目标 home；不要直接覆盖新系统配置。旧 ch341.ko 与构建产物绑定旧内核，重装后应对照 system-snapshot.json 和驱动日志重新构建，不能保证直接加载。

## 安全说明
未复制 .ssh、GitHub CLI 登录资料、NetworkManager 密码及 shell 历史。已扫描项目可检索文本，无确认的凭据命中；官方源码包及二进制不是逐行安全审计。SDK、驱动及第三方源码沿用其原许可。

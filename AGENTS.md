# 工创项目：Windows 上位机约束
适用于 car-ground-station 及其后续 Windows 上位机工程，不改变车端或既有雷达工具的技术栈。

- Windows EXE 是交付格式。桌面壳使用 Tauri v2（Rust），界面使用 Vue 3 SFC、TypeScript、Pinia、Tailwind CSS 和 lucide-vue-next。地图使用 Konva/vue-konva，遥测波形使用 ECharts，本地持久化使用 SQLite。参数表单按 YAML schema 生成。不得以 CSV、localStorage 代替正式数据库。
- 脚手架由用户初始化并锁定；未经明确同意不得生成或替换。工具链缺失、安装失败或构建失败时报告具体阻塞，不得改框架。旧 WinForms EXE、C# 源码和独立网页原型不属于新产品，不得引入新工程或当作交付件。
- 所有业务 UI 使用 Vue 单文件组件和 <script setup lang="ts">。禁止手拼 HTML、innerHTML、document.createElement，禁止新建 C#/WinForms 或独立单文件 HTML。Vite 模板的 index.html 只保留挂载入口。网络、模拟源与持久化均不得耦合到显示组件。
- 每轮只做一个可审查模块。首轮只验收地图主视图（MapCanvas.vue + TopHudBar.vue），只用 Mock，不接通信、不堆设置页。提交实际运行截图和可复现的交互步骤，用户确认视觉后再开始下一模块。不能用编译通过代替视觉验收。
- 地图全窗铺底，状态条悬浮，不用原生按钮拼面板。静态地图、交互航点和动态扫描分层；点云批绘，禁止每点创建节点。数值等宽，色彩按状态使用；无数据不画假波形，模拟数据始终标明来源。验收至少看正常与急停两态、常用窗口与窄窗口下文字是否遮挡地图/互相重叠。

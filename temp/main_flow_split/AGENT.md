# 临时工程：main 函数流程化拆分（temp\main_flow_split）

本目录是**实验用临时副本**，用于验证「把 `main` 拆解为若干流程函数」这一改动是否可以正常构建、运行。
原工程 `D:\neon\program\project\cpp\NeonEngine` 下的任何文件都**没有被修改**：这里复制的是源码与工程文件的副本，
其中只有 `Engine\main.cpp` 是本目录独有的新版本（原 `Engine\main.cpp` 保持原样）。

## 目录结构

```
temp\main_flow_split\
├─ AGENT.md                  <- 本文件（构建/验证流程）
├─ NeonEngine.slnx           <- 解决方案副本
├─ vcpkg.json                <- 清单副本（时间戳与原仓库一致）
├─ vcpkg-configuration.json
├─ scripts\build.ps1         <- 本临时工程的构建脚本
└─ Engine\                   <- 源码副本（不含 x64\、Engine\Engine\、logs\ 等构建产物）
   ├─ main.cpp               <- 拆分后的 main（本次改动的唯一产物）
   ├─ Engine.vcxproj
   └─ core\ graphics\ gameplay\ tool\ assets\
```

## 改动摘要

只做拆分，不考虑复用，逻辑与调用顺序与原 `main` 一致：

| 流程 | 函数 | 对应原 main 中的代码 |
| --- | --- | --- |
| 0 | `AppContext`（结构体） | 原先散落在 main 里的局部变量集中存放 |
| 1 | `startup` | 日志级别、配置、`Game path` 输出 |
| 2 | `initWindow` | glad 初始化、窗口、上下文、viewport、帧率控制器设值 |
| 3 | `initGraphics` | 着色器加载/链接/构建程序（失败返回 false） |
| 4 | `initScene` | 示例实体、顶点/索引/网格、1x1 纹理、10 万实体创建 |
| 5 | `initRenderState` | `glfwSwapInterval`、共享 UBO、view/projection、渲染模式变量 |
| 6 | `updateFrameInput` | 事件轮询 + 数字键切换渲染模式 |
| 7 | `updateFrameSimulation` | `physicsSystem.update()` |
| 8 | `renderFrame` | 清屏、收集组件、三种渲染模式分发绘制 |
| 9 | `updateFrameRateStatistics` | 平均 FPS、标准差、稳定度 |
| 10 | `presentFrame` | 交换缓冲、帧率限制、统计与心跳日志 |
| 11 | `runMainLoop` | `while (!glfwWindowShouldClose(...))` 主循环 |
| 12 | `reportSummary` | 退出后的统计输出 |

`main` 现在只剩：设置控制台编码 → 构造 `AppContext` → 依次调用上述流程 → 返回退出码。

## 构建前提

- Windows + Visual Studio C++ 生成工具（`v145` 工具集）+ Windows 10 SDK（与原工程要求相同）。
- 依赖（fmt / glfw3 / glad / glm / spdlog / stb）已在**原仓库**的 `vcpkg_installed` 中安装好。

> 本临时目录**没有**自己的 `vcpkg_installed`。构建时通过 `/p:VcpkgInstalledDir=<原仓库>\vcpkg_installed`
> 复用原仓库已安装的依赖目录，这样既不用复制 600 MB 的依赖，也不会在临时目录里触发一次真正的 vcpkg 安装
> （`vcpkg.json` 与原仓库同时间戳，早于 `vcpkg_installed\.msbuildstamp-x64-windows.stamp`，MSBuild 判定依赖已是最新）。

## 构建（推荐：脚本）

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1            # Release x64
powershell -NoProfile -ExecutionPolicy Bypass -File .\scripts\build.ps1 -Configuration Debug
```

脚本内部做的事与原仓库 `scripts\build.ps1` 相同：先找 `msbuild.exe`（找不到就用 vswhere 定位 VS 安装目录），
再执行：

```powershell
& $msbuild .\NeonEngine.slnx /m /p:Configuration=Release /p:Platform=x64 `
    /p:VcpkgInstalledDir=D:\neon\program\project\cpp\NeonEngine\vcpkg_installed /v:minimal
```

产物：`x64\Release\Engine.exe`（同目录会有 `fmt.dll` / `glfw3.dll`）。

## 运行验证

着色器路径是相对**进程工作目录**的 `assets/shaders/...`，所以必须把工作目录设为 `Engine`：

```powershell
Push-Location .\Engine
try { & ..\x64\Release\Engine.exe } finally { Pop-Location }
```

窗口打开后按 `1` / `2` / `3` 切换三种渲染模式，按 `Esc` 或关闭窗口退出；
日志同时输出到控制台和 `Engine\log.txt`（心跳日志每 180 帧一条）。

## 验证记录（本次实际执行结果）

### 构建（`scripts\build.ps1`，Release x64）

```
MSBuild          : D:\app\Visual Studio\IDE_26\MSBuild\Current\Bin\MSBuild.exe   (MSBuild 18.10.1)
Solution (temp)  : D:\neon\program\project\cpp\NeonEngine\temp\main_flow_split\NeonEngine.slnx
VcpkgInstalledDir: D:\neon\program\project\cpp\NeonEngine\vcpkg_installed
Engine.vcxproj -> ...\temp\main_flow_split\x64\Release\Engine.exe
Build succeeded: D:\neon\program\project\cpp\NeonEngine\temp\main_flow_split\x64\Release\Engine.exe
--- exit=0 elapsed=10s ---
```

警告与原工程一致（`APIENTRY` 宏重定义、`main.cpp:511` 的 `double`→`float`），无新增警告、无错误。

### 运行（工作目录 = `Engine`）

```
[info] Game path: ...\temp\main_flow_split\x64\Release\Engine.exe
[info] OpenGL Version: 4.6.0 / Renderer: Intel(R) UHD Graphics
[info] Main: OpenGL window and context are ready.
[info] Main: shader program is ready.
[info] Main: sprite mesh is ready.
[info] Main: creating 100000 entities and 4 shared 1x1 color textures.
[info] Main: entity and texture creation completed.
[info] Main: entering render loop; diagnostic checkpoint every 180 frames.
[info] Main loop heartbeat: frame=1   ... FPS=1.6 ...
[info] Main loop heartbeat: frame=181 ... FPS=6.5  average=11.8 stability=43.67%.
[info] Main loop heartbeat: frame=361 ... FPS=6.3  average=6.3  stability=7.51%.
（关闭窗口）
[info] Main: render loop exited at frame 486.
[info] Average Frame Rate: 6.78 FPS
[info] FrameRate difference from target: -99.32 %
[info] FrameRate stability (CV): 13.16 %
Engine.exe exited with code 0
```

结论：拆分后的 main 能正常构建、创建窗口/上下文、加载着色器、创建 10 万实体、跑满整个渲染循环并正常退出，各流程函数的诊断日志（含行号，指向 `main.cpp` 中对应的流程函数）都按原节奏输出。

### 过程中修掉的两个问题

1. `reportSummary` 最初写成 `const AppContext&`，而 `Logger::info` 非 const → C2662。已改为 `AppContext&`。
2. 构建脚本最初用中文注释：Windows PowerShell 5.1 会用 ANSI 代码页读取**无 BOM 的 UTF-8** 脚本，
   中文字符的字节数与 ANSI 双字节规则不匹配时会**吞掉行尾换行**，导致下一行被并入注释
   （现象：脚本里的 `VcpkgInstalledDir` 默认值赋值语句从未执行，构建报 “Join-Path 参数为空”）。
   已把 `scripts\build.ps1` 改为**纯 ASCII 注释**，脚本在 PowerShell 5.1 / 7 下均可正常运行。
   注意：`Engine\main.cpp` 里的中文注释不受影响，因为 MSVC 侧由 `/utf-8` 指定源文件编码。

### 原工程未被改动

- 原 `Engine\main.cpp` 内容、`Engine.vcxproj`、`vcpkg.json` 及 `vcpkg_installed\.msbuildstamp-x64-windows.stamp`
  的时间戳均保持构建前状态；本次所有写入都发生在 `temp\main_flow_split\` 内。

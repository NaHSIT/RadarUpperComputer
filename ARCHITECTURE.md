# 工程架构设计

## 目录结构

```
RadarUpperComputer/
├── src/
│   ├── core/                          # 共享核心库 (radar-core)
│   │   ├── domain/                    # 领域模型
│   │   │   ├── RadarDevice.h/.cpp     # 雷达设备模型
│   │   │   ├── BeamState.h/.cpp       # 波束状态模型
│   │   │   ├── RangeGate.h/.cpp       # 距离层模型
│   │   │   ├── WindProfile.h/.cpp     # 风廓线模型
│   │   │   ├── DeviceHealth.h/.cpp    # 设备健康模型
│   │   │   ├── AlarmEvent.h/.cpp      # 告警事件模型
│   │   │   └── RadarTypes.h           # 通用类型定义
│   │   ├── services/                  # 服务层
│   │   │   ├── DeviceService.h/.cpp   # 设备管理服务
│   │   │   ├── AlarmService.h/.cpp    # 告警管理服务
│   │   │   ├── ExportService.h/.cpp   # 数据导出服务
│   │   │   └── QualityService.h/.cpp  # 质量控制服务
│   │   ├── communication/             # 通信层
│   │   │   ├── IDataSource.h          # 数据源接口
│   │   │   ├── IProtocolAdapter.h     # 协议适配器接口
│   │   │   ├── TcpDataSource.h/.cpp   # TCP 数据源
│   │   │   ├── UdpDataSource.h/.cpp   # UDP 数据源
│   │   │   ├── FrameParser.h/.cpp     # 帧解析器
│   │   │   └── FrameTypes.h           # 帧类型定义
│   │   └── storage/                   # 存储层
│   │       ├── LogRepository.h/.cpp   # 日志存储
│   │       └── ConfigManager.h/.cpp   # 配置管理
│   │
│   ├── client/                        # 桌面客户端 (radar-client)
│   │   ├── app/                       # 应用壳
│   │   │   ├── ClientApp.h/.cpp       # 应用程序类
│   │   │   └── AppContext.h           # 应用上下文
│   │   ├── ui/                        # UI 层
│   │   │   ├── MainWindow.h/.cpp      # 主窗口
│   │   │   ├── navigation/            # 导航组件
│   │   │   ├── pages/                 # 页面
│   │   │   │   ├── DashboardPage.h/.cpp    # 总览页
│   │   │   │   ├── WindFieldPage.h/.cpp    # 风场页
│   │   │   │   ├── BeamPage.h/.cpp         # 波束页
│   │   │   │   ├── SpectrumPage.h/.cpp     # 频谱页
│   │   │   │   ├── DeviceHealthPage.h/.cpp # 设备健康页
│   │   │   │   ├── SettingsPage.h/.cpp     # 设置页
│   │   │   │   ├── DataCenterPage.h/.cpp   # 数据中心页
│   │   │   │   └── MaintenancePage.h/.cpp  # 维护诊断页
│   │   │   └── widgets/               # 通用组件
│   │   │       ├── MetricCard.h/.cpp
│   │   │       ├── WindTrendChart.h/.cpp
│   │   │       ├── WindRoseWidget.h/.cpp
│   │   │       ├── BeamHealthGrid.h/.cpp
│   │   │       ├── AlarmList.h/.cpp
│   │   │       └── RangeGateTable.h/.cpp
│   │   ├── viewmodels/               # ViewModel 层
│   │   │   ├── DashboardViewModel.h/.cpp
│   │   │   ├── WindFieldViewModel.h/.cpp
│   │   │   └── ...
│   │   └── main.cpp                   # 客户端入口
│   │
│   └── radar-web/                     # 雷达端 Web (radar-web-server)
│       ├── server/                    # 后端服务
│       │   ├── RadarWebServer.h/.cpp  # Web 服务主类
│       │   ├── auth/                  # 认证模块
│       │   │   ├── AuthService.h/.cpp
│       │   │   └── SessionManager.h/.cpp
│       │   ├── api/                   # API 控制器
│       │   │   ├── DeviceController.h/.cpp
│       │   │   ├── ConfigController.h/.cpp
│       │   │   └── AlarmController.h/.cpp
│       │   ├── websocket/             # WebSocket 模块
│       │   │   └── WebSocketHub.h/.cpp
│       │   └── main.cpp               # 服务端入口
│       └── frontend/                  # 前端 (Vue 3)
│           ├── src/
│           │   ├── pages/
│           │   ├── components/
│           │   └── stores/
│           ├── package.json
│           └── vite.config.ts
│
├── tests/                             # 测试
│   ├── unit/                          # 单元测试
│   │   ├── test_FrameParser.cpp
│   │   ├── test_WindCalculation.cpp
│   │   └── ...
│   ├── integration/                   # 集成测试
│   └── simulation/                    # 仿真测试
│       ├── simulation_server.cpp
│       └── test_data/
│
├── docs/                              # 文档
│   ├── DESIGN.MD                      # 设计文档
│   ├── API.md                         # 接口文档
│   └── USER_MANUAL.md                 # 用户手册
│
├── reports/                           # 工作报告
│   ├── README.md
│   └── ...
│
├── tools/                             # 工具脚本
│   ├── build.sh                       # 构建脚本
│   ├── deploy.sh                      # 部署脚本
│   └── codegen.py                     # 代码生成
│
├── CMakeLists.txt                     # 顶层 CMake
├── CMakePresets.json                  # CMake 预设
├── .gitignore
└── README.md
```

## 模块依赖关系

```
┌─────────────────────────────────────┐
│           client (桌面客户端)        │
│  依赖: core, Qt6 Widgets            │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│        radar-web-server (雷达端)     │
│  依赖: core, Qt6 HTTP/WebSocket     │
└─────────────┬───────────────────────┘
              │
              ▼
┌─────────────────────────────────────┐
│           core (共享核心库)          │
│  依赖: Qt6 Core, 无 UI 依赖        │
│  包含: domain, services,            │
│        communication, storage       │
└─────────────────────────────────────┘
```

## 构建目标

### CMakeLists.txt 结构

```cmake
# 顶层 CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(RadarUpperComputer VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 共享核心库
add_subdirectory(src/core)

# 桌面客户端
option(BUILD_CLIENT "Build desktop client" ON)
if(BUILD_CLIENT)
    add_subdirectory(src/client)
endif()

# 雷达端 Web 服务
option(BUILD_RADAR_WEB "Build radar web server" ON)
if(BUILD_RADAR_WEB)
    add_subdirectory(src/radar-web/server)
endif()

# 测试
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
```

### 核心库 CMakeLists.txt

```cmake
# src/core/CMakeLists.txt
add_library(radar-core STATIC
    domain/RadarDevice.cpp
    domain/BeamState.cpp
    domain/RangeGate.cpp
    domain/WindProfile.cpp
    domain/DeviceHealth.cpp
    domain/AlarmEvent.cpp
    
    services/DeviceService.cpp
    services/AlarmService.cpp
    services/ExportService.cpp
    services/QualityService.cpp
    
    communication/TcpDataSource.cpp
    communication/UdpDataSource.cpp
    communication/FrameParser.cpp
    
    storage/LogRepository.cpp
    storage/ConfigManager.cpp
)

target_include_directories(radar-core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(radar-core PUBLIC
    Qt6::Core
    Qt6::Network
)
```

### 客户端 CMakeLists.txt

```cmake
# src/client/CMakeLists.txt
add_executable(radar-client
    main.cpp
    app/ClientApp.cpp
    ui/MainWindow.cpp
    ui/pages/DashboardPage.cpp
    ui/pages/WindFieldPage.cpp
    ui/widgets/MetricCard.cpp
    ui/widgets/WindTrendChart.cpp
    ui/widgets/WindRoseWidget.cpp
    ui/widgets/BeamHealthGrid.cpp
    ui/widgets/AlarmList.cpp
    ui/widgets/RangeGateTable.cpp
    viewmodels/DashboardViewModel.cpp
)

target_include_directories(radar-client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(radar-client PRIVATE
    radar-core
    Qt6::Widgets
    Qt6::Charts
)
```

## 代码迁移计划

### 阶段 1：创建核心库结构

1. 创建 `src/core/` 目录
2. 迁移领域模型到 `src/core/domain/`
3. 迁移通信模块到 `src/core/communication/`
4. 创建服务层接口

### 阶段 2：重构桌面客户端

1. 创建 `src/client/` 目录
2. 迁移 UI 组件到 `src/client/ui/widgets/`
3. 迁移页面到 `src/client/ui/pages/`
4. 创建 ViewModel 层
5. 创建应用壳

### 阶段 3：重构雷达端

1. 创建 `src/radar-web/` 目录
2. 创建后端服务框架
3. 迁移前端代码（Vue 3）

### 阶段 4：完善构建系统

1. 编写 CMakeLists.txt
2. 配置依赖管理
3. 编写构建脚本

## 代码规范

### 命名规范

| 类型 | 规范 | 示例 |
|------|------|------|
| 类名 | PascalCase | `DeviceService` |
| 方法名 | camelCase | `getConnectionState` |
| 成员变量 | m_ 前缀 | `m_connectionState` |
| 常量 | UPPER_SNAKE_CASE | `MAX_FRAME_SIZE` |
| 文件名 | PascalCase | `DeviceService.h` |
| 目录名 | kebab-case | `device-service` |

### 文件组织

| 文件类型 | 位置 | 说明 |
|---------|------|------|
| 头文件 | `.h` | 接口定义 |
| 实现文件 | `.cpp` | 实现代码 |
| 测试文件 | `test_*.cpp` | 单元测试 |
| UI 文件 | `*.ui` | Qt Designer 文件 |

### 依赖管理

1. **核心库**：只依赖 Qt6 Core 和 Network
2. **客户端**：依赖核心库 + Qt6 Widgets + Charts
3. **雷达端**：依赖核心库 + Qt6 HTTP/WebSocket
4. **禁止**：核心库依赖 UI 组件

## 迁移检查清单

### 核心库迁移

- [ ] 创建 `src/core/domain/` 目录
- [ ] 迁移 `RadarDevice` 模型
- [ ] 迁移 `BeamState` 模型
- [ ] 迁移 `RangeGate` 模型
- [ ] 迁移 `WindProfile` 模型
- [ ] 迁移 `DeviceHealth` 模型
- [ ] 迁移 `AlarmEvent` 模型
- [ ] 创建 `src/core/communication/` 目录
- [ ] 迁移 `FrameParser`
- [ ] 迁移 `TcpDataSource`
- [ ] 创建 `src/core/services/` 目录
- [ ] 创建服务层接口

### 客户端迁移

- [ ] 创建 `src/client/` 目录
- [ ] 迁移 UI 组件
- [ ] 迁移页面
- [ ] 创建 ViewModel 层
- [ ] 创建应用壳
- [ ] 创建 `main.cpp`

### 构建系统

- [ ] 编写顶层 CMakeLists.txt
- [ ] 编写核心库 CMakeLists.txt
- [ ] 编写客户端 CMakeLists.txt
- [ ] 配置依赖管理
- [ ] 测试构建

---

**最后更新**：2026-07-09
**维护人**：架构守门 Agent

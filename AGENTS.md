# AGENTS.md - GXDE Image Viewer

## 项目概述

GXDE Image Viewer 是 GXDE 桌面环境的图片查看器，基于 Qt5 (QWidget) 和 DTK (Deepin Tool Kit) 开发。项目最初源自 Deepin Image Viewer，使用 GPLv3 许可证。

## 技术栈

- **语言**: C++11
- **UI 框架**: Qt5 (QWidget) + DTK (Deepin Tool Kit)
- **构建系统**: qmake (.pro 文件)
- **包管理**: pkg-config
- **数据库**: SQLite (通过 Qt SQL 模块)
- **图像处理**: FreeImage, libexif, libraw
- **IPC**: D-Bus (Qt5-DBus)
- **其他依赖**: x11, xext, gio-unix-2.0, dtkwidget

## 项目结构

```
gxde-image-viewer/
├── gxde-image-viewer.pro    # 顶层 subdirs 项目文件
├── viewer/                   # 主应用程序
│   ├── viewer.pro            # 应用程序 .pro 文件
│   ├── main.cpp              # 入口点
│   ├── application.h/.cpp    # Application 类 (继承 DApplication)
│   ├── frame/                # 主窗口框架 (MainWindow, MainWidget, 工具栏)
│   ├── module/               # 功能模块
│   │   ├── view/             # 图片查看模块 (核心)
│   │   │   ├── scen/         # 场景/渲染层 (ImageView, ImageWidget, GraphicsItem)
│   │   │   ├── viewpanel.*   # 查看面板主控件
│   │   │   ├── imagestrip.*  # 底部缩略图条
│   │   │   ├── thumbnailwidget.* # 缩略图列表
│   │   │   ├── navigationwidget.* # 导航窗口
│   │   │   └── lockwidget.*  # 锁定控件
│   │   ├── album/            # 相册模块
│   │   ├── timeline/         # 时间线模块
│   │   ├── slideshow/        # 幻灯片模块
│   │   ├── edit/             # 图片编辑模块
│   │   └── JQLibrary/        # 第三方库 (二维码识别)
│   ├── controller/           # 控制器层
│   │   ├── signalmanager.*   # 全局信号管理器 (单例)
│   │   ├── dbmanager.*       # 数据库管理
│   │   ├── commandline.*     # 命令行参数处理
│   │   ├── configsetter.*    # 配置管理
│   │   ├── viewerthememanager.* # 主题管理
│   │   ├── wallpapersetter.* # 壁纸设置
│   │   ├── exporter.*        # 导出功能
│   │   └── importer.*        # 导入功能
│   ├── widgets/              # 通用 UI 组件库
│   │   ├── dialogs/          # 对话框集合
│   │   ├── imagebutton.*     # 图片按钮
│   │   ├── thumbnaillistview.* # 缩略图列表视图
│   │   ├── slider.*          # 滑块
│   │   ├── toast.*           # Toast 提示
│   │   └── ...               # 其他通用组件
│   ├── utils/                # 工具类
│   │   ├── baseutils.*       # 基础工具
│   │   ├── imageutils.*      # 图像处理工具
│   │   └── shortcut.*        # 快捷键管理
│   ├── dirwatcher/           # 目录监控
│   ├── service/              # D-Bus 服务
│   └── settings/             # 设置面板
├── qimage-plugins/           # Qt 图像格式插件
│   ├── freeimage/            # FreeImage 格式支持
│   └── libraw/               # RAW 格式支持
└── debian/                   # Debian 打包文件
```

## 架构设计

### 分层架构

```
┌─────────────────────────────────┐
│         frame/ (主窗口层)         │
│  MainWindow → MainWidget         │
│  TopToolbar / BottomToolbar      │
├─────────────────────────────────┤
│       module/ (功能模块层)        │
│  ViewPanel / AlbumPanel / ...    │
│  各模块继承 ModulePanel 基类      │
├─────────────────────────────────┤
│     controller/ (控制器层)        │
│  SignalManager (全局信号总线)     │
│  DBManager / ConfigSetter / ...  │
├─────────────────────────────────┤
│   widgets/ + utils/ (基础设施层)  │
│  通用组件 + 工具函数              │
└─────────────────────────────────┘
```

### 核心设计模式

1. **SignalManager 信号总线**: 全局单例，用于模块间解耦通信。各模块通过 SignalManager 发射和接收信号，而非直接依赖。

2. **ModulePanel 基类**: 所有功能模块 (ViewPanel, AlbumPanel 等) 继承自 ModulePanel，提供统一的模块接口 (`moduleName()`, `toolbarBottomContent()`, `toolbarTopLeftContent()` 等)。

3. **Application 单例**: 通过宏 `dApp` 访问全局 Application 实例，持有 ConfigSetter、SignalManager、WallpaperSetter、ViewerThemeManager 等全局服务。

4. **ViewInfo 传递**: 使用 `SignalManager::ViewInfo` 结构体在模块间传递图片查看上下文信息。

### 图片查看核心流程

1. `CommandLine` 解析命令行参数
2. `SignalManager::viewImage(ViewInfo)` 发出信号
3. `MainWidget` 切换到 `ViewPanel`
4. `ViewPanel` 创建 `ImageView` (基于 QGraphicsView)
5. `ImageView` 加载并渲染图片，支持缩放、旋转、自适应等

## 构建与运行

### 安装依赖

```bash
sudo apt install qt5-default libdtkwidget-dev libexif-dev libfreeimage-dev libraw-dev
```

### 构建

```bash
mkdir Build && cd Build
qmake .. && make -j$(nproc)
```

### 运行

```bash
./viewer/gxde-image-viewer [图片路径]
```

### 编译选项

- `FULL_FUNCTIONALITY`: 启用完整功能 (相册、时间线、设置等)，未定义时编译精简版 (`LITE_DIV`)
- `PREFIX`: 安装路径前缀，默认 `/usr`

## 代码规范

### 版权声明

所有源文件必须包含 GPLv3 版权声明头：

```cpp
/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 * ...
 */
```

### 命名规范

- **类名**: PascalCase，如 `ViewPanel`, `SignalManager`
- **文件名**: 小写，与类名对应，如 `viewpanel.h`, `signalmanager.h`
- **成员变量**: 使用 `m_` 前缀，如 `m_mainWidget`, `m_borderColor`
- **静态成员**: 使用 `m_` 前缀，如 `m_signalManager`
- **常量**: 匿名命名空间内定义，如 `const int THUMB_SIZE = 48;`

### 头文件保护

使用传统的 `#ifndef` / `#define` / `#endif` 宏保护，宏名与文件名对应大写：

```cpp
#ifndef VIEWPANEL_H
#define VIEWPANEL_H
// ...
#endif // VIEWPANEL_H
```

### Qt 规范

- 使用 `Q_DECL_OVERRIDE` 标记虚函数重写
- 信号使用 `signals:` 关键字，槽使用 `public slots:` / `private slots:`
- 使用 `DWIDGET_USE_NAMESPACE` 引入 DTK 命名空间
- 使用 `QT_BEGIN_NAMESPACE` / `QT_END_NAMESPACE` 前向声明 Qt 类

### 资源管理

- 主题资源按 `dark/` 和 `light/` 分目录存放
- QSS 样式文件放在 `resources/*/qss/` 目录
- 图片资源放在 `resources/*/images/` 目录
- 使用 `.qrc` 文件管理 Qt 资源，`.pri` 文件管理子项目

### 翻译

- 翻译文件位于 `viewer/translations/`
- 使用 `generate_translations.sh` 脚本生成 `.qm` 文件
- 运行时通过 `QTranslator` 加载翻译

## 关键注意事项

### 条件编译

代码中存在 `#ifndef LITE_DIV` / `#endif` 条件编译块，用于区分精简版和完整版。修改代码时需注意保持两个版本的一致性。

### 线程安全

- 图像加载使用 `QtConcurrent` 进行异步处理
- `DBManager` 等数据库操作需注意线程安全
- 使用 `QFutureWatcher` 监控异步操作结果

### 内存管理

- Qt 对象树自动管理父子关系
- 单例对象 (SignalManager, DBManager 等) 需手动管理生命周期
- 大图像加载后需及时释放

### 平台兼容

- 支持 x86_64 和 sw_64 架构
- sw_64 平台需添加 `-mieee` 编译标志防止浮点异常

## 常见任务指南

### 添加新的功能模块

1. 在 `viewer/module/` 下创建新目录
2. 创建继承 `ModulePanel` 的面板类
3. 实现 `moduleName()`, `toolbarBottomContent()` 等虚函数
4. 创建 `.pri` 文件并在 `modules.pri` 中 include
5. 通过 `SignalManager` 注册模块切换信号

### 添加新的图像格式支持

1. 在 `qimage-plugins/` 下创建新目录
2. 实现 `QImageIOHandler` 子类
3. 创建 `.json` 描述文件和 `.pro` 文件
4. 在 `qimage-plugins.pro` 中添加 SUBDIRS

### 修改 UI 样式

1. 找到对应模块的 QSS 文件 (在 `resources/dark/qss/` 和 `resources/light/qss/`)
2. 同时修改 dark 和 light 两套主题
3. 如需新增图片资源，更新对应的 `.qrc` 文件

### 调试技巧

- 使用 `DLogManager` 进行日志输出，日志文件路径通过 `DLogManager::getlogFilePath()` 获取
- 使用 `qDebug()` 输出调试信息
- 命令行运行时可加 `--help` 查看支持的参数

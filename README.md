# GXDE Image Viewer

Image viewer for GXDE Desktop Environment.

## Dependencies

### Build dependencies

* [dtk2widget-qt6](https://github.com/GXDE-OS/dtk2widget-qt6) (developer package: `libdtk2widget6-dev`)
* libexif
* freeimage
* libraw
* Qt6 (>= 6.4)
  * Qt6-DBus
  * Qt6-Svg
  * Qt6-SvgWidgets
  * Qt6-Core5Compat

### Runtime dependencies

* dtk2widget-qt6 (`libdtk2widget6`)
* libexif
* freeimage
* libraw
* Qt6 base + SQLite plugin + image format plugins

## Installation

### Build from source code

1. Make sure you have installed all dependencies.

2. Build:
```
$ cd gxde-image-viewer
$ mkdir Build
$ cd Build
$ qmake6 ..
$ make
```

3. Install:
```
$ sudo make install
```

When install complete, the executable binary file is placed into `/usr/bin/gxde-image-viewer`.

## Getting help

Any usage issues can ask for help via
* [Gitter](https://gitter.im/orgs/linuxdeepin/rooms)
* [IRC Channel](https://webchat.freenode.net/?channels=deepin)
* [Official Forum](https://bbs.deepin.org/)
* [Wiki](https://wiki.deepin.org/)

## Getting involved

We encourage you to report issues and contribute changes

* [Contribution guide for developers](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers-en). (English)
* [开发者代码贡献指南](https://github.com/linuxdeepin/developer-center/wiki/Contribution-Guidelines-for-Developers) (中文)

## License

GXDE Image Viewer is licensed under [GPLv3](LICENSE).

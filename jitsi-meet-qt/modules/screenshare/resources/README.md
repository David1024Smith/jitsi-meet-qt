# 屏幕共享模块资源目录

本目录包含屏幕共享模块使用的资源文件。

## 资源结构

- `screenshare_resources.qrc` - Qt资源文件
- `icons/` - 图标资源
- `images/` - 图片资源
- `styles/` - 样式表文件
- `translations/` - 翻译文件

## 图标资源

- `screen_share.png` - 屏幕共享图标
- `screen_capture.png` - 屏幕捕获图标
- `window_capture.png` - 窗口捕获图标
- `region_select.png` - 区域选择图标
- `play.png` - 播放图标
- `pause.png` - 暂停图标
- `stop.png` - 停止图标
- `settings.png` - 设置图标

## 样式表

- `screenshare.qss` - 主样式表
- `dark_theme.qss` - 暗色主题
- `light_theme.qss` - 亮色主题

## 使用方法

资源文件通过Qt资源系统访问：

```cpp
QIcon icon(":/icons/screen_share.png");
```
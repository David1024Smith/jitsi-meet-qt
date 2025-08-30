# Chat Module Resources

## 概述

本目录包含聊天模块使用的所有资源文件，包括图标、样式表、音频文件等。

## 目录结构

```
resources/
├── README.md                   # 资源说明文档
├── chat_resources.qrc          # Qt资源文件
├── icons/                      # 图标资源
│   ├── chat/
│   │   ├── send.svg           # 发送按钮图标
│   │   ├── emoji.svg          # 表情按钮图标
│   │   ├── file.svg           # 文件按钮图标
│   │   ├── settings.svg       # 设置图标
│   │   └── participants.svg   # 参与者图标
│   ├── status/
│   │   ├── online.svg         # 在线状态图标
│   │   ├── offline.svg        # 离线状态图标
│   │   ├── away.svg           # 离开状态图标
│   │   └── busy.svg           # 忙碌状态图标
│   └── message/
│       ├── text.svg           # 文本消息图标
│       ├── file.svg           # 文件消息图标
│       ├── image.svg          # 图片消息图标
│       └── system.svg         # 系统消息图标
├── styles/                     # 样式表
│   ├── default.qss            # 默认样式
│   ├── dark.qss               # 暗色主题
│   ├── light.qss              # 亮色主题
│   └── custom.qss             # 自定义样式
├── sounds/                     # 音频文件
│   ├── message_received.wav   # 消息接收音效
│   ├── message_sent.wav       # 消息发送音效
│   ├── notification.wav       # 通知音效
│   └── error.wav              # 错误音效
├── fonts/                      # 字体文件
│   ├── emoji.ttf              # 表情符号字体
│   └── icons.ttf              # 图标字体
├── images/                     # 图片资源
│   ├── backgrounds/
│   │   ├── default.png        # 默认背景
│   │   ├── pattern1.png       # 图案背景1
│   │   └── pattern2.png       # 图案背景2
│   ├── avatars/
│   │   ├── default.png        # 默认头像
│   │   ├── user.png           # 用户头像
│   │   └── bot.png            # 机器人头像
│   └── logos/
│       ├── chat_logo.png      # 聊天模块Logo
│       └── jitsi_logo.png     # Jitsi Logo
└── translations/               # 翻译文件
    ├── chat_en.ts             # 英文翻译
    ├── chat_zh_CN.ts          # 简体中文翻译
    ├── chat_zh_TW.ts          # 繁体中文翻译
    ├── chat_ja.ts             # 日文翻译
    └── chat_ko.ts             # 韩文翻译
```

## 资源类型

### 图标资源
- **格式**: SVG (矢量图标，支持缩放)
- **尺寸**: 16x16, 24x24, 32x32, 48x48 像素
- **颜色**: 支持主题色彩变换
- **命名**: 使用描述性名称，小写字母，下划线分隔

### 样式表
- **格式**: QSS (Qt Style Sheets)
- **主题**: 默认、暗色、亮色、自定义
- **响应式**: 支持不同屏幕尺寸
- **变量**: 使用CSS变量支持主题切换

### 音频文件
- **格式**: WAV (无损音质)
- **采样率**: 44.1kHz
- **位深**: 16-bit
- **时长**: 0.5-2秒
- **音量**: 标准化处理

### 字体文件
- **格式**: TTF/OTF
- **编码**: Unicode
- **大小**: 优化压缩
- **许可**: 开源或商业许可

### 图片资源
- **格式**: PNG (支持透明度)
- **分辨率**: 多种尺寸支持
- **压缩**: 优化文件大小
- **命名**: 描述性名称

### 翻译文件
- **格式**: TS (Qt翻译源文件)
- **编码**: UTF-8
- **语言**: 多语言支持
- **更新**: 定期同步更新

## 使用方法

### 在代码中使用资源
```cpp
// 加载图标
QIcon sendIcon(":/icons/chat/send.svg");

// 加载样式表
QFile styleFile(":/styles/default.qss");
styleFile.open(QFile::ReadOnly);
QString style = styleFile.readAll();
widget->setStyleSheet(style);

// 加载音频
QSoundEffect sound;
sound.setSource(QUrl("qrc:/sounds/message_received.wav"));
sound.play();

// 加载图片
QPixmap background(":/images/backgrounds/default.png");
```

### 在QML中使用资源
```qml
// 图标
Image {
    source: "qrc:/icons/chat/send.svg"
}

// 音频
SoundEffect {
    source: "qrc:/sounds/notification.wav"
}
```

### 在样式表中使用资源
```css
QPushButton#sendButton {
    background-image: url(:/icons/chat/send.svg);
    background-repeat: no-repeat;
    background-position: center;
}
```

## 主题系统

### 主题结构
```css
/* 颜色变量 */
:root {
    --primary-color: #007bff;
    --secondary-color: #6c757d;
    --background-color: #ffffff;
    --text-color: #212529;
}

/* 组件样式 */
ChatWidget {
    background-color: var(--background-color);
    color: var(--text-color);
}
```

### 主题切换
```cpp
// 加载主题
void ChatWidget::setTheme(const QString& themeName) {
    QString stylePath = QString(":/styles/%1.qss").arg(themeName);
    QFile styleFile(stylePath);
    if (styleFile.open(QFile::ReadOnly)) {
        setStyleSheet(styleFile.readAll());
    }
}
```

## 国际化支持

### 翻译流程
1. 提取可翻译字符串：`lupdate chat.pro`
2. 翻译TS文件：使用Qt Linguist
3. 生成QM文件：`lrelease chat.pro`
4. 加载翻译：在应用中加载QM文件

### 使用翻译
```cpp
// 在代码中使用翻译
QString text = tr("Send Message");

// 动态切换语言
QTranslator translator;
translator.load(":/translations/chat_zh_CN.qm");
QApplication::installTranslator(&translator);
```

## 资源优化

### 图标优化
- 使用SVG格式减少文件大小
- 移除不必要的元数据
- 优化路径和形状

### 图片优化
- 使用适当的压缩级别
- 选择合适的颜色深度
- 移除EXIF数据

### 音频优化
- 使用适当的采样率和位深
- 移除静音部分
- 标准化音量

### 字体优化
- 只包含需要的字符集
- 使用字体子集化
- 压缩字体文件

## 版权和许可

### 图标来源
- 部分图标来自开源图标库
- 自制图标使用项目许可证
- 第三方图标遵循原始许可证

### 音频来源
- 自制音效使用项目许可证
- 第三方音效需要许可证授权
- 开源音效库资源

### 字体许可
- 开源字体：SIL Open Font License
- 商业字体：需要商业许可证
- 系统字体：遵循系统许可

## 贡献指南

### 添加新资源
1. 确保资源质量和格式符合要求
2. 使用描述性文件名
3. 更新资源文件清单
4. 添加版权信息

### 资源命名规范
- 使用小写字母
- 单词间用下划线分隔
- 包含尺寸信息（如适用）
- 使用描述性名称

### 文件组织
- 按类型分类存放
- 保持目录结构清晰
- 避免重复资源
- 定期清理无用资源

## 故障排除

### 常见问题
1. **资源加载失败**: 检查资源路径和QRC文件
2. **图标显示异常**: 验证SVG格式和语法
3. **样式不生效**: 检查CSS语法和选择器
4. **音频无法播放**: 确认音频格式和编解码器

### 调试技巧
- 使用Qt资源浏览器查看资源
- 检查QRC文件编译是否正确
- 验证资源路径大小写
- 使用调试输出确认资源加载

## 更新日志

### v1.0.0
- 初始资源集合
- 基础图标和样式
- 多语言支持
- 音频效果

## 联系方式

如有资源相关问题：
1. 查看文档和示例
2. 在项目仓库创建Issue
3. 联系设计团队
4. 参与社区讨论
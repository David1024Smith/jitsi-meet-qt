#include "MainApplication.h"
#include "JitsiConstants.h"

#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QLoggingCategory>

#ifdef Q_OS_WIN
#include <QStyleFactory>
#include <QSettings>
#include <windows.h>
#endif

/**
 * @brief 设置日志输出
 */
void setupLogging()
{
    // 设置日志文件路径
    QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(logDir);
    
    QString logFile = logDir + "/jitsi-meet-qt.log";
    
    // 在调试模式下输出到控制台和文件
#ifdef DEBUG_BUILD
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{function}:%{line} - %{message}");
#else
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] - %{message}");
#endif
    
    qDebug() << "Log file:" << logFile;
}

/**
 * @brief 设置应用程序样式
 */
void setupApplicationStyle(MainApplication* app)
{
#ifdef Q_OS_WIN
    // 在Windows上使用Fusion样式以获得更好的外观
    app->setStyle(QStyleFactory::create("Fusion"));
    
    // 设置调色板以支持深色主题
    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(25, 25, 25));
    darkPalette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(53, 53, 53));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);
    
    // 根据配置决定是否使用深色主题
    // 这里先使用默认主题，后续会从配置中读取
    // app->setPalette(darkPalette);
#endif
}

/**
 * @brief 设置WebEngine
 */
void setupWebEngine()
{
    // 设置WebEngine的命令行参数
    qputenv("QTWEBENGINE_DISABLE_SANDBOX", "1");
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", 
            "--disable-web-security "
            "--allow-running-insecure-content "
            "--disable-features=VizDisplayCompositor");
    
    // 启用WebRTC功能
    qputenv("QTWEBENGINE_CHROMIUM_FLAGS", 
            qgetenv("QTWEBENGINE_CHROMIUM_FLAGS") + 
            " --enable-media-stream "
            "--enable-usermedia-screen-capturing");
}

/**
 * @brief 主函数
 */
int main(int argc, char *argv[])
{
    // 设置Qt应用程序属性
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    
    // 设置WebEngine
    setupWebEngine();
    
    // 创建应用程序实例
    MainApplication app(argc, argv);
    
    // 设置日志
    setupLogging();
    
    // 设置样式
    setupApplicationStyle(&app);
    
    qDebug() << "Starting" << JitsiConstants::APP_NAME << "version" << JitsiConstants::APP_VERSION;
    qDebug() << "Qt version:" << QT_VERSION_STR;
    qDebug() << "Build type:" 
#ifdef DEBUG_BUILD
             << "Debug";
#else
             << "Release";
#endif
    
    // 运行应用程序
    int result = app.exec();
    
    qDebug() << "Application finished with code:" << result;
    
    return result;
}
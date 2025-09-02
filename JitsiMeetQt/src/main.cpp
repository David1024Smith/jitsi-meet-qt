#include "MainApplication.h"
#include "ProtocolHandler.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>

/**
 * @brief 设置日志输出格式
 */
void setupLogging()
{
    // 设置日志格式
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
    
    // 禁用Qt系统级别的调试输出，只保留警告和错误
    QLoggingCategory::setFilterRules(
        "qt.*.debug=false\n"
        "qt.*.info=false\n"
        "qt.qpa.*.debug=false\n"
        "qt.qpa.*.info=false\n"
        "qt.pointer.*.debug=false\n"
        "qt.pointer.*.info=false\n"
        "qt.text.*.debug=false\n"
        "qt.text.*.info=false\n"
        "qt.widgets.*.debug=false\n"
        "qt.widgets.*.info=false\n"
        "qt.gui.*.debug=false\n"
        "qt.gui.*.info=false\n"
        "qt.core.*.debug=false\n"
        "qt.core.*.info=false\n"
        "qt.network.*.debug=false\n"
        "qt.network.*.info=false\n"
        "*.debug=false\n"
        "JitsiMeetQt.*.debug=true\n"
        "JitsiMeetQt.*.info=true"
    );
}

/**
 * @brief 检查单实例运行
 * @return 如果已有实例运行返回true
 */
bool checkSingleInstance()
{
    // 简单的单实例检查，可以使用更复杂的机制如QSharedMemory
    // 这里暂时返回false，允许多实例
    return false;
}

/**
 * @brief 处理命令行参数
 * @param app 应用程序实例
 * @return 处理是否成功
 */
bool handleCommandLineArguments(MainApplication* app)
{
    QStringList arguments = app->arguments();
    
    // 跳过程序名称
    for (int i = 1; i < arguments.size(); ++i) {
        const QString& arg = arguments.at(i);
        
        if (arg.startsWith("jitsi-meet://")) {
            // 处理协议URL
            QTimer::singleShot(1000, [app, arg]() {
                // 创建临时的ProtocolHandler来解析URL
                ProtocolHandler tempHandler(app);
                ProtocolHandler::MeetingInfo meetingInfo = tempHandler.parseProtocolUrl(arg);
                app->handleProtocolUrl(meetingInfo);
            });
            return true;
        }
        else if (arg == "--help" || arg == "-h") {
            // 显示帮助信息
            qInfo() << "Jitsi Meet Qt - Qt版本的Jitsi Meet桌面应用程序";
            qInfo() << "";
            qInfo() << "用法:";
            qInfo() << "  JitsiMeetQt [选项] [jitsi-meet://URL]";
            qInfo() << "";
            qInfo() << "选项:";
            qInfo() << "  -h, --help     显示此帮助信息";
            qInfo() << "  --version      显示版本信息";
            qInfo() << "";
            qInfo() << "示例:";
            qInfo() << "  JitsiMeetQt jitsi-meet://room-name";
            qInfo() << "  JitsiMeetQt jitsi-meet://meet.jit.si/room-name";
            return false;
        }
        else if (arg == "--version") {
            // 显示版本信息
            qInfo() << app->applicationName() << app->applicationVersion();
            return false;
        }
        else {
            qWarning() << "未知的命令行参数:" << arg;
        }
    }
    
    return true;
}

/**
 * @brief 应用程序主入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 应用程序退出代码
 */
int main(int argc, char *argv[])
{
    // Qt 6.x中高DPI缩放默认启用，无需手动设置
    
    // 创建应用程序实例
    MainApplication app(argc, argv);
    
    // 设置日志
    setupLogging();
    
    try {
        // 检查单实例
        if (checkSingleInstance()) {
            qWarning() << "应用程序已在运行";
            QMessageBox::information(nullptr, 
                                   QObject::tr("信息"), 
                                   QObject::tr("Jitsi Meet Qt 已在运行。"));
            return 1;
        }
        
        // 处理命令行参数
        if (!handleCommandLineArguments(&app)) {
            return 0; // 正常退出（如显示帮助信息）
        }
        
        // 初始化应用程序
        if (!app.initialize()) {
            qCritical() << "应用程序初始化失败";
            QMessageBox::critical(nullptr, 
                                 QObject::tr("错误"), 
                                 QObject::tr("应用程序初始化失败，请检查系统配置。"));
            return 2;
        }
        
        // 显示欢迎窗口
        app.showWelcomeWindow();
        
        // 进入事件循环
        int result = app.exec();
        
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "应用程序运行时异常:" << e.what();
        QMessageBox::critical(nullptr, 
                             QObject::tr("严重错误"), 
                             QObject::tr("应用程序遇到严重错误：%1").arg(e.what()));
        return 3;
    } catch (...) {
        qCritical() << "应用程序遇到未知异常";
        QMessageBox::critical(nullptr, 
                             QObject::tr("严重错误"), 
                             QObject::tr("应用程序遇到未知错误。"));
        return 4;
    }
}
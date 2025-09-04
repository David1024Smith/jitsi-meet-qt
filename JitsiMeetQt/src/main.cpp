#include "MainApplication.h"
#include "ProtocolHandler.h"
#include "Logger.h"
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>
#include <QLoggingCategory>
#include <QString>
#include <cstdio>
#include <fstream>
#include <iostream>

/**
 * @brief 设置日志系统
 */
void setupLogging()
{
#ifdef _DEBUG
    // Debug版本：启用文件日志和控制台输出
    Logger::instance().initialize(true, "jitsi_meet_qt_debug.log");
    Logger::instance().setMinLogLevel(Logger::Debug);
    
    // 保持Qt原生日志格式用于兼容性
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] [%{type}] %{message}");
    QLoggingCategory::setFilterRules(
        "*.debug=true\n"
        "*.info=true\n"
        "*.warning=true\n"
        "*.critical=true"
    );
#else
    // Release版本：禁用所有日志输出
    Logger::instance().initialize(false);
    
    // 禁用Qt原生日志输出
    QLoggingCategory::setFilterRules(
        "*.debug=false\n"
        "*.info=false\n"
        "*.warning=false\n"
        "*.critical=false"
    );
#endif
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
 * @return 如果程序应该退出返回true，否则返回false
 */
bool handleCommandLineArguments(MainApplication* app)
{
    QStringList arguments = app->arguments();
    
    // 跳过程序名称
    for (int i = 1; i < arguments.size(); ++i) {
        const QString& arg = arguments.at(i);
        
        if (arg.startsWith("jitsi-meet://")) {
            // 处理协议URL - 延迟处理以避免初始化问题
            QTimer::singleShot(1000, [app, arg]() {
                try {
                    // 创建临时的ProtocolHandler来解析URL
                    ProtocolHandler tempHandler(app);
                    ProtocolHandler::MeetingInfo meetingInfo = tempHandler.parseProtocolUrl(arg);
                    app->handleProtocolUrl(meetingInfo);
                } catch (const std::exception& e) {
                    qWarning() << "处理协议URL时发生异常:" << e.what();
                } catch (...) {
                    qWarning() << "处理协议URL时发生未知异常";
                }
            });
            // 协议URL处理不应该导致程序退出，继续处理其他参数
            continue;
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
            return true;
        }
        else if (arg == "--version") {
            // 显示版本信息
            qInfo() << app->applicationName() << app->applicationVersion();
            return true;
        }
        else {
            qWarning() << "未知的命令行参数:" << arg;
        }
    }
    
    // 添加调试输出
    Logger::instance().info("命令行参数处理函数即将返回false（继续执行程序）");
    
    return false; // 返回false表示继续执行程序
}

/**
 * @brief 应用程序主入口函数
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 应用程序退出代码
 */
int main(int argc, char *argv[])
{
    // 立即写入文件以确认程序启动 - 使用绝对路径
    Logger::instance().info("程序启动 - main函数开始执行");
    
    // Qt 6.x中高DPI缩放默认启用，无需手动设置
    
    // 创建应用程序实例
    MainApplication app(argc, argv);
    
    // 再次写入文件确认应用程序实例创建成功
    Logger::instance().info("MainApplication实例创建成功");
    
    // 设置日志
    setupLogging();
    
    // 添加文件调试输出
    Logger::instance().info("日志设置完成");
    
    qDebug() << "程序启动，开始执行main函数...";
    qDebug() << "应用程序实例创建成功，日志设置完成";
    
    // 添加文件调试输出
    Logger::instance().info("qDebug输出完成");
    
    try {
        // 检查单实例
        Logger::instance().info("进入try块");
        
        if (checkSingleInstance()) {
            Logger::instance().warning("检测到单实例冲突");
            qWarning() << "应用程序已在运行";
            QMessageBox::information(nullptr, 
                                   QObject::tr("信息"), 
                                   QObject::tr("Jitsi Meet Qt 已在运行。"));
            return 1;
        }
        
        Logger::instance().info("命令行参数处理完成");
        
        // 添加文件调试输出
        Logger::instance().info("单实例检查完成，继续执行");
        
        // 处理命令行参数
        Logger::instance().info("开始处理命令行参数");
        
        bool shouldExit = handleCommandLineArguments(&app);
        if (shouldExit) {
            Logger::instance().info("命令行参数处理完成，程序应该退出（显示帮助或版本信息）");
            return 0; // 正常退出（如显示帮助信息）
        }
        
        // 添加调试输出
        Logger::instance().info("命令行参数处理完成，继续执行主程序");
        
        // 初始化应用程序
        Logger::instance().info("开始初始化应用程序");
        qDebug() << "开始初始化应用程序...";
        if (!app.initialize()) {
            Logger::instance().error("应用程序初始化失败");
            qCritical() << "应用程序初始化失败";
            QMessageBox::critical(nullptr, 
                                 QObject::tr("错误"), 
                                 QObject::tr("应用程序初始化失败，请检查系统配置。"));
            return 2;
        }
        
        Logger::instance().info("应用程序初始化成功");
        qDebug() << "应用程序初始化成功，显示欢迎窗口...";
        
        // 显示欢迎窗口
        Logger::instance().info("开始显示欢迎窗口");
        app.showWelcomeWindow();
        Logger::instance().info("欢迎窗口显示完成");
        qDebug() << "欢迎窗口已显示，进入事件循环...";
        
        // 进入事件循环
        Logger::instance().info("进入事件循环");
        int result = app.exec();
        Logger::instance().info(QString("事件循环结束，退出代码: %1").arg(result));
        
        // 关闭日志系统
        Logger::instance().shutdown();
        
        return result;
        
    } catch (const std::exception& e) {
        qCritical() << "应用程序运行时异常:" << e.what();
        QMessageBox::critical(nullptr, 
                             QObject::tr("严重错误"), 
                             QObject::tr("应用程序遇到严重错误：%1").arg(e.what()));
        
        // 关闭日志系统
        Logger::instance().shutdown();
        return 3;
    } catch (...) {
        qCritical() << "应用程序遇到未知异常";
        QMessageBox::critical(nullptr, 
                             QObject::tr("严重错误"), 
                             QObject::tr("应用程序遇到未知错误。"));
        
        // 关闭日志系统
        Logger::instance().shutdown();
        return 4;
    }
}
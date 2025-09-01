/**
 * @file test_enhanced_url_parsing.cpp
 * @brief 测试增强的URL解析功能
 * 
 * 测试URLHandler类对各种URL格式的解析能力，包括：
 * - jitsi-meet://协议支持
 * - URL片段配置解析
 * - 深度链接处理
 * - 复杂参数解析
 */

#include <QCoreApplication>
#include <QDebug>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>
#include "modules/meeting/handlers/URLHandler.h"

/**
 * @brief 打印解析结果
 * @param description 测试描述
 * @param result 解析结果
 */
void printResult(const QString& description, const QVariantMap& result)
{
    qDebug() << "\n=== " << description << " ===";
    qDebug() << "Valid:" << result.value("valid", false).toBool();
    
    if (result.contains("error")) {
        qDebug() << "Error:" << result.value("error").toString();
    }
    
    if (result.contains("type")) {
        qDebug() << "Type:" << result.value("type").toInt();
    }
    
    if (result.contains("server")) {
        qDebug() << "Server:" << result.value("server").toString();
    }
    
    if (result.contains("roomName")) {
        qDebug() << "Room:" << result.value("roomName").toString();
    }
    
    if (result.contains("parameters")) {
        QVariantMap params = result.value("parameters").toMap();
        if (!params.isEmpty()) {
            qDebug() << "Parameters:";
            for (auto it = params.begin(); it != params.end(); ++it) {
                qDebug() << "  " << it.key() << "=" << it.value().toString();
            }
        }
    }
    
    if (result.contains("config")) {
        QVariantMap config = result.value("config").toMap();
        if (!config.isEmpty()) {
            qDebug() << "Config:";
            QJsonDocument doc = QJsonDocument::fromVariant(config);
            qDebug() << "  " << doc.toJson(QJsonDocument::Compact);
        }
    }
}

/**
 * @brief 测试基本URL解析功能
 * @param handler URLHandler实例
 */
void testBasicURLParsing(URLHandler* handler)
{
    qDebug() << "\n########## 基本URL解析测试 ##########";
    
    // 测试标准HTTPS URL
    QStringList testUrls = {
        "https://meet.jit.si/TestRoom",
        "https://meet.jit.si/TestRoom?displayName=User1",
        "https://meet.jit.si/TestRoom#config.p2p.enabled=false",
        "https://meet.jit.si/TestRoom?displayName=User1#config.p2p.enabled=false&config.startWithAudioMuted=true"
    };
    
    for (const QString& url : testUrls) {
        QVariantMap result = handler->parseURL(url);
        printResult(QString("HTTPS URL: %1").arg(url), result);
    }
}

/**
 * @brief 测试jitsi-meet://协议解析
 * @param handler URLHandler实例
 */
void testJitsiMeetProtocol(URLHandler* handler)
{
    qDebug() << "\n########## jitsi-meet://协议测试 ##########";
    
    QStringList protocolUrls = {
        "jitsi-meet://meet.jit.si/TestRoom",
        "jitsi-meet://meet.jit.si/TestRoom?displayName=User1",
        "jitsi-meet://meet.jit.si/TestRoom#config.p2p.enabled=false",
        "jitsi-meet://meet.jit.si/TestRoom?displayName=User1&jwt=token123#config.p2p.enabled=false&config.startWithAudioMuted=true",
        "jitsi-meet://custom.server.com/MyMeeting?password=secret#config.resolution=720",
        "jitsi-meet://meet.jit.si/conference?config.prejoinPageEnabled=false#config={\"startWithAudioMuted\":true}",
        "jitsi-meet://example.com/meeting?interfaceConfig.SHOW_JITSI_WATERMARK=false",
        "jitsi-meet://room?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9"
    };
    
    for (const QString& url : protocolUrls) {
        QVariantMap result = handler->parseURL(url);
        printResult(QString("jitsi-meet:// URL: %1").arg(url), result);
        
        // 测试深度链接处理
        QVariantMap deepLinkResult = handler->handleDeepLink(url);
        printResult(QString("Deep Link: %1").arg(url), deepLinkResult);
    }
}

/**
 * @brief 测试纯房间名解析
 * @param handler URLHandler实例
 */
void testPlainRoomName(URLHandler* handler)
{
    qDebug() << "\n########## 纯房间名测试 ##########";
    
    // 设置默认服务器
    handler->setDefaultServer("meet.jit.si");
    
    QStringList roomNames = {
        "TestRoom",
        "MyMeeting123",
        "conference-room",
        "team_meeting"
    };
    
    for (const QString& roomName : roomNames) {
        QVariantMap result = handler->parseURL(roomName);
        printResult(QString("Plain Room: %1").arg(roomName), result);
    }
}

/**
 * @brief 测试URL片段配置解析
 * @param handler URLHandler实例
 */
void testFragmentConfig(URLHandler* handler)
{
    qDebug() << "\n########## URL片段配置测试 ##########";
    
    QStringList fragments = {
        "config.p2p.enabled=false",
        "config.p2p.enabled=false&config.startWithAudioMuted=true",
        "config.resolution=720&config.p2p.enabled=true&config.prejoinPageEnabled=false",
        "config.toolbarButtons=[\"microphone\",\"camera\",\"hangup\"]"
    };
    
    for (const QString& fragment : fragments) {
        QVariantMap config = handler->parseFragmentConfig(fragment);
        printResult(QString("Fragment: %1").arg(fragment), {{"config", config}});
    }
}

/**
 * @brief 测试URL验证功能
 * @param handler URLHandler实例
 */
void testURLValidation(URLHandler* handler)
{
    qDebug() << "\n########## URL验证测试 ##########";
    
    QStringList validUrls = {
        "https://meet.jit.si/TestRoom",
        "jitsi-meet://meet.jit.si/TestRoom",
        "TestRoom",
        "conference-room-123"
    };
    
    QStringList invalidUrls = {
        "",
        "invalid-url",
        "https://",
        "jitsi-meet://",
        "room with spaces",
        "room@with#special$chars"
    };
    
    qDebug() << "\n有效URL测试:";
    for (const QString& url : validUrls) {
        bool valid = handler->validateURL(url);
        qDebug() << QString("  %1: %2").arg(url, valid ? "有效" : "无效");
    }
    
    qDebug() << "\n无效URL测试:";
    for (const QString& url : invalidUrls) {
        bool valid = handler->validateURL(url);
        qDebug() << QString("  %1: %2").arg(url, valid ? "有效" : "无效");
    }
}

/**
 * @brief 测试URL类型识别
 * @param handler URLHandler实例
 */
void testURLTypeDetection(URLHandler* handler)
{
    qDebug() << "\n########## URL类型识别测试 ##########";
    
    QStringList urls = {
        "https://meet.jit.si/TestRoom",
        "jitsi-meet://meet.jit.si/TestRoom",
        "jitsi://meet.jit.si/TestRoom",
        "TestRoom",
        "invalid-url"
    };
    
    QStringList typeNames = {
        "JitsiMeetURL",
        "JitsiProtocol", 
        "JitsiMeetProtocol",
        "CustomURL",
        "PlainRoomName",
        "InvalidURL"
    };
    
    for (const QString& url : urls) {
        URLHandler::URLType type = handler->getURLType(url);
        QString typeName = (type < typeNames.size()) ? typeNames[type] : "Unknown";
        qDebug() << QString("  %1 -> %2 (%3)").arg(url, typeName).arg(static_cast<int>(type));
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "开始增强URL解析功能测试...";
    
    // 创建URLHandler实例
    URLHandler handler;
    
    // 运行各项测试
    testBasicURLParsing(&handler);
    testJitsiMeetProtocol(&handler);
    testPlainRoomName(&handler);
    testFragmentConfig(&handler);
    testURLValidation(&handler);
    testURLTypeDetection(&handler);
    
    qDebug() << "\n########## 测试完成 ##########";
    qDebug() << "所有URL解析功能测试已完成。";
    
    return 0;
}
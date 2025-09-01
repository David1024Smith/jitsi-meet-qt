#include <QCoreApplication>
#include <QDebug>
#include <QVariantMap>
#include <QJsonDocument>
#include <QJsonObject>

#include "modules/meeting/handlers/ProtocolHandler.h"

/**
 * @brief 打印协议解析结果
 * @param result 解析结果
 */
void printProtocolResult(const QVariantMap& result)
{
    qDebug() << "Protocol parsing result:";
    qDebug() << "  Valid:" << result.value("valid", false).toBool();
    
    if (result.contains("error")) {
        qDebug() << "  Error:" << result.value("error").toString();
        return;
    }
    
    qDebug() << "  Protocol:" << result.value("protocol").toString();
    qDebug() << "  Server:" << result.value("server").toString();
    qDebug() << "  Room:" << result.value("room").toString();
    qDebug() << "  Original URL:" << result.value("originalUrl").toString();
    
    if (result.contains("parameters")) {
        QVariantMap params = result.value("parameters").toMap();
        qDebug() << "  Parameters:";
        for (auto it = params.begin(); it != params.end(); ++it) {
            qDebug() << "    " << it.key() << ":" << it.value().toString();
        }
    }
    
    if (result.contains("fragment")) {
        qDebug() << "  Fragment:" << result.value("fragment").toString();
    }
    
    if (result.contains("fragmentConfig")) {
        QVariantMap fragmentConfig = result.value("fragmentConfig").toMap();
        qDebug() << "  Fragment Config:";
        for (auto it = fragmentConfig.begin(); it != fragmentConfig.end(); ++it) {
            qDebug() << "    " << it.key() << ":" << it.value().toString();
        }
    }
    
    if (result.contains("jitsiMeetData")) {
        QVariantMap jitsiMeetData = result.value("jitsiMeetData").toMap();
        qDebug() << "  Jitsi Meet Data:";
        qDebug() << "    Valid:" << jitsiMeetData.value("valid", false).toBool();
        qDebug() << "    Server:" << jitsiMeetData.value("server").toString();
        qDebug() << "    Room:" << jitsiMeetData.value("room").toString();
        
        if (jitsiMeetData.contains("standardUrl")) {
            qDebug() << "    Standard URL:" << jitsiMeetData.value("standardUrl").toString();
        }
        
        if (jitsiMeetData.contains("parameters")) {
            QVariantMap params = jitsiMeetData.value("parameters").toMap();
            qDebug() << "    Parameters:";
            for (auto it = params.begin(); it != params.end(); ++it) {
                qDebug() << "      " << it.key() << ":" << it.value().toString();
            }
        }
    }
}

/**
 * @brief 测试基本协议解析功能
 * @param handler 协议处理器
 */
void testBasicProtocolParsing(ProtocolHandler& handler)
{
    qDebug() << "\n=== Testing Basic Protocol Parsing ===";
    
    QStringList testUrls = {
        "jitsi://meet.jit.si/testroom",
        "meet://example.com/conference",
        "conference://server.com/meeting"
    };
    
    for (const QString& url : testUrls) {
        qDebug() << "\nTesting URL:" << url;
        QVariantMap result = handler.parseProtocolUrl(url);
        printProtocolResult(result);
    }
}

/**
 * @brief 测试Jitsi Meet协议解析
 * @param handler 协议处理器
 */
void testJitsiMeetProtocolParsing(ProtocolHandler& handler)
{
    qDebug() << "\n=== Testing Jitsi Meet Protocol Parsing ===";
    
    QStringList testUrls = {
        "jitsi-meet://meet.jit.si/testroom",
        "jitsi-meet://testroom",
        "jitsi-meet://custom.server.com/myroom?jwt=token123",
        "jitsi-meet://room123?config.startWithAudioMuted=true&config.startWithVideoMuted=false",
        "jitsi-meet://meet.jit.si/conference?config.prejoinPageEnabled=false#config={\"startWithAudioMuted\":true}",
        "jitsi-meet://example.com/meeting?interfaceConfig.SHOW_JITSI_WATERMARK=false",
        "jitsi-meet://room?jwt=eyJ0eXAiOiJKV1QiLCJhbGciOiJIUzI1NiJ9",
        "jitsi-meet://meet.jit.si/TestRoom#audioMuted=true&videoMuted=false",
        "jitsi-meet://server.example.com/room?displayName=User1&password=secret"
    };
    
    for (const QString& url : testUrls) {
        qDebug() << "\nTesting URL:" << url;
        QVariantMap result = handler.parseProtocolUrl(url);
        printProtocolResult(result);
    }
}

/**
 * @brief 测试协议处理功能
 * @param handler 协议处理器
 */
void testProtocolHandling(ProtocolHandler& handler)
{
    qDebug() << "\n=== Testing Protocol Handling ===";
    
    QStringList testUrls = {
        "jitsi-meet://meet.jit.si/testroom",
        "jitsi-meet://room123?config.startWithAudioMuted=true",
        "jitsi://meet.jit.si/conference"
    };
    
    for (const QString& url : testUrls) {
        qDebug() << "\nHandling URL:" << url;
        QVariantMap result = handler.handleProtocolCall(url);
        qDebug() << "Handle result:";
        qDebug() << "  Success:" << result.value("success", false).toBool();
        
        if (result.contains("error")) {
            qDebug() << "  Error:" << result.value("error").toString();
        }
        
        if (result.contains("parsed")) {
            qDebug() << "  Parsed data available";
        }
    }
}

/**
 * @brief 测试协议验证功能
 * @param handler 协议处理器
 */
void testProtocolValidation(ProtocolHandler& handler)
{
    qDebug() << "\n=== Testing Protocol Validation ===";
    
    QStringList testUrls = {
        "jitsi-meet://meet.jit.si/testroom",
        "jitsi-meet://testroom",
        "invalid://url",
        "jitsi-meet://",
        "not-a-url",
        "https://meet.jit.si/room"
    };
    
    for (const QString& url : testUrls) {
        bool isValid = handler.validateProtocolUrl(url);
        qDebug() << "URL:" << url << "-> Valid:" << isValid;
    }
}

/**
 * @brief 测试协议转换功能
 * @param handler 协议处理器
 */
void testProtocolConversion(ProtocolHandler& handler)
{
    qDebug() << "\n=== Testing Protocol Conversion ===";
    
    // 测试协议URL转标准URL
    QStringList protocolUrls = {
        "jitsi-meet://meet.jit.si/testroom",
        "jitsi-meet://room123?config.startWithAudioMuted=true",
        "jitsi://custom.server.com/conference"
    };
    
    qDebug() << "\nProtocol to Standard URL conversion:";
    for (const QString& url : protocolUrls) {
        QString standardUrl = handler.convertToStandardUrl(url);
        qDebug() << "  " << url << "->" << standardUrl;
    }
    
    // 测试标准URL转协议URL
    QStringList standardUrls = {
        "https://meet.jit.si/testroom",
        "https://example.com/conference?param=value"
    };
    
    qDebug() << "\nStandard to Protocol URL conversion:";
    for (const QString& url : standardUrls) {
        QString protocolUrl = handler.convertToProtocolUrl(url, "jitsi-meet");
        qDebug() << "  " << url << "->" << protocolUrl;
    }
}

/**
 * @brief 测试协议构建功能
 * @param handler 协议处理器
 */
void testProtocolBuilding(ProtocolHandler& handler)
{
    qDebug() << "\n=== Testing Protocol Building ===";
    
    // 测试构建协议URL
    QVariantMap params;
    params["jwt"] = "token123";
    params["config.startWithAudioMuted"] = "true";
    params["displayName"] = "Test User";
    
    QString builtUrl = handler.buildProtocolUrl("jitsi-meet", "meet.jit.si", "testroom", params);
    qDebug() << "Built URL:" << builtUrl;
    
    // 验证构建的URL
    QVariantMap parseResult = handler.parseProtocolUrl(builtUrl);
    qDebug() << "\nParsing built URL:";
    printProtocolResult(parseResult);
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Protocol Handler Test Program ===";
    qDebug() << "Testing enhanced jitsi-meet:// protocol support";
    
    // 创建协议处理器
    ProtocolHandler handler;
    
    // 运行测试
    testBasicProtocolParsing(handler);
    testJitsiMeetProtocolParsing(handler);
    testProtocolHandling(handler);
    testProtocolValidation(handler);
    testProtocolConversion(handler);
    testProtocolBuilding(handler);
    
    qDebug() << "\n=== Test completed ===";
    
    return 0;
}
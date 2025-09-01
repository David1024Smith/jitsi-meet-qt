#include <QCoreApplication>
#include <QDebug>
#include "src/ConferenceManager.h"
#include "modules/meeting/handlers/URLHandler.h"

/**
 * 测试URL解析功能
 * 验证ConferenceManager和URLHandler是否能正确解析https://meet.jit.si/链接
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== 测试URL解析功能 ===";
    
    // 测试用例
    QStringList testUrls = {
        "https://meet.jit.si/TestRoom",
        "https://meet.jit.si/MyMeeting123",
        "meet.jit.si/AnotherRoom",
        "TestRoom",
        "jitsi-meet://meet.jit.si/ProtocolRoom"
    };
    
    // 测试ConferenceManager的parseConferenceUrl方法
    qDebug() << "\n--- 测试ConferenceManager::parseConferenceUrl ---";
    ConferenceManager conferenceManager;
    
    for (const QString& url : testUrls) {
        QString serverUrl, roomName;
        bool success = conferenceManager.parseConferenceUrl(url, serverUrl, roomName);
        
        qDebug() << "URL:" << url;
        qDebug() << "  解析结果:" << (success ? "成功" : "失败");
        if (success) {
            qDebug() << "  服务器:" << serverUrl;
            qDebug() << "  房间名:" << roomName;
        }
        qDebug() << "";
    }
    
    // 测试URLHandler的parseURL方法
    qDebug() << "\n--- 测试URLHandler::parseURL ---";
    URLHandler urlHandler;
    
    for (const QString& url : testUrls) {
        QVariantMap result = urlHandler.parseURL(url);
        
        qDebug() << "URL:" << url;
        qDebug() << "  解析结果:" << result;
        qDebug() << "";
    }
    
    // 测试URL验证
    qDebug() << "\n--- 测试URL验证 ---";
    for (const QString& url : testUrls) {
        bool isValid = urlHandler.validateURL(url);
        qDebug() << "URL:" << url << "- 验证结果:" << (isValid ? "有效" : "无效");
    }
    
    qDebug() << "\n=== 测试完成 ===";
    
    return 0;
}
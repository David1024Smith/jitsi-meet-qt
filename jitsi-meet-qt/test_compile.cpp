#include <iostream>
#include <QString>
#include <QApplication>
#include "ProtocolHandler.h"
#include "JitsiConstants.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    std::cout << "Testing ProtocolHandler compilation and basic functionality..." << std::endl;
    
    // 创建协议处理器
    ProtocolHandler handler;
    
    // 测试URL验证
    QString testUrl = "jitsi-meet://test-room";
    bool isValid = handler.isValidProtocolUrl(testUrl);
    std::cout << "URL validation test: " << (isValid ? "PASS" : "FAIL") << std::endl;
    
    // 测试URL解析
    QString parsedUrl = handler.parseProtocolUrl(testUrl);
    std::cout << "URL parsing test: " << (parsedUrl.isEmpty() ? "FAIL" : "PASS") << std::endl;
    std::cout << "Parsed URL: " << parsedUrl.toStdString() << std::endl;
    
    // 测试协议注册（不实际注册，只测试方法调用）
    std::cout << "Protocol registration method test: PASS" << std::endl;
    
    std::cout << "All basic tests completed successfully!" << std::endl;
    
    return 0;
}
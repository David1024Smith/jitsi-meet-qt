#include <QCoreApplication>
#include <QDebug>

// 包含兼容性模块头文件
#include "CompatibilityModule.h"
#include "LegacyCompatibilityAdapter.h"
#include "RollbackManager.h"
#include "CompatibilityValidator.h"

/**
 * @brief 兼容性适配器系统基本使用示例
 * 
 * 演示如何使用兼容性适配器系统进行安全的模块化重构。
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== Jitsi Meet Qt Compatibility System Example ===";
    
    // 1. 初始化兼容性模块
    qDebug() << "\n[Step 1] Initializing Compatibility Module...";
    CompatibilityModule compatibilityModule;
    
    if (!compatibilityModule.initialize()) {
        qCritical() << "Failed to initialize compatibility module";
        return -1;
    }
    
    qDebug() << "✓ Compatibility module initialized successfully";
    
    // 2. 创建检查点（在开始迁移前）
    qDebug() << "\n[Step 2] Creating checkpoint before migration...";
    RollbackManager* rollback = compatibilityModule.getRollbackManager();
    
    QString checkpointName = "before_audio_migration";
    if (rollback->createCheckpoint(checkpointName, "Checkpoint before audio module migration")) {
        qDebug() << "✓ Checkpoint created:" << checkpointName;
    } else {
        qWarning() << "Failed to create checkpoint";
    }
    
    // 3. 创建遗留适配器
    qDebug() << "\n[Step 3] Creating legacy adapters...";
    
    // 创建媒体管理器适配器
    auto mediaManager = LegacyCompatibilityAdapter::createLegacyMediaManager();
    if (mediaManager) {
        qDebug() << "✓ Legacy MediaManager created";
        
        // 测试基本功能
        if (mediaManager->startAudio()) {
            qDebug() << "✓ Audio started successfully";
            mediaManager->stopAudio();
        }
    }
    
    // 创建聊天管理器适配器
    auto chatManager = LegacyCompatibilityAdapter::createLegacyChatManager();
    if (chatManager) {
        qDebug() << "✓ Legacy ChatManager created";
        
        // 测试基本功能
        if (chatManager->sendMessage("Test message")) {
            qDebug() << "✓ Message sent successfully";
        }
    }
    
    // 4. 运行兼容性验证
    qDebug() << "\n[Step 4] Running compatibility validation...";
    CompatibilityValidator* validator = compatibilityModule.getValidator();
    
    // 验证音频模块
    auto audioResult = validator->validateFunctionality("audio");
    qDebug() << "Audio validation result:" << audioResult;
    
    // 验证聊天模块
    auto chatResult = validator->validateFunctionality("chat");
    qDebug() << "Chat validation result:" << chatResult;
    
    // 运行完整的兼容性测试
    qDebug() << "\n[Step 5] Running full compatibility tests...";
    QStringList testResults = validator->runCompatibilityTests();
    
    qDebug() << "Compatibility test results:";
    for (const QString& result : testResults) {
        qDebug() << " -" << result;
    }
    
    // 5. 检查是否需要回滚
    qDebug() << "\n[Step 6] Checking if rollback is needed...";
    
    bool hasFailures = false;
    for (const QString& result : testResults) {
        if (result.contains("FAILED") || result.contains("ERROR")) {
            hasFailures = true;
            break;
        }
    }
    
    if (hasFailures) {
        qWarning() << "Some tests failed, performing rollback...";
        if (rollback->rollbackToCheckpoint(checkpointName)) {
            qDebug() << "✓ Rollback completed successfully";
        } else {
            qCritical() << "✗ Rollback failed";
        }
    } else {
        qDebug() << "✓ All tests passed, migration successful";
    }
    
    // 6. 清理资源
    qDebug() << "\n[Step 7] Cleaning up...";
    
    if (mediaManager) {
        delete mediaManager;
        qDebug() << "✓ MediaManager cleaned up";
    }
    
    if (chatManager) {
        delete chatManager;
        qDebug() << "✓ ChatManager cleaned up";
    }
    
    // 7. 显示系统信息
    qDebug() << "\n[Step 8] System Information:";
    QVariantMap moduleInfo = compatibilityModule.getModuleInfo();
    qDebug() << "Module Name:" << moduleInfo["name"].toString();
    qDebug() << "Module Version:" << moduleInfo["version"].toString();
    qDebug() << "Description:" << moduleInfo["description"].toString();
    
    // 显示可用检查点
    QStringList checkpoints = rollback->availableCheckpoints();
    qDebug() << "Available checkpoints:" << checkpoints;
    
    qDebug() << "\n=== Example completed successfully ===";
    
    return 0;
}
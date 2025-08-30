#ifndef MODULE_INTEGRATION_TEST_H
#define MODULE_INTEGRATION_TEST_H

#include <QObject>
#include <QTest>
#include <QTimer>
#include <QEventLoop>
#include <QVariantMap>
#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QSignalSpy>
#include <QThread>
#include <QMutex>
#include <QWaitCondition>

// Module interfaces
#ifdef AUDIO_MODULE_AVAILABLE
#include "interfaces/IAudioManager.h"
#include "include/AudioModule.h"
#endif

#ifdef NETWORK_MODULE_AVAILABLE
#include "interfaces/INetworkManager.h"
#include "include/NetworkModule.h"
#endif

#ifdef UI_MODULE_AVAILABLE
#include "interfaces/IUIManager.h"
#include "include/UIModule.h"
#endif

#ifdef PERFORMANCE_MODULE_AVAILABLE
#include "interfaces/IPerformanceMonitor.h"
#include "include/PerformanceModule.h"
#endif

#ifdef UTILS_MODULE_AVAILABLE
#include "interfaces/ILogger.h"
#include "include/UtilsModule.h"
#endif

#ifdef SETTINGS_MODULE_AVAILABLE
#include "interfaces/ISettingsManager.h"
#include "include/SettingsModule.h"
#endif

#ifdef CHAT_MODULE_AVAILABLE
#include "interfaces/IChatManager.h"
#include "include/ChatModule.h"
#endif

#ifdef SCREENSHARE_MODULE_AVAILABLE
#include "interfaces/IScreenShareManager.h"
#include "include/ScreenShareModule.h"
#endif

#ifdef MEETING_MODULE_AVAILABLE
#include "interfaces/IMeetingManager.h"
#include "include/MeetingModule.h"
#endif

#ifdef CAMERA_MODULE_AVAILABLE
#include "interfaces/ICameraManager.h"
#include "include/CameraModule.h"
#endif

/**
 * @brief 模块间集成测试类
 * 
 * 这个类负责测试所有模块之间的集成，包括：
 * - 模块加载顺序和依赖关系测试
 * - 模块间通信和数据共享测试
 * - 端到端功能测试和错误传播测试
 * 
 * Requirements: 11.4, 11.5, 12.6
 */
class ModuleIntegrationTest : public QObject
{
    Q_OBJECT

public:
    explicit ModuleIntegrationTest(QObject *parent = nullptr);
    ~ModuleIntegrationTest();

    // 测试结果结构
    struct TestResult {
        QString testName;
        bool passed;
        QString errorMessage;
        QDateTime timestamp;
        qint64 executionTime; // milliseconds
    };

    // 模块状态枚举
    enum ModuleStatus {
        NotLoaded,
        Loading,
        Loaded,
        Initializing,
        Ready,
        Error,
        Unloading
    };

    // 模块依赖关系结构
    struct ModuleDependency {
        QString moduleName;
        QString requiredVersion;
        bool isOptional;
        QString description;
    };

    // 通信测试数据结构
    struct CommunicationTestData {
        QString sourceModule;
        QString targetModule;
        QVariantMap testData;
        bool expectSuccess;
        QString expectedResponse;
    };

private slots:
    // 主要测试方法
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 模块加载和依赖测试
    void testModuleLoadOrder();
    void testModuleDependencies();
    void testModuleUnloading();
    void testModuleVersionCompatibility();
    void testModuleHealthCheck();

    // 模块间通信测试
    void testAudioVideoIntegration();
    void testChatNetworkIntegration();
    void testUIPerformanceIntegration();
    void testSettingsModuleIntegration();
    void testScreenShareIntegration();
    void testMeetingModuleIntegration();

    // 端到端功能测试
    void testCompleteWorkflow();
    void testErrorPropagation();
    void testResourceSharing();
    void testConcurrentOperations();
    void testMemoryLeakDetection();

    // 性能和压力测试
    void testModuleStartupPerformance();
    void testModuleCommunicationLatency();
    void testHighLoadScenarios();
    void testResourceConstraints();

    // 错误处理和恢复测试
    void testModuleFailureRecovery();
    void testCascadingFailureHandling();
    void testGracefulDegradation();

private:
    // 辅助方法
    void setupTestEnvironment();
    void teardownTestEnvironment();
    
    // 模块管理方法
    bool loadModule(const QString& moduleName);
    bool unloadModule(const QString& moduleName);
    ModuleStatus getModuleStatus(const QString& moduleName);
    QStringList getLoadedModules() const;
    QStringList getAvailableModules() const;
    
    // 依赖关系验证
    bool validateDependencies(const QString& moduleName);
    QList<ModuleDependency> getModuleDependencies(const QString& moduleName);
    bool checkDependencyChain(const QString& moduleName, QStringList& visitedModules);
    
    // 通信测试方法
    bool testModuleCommunication(const CommunicationTestData& testData);
    void setupCommunicationChannels();
    void teardownCommunicationChannels();
    
    // 性能测量方法
    qint64 measureModuleStartupTime(const QString& moduleName);
    qint64 measureCommunicationLatency(const QString& sourceModule, const QString& targetModule);
    void collectPerformanceMetrics();
    
    // 错误注入和测试
    void injectModuleError(const QString& moduleName, const QString& errorType);
    void simulateResourceConstraints();
    void simulateNetworkFailure();
    
    // 验证方法
    bool verifyModuleIntegrity(const QString& moduleName);
    bool verifyDataConsistency();
    bool verifyResourceCleanup();
    
    // 报告生成
    void generateTestReport();
    void logTestResult(const TestResult& result);
    QString formatTestResults() const;

private:
    // 测试状态
    QList<TestResult> m_testResults;
    QStringList m_loadedModules;
    QMap<QString, ModuleStatus> m_moduleStatuses;
    QMap<QString, QList<ModuleDependency>> m_moduleDependencies;
    
    // 性能数据
    QMap<QString, qint64> m_startupTimes;
    QMap<QString, qint64> m_communicationLatencies;
    QMap<QString, QVariantMap> m_performanceMetrics;
    
    // 测试配置
    bool m_enablePerformanceTests;
    bool m_enableStressTests;
    int m_testTimeout;
    int m_maxRetries;
    
    // 同步对象
    QMutex m_testMutex;
    QWaitCondition m_testCondition;
    QEventLoop m_eventLoop;
    
    // 模块实例（如果可用）
#ifdef AUDIO_MODULE_AVAILABLE
    AudioModule* m_audioModule;
#endif
#ifdef NETWORK_MODULE_AVAILABLE
    NetworkModule* m_networkModule;
#endif
#ifdef UI_MODULE_AVAILABLE
    UIModule* m_uiModule;
#endif
#ifdef PERFORMANCE_MODULE_AVAILABLE
    PerformanceModule* m_performanceModule;
#endif
#ifdef UTILS_MODULE_AVAILABLE
    UtilsModule* m_utilsModule;
#endif
#ifdef SETTINGS_MODULE_AVAILABLE
    SettingsModule* m_settingsModule;
#endif
#ifdef CHAT_MODULE_AVAILABLE
    ChatModule* m_chatModule;
#endif
#ifdef SCREENSHARE_MODULE_AVAILABLE
    ScreenShareModule* m_screenShareModule;
#endif
#ifdef MEETING_MODULE_AVAILABLE
    MeetingModule* m_meetingModule;
#endif
#ifdef CAMERA_MODULE_AVAILABLE
    CameraModule* m_cameraModule;
#endif
};

#endif // MODULE_INTEGRATION_TEST_H
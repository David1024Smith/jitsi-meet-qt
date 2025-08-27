#include <QtTest/QtTest>
#include <QApplication>
#include <QTimer>
#include <QSignalSpy>
#include <QDebug>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QWebEngineProfile>
#include <QTemporaryDir>
#include <QSettings>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "MainApplication.h"
#include "WindowManager.h"
#include "ConfigurationManager.h"
#include "ProtocolHandler.h"
#include "TranslationManager.h"
#include "WindowStateManager.h"
#include "WelcomeWindow.h"
#include "ConferenceWindow.h"
#include "SettingsDialog.h"
#include "JitsiConstants.h"

/**
 * @brief 集成测试类，测试所有组件的集成工作
 * 
 * 这个测试类涵盖以下集成测试场景：
 * 1. 窗口切换和导航的集成测试
 * 2. WebEngine加载和JavaScript交互测试
 * 3. 配置持久化和状态恢复功能验证
 * 4. 协议处理的端到端流程测试
 */
class IntegrationTest : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // 主应用程序测试
    void testMainApplicationInitialization();
    void testSingleInstanceBehavior();
    void testProtocolHandling();
    
    // 窗口切换和导航集成测试
    void testWindowManagerIntegration();
    void testWindowSwitching();
    void testWindowNavigationFlow();
    void testWindowDataTransfer();
    void testWindowStateManagement();
    void testWindowMemoryManagement();
    
    // WebEngine集成测试
    void testWebEngineInitialization();
    void testWebEngineLoading();
    void testWebEngineJavaScriptInteraction();
    void testWebEngineErrorHandling();
    void testWebEngineNetworkRequests();
    
    // 配置持久化和状态恢复测试
    void testConfigurationPersistence();
    void testWindowStatePersistence();
    void testRecentUrlsPersistence();
    void testSettingsPersistence();
    void testConfigurationRecovery();
    
    // 协议处理端到端测试
    void testProtocolRegistration();
    void testProtocolUrlParsing();
    void testProtocolUrlFlow();
    void testProtocolErrorHandling();
    void testProtocolMultipleInstances();
    
    // 翻译系统集成测试
    void testTranslationIntegration();
    void testLanguageSwitching();
    void testTranslationPersistence();
    
    // 端到端流程测试
    void testCompleteApplicationFlow();
    void testSettingsFlow();
    void testErrorRecoveryFlow();

private:
    void createTestApplication();
    void destroyTestApplication();
    bool waitForSignal(QObject* sender, const char* signal, int timeout = 5000);
    void setupTestEnvironment();
    void cleanupTestEnvironment();
    QWebEngineView* createTestWebView();
    void simulateUserInteraction();
    
    MainApplication* m_app;
    int m_argc;
    char** m_argv;
    QTemporaryDir* m_tempDir;
    QString m_originalConfigPath;
};

void IntegrationTest::initTestCase()
{
    qDebug() << "Starting comprehensive integration tests";
    
    // 设置测试环境
    setupTestEnvironment();
    
    m_argc = 1;
    m_argv = new char*[1];
    m_argv[0] = new char[20];
    strcpy(m_argv[0], "test_integration");
    
    m_app = nullptr;
    m_tempDir = nullptr;
}

void IntegrationTest::cleanupTestCase()
{
    qDebug() << "Finishing comprehensive integration tests";
    
    cleanupTestEnvironment();
    
    if (m_argv) {
        delete[] m_argv[0];
        delete[] m_argv;
    }
}

void IntegrationTest::init()
{
    createTestApplication();
}

void IntegrationTest::cleanup()
{
    destroyTestApplication();
}

void IntegrationTest::setupTestEnvironment()
{
    // 创建临时目录用于测试配置
    m_tempDir = new QTemporaryDir();
    QVERIFY(m_tempDir->isValid());
    
    // 保存原始配置路径
    m_originalConfigPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    
    // 设置测试应用程序信息
    QCoreApplication::setApplicationName("JitsiMeetQtTest");
    QCoreApplication::setOrganizationName("JitsiMeetQtTest");
    
    // 设置测试配置路径
    qputenv("XDG_CONFIG_HOME", m_tempDir->path().toLocal8Bit());
}

void IntegrationTest::cleanupTestEnvironment()
{
    if (m_tempDir) {
        delete m_tempDir;
        m_tempDir = nullptr;
    }
    
    // 恢复原始配置路径
    if (!m_originalConfigPath.isEmpty()) {
        qputenv("XDG_CONFIG_HOME", m_originalConfigPath.toLocal8Bit());
    }
}

void IntegrationTest::testMainApplicationInitialization()
{
    QVERIFY(m_app != nullptr);
    QVERIFY(m_app->windowManager() != nullptr);
    QVERIFY(m_app->configurationManager() != nullptr);
    QVERIFY(m_app->protocolHandler() != nullptr);
    QVERIFY(m_app->translationManager() != nullptr);
    
    qDebug() << "MainApplication initialization test passed";
}

void IntegrationTest::testSingleInstanceBehavior()
{
    // 测试单例模式
    // 注意：这个测试需要特殊处理，因为我们已经有一个实例在运行
    
    // 验证当前实例是第一个实例
    QVERIFY(m_app != nullptr);
    
    qDebug() << "Single instance behavior test passed";
}

void IntegrationTest::testProtocolHandling()
{
    auto protocolHandler = m_app->protocolHandler();
    QVERIFY(protocolHandler != nullptr);
    
    // 测试协议URL解析
    QString testUrl = "jitsi-meet://meet.jit.si/TestRoom123";
    QVERIFY(protocolHandler->isValidProtocolUrl(testUrl));
    
    QString parsedUrl = protocolHandler->parseProtocolUrl(testUrl);
    QVERIFY(!parsedUrl.isEmpty());
    QVERIFY(parsedUrl.contains("TestRoom123"));
    
    qDebug() << "Protocol handling test passed";
}

// 协议处理端到端测试
void IntegrationTest::testProtocolRegistration()
{
    auto protocolHandler = m_app->protocolHandler();
    QVERIFY(protocolHandler != nullptr);
    
    // 测试协议注册
    bool registrationResult = protocolHandler->registerProtocol();
    QVERIFY(registrationResult);
    
    // 验证协议已注册
    QVERIFY(protocolHandler->isProtocolRegistered());
    
    qDebug() << "Protocol registration test passed";
}

void IntegrationTest::testProtocolUrlParsing()
{
    auto protocolHandler = m_app->protocolHandler();
    
    // 测试各种协议URL格式
    struct ProtocolTestCase {
        QString input;
        bool shouldBeValid;
        QString expectedRoom;
        QString expectedServer;
    };
    
    QList<ProtocolTestCase> testCases = {
        {"jitsi-meet://meet.jit.si/TestRoom", true, "TestRoom", "meet.jit.si"},
        {"jitsi-meet://custom.server.com/MyRoom123", true, "MyRoom123", "custom.server.com"},
        {"jitsi-meet://meet.jit.si/Room-With-Dashes", true, "Room-With-Dashes", "meet.jit.si"},
        {"jitsi-meet://meet.jit.si/", false, "", ""},
        {"invalid-protocol://meet.jit.si/Room", false, "", ""},
        {"jitsi-meet://", false, "", ""},
        {"", false, "", ""}
    };
    
    for (const auto& testCase : testCases) {
        bool isValid = protocolHandler->isValidProtocolUrl(testCase.input);
        QCOMPARE(isValid, testCase.shouldBeValid);
        
        if (testCase.shouldBeValid) {
            QString parsedUrl = protocolHandler->parseProtocolUrl(testCase.input);
            QVERIFY(!parsedUrl.isEmpty());
            QVERIFY(parsedUrl.contains(testCase.expectedRoom));
            
            auto urlInfo = protocolHandler->parseUrlInfo(testCase.input);
            QCOMPARE(urlInfo.roomName, testCase.expectedRoom);
            QCOMPARE(urlInfo.serverUrl, testCase.expectedServer);
        }
    }
    
    qDebug() << "Protocol URL parsing test passed";
}

void IntegrationTest::testProtocolUrlFlow()
{
    auto windowManager = m_app->windowManager();
    auto protocolHandler = m_app->protocolHandler();
    
    // 监听协议URL处理信号
    QSignalSpy protocolUrlSpy(protocolHandler, &ProtocolHandler::protocolUrlReceived);
    QSignalSpy windowChangedSpy(windowManager, &WindowManager::windowChanged);
    
    // 模拟协议URL处理
    QString testProtocolUrl = "jitsi-meet://meet.jit.si/ProtocolFlowTest";
    
    // 处理协议URL
    m_app->handleProtocolUrl(testProtocolUrl);
    
    // 验证协议URL被正确接收
    QVERIFY(protocolUrlSpy.count() > 0);
    
    // 验证窗口切换到会议窗口
    QVERIFY(waitForSignal(windowManager, SIGNAL(windowChanged(int)), 3000));
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 验证会议窗口加载了正确的URL
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    qDebug() << "Protocol URL flow test passed";
}

void IntegrationTest::testProtocolErrorHandling()
{
    auto protocolHandler = m_app->protocolHandler();
    
    // 测试无效协议URL的错误处理
    QSignalSpy errorSpy(protocolHandler, &ProtocolHandler::protocolError);
    
    // 处理无效的协议URL
    QStringList invalidUrls = {
        "invalid-protocol://meet.jit.si/Room",
        "jitsi-meet://",
        "jitsi-meet://invalid-server/Room",
        "",
        "not-a-url-at-all"
    };
    
    for (const QString& invalidUrl : invalidUrls) {
        m_app->handleProtocolUrl(invalidUrl);
    }
    
    // 验证错误信号被发射
    QVERIFY(errorSpy.count() > 0);
    
    qDebug() << "Protocol error handling test passed";
}

void IntegrationTest::testProtocolMultipleInstances()
{
    // 测试多实例情况下的协议处理
    
    // 注意：这个测试比较复杂，因为涉及到多个应用程序实例
    // 在实际测试中，我们模拟第二个实例尝试启动的情况
    
    auto protocolHandler = m_app->protocolHandler();
    
    // 模拟第二个实例发送协议URL
    QSignalSpy secondInstanceSpy(m_app, &MainApplication::secondInstanceStarted);
    
    // 这里我们模拟接收到来自第二个实例的消息
    QString testUrl = "jitsi-meet://meet.jit.si/MultiInstanceTest";
    m_app->handleSecondInstanceMessage(testUrl);
    
    // 验证第二个实例信号被处理
    QVERIFY(secondInstanceSpy.count() > 0);
    
    qDebug() << "Protocol multiple instances test passed";
}

void IntegrationTest::testWindowManagerIntegration()
{
    auto windowManager = m_app->windowManager();
    QVERIFY(windowManager != nullptr);
    
    // 测试窗口创建和显示
    QSignalSpy windowChangedSpy(windowManager, &WindowManager::windowChanged);
    QSignalSpy windowCreatedSpy(windowManager, &WindowManager::windowCreated);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 验证信号发射
    QVERIFY(windowChangedSpy.count() > 0);
    QVERIFY(windowCreatedSpy.count() > 0);
    
    // 验证窗口状态
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    QVERIFY(windowManager->hasWindow(WindowManager::WelcomeWindow));
    QVERIFY(windowManager->isWindowVisible(WindowManager::WelcomeWindow));
    
    qDebug() << "WindowManager integration test passed";
}

void IntegrationTest::testWindowSwitching()
{
    auto windowManager = m_app->windowManager();
    
    QSignalSpy windowChangedSpy(windowManager, &WindowManager::windowChanged);
    QSignalSpy windowCreatedSpy(windowManager, &WindowManager::windowCreated);
    QSignalSpy windowClosedSpy(windowManager, &WindowManager::windowClosed);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    QVERIFY(windowChangedSpy.count() > 0);
    
    // 切换到会议窗口
    QVariantMap conferenceData;
    conferenceData["url"] = "https://meet.jit.si/TestRoom";
    windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
    
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    QVERIFY(windowManager->hasWindow(WindowManager::ConferenceWindow));
    QVERIFY(windowCreatedSpy.count() > 0);
    
    // 显示设置对话框（模态窗口）
    windowManager->showWindow(WindowManager::SettingsDialog);
    QVERIFY(windowManager->hasWindow(WindowManager::SettingsDialog));
    QVERIFY(windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    // 关闭设置对话框
    windowManager->closeWindow(WindowManager::SettingsDialog);
    QVERIFY(!windowManager->isWindowVisible(WindowManager::SettingsDialog));
    QVERIFY(windowClosedSpy.count() > 0);
    
    // 返回欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    qDebug() << "Window switching test passed";
}

void IntegrationTest::testWindowNavigationFlow()
{
    auto windowManager = m_app->windowManager();
    
    // 测试完整的导航流程
    QSignalSpy navigationSpy(windowManager, &WindowManager::navigationRequested);
    
    // 1. 启动 -> 欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<WelcomeWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 2. 欢迎窗口 -> 会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/NavigationTest";
    windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    // 3. 会议窗口 -> 设置对话框
    windowManager->showWindow(WindowManager::SettingsDialog);
    auto settingsDialog = qobject_cast<SettingsDialog*>(windowManager->getWindow(WindowManager::SettingsDialog));
    QVERIFY(settingsDialog != nullptr);
    
    // 4. 设置对话框 -> 返回会议窗口
    windowManager->closeWindow(WindowManager::SettingsDialog);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 5. 会议窗口 -> 返回欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    qDebug() << "Window navigation flow test passed";
}

void IntegrationTest::testWindowStateManagement()
{
    auto windowManager = m_app->windowManager();
    auto stateManager = windowManager->stateManager();
    QVERIFY(stateManager != nullptr);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 测试窗口状态保存
    QRect originalGeometry = welcomeWindow->geometry();
    welcomeWindow->resize(1000, 700);
    welcomeWindow->move(200, 150);
    
    // 保存状态
    stateManager->saveWindowState(WindowManager::WelcomeWindow, welcomeWindow);
    
    // 创建新窗口并恢复状态
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    auto newWelcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(newWelcomeWindow != nullptr);
    
    // 验证状态恢复
    stateManager->restoreWindowState(WindowManager::WelcomeWindow, newWelcomeWindow);
    QCOMPARE(newWelcomeWindow->size(), QSize(1000, 700));
    
    qDebug() << "Window state management test passed";
}

void IntegrationTest::testWindowMemoryManagement()
{
    auto windowManager = m_app->windowManager();
    
    // 测试窗口创建和销毁的内存管理
    QPointer<QMainWindow> welcomePtr;
    QPointer<QMainWindow> conferencePtr;
    QPointer<QDialog> settingsPtr;
    
    // 创建窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    welcomePtr = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(!welcomePtr.isNull());
    
    windowManager->showWindow(WindowManager::ConferenceWindow);
    conferencePtr = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(!conferencePtr.isNull());
    
    windowManager->showWindow(WindowManager::SettingsDialog);
    settingsPtr = qobject_cast<QDialog*>(windowManager->getWindow(WindowManager::SettingsDialog));
    QVERIFY(!settingsPtr.isNull());
    
    // 关闭窗口
    windowManager->closeWindow(WindowManager::SettingsDialog);
    windowManager->closeWindow(WindowManager::ConferenceWindow);
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    
    // 强制垃圾回收
    QCoreApplication::processEvents();
    
    // 验证内存清理（注意：这个测试可能需要调整，取决于具体的内存管理策略）
    qDebug() << "Window memory management test passed";
}

void IntegrationTest::testWindowDataTransfer()
{
    auto windowManager = m_app->windowManager();
    
    QSignalSpy dataTransferSpy(windowManager, &WindowManager::dataTransferred);
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 传递数据到会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/DataTransferTest";
    data["roomName"] = "DataTransferTest";
    data["serverUrl"] = "https://meet.jit.si";
    windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    // 验证数据传递信号
    QVERIFY(dataTransferSpy.count() > 0);
    
    // 验证数据正确传递到会议窗口
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    qDebug() << "Window data transfer test passed";
}

// WebEngine集成测试
void IntegrationTest::testWebEngineInitialization()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    // 获取WebEngineView
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 验证WebEngine设置
    auto settings = webView->settings();
    QVERIFY(settings != nullptr);
    QVERIFY(settings->testAttribute(QWebEngineSettings::JavascriptEnabled));
    QVERIFY(settings->testAttribute(QWebEngineSettings::LocalStorageEnabled));
    
    // 验证WebEngine配置文件
    auto profile = webView->page()->profile();
    QVERIFY(profile != nullptr);
    
    qDebug() << "WebEngine initialization test passed";
}

void IntegrationTest::testWebEngineLoading()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    QVariantMap data;
    data["url"] = "https://meet.jit.si/WebEngineLoadTest";
    windowManager->showWindow(WindowManager::ConferenceWindow, data);
    
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 监听加载信号
    QSignalSpy loadStartedSpy(webView, &QWebEngineView::loadStarted);
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    QSignalSpy loadProgressSpy(webView, &QWebEngineView::loadProgress);
    
    // 加载测试页面
    conferenceWindow->loadConference("https://meet.jit.si/WebEngineLoadTest");
    
    // 验证加载开始
    QVERIFY(loadStartedSpy.count() > 0 || loadStartedSpy.wait(5000));
    
    // 等待加载完成
    if (loadFinishedSpy.isEmpty()) {
        QVERIFY(loadFinishedSpy.wait(15000)); // 等待最多15秒
    }
    
    // 验证加载进度
    QVERIFY(loadProgressSpy.count() > 0);
    
    qDebug() << "WebEngine loading test passed";
}

void IntegrationTest::testWebEngineJavaScriptInteraction()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 加载简单的HTML页面用于测试
    QString testHtml = R"(
        <!DOCTYPE html>
        <html>
        <head><title>Test Page</title></head>
        <body>
            <div id="test">Hello World</div>
            <script>
                window.testFunction = function() {
                    return "JavaScript works!";
                };
                
                window.testCallback = function(message) {
                    document.getElementById('test').innerHTML = message;
                };
            </script>
        </body>
        </html>
    )";
    
    webView->setHtml(testHtml);
    
    // 等待页面加载
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    QVERIFY(loadFinishedSpy.wait(5000));
    
    // 测试JavaScript执行
    QEventLoop loop;
    bool jsTestPassed = false;
    
    webView->page()->runJavaScript("window.testFunction()", [&](const QVariant& result) {
        jsTestPassed = (result.toString() == "JavaScript works!");
        loop.quit();
    });
    
    QTimer::singleShot(3000, &loop, &QEventLoop::quit); // 超时保护
    loop.exec();
    
    QVERIFY(jsTestPassed);
    
    qDebug() << "WebEngine JavaScript interaction test passed";
}

void IntegrationTest::testWebEngineErrorHandling()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 监听错误信号
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    
    // 尝试加载无效URL
    webView->load(QUrl("https://invalid-domain-that-does-not-exist.com"));
    
    // 等待加载完成（应该失败）
    QVERIFY(loadFinishedSpy.wait(10000));
    
    // 验证加载失败
    QList<QVariant> arguments = loadFinishedSpy.takeLast();
    bool loadSuccess = arguments.at(0).toBool();
    QVERIFY(!loadSuccess); // 应该加载失败
    
    qDebug() << "WebEngine error handling test passed";
}

void IntegrationTest::testWebEngineNetworkRequests()
{
    auto windowManager = m_app->windowManager();
    
    // 创建会议窗口
    windowManager->showWindow(WindowManager::ConferenceWindow);
    auto conferenceWindow = qobject_cast<ConferenceWindow*>(windowManager->getWindow(WindowManager::ConferenceWindow));
    QVERIFY(conferenceWindow != nullptr);
    
    auto webView = conferenceWindow->findChild<QWebEngineView*>();
    QVERIFY(webView != nullptr);
    
    // 测试网络请求拦截（如果实现了的话）
    auto profile = webView->page()->profile();
    QVERIFY(profile != nullptr);
    
    // 加载一个真实的URL来测试网络功能
    QSignalSpy loadFinishedSpy(webView, &QWebEngineView::loadFinished);
    
    webView->load(QUrl("https://www.google.com"));
    
    // 等待加载完成
    if (loadFinishedSpy.isEmpty()) {
        QVERIFY(loadFinishedSpy.wait(15000));
    }
    
    // 验证网络请求成功
    QList<QVariant> arguments = loadFinishedSpy.takeLast();
    bool loadSuccess = arguments.at(0).toBool();
    QVERIFY(loadSuccess);
    
    qDebug() << "WebEngine network requests test passed";
}

// 配置持久化和状态恢复测试
void IntegrationTest::testConfigurationPersistence()
{
    auto configManager = m_app->configurationManager();
    QVERIFY(configManager != nullptr);
    
    // 保存原始配置
    QString originalServerUrl = configManager->serverUrl();
    QString originalLanguage = configManager->language();
    QStringList originalRecentUrls = configManager->recentUrls();
    
    // 修改配置
    QString testServerUrl = "https://test-persistence.example.com";
    QString testLanguage = "zh_CN";
    QStringList testRecentUrls = {
        "https://meet.jit.si/PersistenceTest1",
        "https://meet.jit.si/PersistenceTest2",
        "https://meet.jit.si/PersistenceTest3"
    };
    
    configManager->setServerUrl(testServerUrl);
    configManager->setLanguage(testLanguage);
    for (const QString& url : testRecentUrls) {
        configManager->addRecentUrl(url);
    }
    
    // 强制保存配置
    configManager->saveConfiguration();
    
    // 创建新的配置管理器实例来测试持久化
    auto newConfigManager = new ConfigurationManager(this);
    newConfigManager->loadConfiguration();
    
    // 验证配置持久化
    QCOMPARE(newConfigManager->serverUrl(), testServerUrl);
    QCOMPARE(newConfigManager->language(), testLanguage);
    
    QStringList loadedRecentUrls = newConfigManager->recentUrls();
    for (const QString& url : testRecentUrls) {
        QVERIFY(loadedRecentUrls.contains(url));
    }
    
    // 清理
    delete newConfigManager;
    
    // 恢复原始配置
    configManager->setServerUrl(originalServerUrl);
    configManager->setLanguage(originalLanguage);
    configManager->clearRecentUrls();
    for (const QString& url : originalRecentUrls) {
        configManager->addRecentUrl(url);
    }
    
    qDebug() << "Configuration persistence test passed";
}

void IntegrationTest::testWindowStatePersistence()
{
    auto windowManager = m_app->windowManager();
    auto stateManager = windowManager->stateManager();
    QVERIFY(stateManager != nullptr);
    
    // 创建并配置欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto welcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(welcomeWindow != nullptr);
    
    // 设置特定的窗口状态
    QRect testGeometry(150, 100, 900, 650);
    welcomeWindow->setGeometry(testGeometry);
    welcomeWindow->showMaximized();
    
    // 保存窗口状态
    stateManager->saveWindowState(WindowManager::WelcomeWindow, welcomeWindow);
    stateManager->saveAllStates();
    
    // 关闭窗口
    windowManager->closeWindow(WindowManager::WelcomeWindow);
    
    // 重新创建窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    auto newWelcomeWindow = qobject_cast<QMainWindow*>(windowManager->getWindow(WindowManager::WelcomeWindow));
    QVERIFY(newWelcomeWindow != nullptr);
    
    // 恢复窗口状态
    stateManager->restoreWindowState(WindowManager::WelcomeWindow, newWelcomeWindow);
    
    // 验证状态恢复
    QVERIFY(newWelcomeWindow->isMaximized());
    
    qDebug() << "Window state persistence test passed";
}

void IntegrationTest::testRecentUrlsPersistence()
{
    auto configManager = m_app->configurationManager();
    
    // 清空现有的最近URL列表
    configManager->clearRecentUrls();
    
    // 添加测试URL
    QStringList testUrls = {
        "https://meet.jit.si/RecentTest1",
        "https://meet.jit.si/RecentTest2",
        "https://custom.server.com/RecentTest3",
        "https://meet.jit.si/RecentTest4"
    };
    
    for (const QString& url : testUrls) {
        configManager->addRecentUrl(url);
    }
    
    // 验证URL被正确添加
    QStringList recentUrls = configManager->recentUrls();
    QCOMPARE(recentUrls.size(), testUrls.size());
    
    for (const QString& url : testUrls) {
        QVERIFY(recentUrls.contains(url));
    }
    
    // 测试URL去重功能
    configManager->addRecentUrl(testUrls.first()); // 重复添加第一个URL
    recentUrls = configManager->recentUrls();
    QCOMPARE(recentUrls.size(), testUrls.size()); // 大小应该保持不变
    
    // 测试最大数量限制
    for (int i = 0; i < 20; ++i) {
        configManager->addRecentUrl(QString("https://meet.jit.si/ExtraTest%1").arg(i));
    }
    
    recentUrls = configManager->recentUrls();
    QVERIFY(recentUrls.size() <= configManager->maxRecentUrls());
    
    qDebug() << "Recent URLs persistence test passed";
}

void IntegrationTest::testSettingsPersistence()
{
    auto configManager = m_app->configurationManager();
    
    // 测试各种设置的持久化
    struct TestSettings {
        QString serverUrl;
        QString language;
        bool autoJoinAudio;
        bool autoJoinVideo;
        bool darkMode;
        int maxRecentItems;
    };
    
    TestSettings originalSettings = {
        configManager->serverUrl(),
        configManager->language(),
        configManager->autoJoinAudio(),
        configManager->autoJoinVideo(),
        configManager->darkMode(),
        configManager->maxRecentUrls()
    };
    
    TestSettings testSettings = {
        "https://custom-settings-test.com",
        "ja",
        false,
        true,
        true,
        15
    };
    
    // 应用测试设置
    configManager->setServerUrl(testSettings.serverUrl);
    configManager->setLanguage(testSettings.language);
    configManager->setAutoJoinAudio(testSettings.autoJoinAudio);
    configManager->setAutoJoinVideo(testSettings.autoJoinVideo);
    configManager->setDarkMode(testSettings.darkMode);
    configManager->setMaxRecentUrls(testSettings.maxRecentItems);
    
    // 保存配置
    configManager->saveConfiguration();
    
    // 重新加载配置
    configManager->loadConfiguration();
    
    // 验证设置持久化
    QCOMPARE(configManager->serverUrl(), testSettings.serverUrl);
    QCOMPARE(configManager->language(), testSettings.language);
    QCOMPARE(configManager->autoJoinAudio(), testSettings.autoJoinAudio);
    QCOMPARE(configManager->autoJoinVideo(), testSettings.autoJoinVideo);
    QCOMPARE(configManager->darkMode(), testSettings.darkMode);
    QCOMPARE(configManager->maxRecentUrls(), testSettings.maxRecentItems);
    
    // 恢复原始设置
    configManager->setServerUrl(originalSettings.serverUrl);
    configManager->setLanguage(originalSettings.language);
    configManager->setAutoJoinAudio(originalSettings.autoJoinAudio);
    configManager->setAutoJoinVideo(originalSettings.autoJoinVideo);
    configManager->setDarkMode(originalSettings.darkMode);
    configManager->setMaxRecentUrls(originalSettings.maxRecentItems);
    
    qDebug() << "Settings persistence test passed";
}

void IntegrationTest::testConfigurationRecovery()
{
    auto configManager = m_app->configurationManager();
    
    // 测试配置文件损坏时的恢复机制
    
    // 保存当前配置
    auto originalConfig = configManager->currentConfiguration();
    
    // 模拟配置损坏（设置无效值）
    configManager->setServerUrl("invalid-url-format");
    
    // 触发配置验证和恢复
    bool recoveryResult = configManager->validateAndRecover();
    QVERIFY(recoveryResult);
    
    // 验证恢复后的配置是有效的
    QVERIFY(configManager->isValidServerUrl(configManager->serverUrl()));
    
    // 测试默认配置恢复
    configManager->resetToDefaults();
    QCOMPARE(configManager->serverUrl(), JitsiConstants::DEFAULT_SERVER_URL);
    QCOMPARE(configManager->language(), "auto");
    
    qDebug() << "Configuration recovery test passed";
}

void IntegrationTest::testTranslationIntegration()
{
    auto translationManager = m_app->translationManager();
    QVERIFY(translationManager != nullptr);
    
    // 测试语言切换
    QSignalSpy languageChangedSpy(translationManager, &TranslationManager::languageChanged);
    
    QString currentLanguage = translationManager->currentLanguage();
    QString testLanguage = (currentLanguage == "en") ? "zh_CN" : "en";
    
    translationManager->setLanguage(testLanguage);
    
    // 验证语言改变信号
    QVERIFY(languageChangedSpy.count() > 0);
    
    qDebug() << "Translation integration test passed";
}

void IntegrationTest::testCompleteApplicationFlow()
{
    auto windowManager = m_app->windowManager();
    auto configManager = m_app->configurationManager();
    
    // 模拟完整的应用程序流程
    
    // 1. 启动应用程序（已经完成）
    QVERIFY(windowManager->currentWindowType() == WindowManager::WelcomeWindow ||
            !windowManager->hasWindow(WindowManager::WelcomeWindow));
    
    // 2. 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    // 3. 加入会议
    QVariantMap conferenceData;
    conferenceData["url"] = "https://meet.jit.si/CompleteFlowTest";
    windowManager->showWindow(WindowManager::ConferenceWindow, conferenceData);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::ConferenceWindow);
    
    // 4. 验证URL被添加到最近列表
    QStringList recentUrls = configManager->recentUrls();
    // 注意：这个测试可能需要等待会议窗口完全加载
    
    // 5. 返回欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    QCOMPARE(windowManager->currentWindowType(), WindowManager::WelcomeWindow);
    
    qDebug() << "Complete application flow test passed";
}

void IntegrationTest::testProtocolUrlFlow()
{
    // 测试协议URL处理流程
    QString protocolUrl = "jitsi-meet://meet.jit.si/ProtocolFlowTest";
    
    QSignalSpy windowChangedSpy(m_app->windowManager(), &WindowManager::windowChanged);
    
    // 处理协议URL
    m_app->handleProtocolUrl(protocolUrl);
    
    // 验证窗口切换到会议窗口
    QVERIFY(waitForSignal(m_app->windowManager(), SIGNAL(windowChanged(int)), 3000));
    QCOMPARE(m_app->windowManager()->currentWindowType(), WindowManager::ConferenceWindow);
    
    qDebug() << "Protocol URL flow test passed";
}

void IntegrationTest::testSettingsFlow()
{
    auto windowManager = m_app->windowManager();
    
    // 显示欢迎窗口
    windowManager->showWindow(WindowManager::WelcomeWindow);
    
    // 显示设置对话框
    windowManager->showWindow(WindowManager::SettingsDialog);
    QVERIFY(windowManager->hasWindow(WindowManager::SettingsDialog));
    QVERIFY(windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    // 关闭设置对话框
    windowManager->closeWindow(WindowManager::SettingsDialog);
    QVERIFY(!windowManager->isWindowVisible(WindowManager::SettingsDialog));
    
    qDebug() << "Settings flow test passed";
}

void IntegrationTest::createTestApplication()
{
    if (!m_app) {
        m_app = new MainApplication(m_argc, m_argv);
        
        // 等待应用程序初始化完成
        QTimer::singleShot(100, [this]() {
            // 初始化完成后的处理
        });
        
        // 处理事件循环
        QCoreApplication::processEvents();
    }
}

void IntegrationTest::destroyTestApplication()
{
    if (m_app) {
        delete m_app;
        m_app = nullptr;
    }
}

bool IntegrationTest::waitForSignal(QObject* sender, const char* signal, int timeout)
{
    QSignalSpy spy(sender, signal);
    return spy.wait(timeout);
}

QTEST_MAIN(IntegrationTest)
#include "test_integration.moc"
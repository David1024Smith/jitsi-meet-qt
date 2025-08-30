#ifndef FUNCTIONVALIDATOR_H
#define FUNCTIONVALIDATOR_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QHash>
#include <QMutex>
#include <functional>

/**
 * @brief 功能验证器
 * 
 * 负责验证各个模块的功能是否正常工作。
 */
class FunctionValidator : public QObject
{
    Q_OBJECT

public:
    explicit FunctionValidator(QObject *parent = nullptr);
    ~FunctionValidator();

    bool initialize();
    
    bool runTest(const QString& testName);
    QStringList runAllTests();
    QStringList getAvailableTests() const;
    
    void registerTest(const QString& testName, std::function<bool()> testFunction);
    void unregisterTest(const QString& testName);
    
    bool isTestRegistered(const QString& testName) const;
    
    void setTestTimeout(int timeoutMs);
    int getTestTimeout() const;

signals:
    void testStarted(const QString& testName);
    void testCompleted(const QString& testName, bool success);
    void allTestsCompleted(const QStringList& results);

private:
    void setupDefaultTests();
    
    // 默认测试函数
    bool testAudioDeviceEnumeration();
    bool testAudioDeviceSelection();
    bool testAudioVolumeControl();
    bool testAudioMuteControl();
    bool testAudioQualitySettings();
    
    bool testNetworkConnectionEstablishment();
    bool testNetworkDataTransmission();
    bool testNetworkQualityMonitoring();
    bool testNetworkProtocolHandling();
    bool testNetworkErrorRecovery();
    
    bool testUIThemeSwitching();
    bool testUILayoutManagement();
    bool testUIWidgetRendering();
    bool testUIEventHandling();
    bool testUIResponsiveness();
    
    bool testChatMessageSending();
    bool testChatMessageReceiving();
    bool testChatHistoryManagement();
    bool testChatParticipantManagement();
    bool testChatFileSharing();
    
    bool testScreenShareCaptureInitialization();
    bool testScreenShareScreenEnumeration();
    bool testScreenShareCaptureStartStop();
    bool testScreenShareQualityAdjustment();
    bool testScreenShareEncodingPerformance();
    
    bool testMeetingLinkParsing();
    bool testMeetingCreation();
    bool testMeetingJoining();
    bool testMeetingAuthentication();
    bool testMeetingRoomManagement();
    
    bool testSettingsLoadSave();
    bool testSettingsValidation();
    bool testSettingsSynchronization();
    bool testSettingsBackupRestore();
    bool testSettingsUIIntegration();
    
    bool testUtilsLoggingFunctionality();
    bool testUtilsFileOperations();
    bool testUtilsEncryptionDecryption();
    bool testUtilsStringProcessing();
    bool testUtilsConfigurationManagement();

    bool m_initialized;
    int m_testTimeout;
    QHash<QString, std::function<bool()>> m_tests;
    mutable QMutex m_mutex;
};

#endif // FUNCTIONVALIDATOR_H
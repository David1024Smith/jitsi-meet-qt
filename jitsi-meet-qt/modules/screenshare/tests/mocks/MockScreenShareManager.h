#ifndef MOCKSCREENSHAREMANAGER_H
#define MOCKSCREENSHAREMANAGER_H

#include "../interfaces/IScreenShareManager.h"
#include <QObject>
#include <QVariantMap>
#include <QStringList>

/**
 * @brief Mock implementation of IScreenShareManager for testing
 * 
 * 用于测试的IScreenShareManager模拟实现
 */
class MockScreenShareManager : public IScreenShareManager
{
    Q_OBJECT

public:
    explicit MockScreenShareManager(QObject *parent = nullptr);
    ~MockScreenShareManager() override;

    // IScreenShareManager interface implementation
    bool initialize() override;
    Status status() const override;
    bool isReady() const override;

    QStringList availableScreens() const override;
    QStringList availableWindows() const override;
    bool selectScreen(const QString& screenId) override;
    bool selectWindow(const QString& windowId) override;
    QString currentSource() const override;

    bool startScreenShare() override;
    void stopScreenShare() override;
    void pauseScreenShare() override;
    void resumeScreenShare() override;
    bool isSharing() const override;

    void setShareMode(ShareMode mode) override;
    ShareMode shareMode() const override;

    void setEncodingFormat(EncodingFormat format) override;
    EncodingFormat encodingFormat() const override;

    void setQuality(IScreenCapture::CaptureQuality quality) override;
    IScreenCapture::CaptureQuality quality() const override;

    void setFrameRate(int frameRate) override;
    int frameRate() const override;

    void setBitrate(int bitrate) override;
    int bitrate() const override;

    void setConfiguration(const QVariantMap& config) override;
    QVariantMap configuration() const override;

    QVariantMap getStatistics() override;
    void resetStatistics() override;
    int getTotalFrames() const override;
    double getCurrentFPS() const override;

    // Mock-specific methods for testing
    void setMockScreens(const QStringList& screens);
    void setMockWindows(const QStringList& windows);
    void setMockStatus(Status status);
    void setMockReady(bool ready);
    void setMockSharing(bool sharing);
    void simulateError(const QString& error);
    void simulateStatusChange(Status newStatus);

    // Test helper methods
    int getInitializeCallCount() const { return m_initializeCallCount; }
    int getStartCallCount() const { return m_startCallCount; }
    int getStopCallCount() const { return m_stopCallCount; }
    QString getLastSelectedSource() const { return m_lastSelectedSource; }
    QVariantMap getLastConfiguration() const { return m_lastConfiguration; }

    // Legacy compatibility methods for testing
    bool migrateConfiguration(const QVariantMap& oldConfig);
    bool loadFromGlobalConfig(const QVariantMap& globalConfig);
    QStringList getLogMessages() const;

private:
    Status m_status;
    bool m_ready;
    bool m_sharing;
    ShareMode m_shareMode;
    EncodingFormat m_encodingFormat;
    IScreenCapture::CaptureQuality m_quality;
    int m_frameRate;
    int m_bitrate;
    QString m_currentSource;
    QVariantMap m_configuration;
    QStringList m_mockScreens;
    QStringList m_mockWindows;
    QStringList m_logMessages;

    // Test counters
    int m_initializeCallCount;
    int m_startCallCount;
    int m_stopCallCount;
    QString m_lastSelectedSource;
    QVariantMap m_lastConfiguration;

    // Mock statistics
    int m_totalFrames;
    double m_currentFPS;
    qint64 m_startTime;
};

#endif // MOCKSCREENSHAREMANAGER_H
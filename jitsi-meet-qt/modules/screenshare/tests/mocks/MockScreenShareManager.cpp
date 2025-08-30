#include "MockScreenShareManager.h"
#include <QDebug>
#include <QElapsedTimer>

MockScreenShareManager::MockScreenShareManager(QObject *parent)
    : IScreenShareManager(parent)
    , m_status(Uninitialized)
    , m_ready(false)
    , m_sharing(false)
    , m_shareMode(LocalShare)
    , m_encodingFormat(H264)
    , m_quality(IScreenCapture::MediumQuality)
    , m_frameRate(30)
    , m_bitrate(2000)
    , m_initializeCallCount(0)
    , m_startCallCount(0)
    , m_stopCallCount(0)
    , m_totalFrames(0)
    , m_currentFPS(0.0)
    , m_startTime(0)
{
    // 设置默认的模拟数据
    m_mockScreens << "Screen 1" << "Screen 2";
    m_mockWindows << "Desktop" << "Test Window" << "Browser Window";
    
    // 默认配置
    m_configuration["quality"] = static_cast<int>(m_quality);
    m_configuration["frameRate"] = m_frameRate;
    m_configuration["bitrate"] = m_bitrate;
    m_configuration["shareMode"] = static_cast<int>(m_shareMode);
    m_configuration["encodingFormat"] = static_cast<int>(m_encodingFormat);
}

MockScreenShareManager::~MockScreenShareManager()
{
}

bool MockScreenShareManager::initialize()
{
    m_initializeCallCount++;
    m_status = Ready;
    m_ready = true;
    
    m_logMessages.append("MockScreenShareManager initialized");
    
    emit statusChanged(m_status);
    return true;
}

IScreenShareManager::Status MockScreenShareManager::status() const
{
    return m_status;
}

bool MockScreenShareManager::isReady() const
{
    return m_ready;
}

QStringList MockScreenShareManager::availableScreens() const
{
    return m_mockScreens;
}

QStringList MockScreenShareManager::availableWindows() const
{
    return m_mockWindows;
}

bool MockScreenShareManager::selectScreen(const QString& screenId)
{
    if (!m_mockScreens.contains(screenId)) {
        return false;
    }
    
    m_currentSource = screenId;
    m_lastSelectedSource = screenId;
    m_logMessages.append(QString("Selected screen: %1").arg(screenId));
    
    emit sourceChanged(screenId);
    return true;
}

bool MockScreenShareManager::selectWindow(const QString& windowId)
{
    if (!m_mockWindows.contains(windowId)) {
        return false;
    }
    
    m_currentSource = windowId;
    m_lastSelectedSource = windowId;
    m_logMessages.append(QString("Selected window: %1").arg(windowId));
    
    emit sourceChanged(windowId);
    return true;
}

QString MockScreenShareManager::currentSource() const
{
    return m_currentSource;
}

bool MockScreenShareManager::startScreenShare()
{
    if (!m_ready) {
        emit shareError("Manager not ready");
        return false;
    }
    
    if (m_sharing) {
        return true; // Already sharing
    }
    
    m_startCallCount++;
    m_sharing = true;
    m_status = Active;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    
    m_logMessages.append("Screen share started");
    
    emit shareStarted();
    emit statusChanged(m_status);
    
    return true;
}

void MockScreenShareManager::stopScreenShare()
{
    if (!m_sharing) {
        return; // Not sharing
    }
    
    m_stopCallCount++;
    m_sharing = false;
    m_status = Ready;
    
    m_logMessages.append("Screen share stopped");
    
    emit shareStopped();
    emit statusChanged(m_status);
}

void MockScreenShareManager::pauseScreenShare()
{
    if (!m_sharing) {
        return;
    }
    
    m_status = Paused;
    m_logMessages.append("Screen share paused");
    
    emit sharePaused();
    emit statusChanged(m_status);
}

void MockScreenShareManager::resumeScreenShare()
{
    if (m_status != Paused) {
        return;
    }
    
    m_status = Active;
    m_logMessages.append("Screen share resumed");
    
    emit shareResumed();
    emit statusChanged(m_status);
}

bool MockScreenShareManager::isSharing() const
{
    return m_sharing;
}

void MockScreenShareManager::setShareMode(ShareMode mode)
{
    m_shareMode = mode;
    m_configuration["shareMode"] = static_cast<int>(mode);
    
    emit configurationChanged();
}

IScreenShareManager::ShareMode MockScreenShareManager::shareMode() const
{
    return m_shareMode;
}

void MockScreenShareManager::setEncodingFormat(EncodingFormat format)
{
    m_encodingFormat = format;
    m_configuration["encodingFormat"] = static_cast<int>(format);
    
    emit configurationChanged();
}

IScreenShareManager::EncodingFormat MockScreenShareManager::encodingFormat() const
{
    return m_encodingFormat;
}

void MockScreenShareManager::setQuality(IScreenCapture::CaptureQuality quality)
{
    m_quality = quality;
    m_configuration["quality"] = static_cast<int>(quality);
    
    emit configurationChanged();
}

IScreenCapture::CaptureQuality MockScreenShareManager::quality() const
{
    return m_quality;
}

void MockScreenShareManager::setFrameRate(int frameRate)
{
    m_frameRate = frameRate;
    m_configuration["frameRate"] = frameRate;
    
    emit configurationChanged();
}

int MockScreenShareManager::frameRate() const
{
    return m_frameRate;
}

void MockScreenShareManager::setBitrate(int bitrate)
{
    m_bitrate = bitrate;
    m_configuration["bitrate"] = bitrate;
    
    emit configurationChanged();
}

int MockScreenShareManager::bitrate() const
{
    return m_bitrate;
}

void MockScreenShareManager::setConfiguration(const QVariantMap& config)
{
    m_configuration = config;
    m_lastConfiguration = config;
    
    // 应用配置
    if (config.contains("quality")) {
        m_quality = static_cast<IScreenCapture::CaptureQuality>(config["quality"].toInt());
    }
    if (config.contains("frameRate")) {
        m_frameRate = config["frameRate"].toInt();
    }
    if (config.contains("bitrate")) {
        m_bitrate = config["bitrate"].toInt();
    }
    if (config.contains("shareMode")) {
        m_shareMode = static_cast<ShareMode>(config["shareMode"].toInt());
    }
    if (config.contains("encodingFormat")) {
        m_encodingFormat = static_cast<EncodingFormat>(config["encodingFormat"].toInt());
    }
    
    emit configurationChanged();
}

QVariantMap MockScreenShareManager::configuration() const
{
    return m_configuration;
}

QVariantMap MockScreenShareManager::getStatistics()
{
    QVariantMap stats;
    
    // 模拟统计数据
    if (m_sharing && m_startTime > 0) {
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_startTime;
        m_currentFPS = m_totalFrames * 1000.0 / elapsed;
        m_totalFrames += m_frameRate / 10; // 模拟帧数增长
    }
    
    stats["totalFrames"] = m_totalFrames;
    stats["currentFPS"] = m_currentFPS;
    stats["currentBitrate"] = m_bitrate;
    stats["quality"] = static_cast<int>(m_quality);
    stats["frameRate"] = m_frameRate;
    stats["isSharing"] = m_sharing;
    stats["uptime"] = m_sharing ? (QDateTime::currentMSecsSinceEpoch() - m_startTime) : 0;
    
    return stats;
}

void MockScreenShareManager::resetStatistics()
{
    m_totalFrames = 0;
    m_currentFPS = 0.0;
    m_startTime = QDateTime::currentMSecsSinceEpoch();
    
    m_logMessages.append("Statistics reset");
}

int MockScreenShareManager::getTotalFrames() const
{
    return m_totalFrames;
}

double MockScreenShareManager::getCurrentFPS() const
{
    return m_currentFPS;
}

// Mock-specific methods
void MockScreenShareManager::setMockScreens(const QStringList& screens)
{
    m_mockScreens = screens;
}

void MockScreenShareManager::setMockWindows(const QStringList& windows)
{
    m_mockWindows = windows;
}

void MockScreenShareManager::setMockStatus(Status status)
{
    m_status = status;
    emit statusChanged(status);
}

void MockScreenShareManager::setMockReady(bool ready)
{
    m_ready = ready;
}

void MockScreenShareManager::setMockSharing(bool sharing)
{
    m_sharing = sharing;
}

void MockScreenShareManager::simulateError(const QString& error)
{
    m_logMessages.append(QString("Error: %1").arg(error));
    emit shareError(error);
}

void MockScreenShareManager::simulateStatusChange(Status newStatus)
{
    m_status = newStatus;
    emit statusChanged(newStatus);
}

// Legacy compatibility methods
bool MockScreenShareManager::migrateConfiguration(const QVariantMap& oldConfig)
{
    QVariantMap newConfig;
    
    // 迁移旧配置格式
    if (oldConfig.contains("screen_id")) {
        newConfig["screenId"] = oldConfig["screen_id"];
    }
    if (oldConfig.contains("quality_level")) {
        newConfig["quality"] = oldConfig["quality_level"];
    }
    if (oldConfig.contains("frame_rate")) {
        newConfig["frameRate"] = oldConfig["frame_rate"];
    }
    if (oldConfig.contains("enable_audio")) {
        newConfig["audioEnabled"] = oldConfig["enable_audio"];
    }
    
    setConfiguration(newConfig);
    m_logMessages.append("Configuration migrated from legacy format");
    
    return true;
}

bool MockScreenShareManager::loadFromGlobalConfig(const QVariantMap& globalConfig)
{
    QVariantMap config;
    
    // 从全局配置加载屏幕共享设置
    if (globalConfig.contains("screenshare.defaultQuality")) {
        config["quality"] = globalConfig["screenshare.defaultQuality"];
    }
    if (globalConfig.contains("screenshare.defaultFrameRate")) {
        config["frameRate"] = globalConfig["screenshare.defaultFrameRate"];
    }
    if (globalConfig.contains("screenshare.enabled")) {
        config["enabled"] = globalConfig["screenshare.enabled"];
    }
    
    setConfiguration(config);
    m_logMessages.append("Configuration loaded from global config");
    
    return true;
}

QStringList MockScreenShareManager::getLogMessages() const
{
    return m_logMessages;
}
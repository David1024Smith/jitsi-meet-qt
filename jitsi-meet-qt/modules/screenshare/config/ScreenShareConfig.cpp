#include "ScreenShareConfig.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

class ScreenShareConfig::Private
{
public:
    Private()
        : enabled(true)
        , captureMode(IScreenCapture::FullScreen)
        , quality(IScreenCapture::MediumQuality)
        , qualityPreset(Balanced)
        , frameRate(30)
        , minFrameRate(5)
        , maxFrameRate(60)
        , bitrate(2000)
        , minBitrate(500)
        , maxBitrate(10000)
        , resolution(1920, 1080)
        , maxResolution(3840, 2160)
        , maintainAspectRatio(true)
        , encodingFormat(IScreenShareManager::H264)
        , shareMode(IScreenShareManager::NetworkShare)
        , keyFrameInterval(30)
        , networkAdaptation(Automatic)
        , autoQualityAdjustment(true)
        , adaptationInterval(5)
        , hardwareAcceleration(true)
        , bufferSize(1024)
        , threadCount(0) // 0 = auto
        , enableCursor(true)
        , enableAudio(false)
        , captureDelay(0)
    {
    }

    bool enabled;
    CaptureMode captureMode;
    CaptureQuality quality;
    QualityPreset qualityPreset;
    
    int frameRate;
    int minFrameRate;
    int maxFrameRate;
    int bitrate;
    int minBitrate;
    int maxBitrate;
    
    QSize resolution;
    QSize maxResolution;
    bool maintainAspectRatio;
    QRect captureRegion;
    QString targetScreen;
    QString targetWindow;
    
    EncodingFormat encodingFormat;
    ShareMode shareMode;
    int keyFrameInterval;
    
    NetworkAdaptation networkAdaptation;
    bool autoQualityAdjustment;
    int adaptationInterval;
    
    bool hardwareAcceleration;
    int bufferSize;
    int threadCount;
    
    bool enableCursor;
    bool enableAudio;
    int captureDelay;
};

ScreenShareConfig::ScreenShareConfig(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    initializeDefaults();
}

ScreenShareConfig::~ScreenShareConfig()
{
    delete d;
}

bool ScreenShareConfig::isEnabled() const
{
    return d->enabled;
}

void ScreenShareConfig::setEnabled(bool enabled)
{
    if (d->enabled != enabled) {
        d->enabled = enabled;
        emit enabledChanged(enabled);
        emit configurationChanged();
    }
}

bool ScreenShareConfig::isValid() const
{
    return validate();
}

void ScreenShareConfig::reset()
{
    initializeDefaults();
    emit configurationChanged();
}

ScreenShareConfig::CaptureMode ScreenShareConfig::captureMode() const
{
    return d->captureMode;
}

void ScreenShareConfig::setCaptureMode(CaptureMode mode)
{
    if (d->captureMode != mode) {
        d->captureMode = mode;
        emit captureModeChanged(mode);
        emit configurationChanged();
    }
}

ScreenShareConfig::CaptureQuality ScreenShareConfig::quality() const
{
    return d->quality;
}

void ScreenShareConfig::setQuality(CaptureQuality quality)
{
    if (d->quality != quality) {
        d->quality = quality;
        emit qualityChanged(quality);
        emit configurationChanged();
    }
}

int ScreenShareConfig::frameRate() const
{
    return d->frameRate;
}

void ScreenShareConfig::setFrameRate(int fps)
{
    fps = qBound(d->minFrameRate, fps, d->maxFrameRate);
    if (d->frameRate != fps) {
        d->frameRate = fps;
        emit frameRateChanged(fps);
        emit configurationChanged();
    }
}

int ScreenShareConfig::bitrate() const
{
    return d->bitrate;
}

void ScreenShareConfig::setBitrate(int kbps)
{
    kbps = qBound(d->minBitrate, kbps, d->maxBitrate);
    if (d->bitrate != kbps) {
        d->bitrate = kbps;
        emit bitrateChanged(kbps);
        emit configurationChanged();
    }
}

QSize ScreenShareConfig::resolution() const
{
    return d->resolution;
}

void ScreenShareConfig::setResolution(const QSize& size)
{
    if (d->resolution != size) {
        d->resolution = size;
        emit configurationChanged();
    }
}

QVariantMap ScreenShareConfig::toVariantMap() const
{
    QVariantMap map;
    
    // 基础配置
    map["enabled"] = d->enabled;
    map["captureMode"] = static_cast<int>(d->captureMode);
    map["quality"] = static_cast<int>(d->quality);
    map["qualityPreset"] = static_cast<int>(d->qualityPreset);
    
    // 帧率和比特率
    map["frameRate"] = d->frameRate;
    map["minFrameRate"] = d->minFrameRate;
    map["maxFrameRate"] = d->maxFrameRate;
    map["bitrate"] = d->bitrate;
    map["minBitrate"] = d->minBitrate;
    map["maxBitrate"] = d->maxBitrate;
    
    // 分辨率
    map["resolution"] = d->resolution;
    map["maxResolution"] = d->maxResolution;
    map["maintainAspectRatio"] = d->maintainAspectRatio;
    
    // 捕获区域
    map["captureRegion"] = d->captureRegion;
    map["targetScreen"] = d->targetScreen;
    map["targetWindow"] = d->targetWindow;
    
    // 编码配置
    map["encodingFormat"] = static_cast<int>(d->encodingFormat);
    map["shareMode"] = static_cast<int>(d->shareMode);
    map["keyFrameInterval"] = d->keyFrameInterval;
    
    // 网络适应
    map["networkAdaptation"] = static_cast<int>(d->networkAdaptation);
    map["autoQualityAdjustment"] = d->autoQualityAdjustment;
    map["adaptationInterval"] = d->adaptationInterval;
    
    // 性能配置
    map["hardwareAcceleration"] = d->hardwareAcceleration;
    map["bufferSize"] = d->bufferSize;
    map["threadCount"] = d->threadCount;
    
    // 高级配置
    map["enableCursor"] = d->enableCursor;
    map["enableAudio"] = d->enableAudio;
    map["captureDelay"] = d->captureDelay;
    
    return map;
}

void ScreenShareConfig::fromVariantMap(const QVariantMap& map)
{
    // 基础配置
    if (map.contains("enabled")) {
        setEnabled(map["enabled"].toBool());
    }
    if (map.contains("captureMode")) {
        setCaptureMode(static_cast<CaptureMode>(map["captureMode"].toInt()));
    }
    if (map.contains("quality")) {
        setQuality(static_cast<CaptureQuality>(map["quality"].toInt()));
    }
    if (map.contains("qualityPreset")) {
        setQualityPreset(static_cast<QualityPreset>(map["qualityPreset"].toInt()));
    }
    
    // 帧率和比特率
    if (map.contains("frameRate")) {
        setFrameRate(map["frameRate"].toInt());
    }
    if (map.contains("minFrameRate")) {
        setMinFrameRate(map["minFrameRate"].toInt());
    }
    if (map.contains("maxFrameRate")) {
        setMaxFrameRate(map["maxFrameRate"].toInt());
    }
    if (map.contains("bitrate")) {
        setBitrate(map["bitrate"].toInt());
    }
    if (map.contains("minBitrate")) {
        setMinBitrate(map["minBitrate"].toInt());
    }
    if (map.contains("maxBitrate")) {
        setMaxBitrate(map["maxBitrate"].toInt());
    }
    
    // 分辨率
    if (map.contains("resolution")) {
        setResolution(map["resolution"].toSize());
    }
    if (map.contains("maxResolution")) {
        setMaxResolution(map["maxResolution"].toSize());
    }
    if (map.contains("maintainAspectRatio")) {
        setMaintainAspectRatio(map["maintainAspectRatio"].toBool());
    }
    
    // 捕获区域
    if (map.contains("captureRegion")) {
        setCaptureRegion(map["captureRegion"].toRect());
    }
    if (map.contains("targetScreen")) {
        setTargetScreen(map["targetScreen"].toString());
    }
    if (map.contains("targetWindow")) {
        setTargetWindow(map["targetWindow"].toString());
    }
    
    // 编码配置
    if (map.contains("encodingFormat")) {
        setEncodingFormat(static_cast<EncodingFormat>(map["encodingFormat"].toInt()));
    }
    if (map.contains("shareMode")) {
        setShareMode(static_cast<ShareMode>(map["shareMode"].toInt()));
    }
    if (map.contains("keyFrameInterval")) {
        setKeyFrameInterval(map["keyFrameInterval"].toInt());
    }
    
    // 网络适应
    if (map.contains("networkAdaptation")) {
        setNetworkAdaptation(static_cast<NetworkAdaptation>(map["networkAdaptation"].toInt()));
    }
    if (map.contains("autoQualityAdjustment")) {
        setAutoQualityAdjustment(map["autoQualityAdjustment"].toBool());
    }
    if (map.contains("adaptationInterval")) {
        setAdaptationInterval(map["adaptationInterval"].toInt());
    }
    
    // 性能配置
    if (map.contains("hardwareAcceleration")) {
        setHardwareAcceleration(map["hardwareAcceleration"].toBool());
    }
    if (map.contains("bufferSize")) {
        setBufferSize(map["bufferSize"].toInt());
    }
    if (map.contains("threadCount")) {
        setThreadCount(map["threadCount"].toInt());
    }
    
    // 高级配置
    if (map.contains("enableCursor")) {
        setEnableCursor(map["enableCursor"].toBool());
    }
    if (map.contains("enableAudio")) {
        setEnableAudio(map["enableAudio"].toBool());
    }
    if (map.contains("captureDelay")) {
        setCaptureDelay(map["captureDelay"].toInt());
    }
    
    emit configurationChanged();
}

bool ScreenShareConfig::validate(QString* errorMessage) const
{
    QStringList errors = getValidationErrors();
    
    if (!errors.isEmpty()) {
        if (errorMessage) {
            *errorMessage = errors.join("; ");
        }
        return false;
    }
    
    return true;
}

QStringList ScreenShareConfig::getValidationErrors() const
{
    QStringList errors;
    
    // 验证帧率
    if (d->frameRate < 1 || d->frameRate > 120) {
        errors.append("Invalid frame rate");
    }
    
    // 验证比特率
    if (d->bitrate < 100 || d->bitrate > 50000) {
        errors.append("Invalid bitrate");
    }
    
    // 验证分辨率
    if (d->resolution.width() < 1 || d->resolution.height() < 1) {
        errors.append("Invalid resolution");
    }
    
    return errors;
}

void ScreenShareConfig::apply()
{
    if (validate()) {
        emit configurationChanged();
    } else {
        QStringList errors = getValidationErrors();
        emit validationFailed(errors);
    }
}

void ScreenShareConfig::restoreDefaults()
{
    reset();
}

void ScreenShareConfig::optimizeForSystem()
{
    // 根据系统性能优化配置
    // 这里可以添加系统检测逻辑
    
    emit configurationChanged();
}

// 质量预设相关方法
ScreenShareConfig::QualityPreset ScreenShareConfig::qualityPreset() const
{
    return d->qualityPreset;
}

void ScreenShareConfig::setQualityPreset(QualityPreset preset)
{
    if (d->qualityPreset != preset) {
        d->qualityPreset = preset;
        applyPreset(preset);
        emit configurationChanged();
    }
}

// 帧率范围方法
int ScreenShareConfig::minFrameRate() const
{
    return d->minFrameRate;
}

void ScreenShareConfig::setMinFrameRate(int fps)
{
    if (d->minFrameRate != fps) {
        d->minFrameRate = fps;
        emit configurationChanged();
    }
}

int ScreenShareConfig::maxFrameRate() const
{
    return d->maxFrameRate;
}

void ScreenShareConfig::setMaxFrameRate(int fps)
{
    if (d->maxFrameRate != fps) {
        d->maxFrameRate = fps;
        emit configurationChanged();
    }
}

// 比特率范围方法
int ScreenShareConfig::minBitrate() const
{
    return d->minBitrate;
}

void ScreenShareConfig::setMinBitrate(int kbps)
{
    if (d->minBitrate != kbps) {
        d->minBitrate = kbps;
        emit configurationChanged();
    }
}

int ScreenShareConfig::maxBitrate() const
{
    return d->maxBitrate;
}

void ScreenShareConfig::setMaxBitrate(int kbps)
{
    if (d->maxBitrate != kbps) {
        d->maxBitrate = kbps;
        emit configurationChanged();
    }
}

// 分辨率相关方法
QSize ScreenShareConfig::maxResolution() const
{
    return d->maxResolution;
}

void ScreenShareConfig::setMaxResolution(const QSize& size)
{
    if (d->maxResolution != size) {
        d->maxResolution = size;
        emit configurationChanged();
    }
}

bool ScreenShareConfig::maintainAspectRatio() const
{
    return d->maintainAspectRatio;
}

void ScreenShareConfig::setMaintainAspectRatio(bool maintain)
{
    if (d->maintainAspectRatio != maintain) {
        d->maintainAspectRatio = maintain;
        emit configurationChanged();
    }
}

// 捕获区域方法
QRect ScreenShareConfig::captureRegion() const
{
    return d->captureRegion;
}

void ScreenShareConfig::setCaptureRegion(const QRect& region)
{
    if (d->captureRegion != region) {
        d->captureRegion = region;
        emit configurationChanged();
    }
}

QString ScreenShareConfig::targetScreen() const
{
    return d->targetScreen;
}

void ScreenShareConfig::setTargetScreen(const QString& screenId)
{
    if (d->targetScreen != screenId) {
        d->targetScreen = screenId;
        emit configurationChanged();
    }
}

QString ScreenShareConfig::targetWindow() const
{
    return d->targetWindow;
}

void ScreenShareConfig::setTargetWindow(const QString& windowId)
{
    if (d->targetWindow != windowId) {
        d->targetWindow = windowId;
        emit configurationChanged();
    }
}

// 编码配置方法
ScreenShareConfig::EncodingFormat ScreenShareConfig::encodingFormat() const
{
    return d->encodingFormat;
}

void ScreenShareConfig::setEncodingFormat(EncodingFormat format)
{
    if (d->encodingFormat != format) {
        d->encodingFormat = format;
        emit configurationChanged();
    }
}

ScreenShareConfig::ShareMode ScreenShareConfig::shareMode() const
{
    return d->shareMode;
}

void ScreenShareConfig::setShareMode(ShareMode mode)
{
    if (d->shareMode != mode) {
        d->shareMode = mode;
        emit configurationChanged();
    }
}

int ScreenShareConfig::keyFrameInterval() const
{
    return d->keyFrameInterval;
}

void ScreenShareConfig::setKeyFrameInterval(int interval)
{
    if (d->keyFrameInterval != interval) {
        d->keyFrameInterval = interval;
        emit configurationChanged();
    }
}

// 网络适应方法
ScreenShareConfig::NetworkAdaptation ScreenShareConfig::networkAdaptation() const
{
    return d->networkAdaptation;
}

void ScreenShareConfig::setNetworkAdaptation(NetworkAdaptation adaptation)
{
    if (d->networkAdaptation != adaptation) {
        d->networkAdaptation = adaptation;
        emit configurationChanged();
    }
}

bool ScreenShareConfig::autoQualityAdjustment() const
{
    return d->autoQualityAdjustment;
}

void ScreenShareConfig::setAutoQualityAdjustment(bool enabled)
{
    if (d->autoQualityAdjustment != enabled) {
        d->autoQualityAdjustment = enabled;
        emit configurationChanged();
    }
}

int ScreenShareConfig::adaptationInterval() const
{
    return d->adaptationInterval;
}

void ScreenShareConfig::setAdaptationInterval(int seconds)
{
    if (d->adaptationInterval != seconds) {
        d->adaptationInterval = seconds;
        emit configurationChanged();
    }
}

// 性能配置方法
bool ScreenShareConfig::hardwareAcceleration() const
{
    return d->hardwareAcceleration;
}

void ScreenShareConfig::setHardwareAcceleration(bool enabled)
{
    if (d->hardwareAcceleration != enabled) {
        d->hardwareAcceleration = enabled;
        emit configurationChanged();
    }
}

int ScreenShareConfig::bufferSize() const
{
    return d->bufferSize;
}

void ScreenShareConfig::setBufferSize(int size)
{
    if (d->bufferSize != size) {
        d->bufferSize = size;
        emit configurationChanged();
    }
}

int ScreenShareConfig::threadCount() const
{
    return d->threadCount;
}

void ScreenShareConfig::setThreadCount(int count)
{
    if (d->threadCount != count) {
        d->threadCount = count;
        emit configurationChanged();
    }
}

// 高级配置方法
bool ScreenShareConfig::enableCursor() const
{
    return d->enableCursor;
}

void ScreenShareConfig::setEnableCursor(bool enabled)
{
    if (d->enableCursor != enabled) {
        d->enableCursor = enabled;
        emit configurationChanged();
    }
}

bool ScreenShareConfig::enableAudio() const
{
    return d->enableAudio;
}

void ScreenShareConfig::setEnableAudio(bool enabled)
{
    if (d->enableAudio != enabled) {
        d->enableAudio = enabled;
        emit configurationChanged();
    }
}

int ScreenShareConfig::captureDelay() const
{
    return d->captureDelay;
}

void ScreenShareConfig::setCaptureDelay(int ms)
{
    if (d->captureDelay != ms) {
        d->captureDelay = ms;
        emit configurationChanged();
    }
}

// 文件操作方法
bool ScreenShareConfig::loadFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool ScreenShareConfig::saveToFile(const QString& filePath) const
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    file.write(doc.toJson());
    return true;
}

// 预设配置方法
void ScreenShareConfig::applyPreset(QualityPreset preset)
{
    QVariantMap config = getPresetConfiguration(preset);
    fromVariantMap(config);
}

QVariantMap ScreenShareConfig::getPresetConfiguration(QualityPreset preset)
{
    QVariantMap config;
    
    switch (preset) {
    case PowerSaving:
        config["frameRate"] = 15;
        config["bitrate"] = 500;
        config["resolution"] = QSize(1280, 720);
        config["quality"] = static_cast<int>(IScreenCapture::LowQuality);
        config["hardwareAcceleration"] = false;
        break;
        
    case Balanced:
        config["frameRate"] = 30;
        config["bitrate"] = 2000;
        config["resolution"] = QSize(1920, 1080);
        config["quality"] = static_cast<int>(IScreenCapture::MediumQuality);
        config["hardwareAcceleration"] = true;
        break;
        
    case HighQuality:
        config["frameRate"] = 60;
        config["bitrate"] = 5000;
        config["resolution"] = QSize(1920, 1080);
        config["quality"] = static_cast<int>(IScreenCapture::HighQuality);
        config["hardwareAcceleration"] = true;
        break;
        
    case UltraQuality:
        config["frameRate"] = 60;
        config["bitrate"] = 10000;
        config["resolution"] = QSize(3840, 2160);
        config["quality"] = static_cast<int>(IScreenCapture::HighQuality);
        config["hardwareAcceleration"] = true;
        break;
        
    case Custom:
        // 保持当前设置
        break;
    }
    
    return config;
}

QStringList ScreenShareConfig::availablePresets() const
{
    return QStringList() << "PowerSaving" << "Balanced" << "HighQuality" << "UltraQuality" << "Custom";
}

void ScreenShareConfig::initializeDefaults()
{
    // 设置默认值已在Private构造函数中完成
}

void ScreenShareConfig::validateAndEmitChanges()
{
    if (!validate()) {
        QStringList errors = getValidationErrors();
        emit validationFailed(errors);
    }
}
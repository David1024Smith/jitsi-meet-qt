#include "RegionCapture.h"
#include <QApplication>
#include <QScreen>
#include <QPixmap>
#include <QTimer>
#include <QMutexLocker>
#include <QDebug>
#include <QCursor>
#include <QBuffer>
#include <QIODevice>

class RegionCapture::Private
{
public:
    Private()
        : status(IScreenCapture::Inactive)
        , initialized(false)
        , captureMode(IScreenCapture::Region)
        , quality(IScreenCapture::MediumQuality)
        , frameRate(30)
        , selectionMode(RegionCapture::ManualSelection)
        , boundaryMode(RegionCapture::Clip)
        , regionLocked(false)
        , interactiveSelectionActive(false)
        , currentPresetIndex(-1)
        , targetScreen(nullptr)
        , captureTimer(nullptr)
        , mousePositionTimer(nullptr)
        , mouseFollowSize(200, 200)
        , mouseFollowOffset(0, 0)
    {
    }

    IScreenCapture::CaptureStatus status;
    bool initialized;
    IScreenCapture::CaptureMode captureMode;
    IScreenCapture::CaptureQuality quality;
    int frameRate;
    
    RegionCapture::SelectionMode selectionMode;
    RegionCapture::BoundaryMode boundaryMode;
    bool regionLocked;
    bool interactiveSelectionActive;
    int currentPresetIndex;
    
    QScreen* targetScreen;
    QRect customRegion;
    QRect captureRegion;
    QList<QRect> presetRegions;
    
    QTimer* captureTimer;
    QTimer* mousePositionTimer;
    
    QSize mouseFollowSize;
    QPoint mouseFollowOffset;
    
    mutable QMutex mutex;
};

RegionCapture::RegionCapture(QObject *parent)
    : IScreenCapture(parent)
    , d(new Private)
{
    d->captureTimer = new QTimer(this);
    connect(d->captureTimer, &QTimer::timeout,
            this, &RegionCapture::onCaptureTimer);
    
    d->mousePositionTimer = new QTimer(this);
    connect(d->mousePositionTimer, &QTimer::timeout,
            this, &RegionCapture::onMousePositionTimer);
}

RegionCapture::~RegionCapture()
{
    stopCapture();
    delete d;
}

bool RegionCapture::initialize()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->initialized) {
        return true;
    }
    
    try {
        // 初始化区域捕获设置
        initializeCapture();
        
        d->initialized = true;
        updateStatus(Inactive);
        return true;
        
    } catch (const std::exception& e) {
        qWarning() << "RegionCapture initialization failed:" << e.what();
        return false;
    }
}

bool RegionCapture::startCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->initialized) {
        emitError("RegionCapture not initialized");
        return false;
    }
    
    if (d->status == Active) {
        return true;
    }
    
    updateStatus(Initializing);
    
    try {
        // 验证捕获区域
        if (d->customRegion.isEmpty()) {
            // 如果没有设置区域，使用全屏
            resetToFullScreen();
        }
        
        d->captureRegion = validateRegion(d->customRegion);
        if (d->captureRegion.isEmpty()) {
            throw std::runtime_error("Invalid capture region");
        }
        
        // 启动捕获定时器
        updateCaptureTimer();
        d->captureTimer->start();
        
        // 如果是跟随鼠标模式，启动鼠标位置定时器
        if (d->selectionMode == FollowMouse) {
            d->mousePositionTimer->start(50); // 20 FPS for mouse following
        }
        
        updateStatus(Active);
        emit captureStarted();
        return true;
        
    } catch (const std::exception& e) {
        emitError(QString("Failed to start region capture: %1").arg(e.what()));
        updateStatus(IScreenCapture::Error);
        return false;
    }
}

void RegionCapture::stopCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status == Inactive) {
        return;
    }
    
    // 停止定时器
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    if (d->mousePositionTimer) {
        d->mousePositionTimer->stop();
    }
    
    updateStatus(Inactive);
    emit captureStopped();
}

void RegionCapture::pauseCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Active) {
        return;
    }
    
    if (d->captureTimer) {
        d->captureTimer->stop();
    }
    if (d->mousePositionTimer) {
        d->mousePositionTimer->stop();
    }
    
    updateStatus(Paused);
    emit capturePaused();
}

void RegionCapture::resumeCapture()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->status != Paused) {
        return;
    }
    
    if (d->captureTimer) {
        d->captureTimer->start();
    }
    if (d->selectionMode == FollowMouse && d->mousePositionTimer) {
        d->mousePositionTimer->start();
    }
    
    updateStatus(Active);
    emit captureResumed();
}

IScreenCapture::CaptureStatus RegionCapture::status() const
{
    QMutexLocker locker(&d->mutex);
    return d->status;
}

bool RegionCapture::isCapturing() const
{
    QMutexLocker locker(&d->mutex);
    return d->status == Active;
}

bool RegionCapture::isInitialized() const
{
    QMutexLocker locker(&d->mutex);
    return d->initialized;
}

void RegionCapture::setCaptureMode(CaptureMode mode)
{
    QMutexLocker locker(&d->mutex);
    // 区域捕获只支持Region模式
    if (mode == Region) {
        d->captureMode = mode;
    }
}

IScreenCapture::CaptureMode RegionCapture::captureMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureMode;
}

void RegionCapture::setCaptureQuality(CaptureQuality quality)
{
    QMutexLocker locker(&d->mutex);
    d->quality = quality;
}

IScreenCapture::CaptureQuality RegionCapture::captureQuality() const
{
    QMutexLocker locker(&d->mutex);
    return d->quality;
}

void RegionCapture::setFrameRate(int fps)
{
    QMutexLocker locker(&d->mutex);
    if (fps > 0 && fps <= 120 && d->frameRate != fps) {
        d->frameRate = fps;
        updateCaptureTimer();
    }
}

int RegionCapture::frameRate() const
{
    QMutexLocker locker(&d->mutex);
    return d->frameRate;
}

void RegionCapture::setCaptureRegion(const QRect& region)
{
    QMutexLocker locker(&d->mutex);
    setCustomRegion(region);
}

QRect RegionCapture::captureRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureRegion;
}

void RegionCapture::setTargetScreen(QScreen* screen)
{
    QMutexLocker locker(&d->mutex);
    if (d->targetScreen != screen) {
        d->targetScreen = screen;
        
        // 如果区域超出新屏幕范围，调整区域
        if (screen && !d->customRegion.isEmpty()) {
            adjustToScreenBounds();
        }
    }
}

QScreen* RegionCapture::targetScreen() const
{
    QMutexLocker locker(&d->mutex);
    return d->targetScreen;
}

QPixmap RegionCapture::captureFrame()
{
    QMutexLocker locker(&d->mutex);
    
    if (!isCapturing()) {
        return QPixmap();
    }
    
    try {
        QPixmap frame = captureRegionInternal();
        if (!frame.isNull()) {
            emit frameCaptured(frame);
        }
        return frame;
        
    } catch (const std::exception& e) {
        emitError(QString("Region frame capture failed: %1").arg(e.what()));
        return QPixmap();
    }
}

QByteArray RegionCapture::captureFrameData()
{
    QPixmap frame = captureFrame();
    if (frame.isNull()) {
        return QByteArray();
    }
    
    QByteArray data;
    QBuffer buffer(&data);
    buffer.open(QIODevice::WriteOnly);
    
    QString format = (d->quality == UltraQuality) ? "PNG" : "JPEG";
    int quality = (d->quality == LowQuality) ? 50 : 
                  (d->quality == MediumQuality) ? 75 : 90;
    
    frame.save(&buffer, format.toUtf8().constData(), quality);
    return data;
}

QSize RegionCapture::captureSize() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureRegion.size();
}

QRect RegionCapture::customRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->customRegion;
}

void RegionCapture::setCustomRegion(const QRect& region)
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->regionLocked && d->customRegion != region) {
        d->customRegion = region;
        
        // 验证并调整区域
        if (d->initialized) {
            d->captureRegion = validateRegion(region);
        }
        
        emit customRegionChanged(region);
    }
}

RegionCapture::SelectionMode RegionCapture::selectionMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->selectionMode;
}

void RegionCapture::setSelectionMode(SelectionMode mode)
{
    QMutexLocker locker(&d->mutex);
    if (d->selectionMode != mode) {
        d->selectionMode = mode;
        
        // 根据模式调整行为
        if (mode == FollowMouse && isCapturing()) {
            d->mousePositionTimer->start(50);
        } else {
            d->mousePositionTimer->stop();
        }
    }
}

RegionCapture::BoundaryMode RegionCapture::boundaryMode() const
{
    QMutexLocker locker(&d->mutex);
    return d->boundaryMode;
}

void RegionCapture::setBoundaryMode(BoundaryMode mode)
{
    QMutexLocker locker(&d->mutex);
    d->boundaryMode = mode;
}

bool RegionCapture::isRegionLocked() const
{
    QMutexLocker locker(&d->mutex);
    return d->regionLocked;
}

void RegionCapture::setRegionLocked(bool locked)
{
    QMutexLocker locker(&d->mutex);
    if (d->regionLocked != locked) {
        d->regionLocked = locked;
        emit regionLockedChanged(locked);
    }
}

bool RegionCapture::isRegionValid() const
{
    QMutexLocker locker(&d->mutex);
    return !d->captureRegion.isEmpty();
}

QRect RegionCapture::adjustedRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->captureRegion;
}

QRect RegionCapture::normalizedRegion() const
{
    QMutexLocker locker(&d->mutex);
    return d->customRegion.normalized();
}

void RegionCapture::startInteractiveSelection()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->interactiveSelectionActive) {
        d->interactiveSelectionActive = true;
        emit interactiveSelectionStarted();
        
        // 这里应该启动交互式选择界面
        // 简化实现：直接使用当前区域
        QTimer::singleShot(1000, this, [this]() {
            onInteractiveSelectionUpdate();
        });
    }
}

void RegionCapture::cancelInteractiveSelection()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->interactiveSelectionActive) {
        d->interactiveSelectionActive = false;
        emit interactiveSelectionCancelled();
    }
}

bool RegionCapture::isInteractiveSelectionActive() const
{
    QMutexLocker locker(&d->mutex);
    return d->interactiveSelectionActive;
}

void RegionCapture::setPresetRegions(const QList<QRect>& regions)
{
    QMutexLocker locker(&d->mutex);
    d->presetRegions = regions;
    d->currentPresetIndex = -1;
}

QList<QRect> RegionCapture::presetRegions() const
{
    QMutexLocker locker(&d->mutex);
    return d->presetRegions;
}

bool RegionCapture::selectPresetRegion(int index)
{
    QMutexLocker locker(&d->mutex);
    
    if (index >= 0 && index < d->presetRegions.size()) {
        d->currentPresetIndex = index;
        setCustomRegion(d->presetRegions[index]);
        return true;
    }
    
    return false;
}

int RegionCapture::currentPresetIndex() const
{
    QMutexLocker locker(&d->mutex);
    return d->currentPresetIndex;
}

void RegionCapture::setMouseFollowSize(const QSize& size)
{
    QMutexLocker locker(&d->mutex);
    if (size.isValid()) {
        d->mouseFollowSize = size;
    }
}

QSize RegionCapture::mouseFollowSize() const
{
    QMutexLocker locker(&d->mutex);
    return d->mouseFollowSize;
}

void RegionCapture::setMouseFollowOffset(const QPoint& offset)
{
    QMutexLocker locker(&d->mutex);
    d->mouseFollowOffset = offset;
}

QPoint RegionCapture::mouseFollowOffset() const
{
    QMutexLocker locker(&d->mutex);
    return d->mouseFollowOffset;
}

void RegionCapture::resetToFullScreen()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->targetScreen) {
        d->targetScreen = QApplication::primaryScreen();
    }
    
    if (d->targetScreen) {
        setCustomRegion(d->targetScreen->geometry());
    }
}

void RegionCapture::centerRegion()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->targetScreen || d->customRegion.isEmpty()) {
        return;
    }
    
    QRect screenRect = d->targetScreen->geometry();
    QRect centeredRect = d->customRegion;
    
    centeredRect.moveCenter(screenRect.center());
    setCustomRegion(centeredRect);
}

void RegionCapture::adjustToScreenBounds()
{
    QMutexLocker locker(&d->mutex);
    
    if (!d->targetScreen || d->customRegion.isEmpty()) {
        return;
    }
    
    QRect screenRect = d->targetScreen->geometry();
    QRect adjustedRect = d->customRegion.intersected(screenRect);
    
    if (adjustedRect != d->customRegion) {
        setCustomRegion(adjustedRect);
    }
}

void RegionCapture::startRegionSelection()
{
    startInteractiveSelection();
}

void RegionCapture::onCaptureTimer()
{
    captureFrame();
}

void RegionCapture::onMousePositionTimer()
{
    if (d->selectionMode == FollowMouse) {
        updateMouseFollowRegion();
    }
}

void RegionCapture::onInteractiveSelectionUpdate()
{
    QMutexLocker locker(&d->mutex);
    
    if (d->interactiveSelectionActive) {
        // 简化实现：使用当前区域完成选择
        QRect selectedRegion = d->customRegion;
        d->interactiveSelectionActive = false;
        
        emit interactiveSelectionFinished(selectedRegion);
    }
}

void RegionCapture::initializeCapture()
{
    // 设置默认屏幕
    if (!d->targetScreen) {
        d->targetScreen = QApplication::primaryScreen();
    }
    
    // 设置默认区域
    if (d->customRegion.isEmpty() && d->targetScreen) {
        d->customRegion = d->targetScreen->geometry();
    }
}

void RegionCapture::updateCaptureTimer()
{
    if (d->captureTimer && d->frameRate > 0) {
        int interval = 1000 / d->frameRate;
        d->captureTimer->setInterval(interval);
    }
}

void RegionCapture::updateStatus(CaptureStatus newStatus)
{
    if (d->status != newStatus) {
        d->status = newStatus;
        emit statusChanged(newStatus);
    }
}

QPixmap RegionCapture::captureRegionInternal()
{
    if (!d->targetScreen || d->captureRegion.isEmpty()) {
        return QPixmap();
    }
    
    // 捕获指定区域
    QPixmap screenshot = d->targetScreen->grabWindow(0,
                                                    d->captureRegion.x(),
                                                    d->captureRegion.y(),
                                                    d->captureRegion.width(),
                                                    d->captureRegion.height());
    
    return screenshot;
}

QRect RegionCapture::validateRegion(const QRect& region) const
{
    if (region.isEmpty()) {
        return QRect();
    }
    
    QRect validRegion = region.normalized();
    
    // 应用边界模式
    validRegion = applyBoundaryMode(validRegion);
    
    return validRegion;
}

QRect RegionCapture::applyBoundaryMode(const QRect& region) const
{
    if (!d->targetScreen) {
        return region;
    }
    
    QRect screenRect = d->targetScreen->geometry();
    QRect result = region;
    
    switch (d->boundaryMode) {
    case Clip:
        result = region.intersected(screenRect);
        break;
    case Extend:
        // 扩展到屏幕边界
        if (region.intersects(screenRect)) {
            result = region.united(screenRect);
        }
        break;
    case Wrap:
        // 环绕处理（简化实现）
        result = region;
        break;
    case Error:
        // 如果超出边界则返回空区域
        if (!screenRect.contains(region)) {
            result = QRect();
        }
        break;
    }
    
    return result;
}

void RegionCapture::updateMouseFollowRegion()
{
    QPoint mousePos = QCursor::pos();
    QPoint adjustedPos = mousePos + d->mouseFollowOffset;
    
    QRect newRegion(adjustedPos - QPoint(d->mouseFollowSize.width() / 2,
                                        d->mouseFollowSize.height() / 2),
                   d->mouseFollowSize);
    
    setCustomRegion(newRegion);
}

void RegionCapture::emitError(const QString& error)
{
    qWarning() << "RegionCapture error:" << error;
    emit captureError(error);
}
#include "CapturePreview.h"
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>

class CapturePreview::Private
{
public:
    Private()
        : previewEnabled(false)
        , previewMode(LivePreview)
        , scaleMode(FitToWidget)
        , refreshRate(30)
        , showControls(true)
        , showStatistics(false)
        , showCrosshair(false)
        , scaleFactor(1.0)
        , frameCount(0)
        , currentFPS(0.0)
        , lastUpdateTime(0)
        , refreshTimer(new QTimer)
    {
        refreshTimer->setInterval(1000 / refreshRate); // 默认30fps
    }

    // 预览配置
    bool previewEnabled;
    PreviewMode previewMode;
    ScaleMode scaleMode;
    int refreshRate;
    QSize previewSize;
    
    // 显示配置
    bool showControls;
    bool showStatistics;
    bool showCrosshair;
    
    // 预览内容
    QPixmap currentFrame;
    QSize originalSize;
    double scaleFactor;
    QRect scaledRect;
    
    // 统计信息
    int frameCount;
    double currentFPS;
    qint64 lastUpdateTime;
    
    // UI组件
    QVBoxLayout* mainLayout;
    QHBoxLayout* controlsLayout;
    QWidget* controlsWidget;
    QPushButton* playPauseButton;
    QPushButton* refreshButton;
    QPushButton* saveButton;
    QSlider* scaleSlider;
    QPushButton* fitToWidgetButton;
    QPushButton* originalSizeButton;
    QCheckBox* statisticsCheckBox;
    QCheckBox* crosshairCheckBox;
    
    // 定时器
    QTimer* refreshTimer;
};

CapturePreview::CapturePreview(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    setupUI();
    connectSignals();
    setMinimumSize(200, 150);
}

CapturePreview::~CapturePreview()
{
    delete d;
}

bool CapturePreview::isPreviewEnabled() const
{
    return d->previewEnabled;
}

void CapturePreview::setPreviewEnabled(bool enabled)
{
    if (d->previewEnabled != enabled) {
        d->previewEnabled = enabled;
        
        if (enabled) {
            if (d->previewMode == LivePreview) {
                d->refreshTimer->start();
            }
        } else {
            d->refreshTimer->stop();
        }
        
        updateControls();
        update();
        emit previewEnabledChanged(enabled);
    }
}

CapturePreview::PreviewMode CapturePreview::previewMode() const
{
    return d->previewMode;
}

void CapturePreview::setPreviewMode(PreviewMode mode)
{
    if (d->previewMode != mode) {
        d->previewMode = mode;
        
        if (d->previewEnabled) {
            if (mode == LivePreview) {
                d->refreshTimer->start();
            } else {
                d->refreshTimer->stop();
            }
        }
        
        updateControls();
    }
}

QSize CapturePreview::previewSize() const
{
    return d->previewSize;
}

void CapturePreview::setPreviewSize(const QSize& size)
{
    if (d->previewSize != size) {
        d->previewSize = size;
        calculateScaledSize();
        update();
        emit previewSizeChanged(size);
    }
}

CapturePreview::ScaleMode CapturePreview::scaleMode() const
{
    return d->scaleMode;
}

void CapturePreview::setScaleMode(ScaleMode mode)
{
    if (d->scaleMode != mode) {
        d->scaleMode = mode;
        calculateScaledSize();
        update();
    }
}

int CapturePreview::refreshRate() const
{
    return d->refreshRate;
}

void CapturePreview::setRefreshRate(int fps)
{
    if (d->refreshRate != fps) {
        d->refreshRate = fps;
        d->refreshTimer->setInterval(1000 / fps);
        emit refreshRateChanged(fps);
    }
}

bool CapturePreview::isShowControls() const
{
    return d->showControls;
}

void CapturePreview::setShowControls(bool show)
{
    d->showControls = show;
    d->controlsWidget->setVisible(show);
}

bool CapturePreview::isShowStatistics() const
{
    return d->showStatistics;
}

void CapturePreview::setShowStatistics(bool show)
{
    d->showStatistics = show;
    d->statisticsCheckBox->setChecked(show);
    update();
}

bool CapturePreview::isShowCrosshair() const
{
    return d->showCrosshair;
}

void CapturePreview::setShowCrosshair(bool show)
{
    d->showCrosshair = show;
    d->crosshairCheckBox->setChecked(show);
    update();
}

QPixmap CapturePreview::currentFrame() const
{
    return d->currentFrame;
}

void CapturePreview::setCurrentFrame(const QPixmap& frame)
{
    d->currentFrame = frame;
    d->originalSize = frame.size();
    d->frameCount++;
    d->lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    
    calculateScaledSize();
    update();
    
    emit frameUpdated(frame);
}

QSize CapturePreview::originalSize() const
{
    return d->originalSize;
}

double CapturePreview::scaleFactor() const
{
    return d->scaleFactor;
}

int CapturePreview::frameCount() const
{
    return d->frameCount;
}

double CapturePreview::currentFPS() const
{
    return d->currentFPS;
}

qint64 CapturePreview::lastUpdateTime() const
{
    return d->lastUpdateTime;
}

void CapturePreview::startPreview()
{
    setPreviewEnabled(true);
    emit previewStarted();
}

void CapturePreview::stopPreview()
{
    setPreviewEnabled(false);
    emit previewStopped();
}

void CapturePreview::pausePreview()
{
    if (d->previewEnabled) {
        d->refreshTimer->stop();
        updateControls();
    }
}

void CapturePreview::resumePreview()
{
    if (d->previewEnabled && d->previewMode == LivePreview) {
        d->refreshTimer->start();
        updateControls();
    }
}

void CapturePreview::refreshPreview()
{
    // 触发预览刷新
    updatePreview();
}

void CapturePreview::captureCurrentFrame()
{
    // 捕获当前帧 - 这里应该从捕获引擎获取
    // 暂时使用当前显示的帧
    if (!d->currentFrame.isNull()) {
        emit frameUpdated(d->currentFrame);
    }
}

void CapturePreview::saveCurrentFrame()
{
    if (d->currentFrame.isNull()) {
        return;
    }
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    QString defaultFileName = QString("screenshare_frame_%1.png").arg(timestamp);
    
    QString fileName = QFileDialog::getSaveFileName(
        this,
        tr("保存当前帧"),
        QDir(defaultPath).filePath(defaultFileName),
        tr("PNG图片 (*.png);;JPEG图片 (*.jpg);;所有文件 (*)")
    );
    
    if (!fileName.isEmpty()) {
        d->currentFrame.save(fileName);
    }
}

void CapturePreview::resetPreview()
{
    d->currentFrame = QPixmap();
    d->originalSize = QSize();
    d->frameCount = 0;
    d->currentFPS = 0.0;
    d->lastUpdateTime = 0;
    
    calculateScaledSize();
    update();
}

void CapturePreview::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    
    // 绘制背景
    painter.fillRect(rect(), Qt::black);
    
    if (!d->currentFrame.isNull() && d->previewEnabled) {
        drawFrame(painter);
    } else {
        // 绘制占位符
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, tr("预览未启用"));
    }
    
    if (d->showCrosshair) {
        drawCrosshair(painter);
    }
    
    if (d->showStatistics) {
        drawStatistics(painter);
    }
}

void CapturePreview::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint originalPos = mapToOriginal(event->pos());
        emit previewClicked(originalPos);
    }
    QWidget::mousePressEvent(event);
}

void CapturePreview::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        QPoint originalPos = mapToOriginal(event->pos());
        emit previewDoubleClicked(originalPos);
    }
    QWidget::mouseDoubleClickEvent(event);
}

void CapturePreview::wheelEvent(QWheelEvent* event)
{
    if (d->scaleMode == CustomScale) {
        int delta = event->angleDelta().y();
        if (delta > 0) {
            d->scaleFactor *= 1.1;
        } else {
            d->scaleFactor /= 1.1;
        }
        
        d->scaleFactor = qBound(0.1, d->scaleFactor, 5.0);
        d->scaleSlider->setValue(static_cast<int>(d->scaleFactor * 100));
        
        calculateScaledSize();
        update();
    }
    
    QWidget::wheelEvent(event);
}

void CapturePreview::resizeEvent(QResizeEvent* event)
{
    calculateScaledSize();
    QWidget::resizeEvent(event);
}

void CapturePreview::onRefreshTimer()
{
    updateStatistics();
    updatePreview();
}

void CapturePreview::onPlayPauseClicked()
{
    if (d->refreshTimer->isActive()) {
        pausePreview();
    } else {
        resumePreview();
    }
}

void CapturePreview::onRefreshClicked()
{
    refreshPreview();
}

void CapturePreview::onSaveClicked()
{
    saveCurrentFrame();
}

void CapturePreview::onScaleSliderChanged(int value)
{
    d->scaleMode = CustomScale;
    d->scaleFactor = value / 100.0;
    calculateScaledSize();
    update();
}

void CapturePreview::onFitToWidgetClicked()
{
    setScaleMode(FitToWidget);
}

void CapturePreview::onOriginalSizeClicked()
{
    setScaleMode(OriginalSize);
}

void CapturePreview::setupUI()
{
    d->mainLayout = new QVBoxLayout(this);
    d->mainLayout->setContentsMargins(0, 0, 0, 0);
    
    setupControls();
    
    // 主预览区域占据剩余空间
    d->mainLayout->addStretch();
    d->mainLayout->addWidget(d->controlsWidget);
}

void CapturePreview::setupControls()
{
    d->controlsWidget = new QWidget(this);
    d->controlsLayout = new QHBoxLayout(d->controlsWidget);
    
    // 播放/暂停按钮
    d->playPauseButton = new QPushButton(tr("播放"), d->controlsWidget);
    d->playPauseButton->setCheckable(true);
    
    // 刷新按钮
    d->refreshButton = new QPushButton(tr("刷新"), d->controlsWidget);
    
    // 保存按钮
    d->saveButton = new QPushButton(tr("保存"), d->controlsWidget);
    
    // 缩放滑块
    d->scaleSlider = new QSlider(Qt::Horizontal, d->controlsWidget);
    d->scaleSlider->setRange(10, 500); // 10% 到 500%
    d->scaleSlider->setValue(100);
    d->scaleSlider->setToolTip(tr("缩放比例"));
    
    // 适应窗口按钮
    d->fitToWidgetButton = new QPushButton(tr("适应"), d->controlsWidget);
    
    // 原始尺寸按钮
    d->originalSizeButton = new QPushButton(tr("1:1"), d->controlsWidget);
    
    // 统计信息复选框
    d->statisticsCheckBox = new QCheckBox(tr("统计"), d->controlsWidget);
    
    // 十字线复选框
    d->crosshairCheckBox = new QCheckBox(tr("十字线"), d->controlsWidget);
    
    // 布局
    d->controlsLayout->addWidget(d->playPauseButton);
    d->controlsLayout->addWidget(d->refreshButton);
    d->controlsLayout->addWidget(d->saveButton);
    d->controlsLayout->addWidget(new QLabel(tr("缩放:")));
    d->controlsLayout->addWidget(d->scaleSlider);
    d->controlsLayout->addWidget(d->fitToWidgetButton);
    d->controlsLayout->addWidget(d->originalSizeButton);
    d->controlsLayout->addStretch();
    d->controlsLayout->addWidget(d->statisticsCheckBox);
    d->controlsLayout->addWidget(d->crosshairCheckBox);
}

void CapturePreview::connectSignals()
{
    connect(d->refreshTimer, &QTimer::timeout,
            this, &CapturePreview::onRefreshTimer);
    
    connect(d->playPauseButton, &QPushButton::clicked,
            this, &CapturePreview::onPlayPauseClicked);
    connect(d->refreshButton, &QPushButton::clicked,
            this, &CapturePreview::onRefreshClicked);
    connect(d->saveButton, &QPushButton::clicked,
            this, &CapturePreview::onSaveClicked);
    
    connect(d->scaleSlider, &QSlider::valueChanged,
            this, &CapturePreview::onScaleSliderChanged);
    connect(d->fitToWidgetButton, &QPushButton::clicked,
            this, &CapturePreview::onFitToWidgetClicked);
    connect(d->originalSizeButton, &QPushButton::clicked,
            this, &CapturePreview::onOriginalSizeClicked);
    
    connect(d->statisticsCheckBox, &QCheckBox::toggled,
            this, &CapturePreview::setShowStatistics);
    connect(d->crosshairCheckBox, &QCheckBox::toggled,
            this, &CapturePreview::setShowCrosshair);
}

void CapturePreview::updatePreview()
{
    // 这里应该从捕获引擎获取最新帧
    // 暂时只是触发重绘
    update();
}

void CapturePreview::updateControls()
{
    bool isActive = d->refreshTimer->isActive();
    d->playPauseButton->setText(isActive ? tr("暂停") : tr("播放"));
    d->playPauseButton->setChecked(isActive);
    
    d->saveButton->setEnabled(!d->currentFrame.isNull());
}

void CapturePreview::updateStatistics()
{
    // 计算FPS
    static qint64 lastTime = 0;
    static int lastFrameCount = 0;
    
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (lastTime > 0) {
        qint64 timeDiff = currentTime - lastTime;
        int frameDiff = d->frameCount - lastFrameCount;
        
        if (timeDiff > 0) {
            d->currentFPS = (frameDiff * 1000.0) / timeDiff;
        }
    }
    
    lastTime = currentTime;
    lastFrameCount = d->frameCount;
}

void CapturePreview::calculateScaledSize()
{
    if (d->originalSize.isEmpty()) {
        d->scaledRect = QRect();
        d->scaleFactor = 1.0;
        return;
    }
    
    QSize availableSize = size();
    if (d->showControls && d->controlsWidget) {
        availableSize.setHeight(availableSize.height() - d->controlsWidget->height());
    }
    
    QSize targetSize;
    
    switch (d->scaleMode) {
    case FitToWidget:
        targetSize = d->originalSize.scaled(availableSize, Qt::KeepAspectRatio);
        d->scaleFactor = static_cast<double>(targetSize.width()) / d->originalSize.width();
        break;
        
    case KeepAspectRatio:
        targetSize = d->originalSize.scaled(availableSize, Qt::KeepAspectRatio);
        d->scaleFactor = static_cast<double>(targetSize.width()) / d->originalSize.width();
        break;
        
    case OriginalSize:
        targetSize = d->originalSize;
        d->scaleFactor = 1.0;
        break;
        
    case CustomScale:
        targetSize = QSize(
            static_cast<int>(d->originalSize.width() * d->scaleFactor),
            static_cast<int>(d->originalSize.height() * d->scaleFactor)
        );
        break;
    }
    
    // 居中显示
    int x = (availableSize.width() - targetSize.width()) / 2;
    int y = (availableSize.height() - targetSize.height()) / 2;
    d->scaledRect = QRect(QPoint(x, y), targetSize);
    
    // 更新滑块值
    if (d->scaleMode != CustomScale) {
        d->scaleSlider->setValue(static_cast<int>(d->scaleFactor * 100));
    }
}

void CapturePreview::drawFrame(QPainter& painter)
{
    if (d->scaledRect.isEmpty()) {
        return;
    }
    
    painter.drawPixmap(d->scaledRect, d->currentFrame);
    
    // 绘制边框
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRect(d->scaledRect);
}

void CapturePreview::drawCrosshair(QPainter& painter)
{
    painter.setPen(QPen(Qt::red, 1, Qt::DashLine));
    
    QPoint center = rect().center();
    
    // 水平线
    painter.drawLine(0, center.y(), width(), center.y());
    
    // 垂直线
    painter.drawLine(center.x(), 0, center.x(), height());
}

void CapturePreview::drawStatistics(QPainter& painter)
{
    if (d->originalSize.isEmpty()) {
        return;
    }
    
    painter.setPen(Qt::white);
    painter.setFont(QFont("Arial", 10));
    
    QStringList stats;
    stats << tr("分辨率: %1x%2").arg(d->originalSize.width()).arg(d->originalSize.height());
    stats << tr("缩放: %1%").arg(static_cast<int>(d->scaleFactor * 100));
    stats << tr("帧数: %1").arg(d->frameCount);
    stats << tr("FPS: %1").arg(d->currentFPS, 0, 'f', 1);
    
    int y = 20;
    for (const QString& stat : stats) {
        painter.drawText(10, y, stat);
        y += 20;
    }
}

QPoint CapturePreview::mapToOriginal(const QPoint& widgetPos) const
{
    if (d->scaledRect.isEmpty() || d->scaleFactor == 0) {
        return QPoint();
    }
    
    QPoint relativePos = widgetPos - d->scaledRect.topLeft();
    
    int originalX = static_cast<int>(relativePos.x() / d->scaleFactor);
    int originalY = static_cast<int>(relativePos.y() / d->scaleFactor);
    
    return QPoint(originalX, originalY);
}

QPoint CapturePreview::mapFromOriginal(const QPoint& originalPos) const
{
    if (d->scaledRect.isEmpty()) {
        return QPoint();
    }
    
    int widgetX = static_cast<int>(originalPos.x() * d->scaleFactor);
    int widgetY = static_cast<int>(originalPos.y() * d->scaleFactor);
    
    return d->scaledRect.topLeft() + QPoint(widgetX, widgetY);
}
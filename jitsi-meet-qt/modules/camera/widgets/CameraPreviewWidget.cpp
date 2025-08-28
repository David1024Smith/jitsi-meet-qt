#include "CameraPreviewWidget.h"
#include <QDebug>
#include <QPixmap>
#include <QApplication>
#include <QStyle>
#include <QFileDialog>
#include <QMessageBox>
#include <QDateTime>
#include <QResizeEvent>

CameraPreviewWidget::CameraPreviewWidget(QWidget* parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_videoWidget(nullptr)
    , m_controlsWidget(nullptr)
    , m_controlsLayout(nullptr)
    , m_startStopButton(nullptr)
    , m_snapshotButton(nullptr)
    , m_refreshButton(nullptr)
    , m_deviceComboBox(nullptr)
    , m_qualityComboBox(nullptr)
    , m_statusWidget(nullptr)
    , m_statusLayout(nullptr)
    , m_statusLabel(nullptr)
    , m_resolutionLabel(nullptr)
    , m_frameRateLabel(nullptr)
    , m_performanceBar(nullptr)
    , m_displayMode(FullInterface)
    , m_previewSize(640, 480)
    , m_cameraManager(nullptr)
    , m_statusUpdateTimer(new QTimer(this))
    , m_frameRateTimer(new QTimer(this))
    , m_frameCount(0)
{
    qDebug() << "CameraPreviewWidget: Initializing...";
    setupUI();
    
    // 设置定时器
    m_statusUpdateTimer->setInterval(1000); // 每秒更新状态
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &CameraPreviewWidget::updateStatusInfo);
    
    m_frameRateTimer->setInterval(1000); // 每秒更新帧率
    connect(m_frameRateTimer, &QTimer::timeout, this, &CameraPreviewWidget::updateFrameRate);
    
    qDebug() << "CameraPreviewWidget: Initialization completed";
}

CameraPreviewWidget::~CameraPreviewWidget()
{
    qDebug() << "CameraPreviewWidget: Destroying...";
    if (m_cameraManager && isCameraActive()) {
        stopPreview();
    }
}

void CameraPreviewWidget::setCameraManager(ICameraManager* manager)
{
    qDebug() << "CameraPreviewWidget: Setting camera manager";
    
    if (m_cameraManager) {
        // 断开旧的连接
        disconnect(m_cameraManager, nullptr, this, nullptr);
    }
    
    m_cameraManager = manager;
    
    if (m_cameraManager) {
        connectSignals();
        updateDeviceList();
        updateQualitySettings();
        
        // 设置视频组件
        if (m_videoWidget) {
            m_cameraManager->setPreviewWidget(m_videoWidget);
        }
    }
}

ICameraManager* CameraPreviewWidget::cameraManager() const
{
    return m_cameraManager;
}

void CameraPreviewWidget::setDisplayMode(DisplayMode mode)
{
    if (m_displayMode != mode) {
        m_displayMode = mode;
        updateControlsVisibility();
    }
}

CameraPreviewWidget::DisplayMode CameraPreviewWidget::displayMode() const
{
    return m_displayMode;
}

QVideoWidget* CameraPreviewWidget::videoWidget() const
{
    return m_videoWidget;
}

bool CameraPreviewWidget::isControlsVisible() const
{
    return m_controlsWidget && m_controlsWidget->isVisible();
}

void CameraPreviewWidget::setControlsVisible(bool visible)
{
    if (m_controlsWidget) {
        m_controlsWidget->setVisible(visible);
    }
}

bool CameraPreviewWidget::isStatusVisible() const
{
    return m_statusWidget && m_statusWidget->isVisible();
}

void CameraPreviewWidget::setStatusVisible(bool visible)
{
    if (m_statusWidget) {
        m_statusWidget->setVisible(visible);
    }
}

void CameraPreviewWidget::setPreviewSize(const QSize& size)
{
    m_previewSize = size;
    if (m_videoWidget) {
        m_videoWidget->setMinimumSize(size);
    }
}

QSize CameraPreviewWidget::previewSize() const
{
    return m_previewSize;
}

bool CameraPreviewWidget::isCameraActive() const
{
    return m_cameraManager ? m_cameraManager->isCameraActive() : false;
}

QString CameraPreviewWidget::currentDeviceName() const
{
    if (!m_cameraManager) return QString();
    
    ICameraDevice* device = m_cameraManager->currentDevice();
    return device ? device->deviceName() : QString();
}

QSize CameraPreviewWidget::currentResolution() const
{
    return m_cameraManager ? m_cameraManager->currentResolution() : QSize();
}

int CameraPreviewWidget::currentFrameRate() const
{
    if (!m_cameraManager) return 0;
    
    ICameraDevice* device = m_cameraManager->currentDevice();
    return device ? device->frameRate() : 0;
}

// === 公共槽函数 ===

void CameraPreviewWidget::startPreview()
{
    qDebug() << "CameraPreviewWidget: Starting preview";
    
    if (!m_cameraManager) {
        showError("No camera manager available");
        return;
    }
    
    if (m_cameraManager->startCamera()) {
        m_statusUpdateTimer->start();
        m_frameRateTimer->start();
        showStatus("Camera started");
        
        if (m_startStopButton) {
            m_startStopButton->setText("Stop");
            m_startStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
        }
        
        emit cameraStatusChanged(true);
    } else {
        showError("Failed to start camera");
    }
}

void CameraPreviewWidget::stopPreview()
{
    qDebug() << "CameraPreviewWidget: Stopping preview";
    
    if (m_cameraManager) {
        m_cameraManager->stopCamera();
    }
    
    m_statusUpdateTimer->stop();
    m_frameRateTimer->stop();
    
    if (m_startStopButton) {
        m_startStopButton->setText("Start");
        m_startStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
    
    showStatus("Camera stopped");
    emit cameraStatusChanged(false);
}

void CameraPreviewWidget::toggleCamera()
{
    if (isCameraActive()) {
        stopPreview();
    } else {
        startPreview();
    }
}

void CameraPreviewWidget::refreshDevices()
{
    qDebug() << "CameraPreviewWidget: Refreshing devices";
    
    if (m_cameraManager) {
        m_cameraManager->refreshDevices();
        updateDeviceList();
        showStatus("Devices refreshed");
    }
}

void CameraPreviewWidget::applyQualityPreset(ICameraDevice::QualityPreset preset)
{
    qDebug() << "CameraPreviewWidget: Applying quality preset:" << preset;
    
    if (m_cameraManager) {
        bool wasActive = isCameraActive();
        if (wasActive) {
            stopPreview();
        }
        
        m_cameraManager->startWithPreset(preset);
        
        if (wasActive) {
            startPreview();
        }
        
        emit qualityChanged(preset);
    }
}

void CameraPreviewWidget::takeSnapshot()
{
    qDebug() << "CameraPreviewWidget: Taking snapshot";
    
    if (!m_videoWidget || !isCameraActive()) {
        showError("Camera not active");
        return;
    }
    
    // 获取视频组件的截图
    QPixmap snapshot = m_videoWidget->grab();
    
    if (!snapshot.isNull()) {
        // 保存截图
        QString fileName = QString("snapshot_%1.png")
                          .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        
        QString filePath = QFileDialog::getSaveFileName(this, 
                                                       "Save Snapshot", 
                                                       fileName,
                                                       "PNG Files (*.png)");
        
        if (!filePath.isEmpty()) {
            if (snapshot.save(filePath)) {
                showStatus(QString("Snapshot saved: %1").arg(filePath));
                emit snapshotTaken(snapshot);
            } else {
                showError("Failed to save snapshot");
            }
        }
    } else {
        showError("Failed to capture snapshot");
    }
}

// === 保护函数 ===

void CameraPreviewWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // 调整视频组件大小
    if (m_videoWidget) {
        // 保持宽高比
        QSize newSize = event->size();
        if (m_controlsWidget && m_controlsWidget->isVisible()) {
            newSize.setHeight(newSize.height() - m_controlsWidget->height());
        }
        if (m_statusWidget && m_statusWidget->isVisible()) {
            newSize.setHeight(newSize.height() - m_statusWidget->height());
        }
    }
}

void CameraPreviewWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

// === 私有槽函数 ===

void CameraPreviewWidget::onCameraManagerStatusChanged(ICameraManager::ManagerStatus status)
{
    QString statusText;
    switch (status) {
        case ICameraManager::Uninitialized:
            statusText = "Uninitialized";
            break;
        case ICameraManager::Initializing:
            statusText = "Initializing...";
            break;
        case ICameraManager::Ready:
            statusText = "Ready";
            break;
        case ICameraManager::Busy:
            statusText = "Busy";
            break;
        case ICameraManager::Error:
            statusText = "Error";
            break;
    }
    
    showStatus(statusText);
}

void CameraPreviewWidget::onCameraStarted()
{
    qDebug() << "CameraPreviewWidget: Camera started signal received";
    
    if (m_startStopButton) {
        m_startStopButton->setText("Stop");
        m_startStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    }
    
    m_statusUpdateTimer->start();
    m_frameRateTimer->start();
    
    emit cameraStatusChanged(true);
}

void CameraPreviewWidget::onCameraStopped()
{
    qDebug() << "CameraPreviewWidget: Camera stopped signal received";
    
    if (m_startStopButton) {
        m_startStopButton->setText("Start");
        m_startStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }
    
    m_statusUpdateTimer->stop();
    m_frameRateTimer->stop();
    
    emit cameraStatusChanged(false);
}

void CameraPreviewWidget::onDeviceSelectionChanged()
{
    if (!m_deviceComboBox || !m_cameraManager) return;
    
    QString deviceId = m_deviceComboBox->currentData().toString();
    if (!deviceId.isEmpty()) {
        bool wasActive = isCameraActive();
        if (wasActive) {
            stopPreview();
        }
        
        m_cameraManager->selectDevice(deviceId);
        
        if (wasActive) {
            startPreview();
        }
        
        emit deviceChanged(deviceId);
    }
}

void CameraPreviewWidget::onQualityPresetChanged()
{
    if (!m_qualityComboBox) return;
    
    ICameraDevice::QualityPreset preset = 
        static_cast<ICameraDevice::QualityPreset>(m_qualityComboBox->currentData().toInt());
    
    applyQualityPreset(preset);
}

void CameraPreviewWidget::onCameraManagerError(const QString& error)
{
    showError(error);
    emit errorOccurred(error);
}

void CameraPreviewWidget::updateStatusInfo()
{
    if (!m_statusLabel || !m_cameraManager) return;
    
    QString status = isCameraActive() ? "Active" : "Inactive";
    m_statusLabel->setText(QString("Status: %1").arg(status));
    
    if (m_resolutionLabel) {
        QSize resolution = currentResolution();
        m_resolutionLabel->setText(QString("Resolution: %1x%2")
                                  .arg(resolution.width())
                                  .arg(resolution.height()));
    }
}

void CameraPreviewWidget::updateFrameRate()
{
    if (!m_frameRateLabel || !m_cameraManager) return;
    
    double frameRate = m_cameraManager->averageFrameRate();
    m_frameRateLabel->setText(QString("FPS: %1").arg(frameRate, 0, 'f', 1));
    
    // 更新性能指示器
    if (m_performanceBar) {
        int targetFps = currentFrameRate();
        if (targetFps > 0) {
            int performance = qMin(100, static_cast<int>((frameRate / targetFps) * 100));
            m_performanceBar->setValue(performance);
        }
    }
}

// === 私有方法 ===

void CameraPreviewWidget::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    setupVideoWidget();
    setupControls();
    setupStatusBar();
    
    updateControlsVisibility();
}

void CameraPreviewWidget::setupVideoWidget()
{
    m_videoWidget = new QVideoWidget(this);
    m_videoWidget->setMinimumSize(m_previewSize);
    m_videoWidget->setStyleSheet(
        "QVideoWidget {"
        "    background-color: #1a1a1a;"
        "    border: 2px solid #4CAF50;"
        "    border-radius: 8px;"
        "}"
    );
    
    m_mainLayout->addWidget(m_videoWidget, 1);
}

void CameraPreviewWidget::setupControls()
{
    m_controlsWidget = new QWidget(this);
    m_controlsLayout = new QHBoxLayout(m_controlsWidget);
    
    // 开始/停止按钮
    m_startStopButton = new QPushButton("Start", m_controlsWidget);
    m_startStopButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    connect(m_startStopButton, &QPushButton::clicked, this, &CameraPreviewWidget::toggleCamera);
    
    // 截图按钮
    m_snapshotButton = new QPushButton("Snapshot", m_controlsWidget);
    m_snapshotButton->setIcon(style()->standardIcon(QStyle::SP_DesktopIcon));
    connect(m_snapshotButton, &QPushButton::clicked, this, &CameraPreviewWidget::takeSnapshot);
    
    // 刷新按钮
    m_refreshButton = new QPushButton("Refresh", m_controlsWidget);
    m_refreshButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    connect(m_refreshButton, &QPushButton::clicked, this, &CameraPreviewWidget::refreshDevices);
    
    // 设备选择
    m_deviceComboBox = new QComboBox(m_controlsWidget);
    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraPreviewWidget::onDeviceSelectionChanged);
    
    // 质量选择
    m_qualityComboBox = new QComboBox(m_controlsWidget);
    m_qualityComboBox->addItem("Low Quality", ICameraDevice::LowQuality);
    m_qualityComboBox->addItem("Standard Quality", ICameraDevice::StandardQuality);
    m_qualityComboBox->addItem("High Quality", ICameraDevice::HighQuality);
    m_qualityComboBox->addItem("Ultra Quality", ICameraDevice::UltraQuality);
    m_qualityComboBox->setCurrentIndex(1); // 默认标准质量
    connect(m_qualityComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CameraPreviewWidget::onQualityPresetChanged);
    
    // 添加到布局
    m_controlsLayout->addWidget(m_startStopButton);
    m_controlsLayout->addWidget(m_snapshotButton);
    m_controlsLayout->addWidget(m_refreshButton);
    m_controlsLayout->addStretch();
    m_controlsLayout->addWidget(new QLabel("Device:"));
    m_controlsLayout->addWidget(m_deviceComboBox);
    m_controlsLayout->addWidget(new QLabel("Quality:"));
    m_controlsLayout->addWidget(m_qualityComboBox);
    
    m_mainLayout->addWidget(m_controlsWidget);
}

void CameraPreviewWidget::setupStatusBar()
{
    m_statusWidget = new QWidget(this);
    m_statusLayout = new QHBoxLayout(m_statusWidget);
    
    m_statusLabel = new QLabel("Status: Inactive", m_statusWidget);
    m_resolutionLabel = new QLabel("Resolution: 0x0", m_statusWidget);
    m_frameRateLabel = new QLabel("FPS: 0.0", m_statusWidget);
    
    m_performanceBar = new QProgressBar(m_statusWidget);
    m_performanceBar->setMaximumWidth(100);
    m_performanceBar->setRange(0, 100);
    m_performanceBar->setValue(0);
    
    m_statusLayout->addWidget(m_statusLabel);
    m_statusLayout->addStretch();
    m_statusLayout->addWidget(m_resolutionLabel);
    m_statusLayout->addWidget(m_frameRateLabel);
    m_statusLayout->addWidget(m_performanceBar);
    
    m_mainLayout->addWidget(m_statusWidget);
}

void CameraPreviewWidget::connectSignals()
{
    if (!m_cameraManager) return;
    
    connect(m_cameraManager, &ICameraManager::statusChanged,
            this, &CameraPreviewWidget::onCameraManagerStatusChanged);
    connect(m_cameraManager, &ICameraManager::cameraStarted,
            this, &CameraPreviewWidget::onCameraStarted);
    connect(m_cameraManager, &ICameraManager::cameraStopped,
            this, &CameraPreviewWidget::onCameraStopped);
    connect(m_cameraManager, &ICameraManager::errorOccurred,
            this, &CameraPreviewWidget::onCameraManagerError);
}

void CameraPreviewWidget::updateControlsVisibility()
{
    bool showControls = (m_displayMode == VideoWithControls || m_displayMode == FullInterface);
    bool showStatus = (m_displayMode == FullInterface);
    
    setControlsVisible(showControls);
    setStatusVisible(showStatus);
}

void CameraPreviewWidget::updateDeviceList()
{
    if (!m_deviceComboBox || !m_cameraManager) return;
    
    m_deviceComboBox->clear();
    
    QStringList devices = m_cameraManager->availableDevices();
    for (const QString& device : devices) {
        m_deviceComboBox->addItem(device, device);
    }
}

void CameraPreviewWidget::updateQualitySettings()
{
    // 质量设置已在setupControls中初始化
}

void CameraPreviewWidget::showError(const QString& error)
{
    qWarning() << "CameraPreviewWidget Error:" << error;
    
    if (m_statusLabel) {
        m_statusLabel->setText(QString("Error: %1").arg(error));
        m_statusLabel->setStyleSheet("color: red;");
        
        // 3秒后恢复正常状态
        QTimer::singleShot(3000, [this]() {
            if (m_statusLabel) {
                m_statusLabel->setStyleSheet("");
                updateStatusInfo();
            }
        });
    }
}

void CameraPreviewWidget::showStatus(const QString& status, int timeout)
{
    qDebug() << "CameraPreviewWidget Status:" << status;
    
    if (m_statusLabel) {
        QString currentText = m_statusLabel->text();
        m_statusLabel->setText(status);
        
        if (timeout > 0) {
            QTimer::singleShot(timeout, [this, currentText]() {
                if (m_statusLabel) {
                    updateStatusInfo();
                }
            });
        }
    }
}

#include "ScreenShareWidget.h"
#include "../include/ScreenShareManager.h"
#include "../config/ScreenShareConfig.h"
#include "ScreenSelector.h"
#include "CapturePreview.h"
#include <QGridLayout>
#include <QSplitter>
#include <QTabWidget>
#include <QStatusBar>
#include <QTimer>
#include <QMessageBox>
#include <QApplication>
#include <QStyle>
#include <QDebug>

class ScreenShareWidget::Private
{
public:
    Private()
        : manager(nullptr)
        , config(nullptr)
        , controlsEnabled(true)
        , previewEnabled(true)
        , sharing(false)
        , statisticsTimer(new QTimer)
    {
        statisticsTimer->setInterval(1000); // 每秒更新统计信息
    }

    // 核心组件
    ScreenShareManager* manager;
    ScreenShareConfig* config;
    
    // UI组件
    QVBoxLayout* mainLayout;
    QHBoxLayout* topLayout;
    QSplitter* mainSplitter;
    
    // 控制组
    QGroupBox* controlsGroup;
    QPushButton* startButton;
    QPushButton* stopButton;
    QPushButton* pauseButton;
    QPushButton* settingsButton;
    QLabel* statusLabel;
    
    // 源选择组
    QGroupBox* sourceGroup;
    ScreenSelector* screenSelector;
    QPushButton* refreshButton;
    
    // 质量控制组
    QGroupBox* qualityGroup;
    QComboBox* qualityCombo;
    QSlider* frameRateSlider;
    QLabel* frameRateLabel;
    QSlider* bitrateSlider;
    QLabel* bitrateLabel;
    
    // 统计信息组
    QGroupBox* statisticsGroup;
    QLabel* fpsLabel;
    QLabel* bitrateActualLabel;
    QLabel* resolutionLabel;
    QProgressBar* cpuUsageBar;
    QProgressBar* memoryUsageBar;
    
    // 预览组
    QGroupBox* previewGroup;
    CapturePreview* capturePreview;
    
    // 状态
    bool controlsEnabled;
    bool previewEnabled;
    bool sharing;
    QString currentSource;
    
    // 定时器
    QTimer* statisticsTimer;
};

ScreenShareWidget::ScreenShareWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    setupUI();
    connectSignals();
    updateUI();
}

ScreenShareWidget::~ScreenShareWidget()
{
    delete d;
}

void ScreenShareWidget::setScreenShareManager(ScreenShareManager* manager)
{
    if (d->manager) {
        // 断开旧连接
        disconnect(d->manager, nullptr, this, nullptr);
    }
    
    d->manager = manager;
    
    if (d->manager) {
        // 连接新的管理器信号
        connect(d->manager, &IScreenShareManager::statusChanged,
                this, &ScreenShareWidget::onManagerStatusChanged);
        connect(d->manager, &IScreenShareManager::shareError,
                this, &ScreenShareWidget::onManagerError);
        connect(d->manager, &IScreenShareManager::statisticsUpdated,
                this, &ScreenShareWidget::onStatisticsUpdated);
        
        updateUI();
    }
}

ScreenShareManager* ScreenShareWidget::screenShareManager() const
{
    return d->manager;
}

bool ScreenShareWidget::isSharing() const
{
    return d->sharing;
}

QString ScreenShareWidget::currentSource() const
{
    return d->currentSource;
}

IScreenShareManager::ManagerStatus ScreenShareWidget::status() const
{
    if (d->manager) {
        return d->manager->status();
    }
    return IScreenShareManager::Uninitialized;
}

void ScreenShareWidget::setConfiguration(ScreenShareConfig* config)
{
    if (d->config) {
        disconnect(d->config, nullptr, this, nullptr);
    }
    
    d->config = config;
    
    if (d->config) {
        connect(d->config, &ScreenShareConfig::configurationChanged,
                this, &ScreenShareWidget::updateQualityControls);
        
        updateQualityControls();
    }
}

ScreenShareConfig* ScreenShareWidget::configuration() const
{
    return d->config;
}

void ScreenShareWidget::setControlsEnabled(bool enabled)
{
    d->controlsEnabled = enabled;
    updateControlButtons();
}

bool ScreenShareWidget::areControlsEnabled() const
{
    return d->controlsEnabled;
}

void ScreenShareWidget::setPreviewEnabled(bool enabled)
{
    d->previewEnabled = enabled;
    d->previewGroup->setVisible(enabled);
}

bool ScreenShareWidget::isPreviewEnabled() const
{
    return d->previewEnabled;
}

void ScreenShareWidget::startSharing()
{
    if (!d->manager || !d->config) {
        QMessageBox::warning(this, tr("错误"), tr("屏幕共享管理器或配置未设置"));
        return;
    }
    
    QString selectedSource = d->screenSelector->selectedSource();
    if (selectedSource.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请选择要共享的屏幕或窗口"));
        return;
    }
    
    // 更新配置
    switch (d->screenSelector->selectionType()) {
    case ScreenSelector::ScreenSelection:
        d->config->setCaptureMode(IScreenCapture::FullScreen);
        d->config->setTargetScreen(selectedSource);
        break;
    case ScreenSelector::WindowSelection:
        d->config->setCaptureMode(IScreenCapture::Window);
        d->config->setTargetWindow(selectedSource);
        break;
    case ScreenSelector::RegionSelection:
        d->config->setCaptureMode(IScreenCapture::Region);
        d->config->setCaptureRegion(d->screenSelector->selectedRegion());
        break;
    }
    
    if (d->manager->startScreenShare()) {
        d->sharing = true;
        d->currentSource = selectedSource;
        d->statisticsTimer->start();
        emit sharingChanged(true);
        emit currentSourceChanged(selectedSource);
        emit startSharingRequested();
    }
    
    updateUI();
}

void ScreenShareWidget::stopSharing()
{
    if (d->manager && d->sharing) {
        d->manager->stopScreenShare();
        d->sharing = false;
        d->currentSource.clear();
        d->statisticsTimer->stop();
        emit sharingChanged(false);
        emit currentSourceChanged(QString());
        emit stopSharingRequested();
    }
    
    updateUI();
}

void ScreenShareWidget::pauseSharing()
{
    if (d->manager && d->sharing) {
        d->manager->pauseScreenShare();
    }
    
    updateUI();
}

void ScreenShareWidget::resumeSharing()
{
    if (d->manager && d->sharing) {
        d->manager->resumeScreenShare();
    }
    
    updateUI();
}

void ScreenShareWidget::refreshSources()
{
    if (d->screenSelector) {
        d->screenSelector->refreshScreens();
        d->screenSelector->refreshWindows();
    }
}

void ScreenShareWidget::showSettings()
{
    // 这里可以显示设置对话框
    emit settingsChanged();
}

void ScreenShareWidget::onStartButtonClicked()
{
    startSharing();
}

void ScreenShareWidget::onStopButtonClicked()
{
    stopSharing();
}

void ScreenShareWidget::onPauseButtonClicked()
{
    if (d->sharing) {
        if (d->manager && d->manager->status() == IScreenShareManager::Paused) {
            resumeSharing();
        } else {
            pauseSharing();
        }
    }
}

void ScreenShareWidget::onSourceSelectionChanged()
{
    updateControlButtons();
}

void ScreenShareWidget::onQualitySliderChanged(int value)
{
    if (d->config) {
        // 根据滑块值设置质量预设
        ScreenShareConfig::QualityPreset preset = static_cast<ScreenShareConfig::QualityPreset>(value);
        d->config->setQualityPreset(preset);
    }
}

void ScreenShareWidget::onFrameRateChanged(int fps)
{
    if (d->config) {
        d->config->setFrameRate(fps);
        d->frameRateLabel->setText(tr("帧率: %1 FPS").arg(fps));
    }
}

void ScreenShareWidget::onManagerStatusChanged(IScreenShareManager::ManagerStatus status)
{
    updateStatusDisplay();
    updateControlButtons();
}

void ScreenShareWidget::onManagerError(const QString& error)
{
    QMessageBox::critical(this, tr("屏幕共享错误"), error);
    updateUI();
}

void ScreenShareWidget::onStatisticsUpdated(const QVariantMap& statistics)
{
    updateStatistics();
}

void ScreenShareWidget::onPreviewClicked()
{
    if (d->capturePreview) {
        if (d->capturePreview->isPreviewEnabled()) {
            d->capturePreview->refreshPreview();
        }
    }
}

void ScreenShareWidget::onSettingsButtonClicked()
{
    showSettings();
}

void ScreenShareWidget::setupUI()
{
    d->mainLayout = new QVBoxLayout(this);
    d->topLayout = new QHBoxLayout;
    d->mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    setupControlsGroup();
    setupSourceGroup();
    setupQualityGroup();
    setupStatisticsGroup();
    setupPreviewGroup();
    
    // 左侧面板
    QWidget* leftPanel = new QWidget;
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->addWidget(d->controlsGroup);
    leftLayout->addWidget(d->sourceGroup);
    leftLayout->addWidget(d->qualityGroup);
    leftLayout->addWidget(d->statisticsGroup);
    leftLayout->addStretch();
    
    // 右侧面板
    QWidget* rightPanel = new QWidget;
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(d->previewGroup);
    
    d->mainSplitter->addWidget(leftPanel);
    d->mainSplitter->addWidget(rightPanel);
    d->mainSplitter->setStretchFactor(0, 1);
    d->mainSplitter->setStretchFactor(1, 2);
    
    d->mainLayout->addWidget(d->mainSplitter);
}

void ScreenShareWidget::setupControlsGroup()
{
    d->controlsGroup = new QGroupBox(tr("控制"), this);
    QVBoxLayout* layout = new QVBoxLayout(d->controlsGroup);
    
    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    
    d->startButton = new QPushButton(tr("开始共享"), this);
    d->startButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    
    d->stopButton = new QPushButton(tr("停止共享"), this);
    d->stopButton->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    
    d->pauseButton = new QPushButton(tr("暂停"), this);
    d->pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    
    d->settingsButton = new QPushButton(tr("设置"), this);
    d->settingsButton->setIcon(style()->standardIcon(QStyle::SP_ComputerIcon));
    
    buttonLayout->addWidget(d->startButton);
    buttonLayout->addWidget(d->stopButton);
    buttonLayout->addWidget(d->pauseButton);
    buttonLayout->addWidget(d->settingsButton);
    
    // 状态标签
    d->statusLabel = new QLabel(tr("未连接"), this);
    d->statusLabel->setAlignment(Qt::AlignCenter);
    
    layout->addLayout(buttonLayout);
    layout->addWidget(d->statusLabel);
}

void ScreenShareWidget::setupSourceGroup()
{
    d->sourceGroup = new QGroupBox(tr("选择源"), this);
    QVBoxLayout* layout = new QVBoxLayout(d->sourceGroup);
    
    d->screenSelector = new ScreenSelector(this);
    d->refreshButton = new QPushButton(tr("刷新"), this);
    d->refreshButton->setIcon(style()->standardIcon(QStyle::SP_BrowserReload));
    
    QHBoxLayout* refreshLayout = new QHBoxLayout;
    refreshLayout->addStretch();
    refreshLayout->addWidget(d->refreshButton);
    
    layout->addWidget(d->screenSelector);
    layout->addLayout(refreshLayout);
}

void ScreenShareWidget::setupQualityGroup()
{
    d->qualityGroup = new QGroupBox(tr("质量设置"), this);
    QGridLayout* layout = new QGridLayout(d->qualityGroup);
    
    // 质量预设
    layout->addWidget(new QLabel(tr("质量预设:")), 0, 0);
    d->qualityCombo = new QComboBox(this);
    d->qualityCombo->addItems(QStringList() << tr("节能") << tr("平衡") << tr("高质量") << tr("超高质量") << tr("自定义"));
    layout->addWidget(d->qualityCombo, 0, 1);
    
    // 帧率控制
    layout->addWidget(new QLabel(tr("帧率:")), 1, 0);
    d->frameRateSlider = new QSlider(Qt::Horizontal, this);
    d->frameRateSlider->setRange(5, 60);
    d->frameRateSlider->setValue(30);
    d->frameRateLabel = new QLabel(tr("30 FPS"), this);
    QHBoxLayout* fpsLayout = new QHBoxLayout;
    fpsLayout->addWidget(d->frameRateSlider);
    fpsLayout->addWidget(d->frameRateLabel);
    layout->addLayout(fpsLayout, 1, 1);
    
    // 比特率控制
    layout->addWidget(new QLabel(tr("比特率:")), 2, 0);
    d->bitrateSlider = new QSlider(Qt::Horizontal, this);
    d->bitrateSlider->setRange(500, 10000);
    d->bitrateSlider->setValue(2000);
    d->bitrateLabel = new QLabel(tr("2000 kbps"), this);
    QHBoxLayout* bitrateLayout = new QHBoxLayout;
    bitrateLayout->addWidget(d->bitrateSlider);
    bitrateLayout->addWidget(d->bitrateLabel);
    layout->addLayout(bitrateLayout, 2, 1);
}

void ScreenShareWidget::setupStatisticsGroup()
{
    d->statisticsGroup = new QGroupBox(tr("统计信息"), this);
    QGridLayout* layout = new QGridLayout(d->statisticsGroup);
    
    // FPS显示
    layout->addWidget(new QLabel(tr("实际帧率:")), 0, 0);
    d->fpsLabel = new QLabel(tr("0 FPS"), this);
    layout->addWidget(d->fpsLabel, 0, 1);
    
    // 实际比特率
    layout->addWidget(new QLabel(tr("实际比特率:")), 1, 0);
    d->bitrateActualLabel = new QLabel(tr("0 kbps"), this);
    layout->addWidget(d->bitrateActualLabel, 1, 1);
    
    // 分辨率
    layout->addWidget(new QLabel(tr("分辨率:")), 2, 0);
    d->resolutionLabel = new QLabel(tr("0x0"), this);
    layout->addWidget(d->resolutionLabel, 2, 1);
    
    // CPU使用率
    layout->addWidget(new QLabel(tr("CPU使用率:")), 3, 0);
    d->cpuUsageBar = new QProgressBar(this);
    d->cpuUsageBar->setRange(0, 100);
    layout->addWidget(d->cpuUsageBar, 3, 1);
    
    // 内存使用
    layout->addWidget(new QLabel(tr("内存使用:")), 4, 0);
    d->memoryUsageBar = new QProgressBar(this);
    d->memoryUsageBar->setRange(0, 100);
    layout->addWidget(d->memoryUsageBar, 4, 1);
}

void ScreenShareWidget::setupPreviewGroup()
{
    d->previewGroup = new QGroupBox(tr("预览"), this);
    QVBoxLayout* layout = new QVBoxLayout(d->previewGroup);
    
    d->capturePreview = new CapturePreview(this);
    layout->addWidget(d->capturePreview);
}

void ScreenShareWidget::connectSignals()
{
    // 控制按钮信号
    connect(d->startButton, &QPushButton::clicked,
            this, &ScreenShareWidget::onStartButtonClicked);
    connect(d->stopButton, &QPushButton::clicked,
            this, &ScreenShareWidget::onStopButtonClicked);
    connect(d->pauseButton, &QPushButton::clicked,
            this, &ScreenShareWidget::onPauseButtonClicked);
    connect(d->settingsButton, &QPushButton::clicked,
            this, &ScreenShareWidget::onSettingsButtonClicked);
    
    // 源选择信号
    connect(d->screenSelector, &ScreenSelector::selectedSourceChanged,
            this, &ScreenShareWidget::onSourceSelectionChanged);
    connect(d->refreshButton, &QPushButton::clicked,
            this, &ScreenShareWidget::refreshSources);
    
    // 质量控制信号
    connect(d->qualityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ScreenShareWidget::onQualitySliderChanged);
    connect(d->frameRateSlider, &QSlider::valueChanged,
            this, &ScreenShareWidget::onFrameRateChanged);
    connect(d->bitrateSlider, &QSlider::valueChanged, [this](int value) {
        if (d->config) {
            d->config->setBitrate(value);
            d->bitrateLabel->setText(tr("%1 kbps").arg(value));
        }
    });
    
    // 预览信号
    connect(d->capturePreview, &CapturePreview::previewClicked,
            this, &ScreenShareWidget::onPreviewClicked);
    
    // 统计定时器
    connect(d->statisticsTimer, &QTimer::timeout,
            this, &ScreenShareWidget::updateStatistics);
}

void ScreenShareWidget::updateUI()
{
    updateControlButtons();
    updateSourceList();
    updateQualityControls();
    updateStatistics();
    updateStatusDisplay();
}

void ScreenShareWidget::updateControlButtons()
{
    bool hasManager = (d->manager != nullptr);
    bool hasSelection = d->screenSelector && d->screenSelector->hasSelection();
    bool canStart = hasManager && hasSelection && !d->sharing && d->controlsEnabled;
    bool canStop = hasManager && d->sharing && d->controlsEnabled;
    bool canPause = hasManager && d->sharing && d->controlsEnabled;
    
    d->startButton->setEnabled(canStart);
    d->stopButton->setEnabled(canStop);
    d->pauseButton->setEnabled(canPause);
    
    // 更新暂停按钮文本
    if (d->manager && d->manager->status() == IScreenShareManager::Paused) {
        d->pauseButton->setText(tr("恢复"));
        d->pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    } else {
        d->pauseButton->setText(tr("暂停"));
        d->pauseButton->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

void ScreenShareWidget::updateSourceList()
{
    if (d->screenSelector) {
        d->screenSelector->refreshScreens();
        d->screenSelector->refreshWindows();
    }
}

void ScreenShareWidget::updateQualityControls()
{
    if (!d->config) return;
    
    // 更新质量预设
    d->qualityCombo->setCurrentIndex(static_cast<int>(d->config->qualityPreset()));
    
    // 更新帧率
    d->frameRateSlider->setValue(d->config->frameRate());
    d->frameRateLabel->setText(tr("%1 FPS").arg(d->config->frameRate()));
    
    // 更新比特率
    d->bitrateSlider->setValue(d->config->bitrate());
    d->bitrateLabel->setText(tr("%1 kbps").arg(d->config->bitrate()));
}

void ScreenShareWidget::updateStatistics()
{
    if (!d->manager) return;
    
    // 这里应该从管理器获取实际统计信息
    // 暂时使用模拟数据
    if (d->sharing) {
        d->fpsLabel->setText(tr("%1 FPS").arg(30));
        d->bitrateActualLabel->setText(tr("%1 kbps").arg(2000));
        d->resolutionLabel->setText(tr("1920x1080"));
        d->cpuUsageBar->setValue(25);
        d->memoryUsageBar->setValue(15);
    } else {
        d->fpsLabel->setText(tr("0 FPS"));
        d->bitrateActualLabel->setText(tr("0 kbps"));
        d->resolutionLabel->setText(tr("0x0"));
        d->cpuUsageBar->setValue(0);
        d->memoryUsageBar->setValue(0);
    }
}

void ScreenShareWidget::updateStatusDisplay()
{
    if (!d->manager) {
        d->statusLabel->setText(tr("未连接"));
        return;
    }
    
    switch (d->manager->status()) {
    case IScreenShareManager::Uninitialized:
        d->statusLabel->setText(tr("未初始化"));
        break;
    case IScreenShareManager::Ready:
        d->statusLabel->setText(tr("就绪"));
        break;
    case IScreenShareManager::Sharing:
        d->statusLabel->setText(tr("正在共享"));
        break;
    case IScreenShareManager::Paused:
        d->statusLabel->setText(tr("已暂停"));
        break;
    case IScreenShareManager::Error:
        d->statusLabel->setText(tr("错误"));
        break;
    }
}
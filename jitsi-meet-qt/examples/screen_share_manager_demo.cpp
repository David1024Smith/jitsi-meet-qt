#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QVideoWidget>
#include <QTextEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QListWidget>
#include <QSlider>
#include <QProgressBar>
#include <QTimer>
#include <QDebug>
#include <QMessageBox>
#include "ScreenShareManager.h"

class ScreenShareManagerDemo : public QMainWindow
{
    Q_OBJECT

public:
    explicit ScreenShareManagerDemo(QWidget *parent = nullptr);
    ~ScreenShareManagerDemo();

private slots:
    // 屏幕共享控制
    void onStartScreenShare();
    void onStopScreenShare();
    void onSelectScreen();
    void onSelectWindow();
    void onShowSelectionDialog();
    
    // 质量设置
    void onQualityChanged();
    void onFrameRateChanged(int frameRate);
    void onBitrateChanged(int bitrate);
    void onAdaptiveQualityToggled(bool enabled);
    
    // 屏幕共享事件
    void onScreenShareStarted();
    void onScreenShareStopped();
    void onWindowShareStarted();
    void onWindowShareStopped();
    
    // 远程屏幕共享
    void onAddRemoteShare();
    void onRemoveRemoteShare();
    void onRemoteScreenShareReceived(const QString& participantId, QVideoWidget* widget);
    void onRemoteScreenShareRemoved(const QString& participantId);
    
    // 错误处理
    void onScreenCaptureError(const QString& error);
    void onWindowCaptureError(const QString& error);
    void onEncodingError(const QString& error);
    
    // 列表更新
    void onScreenListChanged();
    void onWindowListChanged();
    
    // 定时更新
    void updateStatus();
    void updatePerformanceInfo();

private:
    void setupUI();
    void setupControlPanel();
    void setupQualityPanel();
    void setupDisplayPanel();
    void setupStatusPanel();
    void connectSignals();
    void refreshScreenList();
    void refreshWindowList();
    void updateControlButtons();
    void logMessage(const QString& message);
    
    ScreenShareManager* m_screenShareManager;
    
    // 控制面板组件
    QGroupBox* m_controlGroup;
    QPushButton* m_startScreenBtn;
    QPushButton* m_stopShareBtn;
    QPushButton* m_selectScreenBtn;
    QPushButton* m_selectWindowBtn;
    QPushButton* m_showDialogBtn;
    QComboBox* m_screenCombo;
    QComboBox* m_windowCombo;
    
    // 质量设置组件
    QGroupBox* m_qualityGroup;
    QComboBox* m_resolutionCombo;
    QSlider* m_frameRateSlider;
    QSlider* m_bitrateSlider;
    QCheckBox* m_adaptiveQualityCheck;
    QLabel* m_frameRateLabel;
    QLabel* m_bitrateLabel;
    
    // 显示面板组件
    QGroupBox* m_displayGroup;
    QVideoWidget* m_localShareWidget;
    QTabWidget* m_remoteShareTabs;
    
    // 远程屏幕共享控制
    QGroupBox* m_remoteGroup;
    QPushButton* m_addRemoteBtn;
    QPushButton* m_removeRemoteBtn;
    QListWidget* m_remoteList;
    
    // 状态面板组件
    QGroupBox* m_statusGroup;
    QLabel* m_statusLabel;
    QLabel* m_performanceLabel;
    QProgressBar* m_cpuUsageBar;
    QProgressBar* m_memoryUsageBar;
    QTextEdit* m_logText;
    
    // 定时器
    QTimer* m_statusTimer;
    QTimer* m_performanceTimer;
    
    // 布局
    QWidget* m_centralWidget;
    QSplitter* m_mainSplitter;
    QSplitter* m_leftSplitter;
};

ScreenShareManagerDemo::ScreenShareManagerDemo(QWidget *parent)
    : QMainWindow(parent)
    , m_screenShareManager(nullptr)
    , m_statusTimer(new QTimer(this))
    , m_performanceTimer(new QTimer(this))
{
    setWindowTitle("Screen Share Manager Demo");
    setMinimumSize(1200, 800);
    
    // 创建ScreenShareManager
    m_screenShareManager = new ScreenShareManager(this);
    
    setupUI();
    connectSignals();
    
    // 初始化数据
    refreshScreenList();
    refreshWindowList();
    updateControlButtons();
    
    // 启动定时器
    m_statusTimer->start(1000);
    m_performanceTimer->start(2000);
    
    logMessage("Screen Share Manager Demo initialized");
}

ScreenShareManagerDemo::~ScreenShareManagerDemo()
{
}

void ScreenShareManagerDemo::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 创建左侧分割器
    m_leftSplitter = new QSplitter(Qt::Vertical);
    
    setupControlPanel();
    setupQualityPanel();
    setupStatusPanel();
    
    m_leftSplitter->addWidget(m_controlGroup);
    m_leftSplitter->addWidget(m_qualityGroup);
    m_leftSplitter->addWidget(m_statusGroup);
    m_leftSplitter->setSizes({200, 150, 200});
    
    setupDisplayPanel();
    
    m_mainSplitter->addWidget(m_leftSplitter);
    m_mainSplitter->addWidget(m_displayGroup);
    m_mainSplitter->setSizes({400, 800});
    
    QVBoxLayout* mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->addWidget(m_mainSplitter);
}

void ScreenShareManagerDemo::setupControlPanel()
{
    m_controlGroup = new QGroupBox("屏幕共享控制");
    QVBoxLayout* layout = new QVBoxLayout(m_controlGroup);
    
    // 屏幕选择
    layout->addWidget(new QLabel("选择屏幕:"));
    m_screenCombo = new QComboBox();
    layout->addWidget(m_screenCombo);
    
    // 窗口选择
    layout->addWidget(new QLabel("选择窗口:"));
    m_windowCombo = new QComboBox();
    layout->addWidget(m_windowCombo);
    
    // 控制按钮
    QGridLayout* buttonLayout = new QGridLayout();
    
    m_startScreenBtn = new QPushButton("开始屏幕共享");
    m_stopShareBtn = new QPushButton("停止共享");
    m_selectScreenBtn = new QPushButton("选择屏幕");
    m_selectWindowBtn = new QPushButton("选择窗口");
    m_showDialogBtn = new QPushButton("显示选择对话框");
    
    m_stopShareBtn->setEnabled(false);
    
    buttonLayout->addWidget(m_startScreenBtn, 0, 0);
    buttonLayout->addWidget(m_stopShareBtn, 0, 1);
    buttonLayout->addWidget(m_selectScreenBtn, 1, 0);
    buttonLayout->addWidget(m_selectWindowBtn, 1, 1);
    buttonLayout->addWidget(m_showDialogBtn, 2, 0, 1, 2);
    
    layout->addLayout(buttonLayout);
    
    // 远程屏幕共享控制
    m_remoteGroup = new QGroupBox("远程屏幕共享");
    QVBoxLayout* remoteLayout = new QVBoxLayout(m_remoteGroup);
    
    m_remoteList = new QListWidget();
    m_remoteList->setMaximumHeight(100);
    remoteLayout->addWidget(m_remoteList);
    
    QHBoxLayout* remoteButtonLayout = new QHBoxLayout();
    m_addRemoteBtn = new QPushButton("添加远程共享");
    m_removeRemoteBtn = new QPushButton("移除远程共享");
    
    remoteButtonLayout->addWidget(m_addRemoteBtn);
    remoteButtonLayout->addWidget(m_removeRemoteBtn);
    remoteLayout->addLayout(remoteButtonLayout);
    
    layout->addWidget(m_remoteGroup);
}

void ScreenShareManagerDemo::setupQualityPanel()
{
    m_qualityGroup = new QGroupBox("质量设置");
    QVBoxLayout* layout = new QVBoxLayout(m_qualityGroup);
    
    // 分辨率设置
    layout->addWidget(new QLabel("分辨率:"));
    m_resolutionCombo = new QComboBox();
    m_resolutionCombo->addItem("1920x1080", QSize(1920, 1080));
    m_resolutionCombo->addItem("1280x720", QSize(1280, 720));
    m_resolutionCombo->addItem("1024x768", QSize(1024, 768));
    m_resolutionCombo->addItem("800x600", QSize(800, 600));
    layout->addWidget(m_resolutionCombo);
    
    // 帧率设置
    layout->addWidget(new QLabel("帧率:"));
    m_frameRateSlider = new QSlider(Qt::Horizontal);
    m_frameRateSlider->setRange(5, 30);
    m_frameRateSlider->setValue(15);
    m_frameRateLabel = new QLabel("15 FPS");
    
    QHBoxLayout* frameRateLayout = new QHBoxLayout();
    frameRateLayout->addWidget(m_frameRateSlider);
    frameRateLayout->addWidget(m_frameRateLabel);
    layout->addLayout(frameRateLayout);
    
    // 比特率设置
    layout->addWidget(new QLabel("比特率:"));
    m_bitrateSlider = new QSlider(Qt::Horizontal);
    m_bitrateSlider->setRange(500, 5000); // 500K - 5M
    m_bitrateSlider->setValue(2000);
    m_bitrateLabel = new QLabel("2000 Kbps");
    
    QHBoxLayout* bitrateLayout = new QHBoxLayout();
    bitrateLayout->addWidget(m_bitrateSlider);
    bitrateLayout->addWidget(m_bitrateLabel);
    layout->addLayout(bitrateLayout);
    
    // 自适应质量
    m_adaptiveQualityCheck = new QCheckBox("自适应质量");
    m_adaptiveQualityCheck->setChecked(true);
    layout->addWidget(m_adaptiveQualityCheck);
}

void ScreenShareManagerDemo::setupDisplayPanel()
{
    m_displayGroup = new QGroupBox("屏幕共享显示");
    QVBoxLayout* layout = new QVBoxLayout(m_displayGroup);
    
    // 本地屏幕共享显示
    QGroupBox* localGroup = new QGroupBox("本地屏幕共享");
    QVBoxLayout* localLayout = new QVBoxLayout(localGroup);
    
    m_localShareWidget = new QVideoWidget();
    m_localShareWidget->setMinimumSize(400, 300);
    m_localShareWidget->setStyleSheet("QVideoWidget { background-color: black; border: 1px solid #ccc; }");
    localLayout->addWidget(m_localShareWidget);
    
    layout->addWidget(localGroup);
    
    // 远程屏幕共享显示
    QGroupBox* remoteGroup = new QGroupBox("远程屏幕共享");
    QVBoxLayout* remoteLayout = new QVBoxLayout(remoteGroup);
    
    m_remoteShareTabs = new QTabWidget();
    m_remoteShareTabs->setMinimumSize(400, 200);
    remoteLayout->addWidget(m_remoteShareTabs);
    
    layout->addWidget(remoteGroup);
}

void ScreenShareManagerDemo::setupStatusPanel()
{
    m_statusGroup = new QGroupBox("状态信息");
    QVBoxLayout* layout = new QVBoxLayout(m_statusGroup);
    
    // 状态标签
    m_statusLabel = new QLabel("状态: 未共享");
    layout->addWidget(m_statusLabel);
    
    // 性能信息
    m_performanceLabel = new QLabel("性能: 正常");
    layout->addWidget(m_performanceLabel);
    
    // CPU使用率
    layout->addWidget(new QLabel("CPU使用率:"));
    m_cpuUsageBar = new QProgressBar();
    m_cpuUsageBar->setRange(0, 100);
    layout->addWidget(m_cpuUsageBar);
    
    // 内存使用率
    layout->addWidget(new QLabel("内存使用率:"));
    m_memoryUsageBar = new QProgressBar();
    m_memoryUsageBar->setRange(0, 100);
    layout->addWidget(m_memoryUsageBar);
    
    // 日志显示
    layout->addWidget(new QLabel("日志:"));
    m_logText = new QTextEdit();
    m_logText->setMaximumHeight(150);
    m_logText->setReadOnly(true);
    layout->addWidget(m_logText);
}

void ScreenShareManagerDemo::connectSignals()
{
    // 控制按钮信号
    connect(m_startScreenBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onStartScreenShare);
    connect(m_stopShareBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onStopScreenShare);
    connect(m_selectScreenBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onSelectScreen);
    connect(m_selectWindowBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onSelectWindow);
    connect(m_showDialogBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onShowSelectionDialog);
    
    // 远程屏幕共享控制
    connect(m_addRemoteBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onAddRemoteShare);
    connect(m_removeRemoteBtn, &QPushButton::clicked, this, &ScreenShareManagerDemo::onRemoveRemoteShare);
    
    // 质量设置信号
    connect(m_resolutionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ScreenShareManagerDemo::onQualityChanged);
    connect(m_frameRateSlider, &QSlider::valueChanged, this, &ScreenShareManagerDemo::onFrameRateChanged);
    connect(m_bitrateSlider, &QSlider::valueChanged, this, &ScreenShareManagerDemo::onBitrateChanged);
    connect(m_adaptiveQualityCheck, &QCheckBox::toggled, this, &ScreenShareManagerDemo::onAdaptiveQualityToggled);
    
    // ScreenShareManager信号
    connect(m_screenShareManager, &ScreenShareManager::screenShareStarted, this, &ScreenShareManagerDemo::onScreenShareStarted);
    connect(m_screenShareManager, &ScreenShareManager::screenShareStopped, this, &ScreenShareManagerDemo::onScreenShareStopped);
    connect(m_screenShareManager, &ScreenShareManager::windowShareStarted, this, &ScreenShareManagerDemo::onWindowShareStarted);
    connect(m_screenShareManager, &ScreenShareManager::windowShareStopped, this, &ScreenShareManagerDemo::onWindowShareStopped);
    
    connect(m_screenShareManager, &ScreenShareManager::remoteScreenShareReceived, this, &ScreenShareManagerDemo::onRemoteScreenShareReceived);
    connect(m_screenShareManager, &ScreenShareManager::remoteScreenShareRemoved, this, &ScreenShareManagerDemo::onRemoteScreenShareRemoved);
    
    connect(m_screenShareManager, &ScreenShareManager::screenCaptureError, this, &ScreenShareManagerDemo::onScreenCaptureError);
    connect(m_screenShareManager, &ScreenShareManager::windowCaptureError, this, &ScreenShareManagerDemo::onWindowCaptureError);
    connect(m_screenShareManager, &ScreenShareManager::encodingError, this, &ScreenShareManagerDemo::onEncodingError);
    
    connect(m_screenShareManager, &ScreenShareManager::screenListChanged, this, &ScreenShareManagerDemo::onScreenListChanged);
    connect(m_screenShareManager, &ScreenShareManager::windowListChanged, this, &ScreenShareManagerDemo::onWindowListChanged);
    
    // 定时器信号
    connect(m_statusTimer, &QTimer::timeout, this, &ScreenShareManagerDemo::updateStatus);
    connect(m_performanceTimer, &QTimer::timeout, this, &ScreenShareManagerDemo::updatePerformanceInfo);
}

void ScreenShareManagerDemo::onStartScreenShare()
{
    int screenIndex = m_screenCombo->currentIndex();
    if (screenIndex >= 0) {
        QList<ScreenShareManager::ScreenInfo> screens = m_screenShareManager->availableScreens();
        if (screenIndex < screens.size()) {
            int screenId = screens[screenIndex].screenId;
            logMessage(QString("Starting screen share for screen %1").arg(screenId));
            m_screenShareManager->startScreenShare(screenId);
        }
    }
}

void ScreenShareManagerDemo::onStopScreenShare()
{
    logMessage("Stopping screen share");
    m_screenShareManager->stopScreenShare();
}

void ScreenShareManagerDemo::onSelectScreen()
{
    int screenIndex = m_screenCombo->currentIndex();
    if (screenIndex >= 0) {
        QList<ScreenShareManager::ScreenInfo> screens = m_screenShareManager->availableScreens();
        if (screenIndex < screens.size()) {
            logMessage(QString("Selected screen: %1").arg(screens[screenIndex].name));
        }
    }
}

void ScreenShareManagerDemo::onSelectWindow()
{
    int windowIndex = m_windowCombo->currentIndex();
    if (windowIndex >= 0) {
        QList<ScreenShareManager::WindowInfo> windows = m_screenShareManager->availableWindows();
        if (windowIndex < windows.size()) {
            qint64 windowId = windows[windowIndex].windowId;
            logMessage(QString("Starting window share for window %1").arg(windowId));
            m_screenShareManager->startWindowShare(windowId);
        }
    }
}

void ScreenShareManagerDemo::onShowSelectionDialog()
{
    logMessage("Showing screen selection dialog");
    m_screenShareManager->showScreenSelectionDialog();
}

void ScreenShareManagerDemo::onQualityChanged()
{
    ScreenShareManager::ShareQuality quality = m_screenShareManager->shareQuality();
    quality.resolution = m_resolutionCombo->currentData().toSize();
    m_screenShareManager->setShareQuality(quality);
    
    logMessage(QString("Resolution changed to %1x%2")
              .arg(quality.resolution.width())
              .arg(quality.resolution.height()));
}

void ScreenShareManagerDemo::onFrameRateChanged(int frameRate)
{
    m_frameRateLabel->setText(QString("%1 FPS").arg(frameRate));
    
    ScreenShareManager::ShareQuality quality = m_screenShareManager->shareQuality();
    quality.frameRate = frameRate;
    m_screenShareManager->setShareQuality(quality);
    
    logMessage(QString("Frame rate changed to %1 FPS").arg(frameRate));
}

void ScreenShareManagerDemo::onBitrateChanged(int bitrate)
{
    m_bitrateLabel->setText(QString("%1 Kbps").arg(bitrate));
    
    ScreenShareManager::ShareQuality quality = m_screenShareManager->shareQuality();
    quality.bitrate = bitrate * 1000; // 转换为bps
    m_screenShareManager->setShareQuality(quality);
    
    logMessage(QString("Bitrate changed to %1 Kbps").arg(bitrate));
}

void ScreenShareManagerDemo::onAdaptiveQualityToggled(bool enabled)
{
    ScreenShareManager::ShareQuality quality = m_screenShareManager->shareQuality();
    quality.adaptiveQuality = enabled;
    m_screenShareManager->setShareQuality(quality);
    
    logMessage(QString("Adaptive quality %1").arg(enabled ? "enabled" : "disabled"));
}

void ScreenShareManagerDemo::onScreenShareStarted()
{
    logMessage("Screen share started successfully");
    updateControlButtons();
}

void ScreenShareManagerDemo::onScreenShareStopped()
{
    logMessage("Screen share stopped");
    updateControlButtons();
}

void ScreenShareManagerDemo::onWindowShareStarted()
{
    logMessage("Window share started successfully");
    updateControlButtons();
}

void ScreenShareManagerDemo::onWindowShareStopped()
{
    logMessage("Window share stopped");
    updateControlButtons();
}

void ScreenShareManagerDemo::onAddRemoteShare()
{
    static int remoteCounter = 1;
    QString participantId = QString("remote-participant-%1").arg(remoteCounter++);
    
    QVideoWidget* widget = new QVideoWidget();
    widget->setMinimumSize(320, 240);
    widget->setStyleSheet("QVideoWidget { background-color: #333; border: 1px solid #666; }");
    
    m_screenShareManager->addRemoteScreenShare(participantId, widget);
    logMessage(QString("Added remote screen share: %1").arg(participantId));
}

void ScreenShareManagerDemo::onRemoveRemoteShare()
{
    QListWidgetItem* item = m_remoteList->currentItem();
    if (item) {
        QString participantId = item->text();
        m_screenShareManager->removeRemoteScreenShare(participantId);
        logMessage(QString("Removed remote screen share: %1").arg(participantId));
    }
}

void ScreenShareManagerDemo::onRemoteScreenShareReceived(const QString& participantId, QVideoWidget* widget)
{
    // 添加到标签页
    m_remoteShareTabs->addTab(widget, participantId);
    
    // 添加到列表
    m_remoteList->addItem(participantId);
    
    logMessage(QString("Remote screen share received from: %1").arg(participantId));
}

void ScreenShareManagerDemo::onRemoteScreenShareRemoved(const QString& participantId)
{
    // 从标签页移除
    for (int i = 0; i < m_remoteShareTabs->count(); ++i) {
        if (m_remoteShareTabs->tabText(i) == participantId) {
            m_remoteShareTabs->removeTab(i);
            break;
        }
    }
    
    // 从列表移除
    for (int i = 0; i < m_remoteList->count(); ++i) {
        if (m_remoteList->item(i)->text() == participantId) {
            delete m_remoteList->takeItem(i);
            break;
        }
    }
    
    logMessage(QString("Remote screen share removed: %1").arg(participantId));
}

void ScreenShareManagerDemo::onScreenCaptureError(const QString& error)
{
    logMessage(QString("Screen capture error: %1").arg(error));
    QMessageBox::warning(this, "屏幕捕获错误", error);
}

void ScreenShareManagerDemo::onWindowCaptureError(const QString& error)
{
    logMessage(QString("Window capture error: %1").arg(error));
    QMessageBox::warning(this, "窗口捕获错误", error);
}

void ScreenShareManagerDemo::onEncodingError(const QString& error)
{
    logMessage(QString("Encoding error: %1").arg(error));
    QMessageBox::warning(this, "编码错误", error);
}

void ScreenShareManagerDemo::onScreenListChanged()
{
    refreshScreenList();
    logMessage("Screen list updated");
}

void ScreenShareManagerDemo::onWindowListChanged()
{
    refreshWindowList();
    logMessage("Window list updated");
}

void ScreenShareManagerDemo::updateStatus()
{
    QString status;
    if (m_screenShareManager->isScreenSharing()) {
        ScreenShareManager::ScreenInfo screen = m_screenShareManager->currentScreen();
        status = QString("屏幕共享中: %1").arg(screen.name);
    } else if (m_screenShareManager->isWindowSharing()) {
        ScreenShareManager::WindowInfo window = m_screenShareManager->currentWindow();
        status = QString("窗口共享中: %1").arg(window.title);
    } else {
        status = "未共享";
    }
    
    m_statusLabel->setText(QString("状态: %1").arg(status));
}

void ScreenShareManagerDemo::updatePerformanceInfo()
{
    // 模拟性能数据
    static int cpuUsage = 20;
    static int memoryUsage = 45;
    
    cpuUsage += (qrand() % 21) - 10; // -10 到 +10
    cpuUsage = qBound(10, cpuUsage, 90);
    
    memoryUsage += (qrand() % 11) - 5; // -5 到 +5
    memoryUsage = qBound(30, memoryUsage, 80);
    
    m_cpuUsageBar->setValue(cpuUsage);
    m_memoryUsageBar->setValue(memoryUsage);
    
    QString performance = QString("CPU: %1%, 内存: %2%").arg(cpuUsage).arg(memoryUsage);
    m_performanceLabel->setText(QString("性能: %1").arg(performance));
}

void ScreenShareManagerDemo::refreshScreenList()
{
    m_screenCombo->clear();
    
    QList<ScreenShareManager::ScreenInfo> screens = m_screenShareManager->availableScreens();
    for (const auto& screen : screens) {
        QString text = QString("屏幕 %1: %2 (%3x%4)")
                      .arg(screen.screenId)
                      .arg(screen.name)
                      .arg(screen.size.width())
                      .arg(screen.size.height());
        
        if (screen.isPrimary) {
            text += " [主屏幕]";
        }
        
        m_screenCombo->addItem(text);
    }
}

void ScreenShareManagerDemo::refreshWindowList()
{
    m_windowCombo->clear();
    
    QList<ScreenShareManager::WindowInfo> windows = m_screenShareManager->availableWindows();
    for (const auto& window : windows) {
        if (!window.isVisible) {
            continue;
        }
        
        QString text = QString("%1 - %2").arg(window.title).arg(window.processName);
        m_windowCombo->addItem(text);
    }
}

void ScreenShareManagerDemo::updateControlButtons()
{
    bool isSharing = m_screenShareManager->isScreenSharing() || m_screenShareManager->isWindowSharing();
    
    m_startScreenBtn->setEnabled(!isSharing);
    m_selectScreenBtn->setEnabled(!isSharing);
    m_selectWindowBtn->setEnabled(!isSharing);
    m_showDialogBtn->setEnabled(!isSharing);
    m_stopShareBtn->setEnabled(isSharing);
    
    m_screenCombo->setEnabled(!isSharing);
    m_windowCombo->setEnabled(!isSharing);
}

void ScreenShareManagerDemo::logMessage(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString logEntry = QString("[%1] %2").arg(timestamp).arg(message);
    
    m_logText->append(logEntry);
    qDebug() << "ScreenShareManagerDemo:" << message;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ScreenShareManagerDemo demo;
    demo.show();
    
    return app.exec();
}

// MOC will be generated automatically
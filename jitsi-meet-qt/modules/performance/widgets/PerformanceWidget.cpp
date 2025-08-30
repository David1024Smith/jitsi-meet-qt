#include "PerformanceWidget.h"
#include "MetricsChart.h"
#include "PerformanceManager.h"
#include "PerformanceConfig.h"
#include <QApplication>
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSplitter>
#include <QScrollArea>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QComboBox>
#include <QTextEdit>
#include <QDateTime>
#include <QDebug>

PerformanceWidget::PerformanceWidget(QWidget *parent)
    : QWidget(parent)
    , m_performanceManager(nullptr)
    , m_config(nullptr)
    , m_displayMode(DetailedMode)
    , m_tabWidget(nullptr)
    , m_mainLayout(nullptr)
    , m_cpuUsageLabel(nullptr)
    , m_memoryUsageLabel(nullptr)
    , m_networkLatencyLabel(nullptr)
    , m_performanceLevelLabel(nullptr)
    , m_cpuProgressBar(nullptr)
    , m_memoryProgressBar(nullptr)
    , m_networkProgressBar(nullptr)
    , m_systemInfoLabel(nullptr)
    , m_audioMetricsLabel(nullptr)
    , m_videoMetricsLabel(nullptr)
    , m_networkMetricsLabel(nullptr)
    , m_cpuChart(nullptr)
    , m_memoryChart(nullptr)
    , m_networkChart(nullptr)
    , m_optimizeButton(nullptr)
    , m_autoOptimizeButton(nullptr)
    , m_optimizationStatusLabel(nullptr)
    , m_lastOptimizationLabel(nullptr)
    , m_configButton(nullptr)
    , m_exportButton(nullptr)
    , m_resetButton(nullptr)
    , m_startStopButton(nullptr)
    , m_statusLabel(nullptr)
    , m_updateTimer(new QTimer(this))
    , m_realTimeUpdateActive(false)
    , m_updateInterval(1000)
{
    initializeUI();
    
    // 连接定时器
    connect(m_updateTimer, &QTimer::timeout, this, &PerformanceWidget::handleRealTimeUpdate);
}

PerformanceWidget::~PerformanceWidget()
{
    stopRealTimeUpdate();
}void Pe
rformanceWidget::setPerformanceManager(PerformanceManager* manager)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_performanceManager == manager) {
        return;
    }
    
    // 断开旧连接
    if (m_performanceManager) {
        disconnect(m_performanceManager, nullptr, this, nullptr);
    }
    
    m_performanceManager = manager;
    
    // 建立新连接
    if (m_performanceManager) {
        connect(m_performanceManager, &PerformanceManager::metricsUpdated,
                this, &PerformanceWidget::updateMetrics);
        connect(m_performanceManager, &PerformanceManager::performanceLevelChanged,
                this, &PerformanceWidget::updatePerformanceLevel);
        connect(m_performanceManager, &PerformanceManager::thresholdExceeded,
                this, &PerformanceWidget::showThresholdWarning);
        connect(m_performanceManager, &PerformanceManager::optimizationCompleted,
                this, &PerformanceWidget::showOptimizationResult);
    }
    
    refreshDisplay();
}

PerformanceManager* PerformanceWidget::performanceManager() const
{
    QMutexLocker locker(&m_mutex);
    return m_performanceManager;
}

void PerformanceWidget::setConfig(PerformanceConfig* config)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_config == config) {
        return;
    }
    
    // 断开旧连接
    if (m_config) {
        disconnect(m_config, nullptr, this, nullptr);
    }
    
    m_config = config;
    
    // 建立新连接
    if (m_config) {
        connect(m_config, &PerformanceConfig::configChanged,
                this, [this](PerformanceConfig::ConfigCategory category, const QString& key, const QVariant& value) {
                    Q_UNUSED(category)
                    Q_UNUSED(key)
                    Q_UNUSED(value)
                    refreshDisplay();
                });
    }
    
    // 应用配置
    if (m_config) {
        setUpdateInterval(m_config->chartUpdateInterval());
        
        if (m_config->isRealTimeDisplayEnabled()) {
            startRealTimeUpdate();
        } else {
            stopRealTimeUpdate();
        }
    }
}

PerformanceConfig* PerformanceWidget::config() const
{
    QMutexLocker locker(&m_mutex);
    return m_config;
}void Per
formanceWidget::setDisplayMode(DisplayMode mode)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_displayMode == mode) {
        return;
    }
    
    m_displayMode = mode;
    applyDisplayMode();
}

PerformanceWidget::DisplayMode PerformanceWidget::displayMode() const
{
    QMutexLocker locker(&m_mutex);
    return m_displayMode;
}

void PerformanceWidget::setUpdateInterval(int interval)
{
    QMutexLocker locker(&m_mutex);
    
    if (interval < 100 || interval > 60000) {
        qWarning() << "PerformanceWidget: Invalid update interval:" << interval;
        return;
    }
    
    m_updateInterval = interval;
    
    if (m_updateTimer->isActive()) {
        m_updateTimer->setInterval(m_updateInterval);
    }
    
    // 更新图表的更新间隔
    if (m_cpuChart) {
        m_cpuChart->setUpdateInterval(m_updateInterval);
    }
    if (m_memoryChart) {
        m_memoryChart->setUpdateInterval(m_updateInterval);
    }
    if (m_networkChart) {
        m_networkChart->setUpdateInterval(m_updateInterval);
    }
}

int PerformanceWidget::updateInterval() const
{
    QMutexLocker locker(&m_mutex);
    return m_updateInterval;
}

void PerformanceWidget::startRealTimeUpdate()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_realTimeUpdateActive) {
        return;
    }
    
    m_realTimeUpdateActive = true;
    m_updateTimer->start(m_updateInterval);
    
    // 启动图表的实时更新
    if (m_cpuChart) {
        m_cpuChart->startRealTimeUpdate();
    }
    if (m_memoryChart) {
        m_memoryChart->startRealTimeUpdate();
    }
    if (m_networkChart) {
        m_networkChart->startRealTimeUpdate();
    }
    
    updateStatusDisplay();
    
    qDebug() << "PerformanceWidget: Real-time update started";
}

void PerformanceWidget::stopRealTimeUpdate()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_realTimeUpdateActive) {
        return;
    }
    
    m_realTimeUpdateActive = false;
    m_updateTimer->stop();
    
    // 停止图表的实时更新
    if (m_cpuChart) {
        m_cpuChart->stopRealTimeUpdate();
    }
    if (m_memoryChart) {
        m_memoryChart->stopRealTimeUpdate();
    }
    if (m_networkChart) {
        m_networkChart->stopRealTimeUpdate();
    }
    
    updateStatusDisplay();
    
    qDebug() << "PerformanceWidget: Real-time update stopped";
}boo
l PerformanceWidget::isRealTimeUpdateActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_realTimeUpdateActive;
}

void PerformanceWidget::refreshDisplay()
{
    if (!m_performanceManager) {
        return;
    }
    
    // 获取最新指标
    PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
    updateMetrics(metrics);
    
    // 更新性能等级
    PerformanceManager::PerformanceLevel level = m_performanceManager->getCurrentPerformanceLevel();
    updatePerformanceLevel(level);
    
    // 更新系统信息
    updateSystemInfo();
    
    // 更新状态显示
    updateStatusDisplay();
}

void PerformanceWidget::resetInterface()
{
    // 清除图表数据
    if (m_cpuChart) {
        m_cpuChart->clearData();
    }
    if (m_memoryChart) {
        m_memoryChart->clearData();
    }
    if (m_networkChart) {
        m_networkChart->clearData();
    }
    
    // 重置进度条
    if (m_cpuProgressBar) {
        m_cpuProgressBar->setValue(0);
    }
    if (m_memoryProgressBar) {
        m_memoryProgressBar->setValue(0);
    }
    if (m_networkProgressBar) {
        m_networkProgressBar->setValue(0);
    }
    
    // 重置标签
    if (m_cpuUsageLabel) {
        m_cpuUsageLabel->setText("CPU: 0%");
    }
    if (m_memoryUsageLabel) {
        m_memoryUsageLabel->setText("Memory: 0 MB");
    }
    if (m_networkLatencyLabel) {
        m_networkLatencyLabel->setText("Network: 0 ms");
    }
    if (m_performanceLevelLabel) {
        m_performanceLevelLabel->setText("Performance: Unknown");
    }
    
    qDebug() << "PerformanceWidget: Interface reset";
}

bool PerformanceWidget::exportPerformanceData(const QString& filePath, const QString& format)
{
    if (!m_performanceManager) {
        qWarning() << "PerformanceWidget: No performance manager available for export";
        return false;
    }
    
    try {
        QVariantMap exportData;
        exportData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        exportData["format"] = format;
        exportData["displayMode"] = static_cast<int>(m_displayMode);
        exportData["updateInterval"] = m_updateInterval;
        
        // 导出当前指标
        exportData["currentMetrics"] = QVariantMap{
            {"cpu", m_lastMetrics.system.cpuUsage},
            {"memory", static_cast<qint64>(m_lastMetrics.system.memoryUsage)},
            {"networkLatency", m_lastMetrics.network.latency},
            {"networkBandwidth", m_lastMetrics.network.bandwidth},
            {"audioLatency", m_lastMetrics.audio.latency},
            {"videoFrameRate", m_lastMetrics.video.frameRate}
        };
        
        // 导出图表统计信息
        if (m_cpuChart) {
            exportData["cpuStatistics"] = m_cpuChart->getStatistics();
        }
        if (m_memoryChart) {
            exportData["memoryStatistics"] = m_memoryChart->getStatistics();
        }
        if (m_networkChart) {
            exportData["networkStatistics"] = m_networkChart->getStatistics();
        }
        
        // 保存到文件
        QJsonDocument doc = QJsonDocument::fromVariant(exportData);
        
        QFile file(filePath);
        if (!file.open(QIODevice::WriteOnly)) {
            qWarning() << "PerformanceWidget: Failed to open export file:" << filePath;
            return false;
        }
        
        file.write(doc.toJson());
        file.close();
        
        qDebug() << "PerformanceWidget: Performance data exported to:" << filePath;
        return true;
        
    } catch (const std::exception& e) {
        qCritical() << "PerformanceWidget: Exception during export:" << e.what();
        return false;
    }
}QVaria
ntMap PerformanceWidget::getInterfaceState() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap state;
    state["displayMode"] = static_cast<int>(m_displayMode);
    state["updateInterval"] = m_updateInterval;
    state["realTimeUpdateActive"] = m_realTimeUpdateActive;
    
    if (m_tabWidget) {
        state["currentTab"] = m_tabWidget->currentIndex();
    }
    
    return state;
}

void PerformanceWidget::setInterfaceState(const QVariantMap& state)
{
    QMutexLocker locker(&m_mutex);
    
    if (state.contains("displayMode")) {
        m_displayMode = static_cast<DisplayMode>(state["displayMode"].toInt());
        applyDisplayMode();
    }
    
    if (state.contains("updateInterval")) {
        setUpdateInterval(state["updateInterval"].toInt());
    }
    
    if (state.contains("realTimeUpdateActive")) {
        bool active = state["realTimeUpdateActive"].toBool();
        if (active) {
            startRealTimeUpdate();
        } else {
            stopRealTimeUpdate();
        }
    }
    
    if (state.contains("currentTab") && m_tabWidget) {
        m_tabWidget->setCurrentIndex(state["currentTab"].toInt());
    }
}

void PerformanceWidget::updateMetrics(const PerformanceMetrics& metrics)
{
    QMutexLocker locker(&m_mutex);
    
    m_lastMetrics = metrics;
    
    // 更新指标显示
    updateMetricsDisplay(metrics);
    
    // 更新图表显示
    updateChartsDisplay(metrics);
}

void PerformanceWidget::updatePerformanceLevel(PerformanceManager::PerformanceLevel level)
{
    if (!m_performanceLevelLabel) {
        return;
    }
    
    QString levelText;
    QColor levelColor;
    
    switch (level) {
    case PerformanceManager::Excellent:
        levelText = "Excellent";
        levelColor = Qt::green;
        break;
    case PerformanceManager::Good:
        levelText = "Good";
        levelColor = QColor(144, 238, 144); // Light green
        break;
    case PerformanceManager::Fair:
        levelText = "Fair";
        levelColor = Qt::yellow;
        break;
    case PerformanceManager::Poor:
        levelText = "Poor";
        levelColor = QColor(255, 165, 0); // Orange
        break;
    case PerformanceManager::Critical:
        levelText = "Critical";
        levelColor = Qt::red;
        break;
    default:
        levelText = "Unknown";
        levelColor = Qt::gray;
        break;
    }
    
    m_performanceLevelLabel->setText(QString("Performance: %1").arg(levelText));
    
    // 设置颜色
    QPalette palette = m_performanceLevelLabel->palette();
    palette.setColor(QPalette::WindowText, levelColor);
    m_performanceLevelLabel->setPalette(palette);
}void Per
formanceWidget::showThresholdWarning(const QString& metricName, double value, double threshold)
{
    QString message = QString("Performance threshold exceeded!\n\n"
                             "Metric: %1\n"
                             "Current Value: %2\n"
                             "Threshold: %3")
                      .arg(metricName)
                      .arg(value, 0, 'f', 2)
                      .arg(threshold, 0, 'f', 2);
    
    QMessageBox::warning(this, "Performance Warning", message);
    
    qWarning() << "PerformanceWidget: Threshold warning -" << metricName << ":" << value << ">" << threshold;
}

void PerformanceWidget::showOptimizationResult(bool success, const QVariantMap& improvements)
{
    if (!m_optimizationStatusLabel) {
        return;
    }
    
    QString statusText;
    QColor statusColor;
    
    if (success) {
        statusText = "Optimization completed successfully";
        statusColor = Qt::green;
        
        // 显示改进信息
        if (!improvements.isEmpty()) {
            QStringList improvementList;
            for (auto it = improvements.begin(); it != improvements.end(); ++it) {
                improvementList << QString("%1: %2").arg(it.key(), it.value().toString());
            }
            
            QString message = QString("Optimization completed!\n\nImprovements:\n%1")
                              .arg(improvementList.join("\n"));
            
            QMessageBox::information(this, "Optimization Result", message);
        }
    } else {
        statusText = "Optimization failed";
        statusColor = Qt::red;
        
        QMessageBox::warning(this, "Optimization Result", "Optimization failed. Please check system resources.");
    }
    
    m_optimizationStatusLabel->setText(statusText);
    
    QPalette palette = m_optimizationStatusLabel->palette();
    palette.setColor(QPalette::WindowText, statusColor);
    m_optimizationStatusLabel->setPalette(palette);
    
    // 更新上次优化时间
    if (m_lastOptimizationLabel) {
        m_lastOptimizationLabel->setText(QString("Last optimization: %1")
                                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
    }
}

void PerformanceWidget::performManualOptimization()
{
    if (!m_performanceManager) {
        QMessageBox::warning(this, "Error", "No performance manager available");
        return;
    }
    
    // 禁用优化按钮防止重复点击
    if (m_optimizeButton) {
        m_optimizeButton->setEnabled(false);
        m_optimizeButton->setText("Optimizing...");
    }
    
    // 更新状态
    if (m_optimizationStatusLabel) {
        m_optimizationStatusLabel->setText("Optimization in progress...");
        QPalette palette = m_optimizationStatusLabel->palette();
        palette.setColor(QPalette::WindowText, Qt::blue);
        m_optimizationStatusLabel->setPalette(palette);
    }
    
    // 执行优化
    bool success = m_performanceManager->performOptimization();
    
    // 恢复按钮状态
    if (m_optimizeButton) {
        m_optimizeButton->setEnabled(true);
        m_optimizeButton->setText("Optimize Now");
    }
    
    if (!success) {
        showOptimizationResult(false, QVariantMap());
    }
}void P
erformanceWidget::openConfigurationDialog()
{
    if (!m_config) {
        QMessageBox::warning(this, "Error", "No configuration available");
        return;
    }
    
    // 这里应该打开一个配置对话框
    // 为了简化，我们显示一个消息框
    QMessageBox::information(this, "Configuration", 
                           "Configuration dialog would open here.\n"
                           "Current update interval: " + QString::number(m_updateInterval) + " ms");
}

void PerformanceWidget::toggleMonitoring()
{
    if (!m_performanceManager) {
        return;
    }
    
    bool currentlyActive = isRealTimeUpdateActive();
    
    if (currentlyActive) {
        stopRealTimeUpdate();
        if (m_performanceManager->isMonitoringActive()) {
            m_performanceManager->stopMonitoring();
        }
    } else {
        if (!m_performanceManager->isMonitoringActive()) {
            m_performanceManager->startMonitoring();
        }
        startRealTimeUpdate();
    }
    
    emit monitoringToggled(!currentlyActive);
}

void PerformanceWidget::toggleAutoOptimization()
{
    if (!m_performanceManager || !m_config) {
        return;
    }
    
    bool currentlyEnabled = m_config->isAutoOptimizationEnabled();
    m_config->setAutoOptimizationEnabled(!currentlyEnabled);
    
    if (m_autoOptimizeButton) {
        m_autoOptimizeButton->setText(currentlyEnabled ? "Enable Auto Optimization" : "Disable Auto Optimization");
    }
    
    emit autoOptimizationToggled(!currentlyEnabled);
}

void PerformanceWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
}

void PerformanceWidget::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // 应用显示模式以适应新尺寸
    applyDisplayMode();
}

void PerformanceWidget::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);
    
    // 如果配置允许，启动实时更新
    if (m_config && m_config->isRealTimeDisplayEnabled() && !m_realTimeUpdateActive) {
        startRealTimeUpdate();
    }
}

void PerformanceWidget::hideEvent(QHideEvent* event)
{
    QWidget::hideEvent(event);
    
    // 隐藏时停止实时更新以节省资源
    if (m_realTimeUpdateActive) {
        stopRealTimeUpdate();
    }
}

void PerformanceWidget::handleRealTimeUpdate()
{
    if (!m_performanceManager) {
        return;
    }
    
    // 获取最新指标并更新显示
    PerformanceMetrics metrics = m_performanceManager->getCurrentMetrics();
    updateMetrics(metrics);
    
    // 更新性能等级
    PerformanceManager::PerformanceLevel level = m_performanceManager->getCurrentPerformanceLevel();
    updatePerformanceLevel(level);
}void Perf
ormanceWidget::handleTabChanged(int index)
{
    Q_UNUSED(index)
    
    // 根据当前标签页调整更新策略
    if (m_tabWidget) {
        QString tabText = m_tabWidget->tabText(index);
        
        if (tabText == "Charts" && m_realTimeUpdateActive) {
            // 图表标签页需要更频繁的更新
            if (m_cpuChart) m_cpuChart->startRealTimeUpdate();
            if (m_memoryChart) m_memoryChart->startRealTimeUpdate();
            if (m_networkChart) m_networkChart->startRealTimeUpdate();
        }
    }
}

void PerformanceWidget::handleMonitorStatusChanged(const QString& monitorName, const QString& status)
{
    qDebug() << "PerformanceWidget: Monitor status changed -" << monitorName << ":" << status;
    
    // 更新状态显示
    updateStatusDisplay();
}

void PerformanceWidget::initializeUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // 创建标签页组件
    m_tabWidget = new QTabWidget(this);
    
    // 创建各个标签页
    m_tabWidget->addTab(createOverviewTab(), "Overview");
    m_tabWidget->addTab(createDetailsTab(), "Details");
    m_tabWidget->addTab(createChartsTab(), "Charts");
    m_tabWidget->addTab(createOptimizationTab(), "Optimization");
    m_tabWidget->addTab(createSettingsTab(), "Settings");
    
    // 连接标签页切换信号
    connect(m_tabWidget, QOverload<int>::of(&QTabWidget::currentChanged),
            this, &PerformanceWidget::handleTabChanged);
    
    m_mainLayout->addWidget(m_tabWidget);
    
    // 创建状态栏
    QWidget* statusBar = createStatusBar();
    m_mainLayout->addWidget(statusBar);
    
    // 应用显示模式
    applyDisplayMode();
    
    setLayout(m_mainLayout);
}

QWidget* PerformanceWidget::createOverviewTab()
{
    QWidget* overviewWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(overviewWidget);
    
    // 创建系统信息组
    QGroupBox* systemGroup = createSystemInfoGroup();
    layout->addWidget(systemGroup);
    
    // 创建性能指标组
    QGroupBox* metricsGroup = createMetricsGroup();
    layout->addWidget(metricsGroup);
    
    // 创建控制面板组
    QGroupBox* controlGroup = createControlPanelGroup();
    layout->addWidget(controlGroup);
    
    layout->addStretch();
    
    return overviewWidget;
}QWidge
t* PerformanceWidget::createDetailsTab()
{
    QWidget* detailsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(detailsWidget);
    
    // 系统信息
    QGroupBox* systemInfoGroup = new QGroupBox("System Information");
    QVBoxLayout* systemLayout = new QVBoxLayout(systemInfoGroup);
    
    m_systemInfoLabel = new QLabel("System information will be displayed here");
    m_systemInfoLabel->setWordWrap(true);
    systemLayout->addWidget(m_systemInfoLabel);
    
    layout->addWidget(systemInfoGroup);
    
    // 音频指标
    QGroupBox* audioGroup = new QGroupBox("Audio Metrics");
    QVBoxLayout* audioLayout = new QVBoxLayout(audioGroup);
    
    m_audioMetricsLabel = new QLabel("Audio metrics will be displayed here");
    m_audioMetricsLabel->setWordWrap(true);
    audioLayout->addWidget(m_audioMetricsLabel);
    
    layout->addWidget(audioGroup);
    
    // 视频指标
    QGroupBox* videoGroup = new QGroupBox("Video Metrics");
    QVBoxLayout* videoLayout = new QVBoxLayout(videoGroup);
    
    m_videoMetricsLabel = new QLabel("Video metrics will be displayed here");
    m_videoMetricsLabel->setWordWrap(true);
    videoLayout->addWidget(m_videoMetricsLabel);
    
    layout->addWidget(videoGroup);
    
    // 网络指标
    QGroupBox* networkGroup = new QGroupBox("Network Metrics");
    QVBoxLayout* networkLayout = new QVBoxLayout(networkGroup);
    
    m_networkMetricsLabel = new QLabel("Network metrics will be displayed here");
    m_networkMetricsLabel->setWordWrap(true);
    networkLayout->addWidget(m_networkMetricsLabel);
    
    layout->addWidget(networkGroup);
    
    layout->addStretch();
    
    return detailsWidget;
}

QWidget* PerformanceWidget::createChartsTab()
{
    QWidget* chartsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(chartsWidget);
    
    // 创建图表网格
    QGridLayout* chartsLayout = new QGridLayout();
    
    // CPU图表
    m_cpuChart = new MetricsChart();
    m_cpuChart->setMetricType(MetricsChart::CPUUsage);
    m_cpuChart->setChartTitle("CPU Usage");
    m_cpuChart->setYAxisLabel("Usage (%)");
    chartsLayout->addWidget(m_cpuChart, 0, 0);
    
    // 内存图表
    m_memoryChart = new MetricsChart();
    m_memoryChart->setMetricType(MetricsChart::MemoryUsage);
    m_memoryChart->setChartTitle("Memory Usage");
    m_memoryChart->setYAxisLabel("Usage (MB)");
    chartsLayout->addWidget(m_memoryChart, 0, 1);
    
    // 网络图表
    m_networkChart = new MetricsChart();
    m_networkChart->setMetricType(MetricsChart::NetworkLatency);
    m_networkChart->setChartTitle("Network Latency");
    m_networkChart->setYAxisLabel("Latency (ms)");
    chartsLayout->addWidget(m_networkChart, 1, 0, 1, 2);
    
    QWidget* chartsContainer = new QWidget();
    chartsContainer->setLayout(chartsLayout);
    layout->addWidget(chartsContainer);
    
    return chartsWidget;
}QWidget*
 PerformanceWidget::createOptimizationTab()
{
    QWidget* optimizationWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(optimizationWidget);
    
    // 优化控制组
    QGroupBox* controlGroup = new QGroupBox("Optimization Control");
    QVBoxLayout* controlLayout = new QVBoxLayout(controlGroup);
    
    // 手动优化按钮
    m_optimizeButton = new QPushButton("Optimize Now");
    connect(m_optimizeButton, &QPushButton::clicked, this, &PerformanceWidget::performManualOptimization);
    controlLayout->addWidget(m_optimizeButton);
    
    // 自动优化按钮
    m_autoOptimizeButton = new QPushButton("Enable Auto Optimization");
    connect(m_autoOptimizeButton, &QPushButton::clicked, this, &PerformanceWidget::toggleAutoOptimization);
    controlLayout->addWidget(m_autoOptimizeButton);
    
    layout->addWidget(controlGroup);
    
    // 优化状态组
    QGroupBox* statusGroup = new QGroupBox("Optimization Status");
    QVBoxLayout* statusLayout = new QVBoxLayout(statusGroup);
    
    m_optimizationStatusLabel = new QLabel("Ready for optimization");
    statusLayout->addWidget(m_optimizationStatusLabel);
    
    m_lastOptimizationLabel = new QLabel("Last optimization: Never");
    statusLayout->addWidget(m_lastOptimizationLabel);
    
    layout->addWidget(statusGroup);
    
    layout->addStretch();
    
    return optimizationWidget;
}

QWidget* PerformanceWidget::createSettingsTab()
{
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(settingsWidget);
    
    // 设置控制组
    QGroupBox* settingsGroup = new QGroupBox("Settings");
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsGroup);
    
    // 配置按钮
    m_configButton = new QPushButton("Open Configuration");
    connect(m_configButton, &QPushButton::clicked, this, &PerformanceWidget::openConfigurationDialog);
    settingsLayout->addWidget(m_configButton);
    
    // 导出按钮
    m_exportButton = new QPushButton("Export Performance Data");
    connect(m_exportButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Export Performance Data", 
                                                       "performance_data.json", "JSON Files (*.json)");
        if (!fileName.isEmpty()) {
            if (exportPerformanceData(fileName)) {
                QMessageBox::information(this, "Export", "Performance data exported successfully!");
            } else {
                QMessageBox::warning(this, "Export", "Failed to export performance data.");
            }
        }
    });
    settingsLayout->addWidget(m_exportButton);
    
    // 重置按钮
    m_resetButton = new QPushButton("Reset Interface");
    connect(m_resetButton, &QPushButton::clicked, this, &PerformanceWidget::resetInterface);
    settingsLayout->addWidget(m_resetButton);
    
    layout->addWidget(settingsGroup);
    
    layout->addStretch();
    
    return settingsWidget;
}QGroup
Box* PerformanceWidget::createSystemInfoGroup()
{
    QGroupBox* systemGroup = new QGroupBox("System Information");
    QVBoxLayout* layout = new QVBoxLayout(systemGroup);
    
    // 性能等级标签
    m_performanceLevelLabel = new QLabel("Performance: Unknown");
    QFont font = m_performanceLevelLabel->font();
    font.setBold(true);
    font.setPointSize(font.pointSize() + 2);
    m_performanceLevelLabel->setFont(font);
    layout->addWidget(m_performanceLevelLabel);
    
    return systemGroup;
}

QGroupBox* PerformanceWidget::createMetricsGroup()
{
    QGroupBox* metricsGroup = new QGroupBox("Performance Metrics");
    QGridLayout* layout = new QGridLayout(metricsGroup);
    
    // CPU使用率
    layout->addWidget(new QLabel("CPU Usage:"), 0, 0);
    m_cpuUsageLabel = new QLabel("0%");
    layout->addWidget(m_cpuUsageLabel, 0, 1);
    m_cpuProgressBar = new QProgressBar();
    m_cpuProgressBar->setRange(0, 100);
    layout->addWidget(m_cpuProgressBar, 0, 2);
    
    // 内存使用
    layout->addWidget(new QLabel("Memory Usage:"), 1, 0);
    m_memoryUsageLabel = new QLabel("0 MB");
    layout->addWidget(m_memoryUsageLabel, 1, 1);
    m_memoryProgressBar = new QProgressBar();
    m_memoryProgressBar->setRange(0, 100);
    layout->addWidget(m_memoryProgressBar, 1, 2);
    
    // 网络延迟
    layout->addWidget(new QLabel("Network Latency:"), 2, 0);
    m_networkLatencyLabel = new QLabel("0 ms");
    layout->addWidget(m_networkLatencyLabel, 2, 1);
    m_networkProgressBar = new QProgressBar();
    m_networkProgressBar->setRange(0, 1000); // 0-1000ms
    layout->addWidget(m_networkProgressBar, 2, 2);
    
    return metricsGroup;
}

QGroupBox* PerformanceWidget::createControlPanelGroup()
{
    QGroupBox* controlGroup = new QGroupBox("Control Panel");
    QHBoxLayout* layout = new QHBoxLayout(controlGroup);
    
    // 启动/停止按钮
    m_startStopButton = new QPushButton("Start Monitoring");
    connect(m_startStopButton, &QPushButton::clicked, this, &PerformanceWidget::toggleMonitoring);
    layout->addWidget(m_startStopButton);
    
    layout->addStretch();
    
    return controlGroup;
}

QWidget* PerformanceWidget::createStatusBar()
{
    QWidget* statusWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(statusWidget);
    layout->setContentsMargins(0, 0, 0, 0);
    
    m_statusLabel = new QLabel("Ready");
    layout->addWidget(m_statusLabel);
    
    layout->addStretch();
    
    // 添加更新间隔显示
    QLabel* intervalLabel = new QLabel(QString("Update: %1ms").arg(m_updateInterval));
    layout->addWidget(intervalLabel);
    
    return statusWidget;
}void 
PerformanceWidget::updateSystemInfo()
{
    if (!m_systemInfoLabel) {
        return;
    }
    
    QStringList systemInfo;
    systemInfo << QString("Application: %1").arg(QApplication::applicationName());
    systemInfo << QString("Version: %1").arg(QApplication::applicationVersion());
    systemInfo << QString("Qt Version: %1").arg(qVersion());
    systemInfo << QString("Update Time: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss"));
    
    if (m_performanceManager) {
        systemInfo << QString("Monitoring Active: %1").arg(m_performanceManager->isMonitoringActive() ? "Yes" : "No");
    }
    
    m_systemInfoLabel->setText(systemInfo.join("\n"));
}

void PerformanceWidget::updateMetricsDisplay(const PerformanceMetrics& metrics)
{
    // 更新CPU显示
    if (m_cpuUsageLabel && m_cpuProgressBar) {
        double cpuUsage = metrics.system.cpuUsage;
        m_cpuUsageLabel->setText(formatPercentage(cpuUsage));
        m_cpuProgressBar->setValue(static_cast<int>(cpuUsage));
        
        // 设置进度条颜色
        QString style = QString("QProgressBar::chunk { background-color: %1; }")
                       .arg(cpuUsage > 80 ? "red" : (cpuUsage > 60 ? "orange" : "green"));
        m_cpuProgressBar->setStyleSheet(style);
    }
    
    // 更新内存显示
    if (m_memoryUsageLabel && m_memoryProgressBar) {
        qint64 memoryUsage = static_cast<qint64>(metrics.system.memoryUsage);
        m_memoryUsageLabel->setText(formatBytes(memoryUsage * 1024 * 1024)); // MB to bytes
        
        // 假设系统总内存为8GB，计算百分比
        double memoryPercent = (memoryUsage / 8192.0) * 100.0;
        m_memoryProgressBar->setValue(static_cast<int>(memoryPercent));
        
        QString style = QString("QProgressBar::chunk { background-color: %1; }")
                       .arg(memoryPercent > 80 ? "red" : (memoryPercent > 60 ? "orange" : "green"));
        m_memoryProgressBar->setStyleSheet(style);
    }
    
    // 更新网络延迟显示
    if (m_networkLatencyLabel && m_networkProgressBar) {
        double latency = metrics.network.latency;
        m_networkLatencyLabel->setText(formatTime(latency));
        m_networkProgressBar->setValue(static_cast<int>(latency));
        
        QString style = QString("QProgressBar::chunk { background-color: %1; }")
                       .arg(latency > 200 ? "red" : (latency > 100 ? "orange" : "green"));
        m_networkProgressBar->setStyleSheet(style);
    }
    
    // 更新详细指标标签
    if (m_audioMetricsLabel) {
        QStringList audioInfo;
        audioInfo << QString("Latency: %1 ms").arg(metrics.audio.latency, 0, 'f', 2);
        audioInfo << QString("Jitter: %1 ms").arg(metrics.audio.jitter, 0, 'f', 2);
        audioInfo << QString("Packet Loss: %1%").arg(metrics.audio.packetLoss, 0, 'f', 2);
        audioInfo << QString("Sample Rate: %1 Hz").arg(metrics.audio.sampleRate);
        audioInfo << QString("Bitrate: %1 kbps").arg(metrics.audio.bitrate);
        
        m_audioMetricsLabel->setText(audioInfo.join("\n"));
    }
    
    if (m_videoMetricsLabel) {
        QStringList videoInfo;
        videoInfo << QString("Frame Rate: %1 fps").arg(metrics.video.frameRate, 0, 'f', 2);
        videoInfo << QString("Resolution: %1x%2").arg(metrics.video.resolution.width()).arg(metrics.video.resolution.height());
        videoInfo << QString("Bitrate: %1 kbps").arg(metrics.video.bitrate);
        videoInfo << QString("Encoding Time: %1 ms").arg(metrics.video.encodingTime, 0, 'f', 2);
        videoInfo << QString("Decoding Time: %1 ms").arg(metrics.video.decodingTime, 0, 'f', 2);
        
        m_videoMetricsLabel->setText(videoInfo.join("\n"));
    }
    
    if (m_networkMetricsLabel) {
        QStringList networkInfo;
        networkInfo << QString("Bandwidth: %1 Mbps").arg(metrics.network.bandwidth, 0, 'f', 2);
        networkInfo << QString("Latency: %1 ms").arg(metrics.network.latency, 0, 'f', 2);
        networkInfo << QString("Packet Loss: %1%").arg(metrics.network.packetLoss, 0, 'f', 2);
        networkInfo << QString("Connection Quality: %1/100").arg(metrics.network.connectionQuality);
        
        m_networkMetricsLabel->setText(networkInfo.join("\n"));
    }
}v
oid PerformanceWidget::updateChartsDisplay(const PerformanceMetrics& metrics)
{
    QDateTime currentTime = QDateTime::currentDateTime();
    
    // 更新CPU图表
    if (m_cpuChart) {
        m_cpuChart->addDataPoint(currentTime, metrics.system.cpuUsage);
    }
    
    // 更新内存图表
    if (m_memoryChart) {
        m_memoryChart->addDataPoint(currentTime, metrics.system.memoryUsage);
    }
    
    // 更新网络图表
    if (m_networkChart) {
        m_networkChart->addDataPoint(currentTime, metrics.network.latency);
    }
}

void PerformanceWidget::updateStatusDisplay()
{
    if (!m_statusLabel || !m_startStopButton) {
        return;
    }
    
    if (m_realTimeUpdateActive) {
        m_statusLabel->setText("Monitoring Active");
        m_startStopButton->setText("Stop Monitoring");
    } else {
        m_statusLabel->setText("Monitoring Stopped");
        m_startStopButton->setText("Start Monitoring");
    }
}

void PerformanceWidget::applyDisplayMode()
{
    if (!m_tabWidget) {
        return;
    }
    
    switch (m_displayMode) {
    case CompactMode:
        // 只显示概览标签页
        for (int i = 1; i < m_tabWidget->count(); ++i) {
            m_tabWidget->setTabVisible(i, false);
        }
        m_tabWidget->setCurrentIndex(0);
        break;
        
    case DetailedMode:
        // 显示所有标签页
        for (int i = 0; i < m_tabWidget->count(); ++i) {
            m_tabWidget->setTabVisible(i, true);
        }
        break;
        
    case DashboardMode:
        // 显示概览和图表
        m_tabWidget->setTabVisible(0, true);  // Overview
        m_tabWidget->setTabVisible(1, false); // Details
        m_tabWidget->setTabVisible(2, true);  // Charts
        m_tabWidget->setTabVisible(3, true);  // Optimization
        m_tabWidget->setTabVisible(4, false); // Settings
        break;
        
    case MinimalMode:
        // 只显示基本信息
        for (int i = 1; i < m_tabWidget->count(); ++i) {
            m_tabWidget->setTabVisible(i, false);
        }
        m_tabWidget->setCurrentIndex(0);
        break;
    }
}

QString PerformanceWidget::formatBytes(qint64 bytes) const
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / (double)GB, 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / (double)MB, 0, 'f', 1);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / (double)KB, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(bytes);
    }
}

QString PerformanceWidget::formatPercentage(double percentage) const
{
    return QString("%1%").arg(percentage, 0, 'f', 1);
}

QString PerformanceWidget::formatTime(double milliseconds) const
{
    if (milliseconds >= 1000) {
        return QString("%1 s").arg(milliseconds / 1000.0, 0, 'f', 2);
    } else {
        return QString("%1 ms").arg(milliseconds, 0, 'f', 1);
    }
}

QColor PerformanceWidget::getPerformanceLevelColor(PerformanceManager::PerformanceLevel level) const
{
    switch (level) {
    case PerformanceManager::Excellent: return Qt::green;
    case PerformanceManager::Good: return QColor(144, 238, 144);
    case PerformanceManager::Fair: return Qt::yellow;
    case PerformanceManager::Poor: return QColor(255, 165, 0);
    case PerformanceManager::Critical: return Qt::red;
    default: return Qt::gray;
    }
}
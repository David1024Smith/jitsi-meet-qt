#include "NetworkStatusWidget.h"
#include "../interfaces/INetworkManager.h"
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QStyle>
#include <QToolTip>
#include <QDebug>

class NetworkStatusWidget::Private
{
public:
    // 网络管理器
    INetworkManager* networkManager;
    
    // 显示设置
    DisplayMode displayMode;
    bool autoUpdate;
    int updateInterval;
    bool showDetails;
    
    // UI组件
    QVBoxLayout* mainLayout;
    QHBoxLayout* compactLayout;
    QGridLayout* detailedLayout;
    
    // 状态显示组件
    QLabel* connectionStatusLabel;
    QLabel* connectionStatusIcon;
    QLabel* qualityLabel;
    QLabel* qualityIcon;
    QProgressBar* qualityProgressBar;
    
    // 详细信息组件
    QLabel* latencyLabel;
    QLabel* bandwidthLabel;
    QLabel* packetLossLabel;
    QLabel* serverLabel;
    QLabel* protocolLabel;
    
    // 定时器
    QTimer* updateTimer;
    
    // 当前状态
    int currentConnectionState;
    int currentNetworkQuality;
    QVariantMap currentStats;
    
    Private() {
        networkManager = nullptr;
        displayMode = Compact;
        autoUpdate = true;
        updateInterval = 2000; // 2秒
        showDetails = false;
        
        mainLayout = nullptr;
        compactLayout = nullptr;
        detailedLayout = nullptr;
        
        connectionStatusLabel = nullptr;
        connectionStatusIcon = nullptr;
        qualityLabel = nullptr;
        qualityIcon = nullptr;
        qualityProgressBar = nullptr;
        
        latencyLabel = nullptr;
        bandwidthLabel = nullptr;
        packetLossLabel = nullptr;
        serverLabel = nullptr;
        protocolLabel = nullptr;
        
        updateTimer = nullptr;
        
        currentConnectionState = 0; // Disconnected
        currentNetworkQuality = 0;  // Poor
    }
};

NetworkStatusWidget::NetworkStatusWidget(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    initializeUI();
    
    // 创建更新定时器
    d->updateTimer = new QTimer(this);
    connect(d->updateTimer, &QTimer::timeout, this, &NetworkStatusWidget::handleUpdateTimer);
    
    // 设置默认样式
    applyStyles();
    
    // 启动自动更新
    if (d->autoUpdate) {
        startAutoUpdate();
    }
}

NetworkStatusWidget::~NetworkStatusWidget()
{
    delete d;
}

void NetworkStatusWidget::setNetworkManager(INetworkManager* manager)
{
    if (d->networkManager == manager) {
        return;
    }
    
    // 断开旧连接
    if (d->networkManager) {
        disconnect(d->networkManager, nullptr, this, nullptr);
    }
    
    d->networkManager = manager;
    
    // 建立新连接
    if (d->networkManager) {
        connect(d->networkManager, SIGNAL(connectionStateChanged(int)),
                this, SLOT(handleConnectionStateChanged(int)));
        connect(d->networkManager, SIGNAL(networkQualityChanged(int)),
                this, SLOT(handleNetworkQualityChanged(int)));
        
        // 初始化状态
        refreshStatus();
    }
}

INetworkManager* NetworkStatusWidget::networkManager() const
{
    return d->networkManager;
}

void NetworkStatusWidget::setDisplayMode(DisplayMode mode)
{
    if (d->displayMode == mode) {
        return;
    }
    
    d->displayMode = mode;
    
    // 重新创建UI
    initializeUI();
    refreshStatus();
}

NetworkStatusWidget::DisplayMode NetworkStatusWidget::displayMode() const
{
    return d->displayMode;
}

void NetworkStatusWidget::setAutoUpdate(bool enabled)
{
    if (d->autoUpdate == enabled) {
        return;
    }
    
    d->autoUpdate = enabled;
    
    if (enabled) {
        startAutoUpdate();
    } else {
        stopAutoUpdate();
    }
    
    emit autoUpdateChanged(enabled);
}

bool NetworkStatusWidget::autoUpdate() const
{
    return d->autoUpdate;
}

void NetworkStatusWidget::setUpdateInterval(int interval)
{
    if (d->updateInterval == interval) {
        return;
    }
    
    d->updateInterval = interval;
    
    if (d->updateTimer && d->updateTimer->isActive()) {
        d->updateTimer->setInterval(interval);
    }
    
    emit updateIntervalChanged(interval);
}

int NetworkStatusWidget::updateInterval() const
{
    return d->updateInterval;
}

void NetworkStatusWidget::setShowDetails(bool show)
{
    if (d->showDetails == show) {
        return;
    }
    
    d->showDetails = show;
    
    // 更新UI显示
    if (d->displayMode == Detailed) {
        initializeUI();
        refreshStatus();
    }
}

bool NetworkStatusWidget::showDetails() const
{
    return d->showDetails;
}

void NetworkStatusWidget::refreshStatus()
{
    if (!d->networkManager) {
        return;
    }
    
    // 更新连接状态
    // 注意：这里假设INetworkManager有相应的方法
    // 实际实现中需要根据具体接口调整
    updateConnectionStatus(d->currentConnectionState);
    updateNetworkQuality(d->currentNetworkQuality);
    updateStatistics(d->currentStats);
}

void NetworkStatusWidget::startAutoUpdate()
{
    if (d->updateTimer && d->autoUpdate) {
        d->updateTimer->start(d->updateInterval);
    }
}

void NetworkStatusWidget::stopAutoUpdate()
{
    if (d->updateTimer) {
        d->updateTimer->stop();
    }
}

void NetworkStatusWidget::reset()
{
    d->currentConnectionState = 0;
    d->currentNetworkQuality = 0;
    d->currentStats.clear();
    
    updateConnectionStatus(0);
    updateNetworkQuality(0);
    updateStatistics(QVariantMap());
}

void NetworkStatusWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit statusClicked();
        
        if (event->modifiers() & Qt::ControlModifier) {
            emit detailsRequested();
        }
    }
    
    QWidget::mousePressEvent(event);
}

void NetworkStatusWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}

QSize NetworkStatusWidget::sizeHint() const
{
    switch (d->displayMode) {
    case Compact:
        return QSize(200, 30);
    case Detailed:
        return QSize(300, 150);
    case Minimal:
        return QSize(100, 20);
    default:
        return QSize(200, 30);
    }
}

void NetworkStatusWidget::handleConnectionStateChanged(int state)
{
    d->currentConnectionState = state;
    updateConnectionStatus(state);
}

void NetworkStatusWidget::handleNetworkQualityChanged(int quality)
{
    d->currentNetworkQuality = quality;
    updateNetworkQuality(quality);
}

void NetworkStatusWidget::handleNetworkStatsUpdated(const QVariantMap& stats)
{
    d->currentStats = stats;
    updateStatistics(stats);
}

void NetworkStatusWidget::handleUpdateTimer()
{
    refreshStatus();
}

void NetworkStatusWidget::initializeUI()
{
    // 清理现有布局
    if (d->mainLayout) {
        QLayoutItem* item;
        while ((item = d->mainLayout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        delete d->mainLayout;
    }
    
    // 根据显示模式创建UI
    switch (d->displayMode) {
    case Compact:
        createCompactUI();
        break;
    case Detailed:
        createDetailedUI();
        break;
    case Minimal:
        createMinimalUI();
        break;
    }
    
    applyStyles();
}

void NetworkStatusWidget::createCompactUI()
{
    d->mainLayout = new QVBoxLayout(this);
    d->compactLayout = new QHBoxLayout();
    
    // 连接状态图标和文本
    d->connectionStatusIcon = new QLabel();
    d->connectionStatusIcon->setFixedSize(16, 16);
    d->connectionStatusLabel = new QLabel("Disconnected");
    
    // 网络质量指示器
    d->qualityIcon = new QLabel();
    d->qualityIcon->setFixedSize(16, 16);
    d->qualityProgressBar = new QProgressBar();
    d->qualityProgressBar->setMaximum(100);
    d->qualityProgressBar->setTextVisible(false);
    d->qualityProgressBar->setFixedHeight(8);
    
    d->compactLayout->addWidget(d->connectionStatusIcon);
    d->compactLayout->addWidget(d->connectionStatusLabel);
    d->compactLayout->addStretch();
    d->compactLayout->addWidget(d->qualityIcon);
    d->compactLayout->addWidget(d->qualityProgressBar);
    
    d->mainLayout->addLayout(d->compactLayout);
    d->mainLayout->setContentsMargins(5, 2, 5, 2);
}

void NetworkStatusWidget::createDetailedUI()
{
    d->mainLayout = new QVBoxLayout(this);
    d->detailedLayout = new QGridLayout();
    
    // 连接状态
    d->connectionStatusIcon = new QLabel();
    d->connectionStatusIcon->setFixedSize(24, 24);
    d->connectionStatusLabel = new QLabel("Disconnected");
    
    // 网络质量
    d->qualityLabel = new QLabel("Quality:");
    d->qualityProgressBar = new QProgressBar();
    d->qualityProgressBar->setMaximum(100);
    
    // 详细统计信息
    d->latencyLabel = new QLabel("Latency: --");
    d->bandwidthLabel = new QLabel("Bandwidth: --");
    d->packetLossLabel = new QLabel("Packet Loss: --");
    d->serverLabel = new QLabel("Server: --");
    d->protocolLabel = new QLabel("Protocol: --");
    
    // 布局
    d->detailedLayout->addWidget(d->connectionStatusIcon, 0, 0);
    d->detailedLayout->addWidget(d->connectionStatusLabel, 0, 1, 1, 2);
    
    d->detailedLayout->addWidget(d->qualityLabel, 1, 0);
    d->detailedLayout->addWidget(d->qualityProgressBar, 1, 1, 1, 2);
    
    if (d->showDetails) {
        d->detailedLayout->addWidget(d->latencyLabel, 2, 0, 1, 3);
        d->detailedLayout->addWidget(d->bandwidthLabel, 3, 0, 1, 3);
        d->detailedLayout->addWidget(d->packetLossLabel, 4, 0, 1, 3);
        d->detailedLayout->addWidget(d->serverLabel, 5, 0, 1, 3);
        d->detailedLayout->addWidget(d->protocolLabel, 6, 0, 1, 3);
    }
    
    d->mainLayout->addLayout(d->detailedLayout);
    d->mainLayout->setContentsMargins(10, 5, 10, 5);
}

void NetworkStatusWidget::createMinimalUI()
{
    d->mainLayout = new QVBoxLayout(this);
    QHBoxLayout* minimalLayout = new QHBoxLayout();
    
    // 只显示连接状态图标
    d->connectionStatusIcon = new QLabel();
    d->connectionStatusIcon->setFixedSize(16, 16);
    
    minimalLayout->addWidget(d->connectionStatusIcon);
    minimalLayout->addStretch();
    
    d->mainLayout->addLayout(minimalLayout);
    d->mainLayout->setContentsMargins(2, 2, 2, 2);
}

void NetworkStatusWidget::updateConnectionStatus(int state)
{
    QString statusText = getConnectionStateText(state);
    QColor statusColor = getConnectionStateColor(state);
    
    if (d->connectionStatusLabel) {
        d->connectionStatusLabel->setText(statusText);
        d->connectionStatusLabel->setStyleSheet(QString("color: %1;").arg(statusColor.name()));
    }
    
    if (d->connectionStatusIcon) {
        // 创建状态图标
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(statusColor);
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(2, 2, 12, 12);
        
        d->connectionStatusIcon->setPixmap(pixmap);
        d->connectionStatusIcon->setToolTip(statusText);
    }
}

void NetworkStatusWidget::updateNetworkQuality(int quality)
{
    QString qualityText = getNetworkQualityText(quality);
    QColor qualityColor = getNetworkQualityColor(quality);
    
    if (d->qualityProgressBar) {
        d->qualityProgressBar->setValue(quality);
        d->qualityProgressBar->setStyleSheet(QString(
            "QProgressBar::chunk { background-color: %1; }"
        ).arg(qualityColor.name()));
        d->qualityProgressBar->setToolTip(qualityText);
    }
    
    if (d->qualityIcon) {
        // 创建质量图标
        QPixmap pixmap(16, 16);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        
        // 绘制信号强度条
        int bars = (quality / 25) + 1; // 1-4 bars
        for (int i = 0; i < 4; ++i) {
            QColor barColor = (i < bars) ? qualityColor : QColor(200, 200, 200);
            painter.fillRect(i * 3 + 2, 12 - i * 2, 2, i * 2 + 2, barColor);
        }
        
        d->qualityIcon->setPixmap(pixmap);
        d->qualityIcon->setToolTip(qualityText);
    }
}

void NetworkStatusWidget::updateStatistics(const QVariantMap& stats)
{
    if (d->latencyLabel && stats.contains("latency")) {
        int latency = stats["latency"].toInt();
        d->latencyLabel->setText(QString("Latency: %1ms").arg(latency));
    }
    
    if (d->bandwidthLabel && stats.contains("bandwidth")) {
        double bandwidth = stats["bandwidth"].toDouble();
        d->bandwidthLabel->setText(QString("Bandwidth: %1 Mbps").arg(bandwidth, 0, 'f', 1));
    }
    
    if (d->packetLossLabel && stats.contains("packetLoss")) {
        double packetLoss = stats["packetLoss"].toDouble();
        d->packetLossLabel->setText(QString("Packet Loss: %1%").arg(packetLoss, 0, 'f', 1));
    }
    
    if (d->serverLabel && stats.contains("server")) {
        QString server = stats["server"].toString();
        d->serverLabel->setText(QString("Server: %1").arg(server));
    }
    
    if (d->protocolLabel && stats.contains("protocol")) {
        QString protocol = stats["protocol"].toString();
        d->protocolLabel->setText(QString("Protocol: %1").arg(protocol));
    }
}

QString NetworkStatusWidget::getConnectionStateText(int state) const
{
    switch (state) {
    case 0: return "Disconnected";
    case 1: return "Connecting";
    case 2: return "Connected";
    case 3: return "Error";
    default: return "Unknown";
    }
}

QString NetworkStatusWidget::getNetworkQualityText(int quality) const
{
    if (quality >= 75) return "Excellent";
    if (quality >= 50) return "Good";
    if (quality >= 25) return "Fair";
    return "Poor";
}

QColor NetworkStatusWidget::getConnectionStateColor(int state) const
{
    switch (state) {
    case 0: return QColor(128, 128, 128); // Gray - Disconnected
    case 1: return QColor(255, 165, 0);   // Orange - Connecting
    case 2: return QColor(0, 255, 0);     // Green - Connected
    case 3: return QColor(255, 0, 0);     // Red - Error
    default: return QColor(128, 128, 128);
    }
}

QColor NetworkStatusWidget::getNetworkQualityColor(int quality) const
{
    if (quality >= 75) return QColor(0, 255, 0);     // Green - Excellent
    if (quality >= 50) return QColor(173, 255, 47);  // GreenYellow - Good
    if (quality >= 25) return QColor(255, 165, 0);   // Orange - Fair
    return QColor(255, 0, 0);                        // Red - Poor
}

void NetworkStatusWidget::applyStyles()
{
    setStyleSheet(R"(
        NetworkStatusWidget {
            background-color: #f0f0f0;
            border: 1px solid #d0d0d0;
            border-radius: 4px;
        }
        
        NetworkStatusWidget:hover {
            background-color: #e8e8e8;
        }
        
        QLabel {
            font-size: 11px;
            color: #333333;
        }
        
        QProgressBar {
            border: 1px solid #d0d0d0;
            border-radius: 2px;
            background-color: #f8f8f8;
        }
        
        QProgressBar::chunk {
            border-radius: 2px;
        }
    )");
}
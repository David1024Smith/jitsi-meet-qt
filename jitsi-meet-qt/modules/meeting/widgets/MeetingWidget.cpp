#include "MeetingWidget.h"
#include "../include/MeetingManager.h"
#include "../models/Meeting.h"
#include "../config/MeetingConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QProgressBar>
#include <QGroupBox>
#include <QListWidget>
#include <QTabWidget>
#include <QSplitter>
#include <QFrame>
#include <QTimer>
#include <QClipboard>
#include <QApplication>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QToolButton>
#include <QSpacerItem>
#include <QPixmap>
#include <QIcon>
#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>

class MeetingWidget::Private
{
public:
    // 核心组件
    MeetingManager* meetingManager = nullptr;
    Meeting* currentMeeting = nullptr;
    
    // 显示设置
    DisplayMode displayMode = NormalMode;
    bool showControls = true;
    bool showParticipants = true;
    bool showStatistics = false;
    QString currentTheme = "default";
    
    // 主布局
    QVBoxLayout* mainLayout = nullptr;
    QSplitter* mainSplitter = nullptr;
    
    // 会议信息区域
    QGroupBox* meetingInfoGroup = nullptr;
    QLabel* meetingTitleLabel = nullptr;
    QLabel* meetingUrlLabel = nullptr;
    QLabel* meetingStatusLabel = nullptr;
    QLabel* meetingDurationLabel = nullptr;
    QLabel* participantCountLabel = nullptr;
    QProgressBar* connectionQualityBar = nullptr;
    QLabel* infoLabel = nullptr;
    
    // 控制按钮区域
    QGroupBox* controlsGroup = nullptr;
    QPushButton* joinButton = nullptr;
    QPushButton* leaveButton = nullptr;
    QPushButton* createButton = nullptr;
    QPushButton* settingsButton = nullptr;
    QPushButton* inviteButton = nullptr;
    QPushButton* copyLinkButton = nullptr;
    QLineEdit* urlInput = nullptr;
    
    // 参与者列表区域
    QGroupBox* participantsGroup = nullptr;
    QListWidget* participantsList = nullptr;
    QLabel* participantsCountLabel = nullptr;
    
    // 统计信息区域
    QGroupBox* statisticsGroup = nullptr;
    QLabel* audioQualityLabel = nullptr;
    QLabel* videoQualityLabel = nullptr;
    QLabel* networkLatencyLabel = nullptr;
    QLabel* bandwidthLabel = nullptr;
    
    // 状态栏
    QFrame* statusBar = nullptr;
    QLabel* statusLabel = nullptr;
    QProgressBar* loadingBar = nullptr;
    
    // 定时器
    QTimer* updateTimer = nullptr;
    QTimer* durationTimer = nullptr;
    
    // 状态
    bool isLoading = false;
    QDateTime meetingStartTime;
};

MeetingWidget::MeetingWidget(QWidget* parent)
    : QWidget(parent)
    , d(std::make_unique<Private>())
{
    initializeUI();
    setupLayout();
    connectSignals();
    applyStyles();
    
    // 初始化定时器
    d->updateTimer = new QTimer(this);
    d->updateTimer->setInterval(5000); // 5秒更新一次
    connect(d->updateTimer, &QTimer::timeout, this, &MeetingWidget::updateStatistics);
    
    d->durationTimer = new QTimer(this);
    d->durationTimer->setInterval(1000); // 1秒更新一次
    connect(d->durationTimer, &QTimer::timeout, this, &MeetingWidget::updateMeetingInfo);
    
    // 初始状态
    reset();
}

MeetingWidget::~MeetingWidget() = default;

void MeetingWidget::setMeetingManager(MeetingManager* manager)
{
    if (d->meetingManager != manager) {
        d->meetingManager = manager;
        
        if (d->meetingManager) {
            // 连接会议管理器信号
            connect(d->meetingManager, &MeetingManager::stateChanged,
                    this, &MeetingWidget::handleMeetingStatusChanged);
            connect(d->meetingManager, &MeetingManager::meetingLeft,
                    this, [this]() { handleMeetingStatusChanged(IMeetingManager::Disconnected); });
            // 修复信号槽连接问题，使用旧式连接语法
            connect(d->meetingManager, SIGNAL(participantJoined(const QVariantMap&)),
                    this, SLOT(handleParticipantJoined(const QVariantMap&)));
            connect(d->meetingManager, &MeetingManager::participantLeft,
                    this, &MeetingWidget::handleParticipantLeft);
        }
    }
}

MeetingManager* MeetingWidget::meetingManager() const
{
    return d->meetingManager;
}

void MeetingWidget::setCurrentMeeting(Meeting* meeting)
{
    if (d->currentMeeting != meeting) {
        d->currentMeeting = meeting;
        
        if (d->currentMeeting) {
            // 连接会议信号
            connect(d->currentMeeting, &Meeting::statusChanged,
                    this, &MeetingWidget::handleMeetingStatusChanged);
            connect(d->currentMeeting, &Meeting::participantCountChanged,
                    this, &MeetingWidget::updateParticipantsList);
            
            d->meetingStartTime = QDateTime::currentDateTime();
            d->durationTimer->start();
        } else {
            d->durationTimer->stop();
        }
        
        updateMeetingInfo();
        updateControlsState();
    }
}

Meeting* MeetingWidget::currentMeeting() const
{
    return d->currentMeeting;
}

void MeetingWidget::setDisplayMode(DisplayMode mode)
{
    if (d->displayMode != mode) {
        d->displayMode = mode;
        updateLayout();
        emit displayModeChanged(mode);
    }
}

MeetingWidget::DisplayMode MeetingWidget::displayMode() const
{
    return d->displayMode;
}

void MeetingWidget::setShowControls(bool show)
{
    if (d->showControls != show) {
        d->showControls = show;
        d->controlsGroup->setVisible(show);
    }
}

bool MeetingWidget::showControls() const
{
    return d->showControls;
}

void MeetingWidget::setShowParticipants(bool show)
{
    if (d->showParticipants != show) {
        d->showParticipants = show;
        d->participantsGroup->setVisible(show);
    }
}

bool MeetingWidget::showParticipants() const
{
    return d->showParticipants;
}

void MeetingWidget::setShowStatistics(bool show)
{
    if (d->showStatistics != show) {
        d->showStatistics = show;
        d->statisticsGroup->setVisible(show);
        
        if (show) {
            d->updateTimer->start();
        } else {
            d->updateTimer->stop();
        }
    }
}

bool MeetingWidget::showStatistics() const
{
    return d->showStatistics;
}

void MeetingWidget::updateMeetingInfo()
{
    if (!d->meetingManager || !d->infoLabel) {
        return;
    }
    
    QVariantMap meetingInfo = d->meetingManager->getCurrentMeetingInfo();
    QString name = meetingInfo.value("name").toString();
    QString url = meetingInfo.value("url").toString();
    int participantCount = meetingInfo.value("participantCount", 0).toInt();
    
    QString infoText = tr("Meeting: %1").arg(name);
    infoText += "\n" + tr("URL: %1").arg(url);
    infoText += "\n" + tr("Participants: %1").arg(participantCount);
    
    d->infoLabel->setText(infoText);
}

void MeetingWidget::initializeUI()
{
    // 创建会议信息组
    d->meetingInfoGroup = new QGroupBox(tr("Meeting Information"), this);
    d->meetingTitleLabel = new QLabel(tr("No active meeting"), this);
    d->meetingUrlLabel = new QLabel(tr("URL: -"), this);
    d->meetingStatusLabel = new QLabel(tr("Status: Disconnected"), this);
    d->meetingDurationLabel = new QLabel(tr("Duration: 00:00:00"), this);
    d->participantCountLabel = new QLabel(tr("Participants: 0"), this);
    d->connectionQualityBar = new QProgressBar(this);
    d->connectionQualityBar->setRange(0, 100);
    d->connectionQualityBar->setValue(0);
    d->connectionQualityBar->setFormat(tr("Connection Quality: %p%"));
    d->infoLabel = new QLabel(tr("No meeting information available"), this);
    
    // 创建控制按钮组
    d->controlsGroup = new QGroupBox(tr("Meeting Controls"), this);
    d->joinButton = new QPushButton(tr("Join Meeting"), this);
    d->leaveButton = new QPushButton(tr("Leave Meeting"), this);
    d->createButton = new QPushButton(tr("Create Meeting"), this);
    d->settingsButton = new QPushButton(tr("Settings"), this);
    d->inviteButton = new QPushButton(tr("Invite"), this);
    d->copyLinkButton = new QPushButton(tr("Copy Link"), this);
    d->urlInput = new QLineEdit(this);
    d->urlInput->setPlaceholderText(tr("Enter meeting URL..."));
    
    // 创建参与者列表组
    d->participantsGroup = new QGroupBox(tr("Participants"), this);
    d->participantsList = new QListWidget(this);
    d->participantsCountLabel = new QLabel(tr("Total: 0"), this);
    
    // 创建统计信息组
    d->statisticsGroup = new QGroupBox(tr("Statistics"), this);
    d->audioQualityLabel = new QLabel(tr("Audio Quality: -"), this);
    d->videoQualityLabel = new QLabel(tr("Video Quality: -"), this);
    d->networkLatencyLabel = new QLabel(tr("Network Latency: -"), this);
    d->bandwidthLabel = new QLabel(tr("Bandwidth: -"), this);
    
    // 创建状态栏
    d->statusBar = new QFrame(this);
    d->statusBar->setFrameShape(QFrame::StyledPanel);
    d->statusBar->setFrameShadow(QFrame::Sunken);
    d->statusLabel = new QLabel(tr("Ready"), this);
    d->loadingBar = new QProgressBar(this);
    d->loadingBar->setRange(0, 0); // 无限进度条
    d->loadingBar->setVisible(false);
}

void MeetingWidget::setupLayout()
{
    // 主布局
    d->mainLayout = new QVBoxLayout(this);
    d->mainSplitter = new QSplitter(Qt::Horizontal, this);
    
    // 会议信息布局
    QVBoxLayout* infoLayout = new QVBoxLayout(d->meetingInfoGroup);
    infoLayout->addWidget(d->meetingTitleLabel);
    infoLayout->addWidget(d->meetingUrlLabel);
    infoLayout->addWidget(d->meetingStatusLabel);
    infoLayout->addWidget(d->meetingDurationLabel);
    infoLayout->addWidget(d->participantCountLabel);
    infoLayout->addWidget(d->connectionQualityBar);
    infoLayout->addWidget(d->infoLabel);
    
    // 控制按钮布局
    QGridLayout* controlsLayout = new QGridLayout(d->controlsGroup);
    controlsLayout->addWidget(d->urlInput, 0, 0, 1, 3);
    controlsLayout->addWidget(d->joinButton, 1, 0);
    controlsLayout->addWidget(d->leaveButton, 1, 1);
    controlsLayout->addWidget(d->createButton, 1, 2);
    controlsLayout->addWidget(d->settingsButton, 2, 0);
    controlsLayout->addWidget(d->inviteButton, 2, 1);
    controlsLayout->addWidget(d->copyLinkButton, 2, 2);
    
    // 参与者列表布局
    QVBoxLayout* participantsLayout = new QVBoxLayout(d->participantsGroup);
    participantsLayout->addWidget(d->participantsList);
    participantsLayout->addWidget(d->participantsCountLabel);
    
    // 统计信息布局
    QVBoxLayout* statsLayout = new QVBoxLayout(d->statisticsGroup);
    statsLayout->addWidget(d->audioQualityLabel);
    statsLayout->addWidget(d->videoQualityLabel);
    statsLayout->addWidget(d->networkLatencyLabel);
    statsLayout->addWidget(d->bandwidthLabel);
    
    // 状态栏布局
    QHBoxLayout* statusLayout = new QHBoxLayout(d->statusBar);
    statusLayout->addWidget(d->statusLabel);
    statusLayout->addWidget(d->loadingBar);
    statusLayout->setContentsMargins(5, 2, 5, 2);
    
    // 左侧面板
    QWidget* leftPanel = new QWidget(this);
    QVBoxLayout* leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->addWidget(d->meetingInfoGroup);
    leftLayout->addWidget(d->controlsGroup);
    leftLayout->addWidget(d->statisticsGroup);
    
    // 右侧面板
    QWidget* rightPanel = new QWidget(this);
    QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->addWidget(d->participantsGroup);
    
    // 添加到分割器
    d->mainSplitter->addWidget(leftPanel);
    d->mainSplitter->addWidget(rightPanel);
    d->mainSplitter->setStretchFactor(0, 2);
    d->mainSplitter->setStretchFactor(1, 1);
    
    // 添加到主布局
    d->mainLayout->addWidget(d->mainSplitter);
    d->mainLayout->addWidget(d->statusBar);
}

void MeetingWidget::connectSignals()
{
    // 连接按钮信号
    connect(d->joinButton, &QPushButton::clicked, this, &MeetingWidget::joinMeeting);
    connect(d->leaveButton, &QPushButton::clicked, this, &MeetingWidget::leaveMeeting);
    connect(d->createButton, &QPushButton::clicked, this, &MeetingWidget::createMeeting);
    connect(d->settingsButton, &QPushButton::clicked, this, &MeetingWidget::showSettingsRequested);
    connect(d->inviteButton, &QPushButton::clicked, this, &MeetingWidget::inviteParticipants);
    connect(d->copyLinkButton, &QPushButton::clicked, this, &MeetingWidget::copyMeetingLink);
}

void MeetingWidget::applyStyles()
{
    // 设置样式
    d->meetingTitleLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    d->meetingStatusLabel->setStyleSheet("font-weight: bold;");
    d->connectionQualityBar->setStyleSheet("QProgressBar { text-align: center; }");
    d->statusBar->setStyleSheet("background-color: #f0f0f0;");
}

void MeetingWidget::updateLayout()
{
    // 根据显示模式更新布局
    switch (d->displayMode) {
    case CompactMode:
        d->statisticsGroup->setVisible(false);
        d->participantsGroup->setVisible(false);
        break;
        
    case NormalMode:
        d->statisticsGroup->setVisible(d->showStatistics);
        d->participantsGroup->setVisible(d->showParticipants);
        break;
        
    case DetailedMode:
        d->statisticsGroup->setVisible(true);
        d->participantsGroup->setVisible(true);
        break;
    }
}

void MeetingWidget::updateControlsState()
{
    bool hasManager = d->meetingManager != nullptr;
    bool hasMeeting = d->currentMeeting != nullptr;
    bool isConnected = hasMeeting && (d->meetingManager->currentState() == IMeetingManager::Connected || 
                                     d->meetingManager->currentState() == IMeetingManager::InMeeting);
    
    d->joinButton->setEnabled(hasManager && !isConnected);
    d->leaveButton->setEnabled(hasManager && isConnected);
    d->createButton->setEnabled(hasManager && !isConnected);
    d->inviteButton->setEnabled(hasManager && isConnected);
    d->copyLinkButton->setEnabled(hasManager && isConnected);
}

void MeetingWidget::reset()
{
    // 重置状态
    d->meetingTitleLabel->setText(tr("No active meeting"));
    d->meetingUrlLabel->setText(tr("URL: -"));
    d->meetingStatusLabel->setText(tr("Status: Disconnected"));
    d->meetingDurationLabel->setText(tr("Duration: 00:00:00"));
    d->participantCountLabel->setText(tr("Participants: 0"));
    d->connectionQualityBar->setValue(0);
    d->participantsList->clear();
    d->participantsCountLabel->setText(tr("Total: 0"));
    d->audioQualityLabel->setText(tr("Audio Quality: -"));
    d->videoQualityLabel->setText(tr("Video Quality: -"));
    d->networkLatencyLabel->setText(tr("Network Latency: -"));
    d->bandwidthLabel->setText(tr("Bandwidth: -"));
    d->statusLabel->setText(tr("Ready"));
    d->loadingBar->setVisible(false);
    d->infoLabel->setText(tr("No meeting information available"));
    
    updateControlsState();
}

void MeetingWidget::joinMeeting()
{
    if (!d->meetingManager) {
        return;
    }
    
    QString url = d->urlInput->text().trimmed();
    if (url.isEmpty()) {
        QMessageBox::warning(this, tr("Join Meeting"), tr("Please enter a meeting URL"));
        return;
    }
    
    setLoading(true, tr("Joining meeting..."));
    
    // 异步加入会议
    QTimer::singleShot(100, this, [this, url]() {
        bool success = d->meetingManager->joinMeeting(url);
        setLoading(false);
        
        if (!success) {
            QMessageBox::critical(this, tr("Join Meeting"), tr("Failed to join meeting"));
        }
    });
}

void MeetingWidget::leaveMeeting()
{
    if (!d->meetingManager) {
        return;
    }
    
    setLoading(true, tr("Leaving meeting..."));
    
    // 异步离开会议
    QTimer::singleShot(100, this, [this]() {
        bool success = d->meetingManager->leaveMeeting();
        setLoading(false);
        
        if (!success) {
            QMessageBox::critical(this, tr("Leave Meeting"), tr("Failed to leave meeting"));
        }
    });
}

void MeetingWidget::createMeeting()
{
    emit createMeetingRequested("", QVariantMap());
}

void MeetingWidget::inviteParticipants()
{
    emit inviteParticipantRequested("", "");
}

void MeetingWidget::copyMeetingLink()
{
    if (!d->meetingManager) {
        return;
    }
    
    QString url = d->meetingManager->getCurrentMeetingUrl();
    if (!url.isEmpty()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(url);
        
        d->statusLabel->setText(tr("Meeting link copied to clipboard"));
        QTimer::singleShot(3000, this, [this]() {
            d->statusLabel->setText(tr("Ready"));
        });
    }
}

void MeetingWidget::setLoading(bool loading, const QString& message)
{
    d->isLoading = loading;
    d->loadingBar->setVisible(loading);
    
    if (!message.isEmpty()) {
        d->statusLabel->setText(message);
    } else if (!loading) {
        d->statusLabel->setText(tr("Ready"));
    }
    
    // 禁用控件
    d->joinButton->setEnabled(!loading);
    d->leaveButton->setEnabled(!loading);
    d->createButton->setEnabled(!loading);
    d->settingsButton->setEnabled(!loading);
    d->inviteButton->setEnabled(!loading);
    d->copyLinkButton->setEnabled(!loading);
    d->urlInput->setEnabled(!loading);
}

void MeetingWidget::handleMeetingStatusChanged(int state)
{
    QString statusText;
    
    switch (state) {
    case IMeetingManager::Connecting:
        statusText = tr("Connecting...");
        break;
    case IMeetingManager::Connected:
        statusText = tr("Connected");
        break;
    case IMeetingManager::Leaving:
        statusText = tr("Disconnecting...");
        break;
    case IMeetingManager::Disconnected:
        statusText = tr("Disconnected");
        break;
    case IMeetingManager::Error:
        statusText = tr("Error");
        break;
    }
    
    d->meetingStatusLabel->setText(tr("Status: %1").arg(statusText));
    updateControlsState();
    updateMeetingInfo();
}

void MeetingWidget::handleParticipantJoined(const QString& participantId, const QVariantMap& info)
{
    updateParticipantsList();
}

void MeetingWidget::handleParticipantLeft(const QString& participantId)
{
    updateParticipantsList();
}

void MeetingWidget::updateParticipantsList()
{
    if (!d->meetingManager) {
        return;
    }
    
    QVariantList participants = d->meetingManager->getParticipants();
    d->participantsList->clear();
    
    for (const QVariant& participant : participants) {
        QVariantMap info = participant.toMap();
        QString name = info.value("name").toString();
        QString id = info.value("id").toString();
        
        QListWidgetItem* item = new QListWidgetItem(name);
        item->setData(Qt::UserRole, id);
        d->participantsList->addItem(item);
    }
    
    d->participantsCountLabel->setText(tr("Total: %1").arg(participants.size()));
    d->participantCountLabel->setText(tr("Participants: %1").arg(participants.size()));
}

void MeetingWidget::updateStatistics()
{
    if (!d->meetingManager || !d->showStatistics) {
        return;
    }
    
    int quality = d->meetingManager->getConnectionQuality();
    d->connectionQualityBar->setValue(quality);
    
    // 更新其他统计信息
    // ...
}

void MeetingWidget::paintEvent(QPaintEvent* event)
{
    QStyleOption opt;
    opt.initFrom(this);
    QPainter p(this);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
    
    QWidget::paintEvent(event);
}
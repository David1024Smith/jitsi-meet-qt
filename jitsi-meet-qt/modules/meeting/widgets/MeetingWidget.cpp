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
            connect(d->meetingManager, &MeetingManager::meetingJoined,
                    this, &MeetingWidget::handleMeetingStatusChanged);
            connect(d->meetingManager, &MeetingManager::meetingLeft,
                    this, &MeetingWidget::handleMeetingStatusChanged);
            connect(d->meetingManager, &MeetingManager::participantJoined,
                    this, &MeetingWidget::handleParticipantJoined);
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

void
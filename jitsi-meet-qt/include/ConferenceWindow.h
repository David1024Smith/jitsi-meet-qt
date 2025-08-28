#ifndef CONFERENCEWINDOW_H
#define CONFERENCEWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QVideoWidget>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QScrollArea>
#include <QFrame>
#include <QProgressBar>
#include <QTimer>
#include <QMap>
#include <QList>
#include "NavigationBar.h"
#include "ConferenceManager.h"
#include "MediaManager.h"
#include "ChatManager.h"
#include "ScreenShareManager.h"
#include "JitsiError.h"

class ConferenceWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConferenceWindow(QWidget *parent = nullptr);
    ~ConferenceWindow();

    // 会议控制
    void joinConference(const QString& url);
    void leaveConference();
    
    // 获取当前会议URL
    QString currentUrl() const;

public slots:
    // 更新翻译文本
    void retranslateUi();

protected:
    // 处理事件变化（包括语言变化）
    void changeEvent(QEvent* event) override;

signals:
    void backToWelcome();
    void conferenceJoined(const QString& url);
    void conferenceLeft();
    void settingsRequested();

private slots:
    // 会议管理器事件
    void onConnectionStateChanged(ConferenceManager::ConnectionState state);
    void onConferenceStateChanged(ConferenceManager::ConferenceState state);
    void onConferenceJoined(const ConferenceManager::ConferenceInfo& info);
    void onConferenceLeft();
    void onParticipantJoined(const ConferenceManager::ParticipantInfo& participant);
    void onParticipantLeft(const QString& jid);
    void onParticipantUpdated(const ConferenceManager::ParticipantInfo& participant);
    void onLocalMediaStateChanged(bool audioMuted, bool videoMuted);
    void onScreenShareStateChanged(bool isSharing, const QString& participantJid);
    void onErrorOccurred(const JitsiError& error);
    
    // 媒体管理器事件
    void onLocalVideoStarted();
    void onLocalVideoStopped();
    void onRemoteVideoReceived(const QString& participantId, QVideoWidget* widget);
    void onRemoteVideoRemoved(const QString& participantId);
    
    // 聊天管理器事件
    void onChatMessageReceived(const ChatManager::ChatMessage& message);
    void onChatMessageSent(const ChatManager::ChatMessage& message);
    void onUnreadCountChanged(int count);
    
    // 屏幕共享管理器事件
    void onScreenShareStarted();
    void onScreenShareStopped();
    void onRemoteScreenShareReceived(const QString& participantId, QVideoWidget* widget);
    void onRemoteScreenShareRemoved(const QString& participantId);
    
    // UI事件处理
    void onBackButtonClicked();
    void onMuteAudioClicked();
    void onMuteVideoClicked();
    void onScreenShareClicked();
    void onChatToggleClicked();
    void onParticipantsToggleClicked();
    void onSendChatMessage();
    void onChatInputReturnPressed();
    void onParticipantItemClicked();
    
    // 连接状态更新
    void updateConnectionStatus();

protected:
    void closeEvent(QCloseEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupUI();
    void setupManagers();
    void setupConnections();
    void applyStyles();
    
    // UI布局方法
    void createMainLayout();
    void createVideoArea();
    void createControlPanel();
    void createChatPanel();
    void createParticipantsPanel();
    void createStatusBar();
    
    // 视频布局管理
    void updateVideoLayout();
    void arrangeVideoWidgets();
    void addVideoWidget(const QString& participantId, QVideoWidget* widget);
    void removeVideoWidget(const QString& participantId);
    void setMainVideoWidget(QVideoWidget* widget);
    
    // 聊天功能
    void addChatMessage(const ChatManager::ChatMessage& message);
    void updateChatUnreadIndicator();
    void scrollChatToBottom();
    
    // 参与者列表管理
    void updateParticipantsList();
    void addParticipantToList(const ConferenceManager::ParticipantInfo& participant);
    void removeParticipantFromList(const QString& jid);
    void updateParticipantInList(const ConferenceManager::ParticipantInfo& participant);
    
    // 状态显示
    void showConnectionStatus(const QString& status);
    void showError(const QString& message);
    void showLoading(const QString& message);
    void hideLoading();
    
    // 控制面板更新
    void updateControlButtons();
    void updateMediaButtons();
    void updateScreenShareButton();
    
    // 面板显示控制
    void showChatPanel(bool show);
    void showParticipantsPanel(bool show);
    void toggleChatPanel();
    void toggleParticipantsPanel();

    // 管理器组件
    ConferenceManager* m_conferenceManager;
    MediaManager* m_mediaManager;
    ChatManager* m_chatManager;
    ScreenShareManager* m_screenShareManager;

    // 主要UI组件
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    NavigationBar* m_navigationBar;
    
    // 视频区域
    QWidget* m_videoArea;
    QGridLayout* m_videoLayout;
    QScrollArea* m_videoScrollArea;
    QVideoWidget* m_mainVideoWidget;
    QMap<QString, QVideoWidget*> m_videoWidgets;
    QLabel* m_noVideoLabel;
    
    // 侧边面板
    QSplitter* m_mainSplitter;
    QWidget* m_sidePanel;
    QVBoxLayout* m_sidePanelLayout;
    
    // 聊天面板
    QWidget* m_chatPanel;
    QVBoxLayout* m_chatLayout;
    QTextEdit* m_chatDisplay;
    QLineEdit* m_chatInput;
    QPushButton* m_sendButton;
    QLabel* m_chatUnreadLabel;
    bool m_chatPanelVisible;
    
    // 参与者面板
    QWidget* m_participantsPanel;
    QVBoxLayout* m_participantsLayout;
    QListWidget* m_participantsList;
    QLabel* m_participantsCountLabel;
    bool m_participantsPanelVisible;
    
    // 控制面板
    QWidget* m_controlPanel;
    QHBoxLayout* m_controlLayout;
    QPushButton* m_muteAudioButton;
    QPushButton* m_muteVideoButton;
    QPushButton* m_screenShareButton;
    QPushButton* m_chatToggleButton;
    QPushButton* m_participantsToggleButton;
    
    // 状态栏
    QWidget* m_statusBar;
    QHBoxLayout* m_statusLayout;
    QLabel* m_connectionStatusLabel;
    QLabel* m_participantCountLabel;
    QProgressBar* m_connectionProgress;
    
    // 状态变量
    QString m_currentUrl;
    bool m_isInConference;
    bool m_isAudioMuted;
    bool m_isVideoMuted;
    bool m_isScreenSharing;
    int m_participantCount;
    int m_unreadChatCount;
    
    // 视频布局
    int m_videoGridColumns;
    int m_videoGridRows;
    QSize m_videoWidgetSize;
};

#endif // CONFERENCEWINDOW_H
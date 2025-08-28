#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QListWidget>
#include <QSplitter>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QTimer>

#include "ChatManager.h"
#include "XMPPClient.h"

/**
 * @brief ChatManager功能演示程序
 * 
 * 该程序演示了ChatManager的核心功能：
 * - 消息发送和接收
 * - 消息历史记录管理
 * - 未读消息计数
 * - 消息持久化
 * - 消息搜索和导出
 */
class ChatManagerDemo : public QMainWindow
{
    Q_OBJECT

public:
    ChatManagerDemo(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_chatManager(new ChatManager(this))
        , m_xmppClient(new XMPPClient(this))
        , m_messageHistory(new QTextEdit(this))
        , m_messageInput(new QLineEdit(this))
        , m_sendButton(new QPushButton("发送", this))
        , m_connectButton(new QPushButton("连接", this))
        , m_disconnectButton(new QPushButton("断开", this))
        , m_unreadLabel(new QLabel("未读: 0", this))
        , m_roomInput(new QLineEdit(this))
        , m_searchInput(new QLineEdit(this))
        , m_searchButton(new QPushButton("搜索", this))
        , m_clearButton(new QPushButton("清空历史", this))
        , m_exportButton(new QPushButton("导出", this))
        , m_importButton(new QPushButton("导入", this))
        , m_maxHistorySpinBox(new QSpinBox(this))
        , m_persistenceCheckBox(new QCheckBox("启用持久化", this))
    {
        setupUI();
        setupConnections();
        setupChatManager();
        
        // 模拟连接状态
        updateConnectionState(false);
        
        setWindowTitle("ChatManager 功能演示");
        resize(800, 600);
    }

private slots:
    void onSendMessage()
    {
        QString message = m_messageInput->text().trimmed();
        if (message.isEmpty()) {
            return;
        }

        if (m_chatManager->sendMessage(message)) {
            m_messageInput->clear();
        } else {
            QMessageBox::warning(this, "发送失败", "无法发送消息，请检查连接状态");
        }
    }

    void onConnectClicked()
    {
        QString room = m_roomInput->text().trimmed();
        if (room.isEmpty()) {
            room = "test-room";
            m_roomInput->setText(room);
        }

        // 模拟连接到服务器
        m_xmppClient->connectToServer("https://meet.jit.si", room, "TestUser");
        updateConnectionState(true);
        
        // 模拟加入房间成功
        QTimer::singleShot(1000, [this]() {
            m_chatManager->setCurrentRoom(m_roomInput->text());
            addSystemMessage("已连接到房间: " + m_roomInput->text());
        });
    }

    void onDisconnectClicked()
    {
        m_xmppClient->disconnect();
        updateConnectionState(false);
        addSystemMessage("已断开连接");
    }

    void onMessageReceived(const ChatManager::ChatMessage& message)
    {
        QString displayText = QString("[%1] %2: %3")
                             .arg(message.timestamp.toString("hh:mm:ss"))
                             .arg(message.senderName)
                             .arg(message.content);
        
        if (!message.isLocal) {
            displayText = "<font color='blue'>" + displayText + "</font>";
        }
        
        m_messageHistory->append(displayText);
        updateUnreadCount();
    }

    void onMessageSent(const ChatManager::ChatMessage& message)
    {
        QString displayText = QString("[%1] %2: %3")
                             .arg(message.timestamp.toString("hh:mm:ss"))
                             .arg(message.senderName)
                             .arg(message.content);
        
        displayText = "<font color='green'>" + displayText + "</font>";
        m_messageHistory->append(displayText);
    }

    void onMessageSendFailed(const QString& content, const QString& error)
    {
        QString errorText = QString("<font color='red'>发送失败: %1 - %2</font>")
                           .arg(content).arg(error);
        m_messageHistory->append(errorText);
    }

    void onUnreadCountChanged(int count)
    {
        m_unreadLabel->setText(QString("未读: %1").arg(count));
        
        if (count > 0) {
            m_unreadLabel->setStyleSheet("color: red; font-weight: bold;");
        } else {
            m_unreadLabel->setStyleSheet("");
        }
    }

    void onSearchMessages()
    {
        QString query = m_searchInput->text().trimmed();
        if (query.isEmpty()) {
            return;
        }

        QList<ChatManager::ChatMessage> results = m_chatManager->searchMessages(query);
        
        QString searchResults = QString("\n=== 搜索结果 (%1 条) ===\n").arg(results.size());
        for (const auto& message : results) {
            searchResults += QString("[%1] %2: %3\n")
                            .arg(message.timestamp.toString("yyyy-MM-dd hh:mm:ss"))
                            .arg(message.senderName)
                            .arg(message.content);
        }
        
        m_messageHistory->append(searchResults);
    }

    void onClearHistory()
    {
        int ret = QMessageBox::question(this, "确认", "确定要清空消息历史吗？",
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            m_chatManager->clearHistory();
            m_messageHistory->clear();
            addSystemMessage("消息历史已清空");
        }
    }

    void onExportHistory()
    {
        QString fileName = QFileDialog::getSaveFileName(this, "导出消息历史", 
                                                       "chat_history.json", 
                                                       "JSON Files (*.json)");
        if (!fileName.isEmpty()) {
            if (m_chatManager->exportHistory(fileName)) {
                QMessageBox::information(this, "成功", "消息历史导出成功");
            } else {
                QMessageBox::warning(this, "失败", "消息历史导出失败");
            }
        }
    }

    void onImportHistory()
    {
        QString fileName = QFileDialog::getOpenFileName(this, "导入消息历史", 
                                                       "", 
                                                       "JSON Files (*.json)");
        if (!fileName.isEmpty()) {
            if (m_chatManager->importHistory(fileName)) {
                QMessageBox::information(this, "成功", "消息历史导入成功");
                loadHistoryToDisplay();
            } else {
                QMessageBox::warning(this, "失败", "消息历史导入失败");
            }
        }
    }

    void onMaxHistorySizeChanged(int value)
    {
        m_chatManager->setMaxHistorySize(value);
        addSystemMessage(QString("最大历史记录数量设置为: %1").arg(value));
    }

    void onPersistenceToggled(bool enabled)
    {
        m_chatManager->setPersistenceEnabled(enabled);
        addSystemMessage(QString("消息持久化: %1").arg(enabled ? "启用" : "禁用"));
    }

    void simulateIncomingMessage()
    {
        static int messageCount = 1;
        static QStringList senders = {"Alice", "Bob", "Charlie", "Diana"};
        static QStringList messages = {
            "大家好！",
            "会议什么时候开始？",
            "我需要共享一下屏幕",
            "音频有问题吗？",
            "谢谢大家的参与",
            "下次会议见！"
        };

        QString sender = senders[messageCount % senders.size()];
        QString message = messages[messageCount % messages.size()];
        
        // 模拟接收到XMPP消息
        QString fromJid = QString("testroom@conference.meet.jit.si/%1").arg(sender);
        emit m_xmppClient->chatMessageReceived(fromJid, message, QDateTime::currentDateTime());
        
        messageCount++;
    }

private:
    void setupUI()
    {
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        QHBoxLayout* mainLayout = new QHBoxLayout(centralWidget);

        // 左侧：聊天区域
        QSplitter* leftSplitter = new QSplitter(Qt::Vertical, this);
        
        // 消息历史显示
        QGroupBox* historyGroup = new QGroupBox("消息历史", this);
        QVBoxLayout* historyLayout = new QVBoxLayout(historyGroup);
        
        m_messageHistory->setReadOnly(true);
        historyLayout->addWidget(m_messageHistory);
        
        // 未读消息计数
        historyLayout->addWidget(m_unreadLabel);
        
        leftSplitter->addWidget(historyGroup);

        // 消息输入区域
        QGroupBox* inputGroup = new QGroupBox("发送消息", this);
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        
        QHBoxLayout* messageLayout = new QHBoxLayout();
        messageLayout->addWidget(m_messageInput);
        messageLayout->addWidget(m_sendButton);
        inputLayout->addLayout(messageLayout);
        
        leftSplitter->addWidget(inputGroup);
        leftSplitter->setSizes({400, 100});

        mainLayout->addWidget(leftSplitter, 2);

        // 右侧：控制面板
        QWidget* rightPanel = new QWidget(this);
        QVBoxLayout* rightLayout = new QVBoxLayout(rightPanel);

        // 连接控制
        QGroupBox* connectionGroup = new QGroupBox("连接控制", this);
        QVBoxLayout* connectionLayout = new QVBoxLayout(connectionGroup);
        
        m_roomInput->setPlaceholderText("房间名称");
        m_roomInput->setText("test-room");
        connectionLayout->addWidget(new QLabel("房间:"));
        connectionLayout->addWidget(m_roomInput);
        
        QHBoxLayout* connectionButtonsLayout = new QHBoxLayout();
        connectionButtonsLayout->addWidget(m_connectButton);
        connectionButtonsLayout->addWidget(m_disconnectButton);
        connectionLayout->addLayout(connectionButtonsLayout);
        
        rightLayout->addWidget(connectionGroup);

        // 搜索功能
        QGroupBox* searchGroup = new QGroupBox("消息搜索", this);
        QVBoxLayout* searchLayout = new QVBoxLayout(searchGroup);
        
        m_searchInput->setPlaceholderText("搜索关键词");
        searchLayout->addWidget(m_searchInput);
        searchLayout->addWidget(m_searchButton);
        
        rightLayout->addWidget(searchGroup);

        // 历史管理
        QGroupBox* historyManagementGroup = new QGroupBox("历史管理", this);
        QVBoxLayout* historyManagementLayout = new QVBoxLayout(historyManagementGroup);
        
        historyManagementLayout->addWidget(m_clearButton);
        
        QHBoxLayout* importExportLayout = new QHBoxLayout();
        importExportLayout->addWidget(m_exportButton);
        importExportLayout->addWidget(m_importButton);
        historyManagementLayout->addLayout(importExportLayout);
        
        rightLayout->addWidget(historyManagementGroup);

        // 设置选项
        QGroupBox* settingsGroup = new QGroupBox("设置", this);
        QVBoxLayout* settingsLayout = new QVBoxLayout(settingsGroup);
        
        settingsLayout->addWidget(new QLabel("最大历史记录:"));
        m_maxHistorySpinBox->setRange(10, 10000);
        m_maxHistorySpinBox->setValue(1000);
        settingsLayout->addWidget(m_maxHistorySpinBox);
        
        m_persistenceCheckBox->setChecked(true);
        settingsLayout->addWidget(m_persistenceCheckBox);
        
        rightLayout->addWidget(settingsGroup);

        // 测试功能
        QGroupBox* testGroup = new QGroupBox("测试功能", this);
        QVBoxLayout* testLayout = new QVBoxLayout(testGroup);
        
        QPushButton* simulateButton = new QPushButton("模拟接收消息", this);
        connect(simulateButton, &QPushButton::clicked, this, &ChatManagerDemo::simulateIncomingMessage);
        testLayout->addWidget(simulateButton);
        
        rightLayout->addWidget(testGroup);

        rightLayout->addStretch();
        mainLayout->addWidget(rightPanel, 1);
    }

    void setupConnections()
    {
        // 消息输入
        connect(m_messageInput, &QLineEdit::returnPressed, this, &ChatManagerDemo::onSendMessage);
        connect(m_sendButton, &QPushButton::clicked, this, &ChatManagerDemo::onSendMessage);

        // 连接控制
        connect(m_connectButton, &QPushButton::clicked, this, &ChatManagerDemo::onConnectClicked);
        connect(m_disconnectButton, &QPushButton::clicked, this, &ChatManagerDemo::onDisconnectClicked);

        // 搜索和管理
        connect(m_searchButton, &QPushButton::clicked, this, &ChatManagerDemo::onSearchMessages);
        connect(m_searchInput, &QLineEdit::returnPressed, this, &ChatManagerDemo::onSearchMessages);
        connect(m_clearButton, &QPushButton::clicked, this, &ChatManagerDemo::onClearHistory);
        connect(m_exportButton, &QPushButton::clicked, this, &ChatManagerDemo::onExportHistory);
        connect(m_importButton, &QPushButton::clicked, this, &ChatManagerDemo::onImportHistory);

        // 设置
        connect(m_maxHistorySpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
                this, &ChatManagerDemo::onMaxHistorySizeChanged);
        connect(m_persistenceCheckBox, &QCheckBox::toggled, this, &ChatManagerDemo::onPersistenceToggled);
    }

    void setupChatManager()
    {
        // 设置XMPP客户端
        m_chatManager->setXMPPClient(m_xmppClient);

        // 连接ChatManager信号
        connect(m_chatManager, &ChatManager::messageReceived, this, &ChatManagerDemo::onMessageReceived);
        connect(m_chatManager, &ChatManager::messageSent, this, &ChatManagerDemo::onMessageSent);
        connect(m_chatManager, &ChatManager::messageSendFailed, this, &ChatManagerDemo::onMessageSendFailed);
        connect(m_chatManager, &ChatManager::unreadCountChanged, this, &ChatManagerDemo::onUnreadCountChanged);

        // 加载现有历史记录
        loadHistoryToDisplay();
    }

    void updateConnectionState(bool connected)
    {
        m_connectButton->setEnabled(!connected);
        m_disconnectButton->setEnabled(connected);
        m_sendButton->setEnabled(connected);
        m_messageInput->setEnabled(connected);
        m_roomInput->setEnabled(!connected);
    }

    void addSystemMessage(const QString& message)
    {
        QString systemMessage = QString("<font color='gray'>[系统] %1</font>")
                               .arg(message);
        m_messageHistory->append(systemMessage);
    }

    void loadHistoryToDisplay()
    {
        QList<ChatManager::ChatMessage> history = m_chatManager->messageHistory();
        
        m_messageHistory->clear();
        addSystemMessage("加载消息历史...");
        
        for (const auto& message : history) {
            QString displayText = QString("[%1] %2: %3")
                                 .arg(message.timestamp.toString("hh:mm:ss"))
                                 .arg(message.senderName)
                                 .arg(message.content);
            
            if (message.isLocal) {
                displayText = "<font color='green'>" + displayText + "</font>";
            } else {
                displayText = "<font color='blue'>" + displayText + "</font>";
            }
            
            m_messageHistory->append(displayText);
        }
        
        updateUnreadCount();
    }

    void updateUnreadCount()
    {
        int count = m_chatManager->unreadCount();
        onUnreadCountChanged(count);
    }

private:
    ChatManager* m_chatManager;
    XMPPClient* m_xmppClient;
    
    // UI组件
    QTextEdit* m_messageHistory;
    QLineEdit* m_messageInput;
    QPushButton* m_sendButton;
    QPushButton* m_connectButton;
    QPushButton* m_disconnectButton;
    QLabel* m_unreadLabel;
    QLineEdit* m_roomInput;
    QLineEdit* m_searchInput;
    QPushButton* m_searchButton;
    QPushButton* m_clearButton;
    QPushButton* m_exportButton;
    QPushButton* m_importButton;
    QSpinBox* m_maxHistorySpinBox;
    QCheckBox* m_persistenceCheckBox;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("ChatManager Demo");
    app.setApplicationVersion("1.0");
    app.setOrganizationName("Jitsi Meet Qt");

    ChatManagerDemo demo;
    demo.show();

    return app.exec();
}

#include "chat_manager_demo.moc"
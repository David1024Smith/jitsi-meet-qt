#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QDebug>
#include <QTimer>
#include <QMessageBox>

#include "include/ScreenShareManager.h"
#include "include/WebRTCEngine.h"

class ScreenShareTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    ScreenShareTestWindow(QWidget* parent = nullptr)
        : QMainWindow(parent)
        , m_screenShareManager(new ScreenShareManager(this))
        , m_webrtcEngine(new WebRTCEngine(this))
    {
        setupUI();
        setupConnections();
        
        // 集成WebRTC引擎
        m_screenShareManager->setWebRTCEngine(m_webrtcEngine);
        
        // 刷新屏幕和窗口列表
        refreshLists();
    }

private slots:
    void onStartScreenShareClicked()
    {
        int screenId = m_screenComboBox->currentData().toInt();
        if (screenId >= 0) {
            if (m_screenShareManager->startScreenShare(screenId)) {
                m_statusLabel->setText("屏幕共享已开始");
                m_startScreenButton->setEnabled(false);
                m_startWindowButton->setEnabled(false);
                m_stopButton->setEnabled(true);
            } else {
                m_statusLabel->setText("启动屏幕共享失败");
            }
        }
    }
    
    void onStartWindowShareClicked()
    {
        qint64 windowId = m_windowComboBox->currentData().toLongLong();
        if (windowId > 0) {
            if (m_screenShareManager->startWindowShare(windowId)) {
                m_statusLabel->setText("窗口共享已开始");
                m_startScreenButton->setEnabled(false);
                m_startWindowButton->setEnabled(false);
                m_stopButton->setEnabled(true);
            } else {
                m_statusLabel->setText("启动窗口共享失败");
            }
        }
    }
    
    void onStopShareClicked()
    {
        m_screenShareManager->stopScreenShare();
        m_statusLabel->setText("共享已停止");
        m_startScreenButton->setEnabled(true);
        m_startWindowButton->setEnabled(true);
        m_stopButton->setEnabled(false);
    }
    
    void onShowSelectionDialogClicked()
    {
        if (m_screenShareManager->showScreenSelectionDialog()) {
            m_statusLabel->setText("通过对话框启动共享");
            m_startScreenButton->setEnabled(false);
            m_startWindowButton->setEnabled(false);
            m_stopButton->setEnabled(true);
        }
    }
    
    void onRefreshClicked()
    {
        refreshLists();
        m_statusLabel->setText("列表已刷新");
    }
    
    void onScreenShareStarted()
    {
        qDebug() << "Screen share started signal received";
        auto currentScreen = m_screenShareManager->currentScreen();
        m_statusLabel->setText(QString("屏幕共享: %1 (%2x%3)")
                              .arg(currentScreen.name)
                              .arg(currentScreen.size.width())
                              .arg(currentScreen.size.height()));
    }
    
    void onWindowShareStarted()
    {
        qDebug() << "Window share started signal received";
        auto currentWindow = m_screenShareManager->currentWindow();
        m_statusLabel->setText(QString("窗口共享: %1")
                              .arg(currentWindow.title));
    }
    
    void onShareStopped()
    {
        qDebug() << "Share stopped signal received";
        m_statusLabel->setText("共享已停止");
        m_startScreenButton->setEnabled(true);
        m_startWindowButton->setEnabled(true);
        m_stopButton->setEnabled(false);
    }
    
    void onScreenCaptureError(const QString& error)
    {
        QMessageBox::warning(this, "屏幕捕获错误", error);
        m_statusLabel->setText("错误: " + error);
    }
    
    void onWindowCaptureError(const QString& error)
    {
        QMessageBox::warning(this, "窗口捕获错误", error);
        m_statusLabel->setText("错误: " + error);
    }

private:
    void setupUI()
    {
        setWindowTitle("屏幕共享测试");
        setMinimumSize(600, 400);
        
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // 屏幕选择组
        QGroupBox* screenGroup = new QGroupBox("屏幕共享");
        QVBoxLayout* screenLayout = new QVBoxLayout(screenGroup);
        
        m_screenComboBox = new QComboBox();
        screenLayout->addWidget(new QLabel("选择屏幕:"));
        screenLayout->addWidget(m_screenComboBox);
        
        m_startScreenButton = new QPushButton("开始屏幕共享");
        screenLayout->addWidget(m_startScreenButton);
        
        mainLayout->addWidget(screenGroup);
        
        // 窗口选择组
        QGroupBox* windowGroup = new QGroupBox("窗口共享");
        QVBoxLayout* windowLayout = new QVBoxLayout(windowGroup);
        
        m_windowComboBox = new QComboBox();
        windowLayout->addWidget(new QLabel("选择窗口:"));
        windowLayout->addWidget(m_windowComboBox);
        
        m_startWindowButton = new QPushButton("开始窗口共享");
        windowLayout->addWidget(m_startWindowButton);
        
        mainLayout->addWidget(windowGroup);
        
        // 控制按钮
        QHBoxLayout* controlLayout = new QHBoxLayout();
        
        m_stopButton = new QPushButton("停止共享");
        m_stopButton->setEnabled(false);
        
        QPushButton* dialogButton = new QPushButton("显示选择对话框");
        QPushButton* refreshButton = new QPushButton("刷新列表");
        
        controlLayout->addWidget(m_stopButton);
        controlLayout->addWidget(dialogButton);
        controlLayout->addWidget(refreshButton);
        controlLayout->addStretch();
        
        mainLayout->addLayout(controlLayout);
        
        // 状态标签
        m_statusLabel = new QLabel("就绪");
        m_statusLabel->setStyleSheet("QLabel { padding: 10px; background-color: #f0f0f0; border: 1px solid #ccc; }");
        mainLayout->addWidget(m_statusLabel);
        
        // 连接按钮信号
        connect(m_startScreenButton, &QPushButton::clicked, this, &ScreenShareTestWindow::onStartScreenShareClicked);
        connect(m_startWindowButton, &QPushButton::clicked, this, &ScreenShareTestWindow::onStartWindowShareClicked);
        connect(m_stopButton, &QPushButton::clicked, this, &ScreenShareTestWindow::onStopShareClicked);
        connect(dialogButton, &QPushButton::clicked, this, &ScreenShareTestWindow::onShowSelectionDialogClicked);
        connect(refreshButton, &QPushButton::clicked, this, &ScreenShareTestWindow::onRefreshClicked);
    }
    
    void setupConnections()
    {
        // 连接ScreenShareManager信号
        connect(m_screenShareManager, &ScreenShareManager::screenShareStarted,
                this, &ScreenShareTestWindow::onScreenShareStarted);
        connect(m_screenShareManager, &ScreenShareManager::windowShareStarted,
                this, &ScreenShareTestWindow::onWindowShareStarted);
        connect(m_screenShareManager, &ScreenShareManager::screenShareStopped,
                this, &ScreenShareTestWindow::onShareStopped);
        connect(m_screenShareManager, &ScreenShareManager::windowShareStopped,
                this, &ScreenShareTestWindow::onShareStopped);
        connect(m_screenShareManager, &ScreenShareManager::screenCaptureError,
                this, &ScreenShareTestWindow::onScreenCaptureError);
        connect(m_screenShareManager, &ScreenShareManager::windowCaptureError,
                this, &ScreenShareTestWindow::onWindowCaptureError);
    }
    
    void refreshLists()
    {
        // 刷新屏幕列表
        m_screenComboBox->clear();
        auto screens = m_screenShareManager->availableScreens();
        for (const auto& screen : screens) {
            QString text = QString("屏幕 %1: %2 (%3x%4)")
                          .arg(screen.screenId)
                          .arg(screen.name)
                          .arg(screen.size.width())
                          .arg(screen.size.height());
            if (screen.isPrimary) {
                text += " [主屏幕]";
            }
            m_screenComboBox->addItem(text, screen.screenId);
        }
        
        // 刷新窗口列表
        m_windowComboBox->clear();
        auto windows = m_screenShareManager->availableWindows();
        for (const auto& window : windows) {
            if (!window.isVisible) continue;
            
            QString text = QString("%1 - %2 (%3x%4)")
                          .arg(window.title)
                          .arg(window.processName)
                          .arg(window.geometry.width())
                          .arg(window.geometry.height());
            m_windowComboBox->addItem(text, static_cast<qulonglong>(window.windowId));
        }
        
        qDebug() << "Refreshed lists - Screens:" << screens.size() << "Windows:" << windows.size();
    }

private:
    ScreenShareManager* m_screenShareManager;
    WebRTCEngine* m_webrtcEngine;
    
    QComboBox* m_screenComboBox;
    QComboBox* m_windowComboBox;
    QPushButton* m_startScreenButton;
    QPushButton* m_startWindowButton;
    QPushButton* m_stopButton;
    QLabel* m_statusLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    ScreenShareTestWindow window;
    window.show();
    
    return app.exec();
}

#include "test_screen_sharing_simple.moc"
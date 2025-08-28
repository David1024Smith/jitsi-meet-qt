#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QListWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QSlider>
#include <QSpinBox>
#include <QTextEdit>
#include <QMenuBar>
#include <QMenu>
#include <QStatusBar>
#include <QSplitter>
#include <QTabWidget>
#include <QWidget>
#include <QIcon>
#include <QDebug>

#include "ThemeManager.h"
#include "StyleHelper.h"
#include "StyleUtils.h"

class ThemeTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    ThemeTestWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_themeManager(new ThemeManager(this))
    {
        setupUI();
        setupConnections();
        
        // Apply initial theme
        m_themeManager->setTheme(ThemeManager::Theme::Light);
    }

private slots:
    void onThemeChanged()
    {
        QString themeName = m_themeCombo->currentText();
        ThemeManager::Theme theme = ThemeManager::stringToTheme(themeName);
        m_themeManager->setTheme(theme);
        
        // Update status
        m_statusLabel->setText(QString("Theme changed to: %1").arg(themeName));
    }
    
    void onTestAnimation()
    {
        // Test fade animation
        QPropertyAnimation* fadeAnim = StyleUtils::createFadeAnimation(m_testLabel, 1000, 1.0, 0.3);
        fadeAnim->start(QAbstractAnimation::DeleteWhenStopped);
        
        // Test button hover effect
        StyleUtils::addHoverScaleEffect(m_testButton, 1.1, 200);
    }

private:
    void setupUI()
    {
        setWindowTitle("Jitsi Meet Qt - Theme & Resource Test");
        setWindowIcon(QIcon(":/icons/app.svg"));
        resize(800, 600);
        
        // Central widget
        QWidget* centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        // Main layout
        QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
        
        // Theme selection
        QHBoxLayout* themeLayout = new QHBoxLayout();
        themeLayout->addWidget(new QLabel("Theme:"));
        
        m_themeCombo = new QComboBox();
        m_themeCombo->addItems(m_themeManager->availableThemes());
        themeLayout->addWidget(m_themeCombo);
        
        QPushButton* applyButton = new QPushButton("Apply Theme");
        applyButton->setIcon(QIcon(":/icons/settings.svg"));
        StyleHelper::styleButton(applyButton, StyleHelper::ButtonStyle::Primary);
        themeLayout->addWidget(applyButton);
        
        themeLayout->addStretch();
        mainLayout->addLayout(themeLayout);
        
        // Tab widget for different UI elements
        QTabWidget* tabWidget = new QTabWidget();
        
        // Basic Controls Tab
        QWidget* basicTab = createBasicControlsTab();
        tabWidget->addTab(basicTab, QIcon(":/icons/settings.svg"), "Basic Controls");
        
        // Conference Controls Tab
        QWidget* conferenceTab = createConferenceControlsTab();
        tabWidget->addTab(conferenceTab, QIcon(":/icons/camera.svg"), "Conference Controls");
        
        // Chat Tab
        QWidget* chatTab = createChatTab();
        tabWidget->addTab(chatTab, QIcon(":/icons/chat.svg"), "Chat Interface");
        
        mainLayout->addWidget(tabWidget);
        
        // Test animation section
        QGroupBox* animationGroup = new QGroupBox("Animation Tests");
        QHBoxLayout* animLayout = new QHBoxLayout(animationGroup);
        
        m_testLabel = new QLabel("Test Label for Animations");
        StyleHelper::styleLabel(m_testLabel, "title");
        animLayout->addWidget(m_testLabel);
        
        m_testButton = new QPushButton("Test Animations");
        m_testButton->setIcon(QIcon(":/icons/refresh.svg"));
        StyleHelper::styleButton(m_testButton, StyleHelper::ButtonStyle::Secondary);
        animLayout->addWidget(m_testButton);
        
        mainLayout->addWidget(animationGroup);
        
        // Status bar
        m_statusLabel = new QLabel("Ready");
        statusBar()->addWidget(m_statusLabel);
        
        // Menu bar
        setupMenuBar();
    }
    
    QWidget* createBasicControlsTab()
    {
        QWidget* tab = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(tab);
        
        // Input controls
        QGroupBox* inputGroup = new QGroupBox("Input Controls");
        QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
        
        QLineEdit* lineEdit = new QLineEdit();
        lineEdit->setPlaceholderText("Enter meeting URL...");
        StyleHelper::styleLineEdit(lineEdit, StyleHelper::InputStyle::Default);
        inputLayout->addWidget(lineEdit);
        
        QLineEdit* roundedEdit = new QLineEdit();
        roundedEdit->setPlaceholderText("Rounded input...");
        StyleHelper::styleLineEdit(roundedEdit, StyleHelper::InputStyle::Rounded);
        inputLayout->addWidget(roundedEdit);
        
        QComboBox* combo = new QComboBox();
        combo->addItems({"Option 1", "Option 2", "Option 3"});
        inputLayout->addWidget(combo);
        
        QSpinBox* spinBox = new QSpinBox();
        spinBox->setRange(1, 100);
        spinBox->setValue(50);
        inputLayout->addWidget(spinBox);
        
        layout->addWidget(inputGroup);
        
        // Button controls
        QGroupBox* buttonGroup = new QGroupBox("Button Styles");
        QHBoxLayout* buttonLayout = new QHBoxLayout(buttonGroup);
        
        QPushButton* primaryBtn = new QPushButton("Primary");
        primaryBtn->setIcon(QIcon(":/icons/join.svg"));
        StyleHelper::styleButton(primaryBtn, StyleHelper::ButtonStyle::Primary);
        buttonLayout->addWidget(primaryBtn);
        
        QPushButton* secondaryBtn = new QPushButton("Secondary");
        secondaryBtn->setIcon(QIcon(":/icons/settings.svg"));
        StyleHelper::styleButton(secondaryBtn, StyleHelper::ButtonStyle::Secondary);
        buttonLayout->addWidget(secondaryBtn);
        
        QPushButton* successBtn = new QPushButton("Success");
        successBtn->setIcon(QIcon(":/icons/success.svg"));
        StyleHelper::styleButton(successBtn, StyleHelper::ButtonStyle::Success);
        buttonLayout->addWidget(successBtn);
        
        QPushButton* errorBtn = new QPushButton("Error");
        errorBtn->setIcon(QIcon(":/icons/error.svg"));
        StyleHelper::styleButton(errorBtn, StyleHelper::ButtonStyle::Error);
        buttonLayout->addWidget(errorBtn);
        
        layout->addWidget(buttonGroup);
        
        // Progress and sliders
        QGroupBox* progressGroup = new QGroupBox("Progress & Sliders");
        QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
        
        QProgressBar* progressBar = new QProgressBar();
        progressBar->setValue(65);
        progressLayout->addWidget(progressBar);
        
        QSlider* slider = new QSlider(Qt::Horizontal);
        slider->setRange(0, 100);
        slider->setValue(75);
        progressLayout->addWidget(slider);
        
        layout->addWidget(progressGroup);
        
        layout->addStretch();
        return tab;
    }
    
    QWidget* createConferenceControlsTab()
    {
        QWidget* tab = new QWidget();
        QVBoxLayout* layout = new QVBoxLayout(tab);
        
        // Video area simulation
        QLabel* videoArea = new QLabel("Video Conference Area");
        videoArea->setObjectName("VideoArea");
        videoArea->setMinimumHeight(200);
        videoArea->setAlignment(Qt::AlignCenter);
        videoArea->setStyleSheet(
            "QLabel#VideoArea {"
            "    background-color: #212121;"
            "    color: #BDBDBD;"
            "    font-size: 18pt;"
            "    border-radius: 8px;"
            "}"
        );
        layout->addWidget(videoArea);
        
        // Control panel
        QWidget* controlPanel = new QWidget();
        controlPanel->setObjectName("ControlPanel");
        QHBoxLayout* controlLayout = new QHBoxLayout(controlPanel);
        controlLayout->setSpacing(16);
        
        // Conference control buttons
        QPushButton* muteBtn = new QPushButton();
        muteBtn->setObjectName("MuteAudioButton");
        muteBtn->setIcon(QIcon(":/icons/microphone.svg"));
        muteBtn->setCheckable(true);
        muteBtn->setToolTip("Toggle Microphone");
        
        QPushButton* videoBtn = new QPushButton();
        videoBtn->setObjectName("MuteVideoButton");
        videoBtn->setIcon(QIcon(":/icons/camera.svg"));
        videoBtn->setCheckable(true);
        videoBtn->setToolTip("Toggle Camera");
        
        QPushButton* shareBtn = new QPushButton();
        shareBtn->setObjectName("ScreenShareButton");
        shareBtn->setIcon(QIcon(":/icons/screen-share.svg"));
        shareBtn->setCheckable(true);
        shareBtn->setToolTip("Share Screen");
        
        QPushButton* chatBtn = new QPushButton();
        chatBtn->setObjectName("ChatToggleButton");
        chatBtn->setIcon(QIcon(":/icons/chat.svg"));
        chatBtn->setCheckable(true);
        chatBtn->setToolTip("Toggle Chat");
        
        QPushButton* participantsBtn = new QPushButton();
        participantsBtn->setObjectName("ParticipantsToggleButton");
        participantsBtn->setIcon(QIcon(":/icons/participants.svg"));
        participantsBtn->setCheckable(true);
        participantsBtn->setToolTip("Show Participants");
        
        QPushButton* hangupBtn = new QPushButton();
        hangupBtn->setObjectName("HangupButton");
        hangupBtn->setIcon(QIcon(":/icons/phone-hangup.svg"));
        hangupBtn->setToolTip("Leave Meeting");
        
        controlLayout->addStretch();
        controlLayout->addWidget(muteBtn);
        controlLayout->addWidget(videoBtn);
        controlLayout->addWidget(shareBtn);
        controlLayout->addWidget(chatBtn);
        controlLayout->addWidget(participantsBtn);
        controlLayout->addWidget(hangupBtn);
        controlLayout->addStretch();
        
        layout->addWidget(controlPanel);
        
        return tab;
    }
    
    QWidget* createChatTab()
    {
        QWidget* tab = new QWidget();
        QHBoxLayout* layout = new QHBoxLayout(tab);
        
        // Main content area
        QWidget* mainArea = new QWidget();
        QVBoxLayout* mainLayout = new QVBoxLayout(mainArea);
        
        QLabel* mainLabel = new QLabel("Main Conference Content");
        mainLabel->setAlignment(Qt::AlignCenter);
        mainLabel->setMinimumHeight(300);
        mainLabel->setStyleSheet("background-color: #F5F5F5; border: 1px solid #E0E0E0; border-radius: 8px;");
        mainLayout->addWidget(mainLabel);
        
        layout->addWidget(mainArea, 2);
        
        // Chat panel
        QWidget* chatPanel = new QWidget();
        chatPanel->setObjectName("ChatPanel");
        QVBoxLayout* chatLayout = new QVBoxLayout(chatPanel);
        
        QLabel* chatTitle = new QLabel("Chat");
        chatTitle->setStyleSheet("font-weight: bold; padding: 8px;");
        chatLayout->addWidget(chatTitle);
        
        QTextEdit* chatDisplay = new QTextEdit();
        chatDisplay->setObjectName("ChatDisplay");
        chatDisplay->setReadOnly(true);
        chatDisplay->setPlainText("John: Hello everyone!\nJane: Hi there!\nBob: Good morning!");
        chatLayout->addWidget(chatDisplay);
        
        QHBoxLayout* chatInputLayout = new QHBoxLayout();
        QLineEdit* chatInput = new QLineEdit();
        chatInput->setObjectName("ChatInput");
        chatInput->setPlaceholderText("Type a message...");
        
        QPushButton* sendBtn = new QPushButton();
        sendBtn->setObjectName("SendButton");
        sendBtn->setIcon(QIcon(":/icons/send.svg"));
        sendBtn->setToolTip("Send Message");
        
        chatInputLayout->addWidget(chatInput);
        chatInputLayout->addWidget(sendBtn);
        chatLayout->addLayout(chatInputLayout);
        
        layout->addWidget(chatPanel, 1);
        
        return tab;
    }
    
    void setupMenuBar()
    {
        QMenuBar* menuBar = this->menuBar();
        
        // File menu
        QMenu* fileMenu = menuBar->addMenu(QIcon(":/icons/settings.svg"), "&File");
        fileMenu->addAction(QIcon(":/icons/join.svg"), "&Join Meeting", [this]() {
            m_statusLabel->setText("Join Meeting clicked");
        });
        fileMenu->addSeparator();
        fileMenu->addAction(QIcon(":/icons/close.svg"), "&Exit", this, &QWidget::close);
        
        // View menu
        QMenu* viewMenu = menuBar->addMenu("&View");
        viewMenu->addAction(QIcon(":/icons/fullscreen.svg"), "&Fullscreen");
        viewMenu->addAction(QIcon(":/icons/settings.svg"), "&Settings");
        
        // Help menu
        QMenu* helpMenu = menuBar->addMenu("&Help");
        helpMenu->addAction(QIcon(":/icons/about.svg"), "&About");
    }
    
    void setupConnections()
    {
        connect(m_themeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &ThemeTestWindow::onThemeChanged);
        
        connect(m_testButton, &QPushButton::clicked,
                this, &ThemeTestWindow::onTestAnimation);
        
        connect(m_themeManager, &ThemeManager::themeChanged,
                [this](ThemeManager::Theme theme) {
                    qDebug() << "Theme changed to:" << ThemeManager::themeToString(theme);
                });
    }

private:
    ThemeManager* m_themeManager;
    QComboBox* m_themeCombo;
    QLabel* m_statusLabel;
    QLabel* m_testLabel;
    QPushButton* m_testButton;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("Jitsi Meet Qt");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Jitsi");
    app.setApplicationDisplayName("Jitsi Meet Qt - Theme Test");
    
    // Test resource loading
    qDebug() << "Testing resource loading...";
    
    // Test icon loading
    QIcon appIcon(":/icons/app.svg");
    if (appIcon.isNull()) {
        qWarning() << "Failed to load app icon from resources";
    } else {
        qDebug() << "App icon loaded successfully";
    }
    
    // Test style loading
    QFile styleFile(":/styles/default.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        qDebug() << "Default stylesheet loaded successfully, size:" << styleFile.size() << "bytes";
        styleFile.close();
    } else {
        qWarning() << "Failed to load default stylesheet";
    }
    
    // Create and show main window
    ThemeTestWindow window;
    window.show();
    
    return app.exec();
}

#include "test_theme_resources.moc"
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QTextEdit>
#include <QGroupBox>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

#include "include/TranslationManager.h"

class TranslationTestWindow : public QMainWindow
{
    Q_OBJECT

public:
    TranslationTestWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_translationManager(new TranslationManager(this))
    {
        setupUI();
        setupConnections();
        
        // Initialize translation manager
        if (m_translationManager->initialize()) {
            qDebug() << "TranslationManager initialized successfully";
            updateLanguageCombo();
            updateTranslationDisplay();
        } else {
            qDebug() << "Failed to initialize TranslationManager";
        }
    }

private slots:
    void onLanguageChanged(int index)
    {
        if (index < 0 || index >= m_languageCombo->count()) {
            return;
        }
        
        QString languageCode = m_languageCombo->itemData(index).toString();
        qDebug() << "Changing language to:" << languageCode;
        
        if (m_translationManager->setLanguage(languageCode)) {
            updateTranslationDisplay();
            qDebug() << "Language changed successfully";
        } else {
            qDebug() << "Failed to change language";
        }
    }
    
    void onTranslationManagerLanguageChanged(TranslationManager::Language language, const QString& languageCode)
    {
        qDebug() << "TranslationManager language changed to:" << languageCode;
        updateTranslationDisplay();
        
        // Update combo box selection
        for (int i = 0; i < m_languageCombo->count(); ++i) {
            if (m_languageCombo->itemData(i).toString() == languageCode) {
                m_languageCombo->blockSignals(true);
                m_languageCombo->setCurrentIndex(i);
                m_languageCombo->blockSignals(false);
                break;
            }
        }
    }
    
    void onReloadTranslations()
    {
        m_translationManager->reloadTranslations();
        updateTranslationDisplay();
    }

private:
    void setupUI()
    {
        setWindowTitle("Translation Manager Test");
        setMinimumSize(600, 400);
        
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
        
        // Language selection
        QGroupBox *languageGroup = new QGroupBox("Language Selection", this);
        QHBoxLayout *languageLayout = new QHBoxLayout(languageGroup);
        
        QLabel *languageLabel = new QLabel("Language:", this);
        m_languageCombo = new QComboBox(this);
        m_reloadButton = new QPushButton("Reload Translations", this);
        
        languageLayout->addWidget(languageLabel);
        languageLayout->addWidget(m_languageCombo);
        languageLayout->addWidget(m_reloadButton);
        languageLayout->addStretch();
        
        mainLayout->addWidget(languageGroup);
        
        // Translation display
        QGroupBox *displayGroup = new QGroupBox("Translation Display", this);
        QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);
        
        m_translationDisplay = new QTextEdit(this);
        m_translationDisplay->setReadOnly(true);
        
        displayLayout->addWidget(m_translationDisplay);
        mainLayout->addWidget(displayGroup);
        
        // Status
        QGroupBox *statusGroup = new QGroupBox("Status", this);
        QVBoxLayout *statusLayout = new QVBoxLayout(statusGroup);
        
        m_statusLabel = new QLabel(this);
        statusLayout->addWidget(m_statusLabel);
        
        mainLayout->addWidget(statusGroup);
    }
    
    void setupConnections()
    {
        connect(m_languageCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
                this, &TranslationTestWindow::onLanguageChanged);
        
        connect(m_reloadButton, &QPushButton::clicked,
                this, &TranslationTestWindow::onReloadTranslations);
        
        connect(m_translationManager, &TranslationManager::languageChanged,
                this, &TranslationTestWindow::onTranslationManagerLanguageChanged);
    }
    
    void updateLanguageCombo()
    {
        m_languageCombo->clear();
        
        // Add auto-detect option
        m_languageCombo->addItem("Auto-detect", "auto");
        
        // Add available languages
        auto availableLanguages = m_translationManager->availableLanguages();
        for (const auto& langInfo : availableLanguages) {
            QString displayName = QString("%1 (%2)").arg(langInfo.nativeName, langInfo.code);
            m_languageCombo->addItem(displayName, langInfo.code);
        }
        
        // Set current language
        QString currentCode = m_translationManager->currentLanguageCode();
        for (int i = 0; i < m_languageCombo->count(); ++i) {
            if (m_languageCombo->itemData(i).toString() == currentCode) {
                m_languageCombo->setCurrentIndex(i);
                break;
            }
        }
    }
    
    void updateTranslationDisplay()
    {
        QString display;
        
        // Current language info
        display += QString("Current Language: %1 (%2)\n")
                   .arg(m_translationManager->currentLanguageCode())
                   .arg(static_cast<int>(m_translationManager->currentLanguage()));
        
        display += QString("System Language: %1\n\n")
                   .arg(static_cast<int>(m_translationManager->systemLanguage()));
        
        // Sample translations
        display += "Sample Translations:\n";
        display += "===================\n";
        
        // Test some common UI strings
        QStringList testStrings = {
            "Jitsi Meet",
            "Enter meeting URL or room name", 
            "Join",
            "Settings",
            "Mute",
            "Camera On",
            "Share Screen",
            "Chat",
            "Participants"
        };
        
        for (const QString& str : testStrings) {
            QString translation = m_translationManager->translate("WelcomeWindow", str);
            if (translation == str) {
                translation = m_translationManager->translate("ConferenceWindow", str);
            }
            display += QString("%1 -> %2\n").arg(str, translation);
        }
        
        m_translationDisplay->setPlainText(display);
        
        // Update status
        QString status = QString("Translation Manager Status: %1\n")
                        .arg(m_translationManager->currentLanguageCode());
        
        auto availableLanguages = m_translationManager->availableLanguages();
        status += QString("Available Languages: %1").arg(availableLanguages.size());
        
        m_statusLabel->setText(status);
    }

private:
    TranslationManager *m_translationManager;
    QComboBox *m_languageCombo;
    QPushButton *m_reloadButton;
    QTextEdit *m_translationDisplay;
    QLabel *m_statusLabel;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties for QSettings
    QCoreApplication::setApplicationName("JitsiMeetQt");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("JitsiMeet");
    
    qDebug() << "Starting Translation Manager Test";
    qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();
    qDebug() << "Working directory:" << QDir::currentPath();
    
    TranslationTestWindow window;
    window.show();
    
    return app.exec();
}

#include "test_translation_simple.moc"
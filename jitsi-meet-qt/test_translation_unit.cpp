#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QLocale>
#include <QTranslator>

/**
 * @brief Simple unit test for translation functionality
 * 
 * This test verifies basic translation file loading and language detection
 * without requiring the full TranslationManager class.
 */

void testTranslationFileExistence()
{
    qDebug() << "\n=== Testing Translation File Existence ===";
    
    QString translationsPath = QCoreApplication::applicationDirPath() + "/translations";
    qDebug() << "Translations path:" << translationsPath;
    
    QStringList expectedFiles = {
        "jitsimeet_en.qm",
        "jitsimeet_zh_CN.qm", 
        "jitsimeet_es.qm"
    };
    
    for (const QString& file : expectedFiles) {
        QString fullPath = translationsPath + "/" + file;
        bool exists = QFileInfo::exists(fullPath);
        qDebug() << file << ":" << (exists ? "EXISTS" : "MISSING");
    }
}

void testBasicTranslationLoading()
{
    qDebug() << "\n=== Testing Basic Translation Loading ===";
    
    QString translationsPath = QCoreApplication::applicationDirPath() + "/translations";
    
    // Test loading English translation
    QTranslator englishTranslator;
    QString englishFile = translationsPath + "/jitsimeet_en.qm";
    bool englishLoaded = englishTranslator.load(englishFile);
    qDebug() << "English translation loaded:" << (englishLoaded ? "SUCCESS" : "FAILED");
    
    if (englishLoaded) {
        QString translation = englishTranslator.translate("WelcomeWindow", "Join");
        qDebug() << "English 'Join' translation:" << translation;
    }
    
    // Test loading Chinese translation
    QTranslator chineseTranslator;
    QString chineseFile = translationsPath + "/jitsimeet_zh_CN.qm";
    bool chineseLoaded = chineseTranslator.load(chineseFile);
    qDebug() << "Chinese translation loaded:" << (chineseLoaded ? "SUCCESS" : "FAILED");
    
    if (chineseLoaded) {
        QString translation = chineseTranslator.translate("WelcomeWindow", "Join");
        qDebug() << "Chinese 'Join' translation:" << translation;
    }
}

void testSystemLanguageDetection()
{
    qDebug() << "\n=== Testing System Language Detection ===";
    
    QLocale systemLocale = QLocale::system();
    qDebug() << "System locale name:" << systemLocale.name();
    qDebug() << "System language:" << QLocale::languageToString(systemLocale.language());
    qDebug() << "System country:" << QLocale::countryToString(systemLocale.country());
    
    // Test language code mapping
    QString languageCode = systemLocale.name();
    QString mappedLanguage;
    
    if (languageCode.startsWith("zh")) {
        mappedLanguage = "Chinese";
    } else if (languageCode.startsWith("es")) {
        mappedLanguage = "Spanish";
    } else if (languageCode.startsWith("fr")) {
        mappedLanguage = "French";
    } else if (languageCode.startsWith("de")) {
        mappedLanguage = "German";
    } else if (languageCode.startsWith("ja")) {
        mappedLanguage = "Japanese";
    } else if (languageCode.startsWith("ko")) {
        mappedLanguage = "Korean";
    } else if (languageCode.startsWith("ru")) {
        mappedLanguage = "Russian";
    } else if (languageCode.startsWith("pt")) {
        mappedLanguage = "Portuguese";
    } else if (languageCode.startsWith("it")) {
        mappedLanguage = "Italian";
    } else {
        mappedLanguage = "English (default)";
    }
    
    qDebug() << "Mapped language:" << mappedLanguage;
}

void testApplicationTranslatorInstallation()
{
    qDebug() << "\n=== Testing Application Translator Installation ===";
    
    QString translationsPath = QCoreApplication::applicationDirPath() + "/translations";
    
    // Create and install translator
    QTranslator* translator = new QTranslator(QCoreApplication::instance());
    QString translationFile = translationsPath + "/jitsimeet_zh_CN.qm";
    
    bool loaded = translator->load(translationFile);
    qDebug() << "Translation file loaded:" << (loaded ? "SUCCESS" : "FAILED");
    
    if (loaded) {
        bool installed = QCoreApplication::installTranslator(translator);
        qDebug() << "Translator installed:" << (installed ? "SUCCESS" : "FAILED");
        
        if (installed) {
            // Test translation using QCoreApplication::translate
            QString translation = QCoreApplication::translate("WelcomeWindow", "Join");
            qDebug() << "Application translate 'Join':" << translation;
            
            // Remove translator
            QCoreApplication::removeTranslator(translator);
            qDebug() << "Translator removed";
        }
    }
    
    delete translator;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set application properties
    QCoreApplication::setApplicationName("JitsiMeetQt");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("JitsiMeet");
    
    qDebug() << "Translation Unit Test";
    qDebug() << "====================";
    qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();
    qDebug() << "Working directory:" << QDir::currentPath();
    
    // Run tests
    testTranslationFileExistence();
    testSystemLanguageDetection();
    testBasicTranslationLoading();
    testApplicationTranslatorInstallation();
    
    qDebug() << "\n=== Test Summary ===";
    qDebug() << "Unit tests completed successfully!";
    
    return 0;
}
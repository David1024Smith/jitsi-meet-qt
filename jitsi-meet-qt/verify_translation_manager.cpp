#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QTranslator>
#include <QLocale>

#include "include/TranslationManager.h"

/**
 * @brief Verification test for TranslationManager implementation
 * 
 * This test verifies that the TranslationManager correctly:
 * 1. Initializes and detects system language
 * 2. Loads available translation files
 * 3. Switches between languages
 * 4. Provides translation functionality
 * 5. Handles error cases gracefully
 */

void testBasicFunctionality(TranslationManager* manager)
{
    qDebug() << "\n=== Testing Basic Functionality ===";
    
    // Test initialization
    bool initialized = manager->initialize();
    qDebug() << "Initialization:" << (initialized ? "SUCCESS" : "FAILED");
    
    // Test current language
    auto currentLang = manager->currentLanguage();
    QString currentCode = manager->currentLanguageCode();
    qDebug() << "Current language:" << currentCode << "(" << static_cast<int>(currentLang) << ")";
    
    // Test system language detection
    auto systemLang = manager->systemLanguage();
    qDebug() << "System language:" << static_cast<int>(systemLang);
    
    // Test available languages
    auto availableLanguages = manager->availableLanguages();
    qDebug() << "Available languages:" << availableLanguages.size();
    for (const auto& langInfo : availableLanguages) {
        qDebug() << "  -" << langInfo.code << ":" << langInfo.nativeName 
                 << "(" << langInfo.englishName << ") - Available:" << langInfo.available;
    }
}

void testLanguageSwitching(TranslationManager* manager)
{
    qDebug() << "\n=== Testing Language Switching ===";
    
    // Test switching to English
    bool success = manager->setLanguage(TranslationManager::Language::English);
    qDebug() << "Switch to English:" << (success ? "SUCCESS" : "FAILED");
    qDebug() << "Current language after switch:" << manager->currentLanguageCode();
    
    // Test switching to Chinese (if available)
    success = manager->setLanguage(TranslationManager::Language::Chinese);
    qDebug() << "Switch to Chinese:" << (success ? "SUCCESS" : "FAILED");
    qDebug() << "Current language after switch:" << manager->currentLanguageCode();
    
    // Test switching by language code
    success = manager->setLanguage("es");
    qDebug() << "Switch to Spanish by code:" << (success ? "SUCCESS" : "FAILED");
    qDebug() << "Current language after switch:" << manager->currentLanguageCode();
    
    // Test auto-detection
    success = manager->setLanguage(TranslationManager::Language::Auto);
    qDebug() << "Switch to Auto:" << (success ? "SUCCESS" : "FAILED");
    qDebug() << "Current language after auto-detect:" << manager->currentLanguageCode();
}

void testTranslationFunctionality(TranslationManager* manager)
{
    qDebug() << "\n=== Testing Translation Functionality ===";
    
    // Test some common translations
    QStringList testKeys = {
        "Jitsi Meet",
        "Enter meeting URL or room name",
        "Join",
        "Settings",
        "Mute",
        "Camera On",
        "Share Screen"
    };
    
    // Test in English
    manager->setLanguage(TranslationManager::Language::English);
    qDebug() << "English translations:";
    for (const QString& key : testKeys) {
        QString translation = manager->translate("WelcomeWindow", key);
        qDebug() << "  " << key << "->" << translation;
    }
    
    // Test in Chinese (if available)
    if (manager->isLanguageSupported(TranslationManager::Language::Chinese)) {
        manager->setLanguage(TranslationManager::Language::Chinese);
        qDebug() << "Chinese translations:";
        for (const QString& key : testKeys) {
            QString translation = manager->translate("WelcomeWindow", key);
            qDebug() << "  " << key << "->" << translation;
        }
    }
}

void testLanguageInfo(TranslationManager* manager)
{
    qDebug() << "\n=== Testing Language Info ===";
    
    // Test getting language info by enum
    auto englishInfo = manager->getLanguageInfo(TranslationManager::Language::English);
    if (englishInfo.has_value()) {
        qDebug() << "English info:" << englishInfo->code << englishInfo->nativeName;
    }
    
    // Test getting language info by code
    auto chineseInfo = manager->getLanguageInfo("zh_CN");
    if (chineseInfo.has_value()) {
        qDebug() << "Chinese info:" << chineseInfo->code << chineseInfo->nativeName;
    }
    
    // Test language support checking
    qDebug() << "English supported:" << manager->isLanguageSupported(TranslationManager::Language::English);
    qDebug() << "Chinese supported:" << manager->isLanguageSupported(TranslationManager::Language::Chinese);
    qDebug() << "Spanish supported:" << manager->isLanguageSupported("es");
}

void testErrorHandling(TranslationManager* manager)
{
    qDebug() << "\n=== Testing Error Handling ===";
    
    // Test invalid language code
    bool success = manager->setLanguage("invalid_code");
    qDebug() << "Invalid language code:" << (success ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");
    
    // Test unsupported language
    success = manager->setLanguage("xx");
    qDebug() << "Unsupported language:" << (success ? "UNEXPECTED SUCCESS" : "CORRECTLY FAILED");
    
    // Test translation with invalid context
    QString translation = manager->translate("InvalidContext", "test_key");
    qDebug() << "Invalid context translation:" << translation;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    QCoreApplication::setApplicationName("JitsiMeetQt");
    QCoreApplication::setApplicationVersion("1.0.0");
    QCoreApplication::setOrganizationName("JitsiMeet");
    
    qDebug() << "TranslationManager Verification Test";
    qDebug() << "====================================";
    qDebug() << "Application directory:" << QCoreApplication::applicationDirPath();
    qDebug() << "Working directory:" << QDir::currentPath();
    qDebug() << "System locale:" << QLocale::system().name();
    
    // Create TranslationManager
    TranslationManager manager;
    
    // Run tests
    testBasicFunctionality(&manager);
    testLanguageSwitching(&manager);
    testTranslationFunctionality(&manager);
    testLanguageInfo(&manager);
    testErrorHandling(&manager);
    
    qDebug() << "\n=== Test Summary ===";
    qDebug() << "All tests completed. Check output above for results.";
    qDebug() << "Final language:" << manager.currentLanguageCode();
    
    return 0;
}
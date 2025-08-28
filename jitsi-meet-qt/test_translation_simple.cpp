#include <QApplication>
#include <QDebug>
#include <QTranslator>
#include <QLocale>
#include "TranslationManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    qDebug() << "Testing Translation System...";
    
    // Create TranslationManager
    TranslationManager translationManager;
    translationManager.initialize();
    
    // Test system language detection
    QString systemLang = translationManager.detectSystemLanguage();
    qDebug() << "System language detected:" << systemLang;
    
    // Test available languages
    QStringList availableLanguages = translationManager.availableLanguages();
    qDebug() << "Available languages:" << availableLanguages;
    
    // Test language switching
    QString currentLang = translationManager.currentLanguage();
    qDebug() << "Current language:" << currentLang;
    
    // Test switching to Chinese
    if (translationManager.setLanguage("zh_CN")) {
        qDebug() << "Successfully switched to Chinese";
        qDebug() << "Current language:" << translationManager.currentLanguage();
    } else {
        qDebug() << "Failed to switch to Chinese";
    }
    
    // Test switching back to English
    if (translationManager.setLanguage("en")) {
        qDebug() << "Successfully switched to English";
        qDebug() << "Current language:" << translationManager.currentLanguage();
    } else {
        qDebug() << "Failed to switch to English";
    }
    
    // Test language display names
    for (const QString& lang : availableLanguages) {
        QString displayName = translationManager.languageDisplayName(lang);
        qDebug() << "Language:" << lang << "Display name:" << displayName;
    }
    
    qDebug() << "Translation system test completed successfully!";
    
    return 0;
}
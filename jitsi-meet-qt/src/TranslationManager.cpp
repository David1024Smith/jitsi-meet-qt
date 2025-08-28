#include "TranslationManager.h"
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>

TranslationManager* TranslationManager::s_instance = nullptr;

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
    , m_currentLanguage(Language::Auto)
    , m_currentLanguageCode("en")
    , m_translator(std::make_unique<QTranslator>())
    , m_qtTranslator(std::make_unique<QTranslator>())
    , m_initialized(false)
{
    s_instance = this;
    
    // Set translations path
    m_translationsPath = QCoreApplication::applicationDirPath() + "/translations";
    
    // Initialize language information
    initializeLanguageInfo();
}

TranslationManager::~TranslationManager()
{
    unloadTranslation();
    s_instance = nullptr;
}

TranslationManager* TranslationManager::instance()
{
    return s_instance;
}

bool TranslationManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing TranslationManager...";
    qDebug() << "Translations path:" << m_translationsPath;

    // Update available languages based on existing files
    updateAvailableLanguages();

    // Detect and set system language
    Language systemLang = detectSystemLanguage();
    qDebug() << "Detected system language:" << languageToCode(systemLang);

    // Set initial language (auto-detect)
    bool success = setLanguage(Language::Auto);
    
    m_initialized = success;
    return success;
}

bool TranslationManager::setLanguage(Language language)
{
    Language targetLanguage = language;
    
    // Handle auto-detection
    if (language == Language::Auto) {
        targetLanguage = detectSystemLanguage();
        qDebug() << "Auto-detected language:" << languageToCode(targetLanguage);
    }

    QString languageCode = languageToCode(targetLanguage);
    
    // Check if language is supported
    if (!isLanguageSupported(targetLanguage)) {
        qWarning() << "Language not supported:" << languageCode << "- falling back to English";
        targetLanguage = Language::English;
        languageCode = "en";
    }

    // Don't reload if it's the same language
    if (m_currentLanguage == targetLanguage && m_currentLanguageCode == languageCode) {
        return true;
    }

    // Unload current translation
    unloadTranslation();

    // Load new translation (skip for English as it's the source language)
    bool success = true;
    if (targetLanguage != Language::English) {
        success = loadTranslation(languageCode);
        if (!success) {
            qWarning() << "Failed to load translation for:" << languageCode << "- falling back to English";
            targetLanguage = Language::English;
            languageCode = "en";
        }
    }

    // Update current language
    Language oldLanguage = m_currentLanguage;
    m_currentLanguage = targetLanguage;
    m_currentLanguageCode = languageCode;

    // Emit signal if language actually changed
    if (oldLanguage != targetLanguage) {
        emit languageChanged(targetLanguage, languageCode);
        qDebug() << "Language changed to:" << languageCode;
    }

    return success;
}

bool TranslationManager::setLanguage(const QString& languageCode)
{
    Language language = codeToLanguage(languageCode);
    if (language == Language::Auto && languageCode != "auto") {
        qWarning() << "Unknown language code:" << languageCode;
        return false;
    }
    
    return setLanguage(language);
}

TranslationManager::Language TranslationManager::currentLanguage() const
{
    return m_currentLanguage;
}

QString TranslationManager::currentLanguageCode() const
{
    return m_currentLanguageCode;
}

TranslationManager::Language TranslationManager::systemLanguage() const
{
    return detectSystemLanguage();
}

QList<TranslationManager::LanguageInfo> TranslationManager::availableLanguages() const
{
    QList<LanguageInfo> available;
    for (const auto& info : m_languageInfo) {
        if (info.available || info.language == Language::English) {
            available.append(info);
        }
    }
    return available;
}

std::optional<TranslationManager::LanguageInfo> TranslationManager::getLanguageInfo(Language language) const
{
    for (const auto& info : m_languageInfo) {
        if (info.language == language) {
            return info;
        }
    }
    return std::nullopt;
}

std::optional<TranslationManager::LanguageInfo> TranslationManager::getLanguageInfo(const QString& languageCode) const
{
    for (const auto& info : m_languageInfo) {
        if (info.code == languageCode) {
            return info;
        }
    }
    return std::nullopt;
}

bool TranslationManager::isLanguageSupported(Language language) const
{
    auto info = getLanguageInfo(language);
    return info.has_value() && (info->available || language == Language::English);
}

bool TranslationManager::isLanguageSupported(const QString& languageCode) const
{
    auto info = getLanguageInfo(languageCode);
    return info.has_value() && (info->available || languageCode == "en");
}

QString TranslationManager::translate(const QString& context, const QString& key, const QString& disambiguation) const
{
    if (!m_translator) {
        return key;
    }
    
    QString translation = m_translator->translate(context.toUtf8().constData(), 
                                                 key.toUtf8().constData(), 
                                                 disambiguation.toUtf8().constData());
    
    // Return key if translation not found
    return translation.isEmpty() ? key : translation;
}

void TranslationManager::reloadTranslations()
{
    Language currentLang = m_currentLanguage;
    setLanguage(Language::English);  // Temporarily switch to English
    setLanguage(currentLang);        // Switch back to reload
}

void TranslationManager::onApplicationLanguageChanged()
{
    // Handle system language changes
    if (m_currentLanguage == Language::Auto) {
        Language newSystemLang = detectSystemLanguage();
        if (newSystemLang != codeToLanguage(m_currentLanguageCode)) {
            setLanguage(Language::Auto);
        }
    }
}

TranslationManager::Language TranslationManager::detectSystemLanguage() const
{
    QLocale systemLocale = QLocale::system();
    QString languageCode = systemLocale.name();
    
    qDebug() << "System locale:" << languageCode;
    
    // Handle specific locale mappings
    if (languageCode.startsWith("zh")) {
        return Language::Chinese;
    } else if (languageCode.startsWith("es")) {
        return Language::Spanish;
    } else if (languageCode.startsWith("fr")) {
        return Language::French;
    } else if (languageCode.startsWith("de")) {
        return Language::German;
    } else if (languageCode.startsWith("ja")) {
        return Language::Japanese;
    } else if (languageCode.startsWith("ko")) {
        return Language::Korean;
    } else if (languageCode.startsWith("ru")) {
        return Language::Russian;
    } else if (languageCode.startsWith("pt")) {
        return Language::Portuguese;
    } else if (languageCode.startsWith("it")) {
        return Language::Italian;
    }
    
    // Default to English
    return Language::English;
}

QString TranslationManager::languageToCode(Language language) const
{
    switch (language) {
        case Language::Auto: return "auto";
        case Language::English: return "en";
        case Language::Chinese: return "zh_CN";
        case Language::Spanish: return "es";
        case Language::French: return "fr";
        case Language::German: return "de";
        case Language::Japanese: return "ja";
        case Language::Korean: return "ko";
        case Language::Russian: return "ru";
        case Language::Portuguese: return "pt";
        case Language::Italian: return "it";
    }
    return "en";
}

TranslationManager::Language TranslationManager::codeToLanguage(const QString& code) const
{
    if (code == "auto") return Language::Auto;
    if (code == "en") return Language::English;
    if (code == "zh_CN" || code == "zh") return Language::Chinese;
    if (code == "es") return Language::Spanish;
    if (code == "fr") return Language::French;
    if (code == "de") return Language::German;
    if (code == "ja") return Language::Japanese;
    if (code == "ko") return Language::Korean;
    if (code == "ru") return Language::Russian;
    if (code == "pt") return Language::Portuguese;
    if (code == "it") return Language::Italian;
    
    return Language::Auto;  // Unknown code
}

bool TranslationManager::loadTranslation(const QString& languageCode)
{
    QString translationFile = getTranslationFilePath(languageCode);
    
    qDebug() << "Loading translation file:" << translationFile;
    
    if (!QFileInfo::exists(translationFile)) {
        qWarning() << "Translation file not found:" << translationFile;
        emit translationLoadFailed(languageCode, "Translation file not found");
        return false;
    }

    // Load application translation
    if (!m_translator->load(translationFile)) {
        qWarning() << "Failed to load translation:" << translationFile;
        emit translationLoadFailed(languageCode, "Failed to load translation file");
        return false;
    }

    // Install application translator
    if (!QCoreApplication::installTranslator(m_translator.get())) {
        qWarning() << "Failed to install translator for:" << languageCode;
        emit translationLoadFailed(languageCode, "Failed to install translator");
        return false;
    }

    // Load Qt's built-in translations
    QString qtTranslationFile = QString("qt_%1").arg(languageCode);
    QString qtTranslationPath = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
    
    if (m_qtTranslator->load(qtTranslationFile, qtTranslationPath)) {
        QCoreApplication::installTranslator(m_qtTranslator.get());
        qDebug() << "Loaded Qt translation:" << qtTranslationFile;
    } else {
        qDebug() << "Qt translation not available for:" << languageCode;
    }

    qDebug() << "Successfully loaded translation for:" << languageCode;
    return true;
}

void TranslationManager::unloadTranslation()
{
    if (m_translator) {
        QCoreApplication::removeTranslator(m_translator.get());
    }
    
    if (m_qtTranslator) {
        QCoreApplication::removeTranslator(m_qtTranslator.get());
    }
}

QString TranslationManager::getTranslationFilePath(const QString& languageCode) const
{
    return QString("%1/jitsimeet_%2.qm").arg(m_translationsPath, languageCode);
}

bool TranslationManager::translationFileExists(const QString& languageCode) const
{
    return QFileInfo::exists(getTranslationFilePath(languageCode));
}

void TranslationManager::initializeLanguageInfo()
{
    m_languageInfo = {
        {Language::English, "en", "English", "English", true},
        {Language::Chinese, "zh_CN", "中文", "Chinese", false},
        {Language::Spanish, "es", "Español", "Spanish", false},
        {Language::French, "fr", "Français", "French", false},
        {Language::German, "de", "Deutsch", "German", false},
        {Language::Japanese, "ja", "日本語", "Japanese", false},
        {Language::Korean, "ko", "한국어", "Korean", false},
        {Language::Russian, "ru", "Русский", "Russian", false},
        {Language::Portuguese, "pt", "Português", "Portuguese", false},
        {Language::Italian, "it", "Italiano", "Italian", false}
    };
}

void TranslationManager::updateAvailableLanguages()
{
    for (auto& info : m_languageInfo) {
        if (info.language != Language::English) {
            info.available = translationFileExists(info.code);
            qDebug() << "Language" << info.code << "available:" << info.available;
        }
    }
}
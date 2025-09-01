#include "TranslationManager.h"
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QStandardPaths>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

TranslationManager* TranslationManager::s_instance = nullptr;

TranslationManager::TranslationManager(QObject *parent)
    : QObject(parent)
    , m_currentLanguage("en")
    , m_translationsPath(QCoreApplication::applicationDirPath() + "/translations")
    , m_initialized(false)
{
    s_instance = this;
    loadLanguageNames();
    setupDefaultLanguage();
}

TranslationManager::~TranslationManager()
{
    removeCurrentTranslator();
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

    detectAvailableTranslations();
    
    // Set system language as default
    QString systemLang = getSystemLanguage();
    qDebug() << "Detected system language:" << systemLang;

    bool success = setCurrentLanguage(systemLang);
    if (!success) {
        success = setCurrentLanguage("en");
    }

    m_initialized = success;
    return success;
}

void TranslationManager::shutdown()
{
    removeCurrentTranslator();
    m_initialized = false;
}

bool TranslationManager::setCurrentLanguage(const QString& language)
{
    if (m_currentLanguage == language) {
        return true;
    }

    if (!isLanguageAvailable(language) && language != "en") {
        qWarning() << "Language not available:" << language;
        return false;
    }

    removeCurrentTranslator();
    
    bool success = installTranslator(language);
    if (success) {
        m_currentLanguage = language;
        emit currentLanguageChanged(language);
        qDebug() << "Language changed to:" << language;
    }

    return success;
}

bool TranslationManager::setLanguage(const QString& language)
{
    return setCurrentLanguage(language);
}

QString TranslationManager::currentLanguage() const
{
    return m_currentLanguage;
}

QString TranslationManager::currentLanguageCode() const
{
    return m_currentLanguage;
}

QString TranslationManager::currentLanguageName() const
{
    return getLanguageName(m_currentLanguage);
}

QLocale::Language TranslationManager::systemLanguage() const
{
    return QLocale::system().language();
}

QStringList TranslationManager::availableLanguages() const
{
    return m_availableLanguages;
}

QString TranslationManager::getLanguageName(const QString& language) const
{
    return m_languageNames.value(language, language);
}

QString TranslationManager::getLanguageCode(const QString& languageName) const
{
    for (auto it = m_languageNames.constBegin(); it != m_languageNames.constEnd(); ++it) {
        if (it.value() == languageName) {
            return it.key();
        }
    }
    return languageName;
}

bool TranslationManager::loadTranslationFile(const QString& filePath, const QString& language)
{
    QTranslator* translator = new QTranslator(this);
    if (translator->load(filePath)) {
        QCoreApplication::installTranslator(translator);
        qDebug() << "Loaded translation file:" << filePath;
        emit translationLoaded(language, true);
        return true;
    } else {
        delete translator;
        qWarning() << "Failed to load translation file:" << filePath;
        emit translationLoaded(language, false);
        return false;
    }
}

bool TranslationManager::loadAllTranslations()
{
    detectAvailableTranslations();
    return !m_availableLanguages.isEmpty();
}

QString TranslationManager::getSystemLanguage() const
{
    QLocale systemLocale = QLocale::system();
    QString langCode = systemLocale.name().left(2);
    
    if (isLanguageAvailable(langCode)) {
        return langCode;
    }
    
    return "en"; // fallback to English
}

QString TranslationManager::translate(const QString& text, const QString& context) const
{
    if (context.isEmpty()) {
        return QCoreApplication::translate("", text.toUtf8().constData());
    } else {
        return QCoreApplication::translate(context.toUtf8().constData(), text.toUtf8().constData());
    }
}

void TranslationManager::setTranslationsPath(const QString& path)
{
    m_translationsPath = path;
    detectAvailableTranslations();
}

QString TranslationManager::translationsPath() const
{
    return m_translationsPath;
}

bool TranslationManager::reloadCurrentTranslation()
{
    QString current = m_currentLanguage;
    removeCurrentTranslator();
    return setCurrentLanguage(current);
}

bool TranslationManager::isLanguageAvailable(const QString& language) const
{
    return m_availableLanguages.contains(language);
}

void TranslationManager::onLocaleChanged(const QLocale& locale)
{
    QString langCode = locale.name().left(2);
    if (isLanguageAvailable(langCode)) {
        setCurrentLanguage(langCode);
    }
}

void TranslationManager::detectAvailableTranslations()
{
    m_availableLanguages.clear();
    
    // Always include English as default
    m_availableLanguages << "en";
    
    QDir translationsDir(m_translationsPath);
    if (!translationsDir.exists()) {
        qWarning() << "Translations directory does not exist:" << m_translationsPath;
        emit availableLanguagesChanged(m_availableLanguages);
        return;
    }
    
    QStringList filters;
    filters << "*.qm";
    QFileInfoList files = translationsDir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo& fileInfo : files) {
        QString baseName = fileInfo.baseName();
        // Extract language code from filename (e.g., "app_zh_CN.qm" -> "zh_CN")
        if (baseName.contains('_')) {
            QString langCode = baseName.section('_', 1);
            if (!langCode.isEmpty() && !m_availableLanguages.contains(langCode)) {
                m_availableLanguages << langCode;
            }
        }
    }
    
    qDebug() << "Available languages:" << m_availableLanguages;
    emit availableLanguagesChanged(m_availableLanguages);
}

bool TranslationManager::installTranslator(const QString& language)
{
    if (language == "en") {
        // English is the default, no translation file needed
        return true;
    }
    
    QString filePath = getTranslationFilePath(language);
    if (filePath.isEmpty()) {
        qWarning() << "No translation file found for language:" << language;
        return false;
    }
    
    if (m_translator.load(filePath)) {
        QCoreApplication::installTranslator(&m_translator);
        qDebug() << "Installed translator for language:" << language;
        return true;
    } else {
        qWarning() << "Failed to load translation file:" << filePath;
        return false;
    }
}

void TranslationManager::removeCurrentTranslator()
{
    QCoreApplication::removeTranslator(&m_translator);
}

QString TranslationManager::getTranslationFilePath(const QString& language) const
{
    QDir translationsDir(m_translationsPath);
    
    // Try different naming patterns
    QStringList patterns;
    patterns << QString("app_%1.qm").arg(language);
    patterns << QString("jitsi_%1.qm").arg(language);
    patterns << QString("%1.qm").arg(language);
    
    for (const QString& pattern : patterns) {
        QString filePath = translationsDir.absoluteFilePath(pattern);
        if (QFileInfo::exists(filePath)) {
            return filePath;
        }
    }
    
    return QString();
}

void TranslationManager::loadLanguageNames()
{
    m_languageNames.clear();
    m_languageNames["en"] = "English";
    m_languageNames["zh"] = "中文";
    m_languageNames["zh_CN"] = "简体中文";
    m_languageNames["zh_TW"] = "繁體中文";
    m_languageNames["ja"] = "日本語";
    m_languageNames["ko"] = "한국어";
    m_languageNames["fr"] = "Français";
    m_languageNames["de"] = "Deutsch";
    m_languageNames["es"] = "Español";
    m_languageNames["it"] = "Italiano";
    m_languageNames["pt"] = "Português";
    m_languageNames["ru"] = "Русский";
    m_languageNames["ar"] = "العربية";
}

void TranslationManager::setupDefaultLanguage()
{
    m_currentLanguage = "en";
    detectAvailableTranslations();
}
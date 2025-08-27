#include "TranslationManager.h"
#include "JitsiConstants.h"

#include <QApplication>
#include <QDir>
#include <QLibraryInfo>
#include <QLocale>
#include <QDebug>

TranslationManager::TranslationManager(QObject* parent)
    : QObject(parent)
    , m_appTranslator(nullptr)
    , m_qtTranslator(nullptr)
    , m_currentLanguage("en")
{
    initializeAvailableLanguages();
    qDebug() << "TranslationManager created";
}

TranslationManager::~TranslationManager()
{
    unloadCurrentTranslation();
}

void TranslationManager::initialize()
{
    // 检测系统语言
    QString systemLanguage = detectSystemLanguage();
    qDebug() << "System language detected:" << systemLanguage;
    
    // 设置默认语言
    setLanguage(systemLanguage);
}

QStringList TranslationManager::availableLanguages() const
{
    return m_availableLanguages;
}

QString TranslationManager::languageDisplayName(const QString& languageCode) const
{
    return m_languageNames.value(languageCode, languageCode);
}

QString TranslationManager::currentLanguage() const
{
    return m_currentLanguage;
}

bool TranslationManager::setLanguage(const QString& languageCode)
{
    QString targetLanguage = languageCode;
    
    // 处理自动检测
    if (targetLanguage == "auto") {
        targetLanguage = detectSystemLanguage();
    }
    
    // 检查语言是否可用
    if (!m_availableLanguages.contains(targetLanguage)) {
        qWarning() << "Language not available:" << targetLanguage << "falling back to English";
        targetLanguage = "en";
    }
    
    if (m_currentLanguage == targetLanguage) {
        qDebug() << "Language already set to:" << targetLanguage;
        return true;
    }
    
    // 卸载当前翻译
    unloadCurrentTranslation();
    
    // 加载新翻译
    bool success = loadTranslation(targetLanguage);
    if (success) {
        m_currentLanguage = targetLanguage;
        emit languageChanged(targetLanguage);
        qDebug() << "Language changed to:" << targetLanguage;
    } else {
        qWarning() << "Failed to load translation for:" << targetLanguage;
        // 尝试加载英语作为后备
        if (targetLanguage != "en") {
            return setLanguage("en");
        }
    }
    
    return success;
}

QString TranslationManager::detectSystemLanguage() const
{
    QLocale systemLocale = QLocale::system();
    QString languageCode = systemLocale.name();
    
    // 提取语言部分（例如从 "zh_CN" 提取 "zh_CN"）
    if (m_availableLanguages.contains(languageCode)) {
        return languageCode;
    }
    
    // 尝试只使用语言代码（例如从 "zh_CN" 提取 "zh"）
    QString shortCode = languageCode.split('_').first();
    if (m_availableLanguages.contains(shortCode)) {
        return shortCode;
    }
    
    // 默认返回英语
    return "en";
}

QString TranslationManager::translate(const QString& key, const QString& defaultText) const
{
    QString translated = QApplication::translate("", key.toUtf8().constData());
    
    // 如果翻译失败，返回默认文本或键名
    if (translated == key) {
        return defaultText.isEmpty() ? key : defaultText;
    }
    
    return translated;
}

void TranslationManager::onConfigLanguageChanged(const QString& language)
{
    setLanguage(language);
}

bool TranslationManager::loadTranslation(const QString& languageCode)
{
    // 创建翻译器
    m_appTranslator = new QTranslator(this);
    m_qtTranslator = new QTranslator(this);
    
    bool appLoaded = false;
    bool qtLoaded = false;
    
    // 加载应用程序翻译
    QString appTranslationFile = getTranslationFilePath(languageCode);
    if (QFile::exists(appTranslationFile)) {
        appLoaded = m_appTranslator->load(appTranslationFile);
        if (appLoaded) {
            QApplication::installTranslator(m_appTranslator);
            qDebug() << "App translation loaded:" << appTranslationFile;
        } else {
            qWarning() << "Failed to load app translation:" << appTranslationFile;
        }
    } else {
        qDebug() << "App translation file not found:" << appTranslationFile;
        // 对于英语，这是正常的，因为源代码就是英语
        if (languageCode == "en") {
            appLoaded = true;
        }
    }
    
    // 加载Qt翻译
    QString qtTranslationFile = QString("qt_%1").arg(languageCode);
    QString qtTranslationPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    
    qtLoaded = m_qtTranslator->load(qtTranslationFile, qtTranslationPath);
    if (qtLoaded) {
        QApplication::installTranslator(m_qtTranslator);
        qDebug() << "Qt translation loaded:" << qtTranslationFile;
    } else {
        qDebug() << "Qt translation not found or failed to load:" << qtTranslationFile;
        // Qt翻译不是必需的
        qtLoaded = true;
    }
    
    return appLoaded && qtLoaded;
}

void TranslationManager::unloadCurrentTranslation()
{
    if (m_appTranslator) {
        QApplication::removeTranslator(m_appTranslator);
        m_appTranslator->deleteLater();
        m_appTranslator = nullptr;
    }
    
    if (m_qtTranslator) {
        QApplication::removeTranslator(m_qtTranslator);
        m_qtTranslator->deleteLater();
        m_qtTranslator = nullptr;
    }
}

QString TranslationManager::getTranslationFilePath(const QString& languageCode) const
{
    return QString(":/translations/jitsi_%1.qm").arg(languageCode);
}

void TranslationManager::initializeAvailableLanguages()
{
    // 初始化可用语言列表
    m_availableLanguages << "en" << "zh_CN" << "zh_TW" << "ja" << "ko" 
                        << "fr" << "de" << "es" << "ru";
    
    // 初始化语言显示名称
    m_languageNames["en"] = "English";
    m_languageNames["zh_CN"] = "简体中文";
    m_languageNames["zh_TW"] = "繁體中文";
    m_languageNames["ja"] = "日本語";
    m_languageNames["ko"] = "한국어";
    m_languageNames["fr"] = "Français";
    m_languageNames["de"] = "Deutsch";
    m_languageNames["es"] = "Español";
    m_languageNames["ru"] = "Русский";
    
    qDebug() << "Available languages:" << m_availableLanguages;
}
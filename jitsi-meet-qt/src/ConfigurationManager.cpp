#include "../include/ConfigurationManager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>

ConfigurationManager* ConfigurationManager::s_instance = nullptr;

ConfigurationManager::ConfigurationManager(QObject *parent)
    : QObject(parent)
    , m_settings(nullptr)
    , m_initialized(false)
{
    s_instance = this;
    
    // Initialize settings
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir().mkpath(configPath);
    
    QString settingsFile = configPath + "/config.ini";
    m_settings = new QSettings(settingsFile, QSettings::IniFormat, this);
    
    qDebug() << "ConfigurationManager initialized with settings file:" << settingsFile;
}

ConfigurationManager::~ConfigurationManager()
{
    if (m_settings) {
        m_settings->sync();
    }
    s_instance = nullptr;
}

ConfigurationManager* ConfigurationManager::instance()
{
    return s_instance;
}

bool ConfigurationManager::initialize()
{
    if (m_initialized) {
        return true;
    }
    
    qDebug() << "Initializing ConfigurationManager...";
    
    // Load default values if not set
    if (!m_settings->contains("ui/language")) {
        m_settings->setValue("ui/language", "en");
    }
    
    if (!m_settings->contains("ui/darkMode")) {
        m_settings->setValue("ui/darkMode", false);
    }
    
    if (!m_settings->contains("audio/inputDevice")) {
        m_settings->setValue("audio/inputDevice", "default");
    }
    
    if (!m_settings->contains("audio/outputDevice")) {
        m_settings->setValue("audio/outputDevice", "default");
    }
    
    if (!m_settings->contains("video/camera")) {
        m_settings->setValue("video/camera", "default");
    }
    
    m_settings->sync();
    m_initialized = true;
    
    qDebug() << "ConfigurationManager initialized successfully";
    return true;
}

void ConfigurationManager::shutdown()
{
    if (m_settings) {
        m_settings->sync();
    }
    m_initialized = false;
}

QVariant ConfigurationManager::getValue(const QString& key, const QVariant& defaultValue) const
{
    if (!m_settings) {
        return defaultValue;
    }
    
    return m_settings->value(key, defaultValue);
}

void ConfigurationManager::setValue(const QString& key, const QVariant& value)
{
    if (!m_settings) {
        return;
    }
    
    QVariant oldValue = m_settings->value(key);
    if (oldValue != value) {
        m_settings->setValue(key, value);
        m_settings->sync();
        
        emit configurationChanged(key, value);
        
        // Emit specific signals for common settings
        if (key == "ui/language") {
            emit languageChanged(value.toString());
        } else if (key == "ui/darkMode") {
            emit darkModeChanged(value.toBool());
        }
    }
}

bool ConfigurationManager::hasKey(const QString& key) const
{
    if (!m_settings) {
        return false;
    }
    
    return m_settings->contains(key);
}

void ConfigurationManager::removeKey(const QString& key)
{
    if (!m_settings) {
        return;
    }
    
    if (m_settings->contains(key)) {
        m_settings->remove(key);
        m_settings->sync();
        emit configurationChanged(key, QVariant());
    }
}

QStringList ConfigurationManager::getAllKeys() const
{
    if (!m_settings) {
        return QStringList();
    }
    
    return m_settings->allKeys();
}

void ConfigurationManager::resetToDefaults()
{
    if (!m_settings) {
        return;
    }
    
    qDebug() << "Resetting configuration to defaults...";
    
    m_settings->clear();
    
    // Set default values
    m_settings->setValue("ui/language", "en");
    m_settings->setValue("ui/darkMode", false);
    m_settings->setValue("audio/inputDevice", "default");
    m_settings->setValue("audio/outputDevice", "default");
    m_settings->setValue("video/camera", "default");
    
    m_settings->sync();
    
    // Emit signals for all changed values
    emit languageChanged("en");
    emit darkModeChanged(false);
    emit configurationChanged("", QVariant()); // Signal that all config was reset
}

bool ConfigurationManager::isDarkMode() const
{
    return getValue("ui/darkMode", false).toBool();
}
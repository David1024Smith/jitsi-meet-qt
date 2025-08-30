#include "UIModule.h"
#include "UIManager.h"
#include "ThemeFactory.h"
#include "config/UIConfig.h"
#include <QDebug>

UIModule* UIModule::s_instance = nullptr;

UIModule::UIModule(QObject *parent)
    : QObject(parent)
    , m_status(NotInitialized)
    , m_uiManager(nullptr)
    , m_themeFactory(nullptr)
    , m_config(nullptr)
{
    s_instance = this;
}

UIModule::~UIModule()
{
    shutdown();
    s_instance = nullptr;
}

bool UIModule::initialize()
{
    if (m_status == Ready) {
        return true;
    }

    m_status = Initializing;
    emit statusChanged(m_status);

    try {
        // 创建配置对象
        m_config = std::make_unique<UIConfig>(this);
        m_config->loadDefaults();

        // 初始化组件
        initializeComponents();

        // 建立连接
        setupConnections();

        m_status = Ready;
        emit statusChanged(m_status);
        emit initialized();

        qDebug() << "UI Module initialized successfully";
        return true;

    } catch (const std::exception& e) {
        m_status = Error;
        emit statusChanged(m_status);
        emit errorOccurred(QString("Failed to initialize UI module: %1").arg(e.what()));
        return false;
    }
}

void UIModule::shutdown()
{
    if (m_status == NotInitialized) {
        return;
    }

    emit shutdownRequested();
    
    cleanupComponents();
    
    m_status = NotInitialized;
    emit statusChanged(m_status);
    
    qDebug() << "UI Module shutdown completed";
}

bool UIModule::isInitialized() const
{
    return m_status == Ready;
}

UIModule::ModuleStatus UIModule::status() const
{
    return m_status;
}

QString UIModule::moduleName() const
{
    return "UI Module";
}

QString UIModule::moduleVersion() const
{
    return "1.0.0";
}

QStringList UIModule::dependencies() const
{
    return QStringList() << "Utils Module";
}

UIModule* UIModule::instance()
{
    if (!s_instance) {
        s_instance = new UIModule();
    }
    return s_instance;
}

UIManager* UIModule::uiManager() const
{
    return m_uiManager.get();
}

ThemeFactory* UIModule::themeFactory() const
{
    return m_themeFactory.get();
}

void UIModule::initializeComponents()
{
    // 创建主题工厂
    m_themeFactory = std::make_unique<ThemeFactory>(this);
    m_themeFactory->registerBuiltinThemes();

    // 创建UI管理器
    m_uiManager = std::make_unique<UIManager>(this);
    if (!m_uiManager->initialize()) {
        throw std::runtime_error("Failed to initialize UI Manager");
    }
}

void UIModule::cleanupComponents()
{
    if (m_uiManager) {
        m_uiManager->shutdown();
        m_uiManager.reset();
    }

    if (m_themeFactory) {
        m_themeFactory.reset();
    }

    if (m_config) {
        m_config.reset();
    }
}

void UIModule::setupConnections()
{
    if (m_uiManager) {
        connect(m_uiManager.get(), &UIManager::errorOccurred,
                this, &UIModule::onManagerError);
        connect(m_uiManager.get(), &UIManager::themeChanged,
                this, &UIModule::onThemeChanged);
    }
}

void UIModule::onManagerError(const QString& error)
{
    emit errorOccurred(QString("UI Manager error: %1").arg(error));
}

void UIModule::onThemeChanged(const QString& themeName)
{
    qDebug() << "Theme changed to:" << themeName;
    emit configurationChanged();
}

bool UIModule::loadConfiguration(const QVariantMap& config)
{
    if (m_config) {
        return m_config->fromVariantMap(config);
    }
    return false;
}

QVariantMap UIModule::saveConfiguration() const
{
    if (m_config) {
        return m_config->toVariantMap();
    }
    return QVariantMap();
}

bool UIModule::validateConfiguration(const QVariantMap& config) const
{
    // 基本验证
    if (config.isEmpty()) {
        return false;
    }

    // 检查必需字段
    QStringList requiredFields = {"theme", "layout"};
    for (const QString& field : requiredFields) {
        if (!config.contains(field)) {
            return false;
        }
    }

    return true;
}
#include "UIManager.h"
#include "interfaces/IThemeManager.h"
#include "../interfaces/ILayoutManager.h"
#include "../include/ThemeManager.h"
#include "ThemeFactory.h"
#include "LayoutManager.h"
#include "../config/UIConfig.h"
#include "../themes/BaseTheme.h"
#include <QApplication>
#include <QWidget>
#include <QFile>
#include <QTextStream>
#include <QDebug>

UIManager* UIManager::s_instance = nullptr;

UIManager::UIManager(QObject *parent)
    : QObject(parent)
    , m_status(NotInitialized)
    , m_currentTheme("default")
    , m_currentLayout("main")
    , m_mainWindow(nullptr)
{
}

UIManager::~UIManager()
{
    shutdown();
}

bool UIManager::initialize()
{
    if (m_status == Running) {
        return true;
    }

    m_status = Initializing;

    try {
        // 初始化配置
        m_config = std::make_unique<UIConfig>(this);
        m_config->loadDefaults();

        // 设置管理器
        setupManagers();

        // 建立连接
        setupConnections();

        // 应用默认配置
        applyDefaultConfiguration();

        m_status = Running;
        qDebug() << "UIManager initialized successfully";
        return true;

    } catch (const std::exception& e) {
        m_status = Error;
        emit errorOccurred(QString("Failed to initialize UIManager: %1").arg(e.what()));
        return false;
    }
}

void UIManager::shutdown()
{
    if (m_status == NotInitialized) {
        return;
    }

    // 保存配置
    saveConfiguration();

    // 清理管理器
    if (m_themeManager) {
        m_themeManager->shutdown();
        m_themeManager.reset();
    }

    if (m_layoutManager) {
        m_layoutManager.reset();
    }

    // 清理注册的组件
    m_registeredWidgets.clear();
    m_mainWindow = nullptr;

    m_status = NotInitialized;
    qDebug() << "UIManager shutdown completed";
}

UIManager::ManagerStatus UIManager::status() const
{
    return m_status;
}

bool UIManager::setTheme(const QString& themeName)
{
    if (!validateThemeName(themeName)) {
        emit errorOccurred(QString("Invalid theme name: %1").arg(themeName));
        return false;
    }

    if (m_themeManager && m_themeManager->applyTheme(themeName)) {
        QString oldTheme = m_currentTheme;
        m_currentTheme = themeName;
        emit themeChanged(themeName);
        qDebug() << "Theme changed from" << oldTheme << "to" << themeName;
        return true;
    }

    return false;
}

QString UIManager::currentTheme() const
{
    return m_currentTheme;
}

QStringList UIManager::availableThemes() const
{
    if (m_themeManager) {
        return m_themeManager->availableThemes();
    }
    return QStringList() << "default" << "dark" << "light";
}

bool UIManager::applyTheme(std::shared_ptr<BaseTheme> theme)
{
    if (!theme) {
        emit errorOccurred("Cannot apply null theme");
        return false;
    }

    if (m_themeManager && m_themeManager->applyTheme(theme)) {
        m_currentTheme = theme->name();
        emit themeChanged(m_currentTheme);
        return true;
    }

    return false;
}

bool UIManager::setLayout(const QString& layoutName)
{
    if (!validateLayoutName(layoutName)) {
        emit errorOccurred(QString("Invalid layout name: %1").arg(layoutName));
        return false;
    }

    if (m_layoutManager && m_layoutManager->setLayout(layoutName)) {
        QString oldLayout = m_currentLayout;
        m_currentLayout = layoutName;
        emit layoutChanged(layoutName);
        qDebug() << "Layout changed from" << oldLayout << "to" << layoutName;
        return true;
    }

    return false;
}

QString UIManager::currentLayout() const
{
    return m_currentLayout;
}

QStringList UIManager::availableLayouts() const
{
    if (m_layoutManager) {
        return m_layoutManager->availableLayouts();
    }
    return QStringList() << "main" << "conference" << "settings";
}

bool UIManager::updateLayout()
{
    if (m_layoutManager) {
        return m_layoutManager->updateLayout();
    }
    return false;
}

bool UIManager::setMainWindow(QWidget* window)
{
    if (!window) {
        emit errorOccurred("Cannot set null main window");
        return false;
    }

    m_mainWindow = window;
    qDebug() << "Main window set successfully";
    return true;
}

QWidget* UIManager::mainWindow() const
{
    return m_mainWindow;
}

bool UIManager::showWindow(const QString& windowName)
{
    QWidget* widget = getWidget(windowName);
    if (widget) {
        widget->show();
        emit windowShown(windowName);
        return true;
    }

    emit errorOccurred(QString("Window not found: %1").arg(windowName));
    return false;
}

bool UIManager::hideWindow(const QString& windowName)
{
    QWidget* widget = getWidget(windowName);
    if (widget) {
        widget->hide();
        emit windowHidden(windowName);
        return true;
    }

    emit errorOccurred(QString("Window not found: %1").arg(windowName));
    return false;
}

bool UIManager::applyConfiguration(const UIConfig& config)
{
    try {
        *m_config = config;
        
        // 应用主题配置
        if (!config.theme().isEmpty()) {
            setTheme(config.theme());
        }

        // 应用布局配置
        if (!config.layout().isEmpty()) {
            setLayout(config.layout());
        }

        // 应用样式表
        if (!config.customStyleSheet().isEmpty()) {
            applyStyleSheet(config.customStyleSheet());
        }

        emit configurationChanged();
        return true;

    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to apply configuration: %1").arg(e.what()));
        return false;
    }
}

UIConfig UIManager::currentConfiguration() const
{
    if (m_config) {
        return *m_config;
    }
    return UIConfig();
}

bool UIManager::saveConfiguration()
{
    if (m_config) {
        // 保存配置到文件
        QVariantMap configMap = m_config->toVariantMap();
        // 这里需要实现配置保存逻辑
        return true;
    }
    return false;
}

bool UIManager::loadConfiguration()
{
    if (m_config) {
        // 从文件加载配置
        // 这里需要实现配置加载逻辑
        return true;
    }
    return false;
}

bool UIManager::applyStyleSheet(const QString& styleSheet)
{
    if (QApplication::instance()) {
        qApp->setStyleSheet(styleSheet);
        m_currentStyleSheet = styleSheet;
        emit styleSheetChanged();
        return true;
    }
    return false;
}

QString UIManager::currentStyleSheet() const
{
    return m_currentStyleSheet;
}

bool UIManager::loadStyleFromFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QString("Cannot open style file: %1").arg(filePath));
        return false;
    }

    QTextStream in(&file);
    QString styleSheet = in.readAll();
    return applyStyleSheet(styleSheet);
}

bool UIManager::registerWidget(const QString& name, QWidget* widget)
{
    if (!widget) {
        emit errorOccurred("Cannot register null widget");
        return false;
    }

    if (m_registeredWidgets.contains(name)) {
        emit errorOccurred(QString("Widget already registered: %1").arg(name));
        return false;
    }

    m_registeredWidgets[name] = widget;
    emit widgetRegistered(name);
    qDebug() << "Widget registered:" << name;
    return true;
}

QWidget* UIManager::getWidget(const QString& name) const
{
    return m_registeredWidgets.value(name, nullptr);
}

bool UIManager::unregisterWidget(const QString& name)
{
    if (m_registeredWidgets.remove(name) > 0) {
        emit widgetUnregistered(name);
        qDebug() << "Widget unregistered:" << name;
        return true;
    }
    return false;
}

QStringList UIManager::registeredWidgets() const
{
    return m_registeredWidgets.keys();
}

UIManager* UIManager::instance()
{
    if (!s_instance) {
        s_instance = new UIManager();
    }
    return s_instance;
}

void UIManager::onThemeManagerError(const QString& error)
{
    emit errorOccurred(QString("Theme Manager error: %1").arg(error));
}

void UIManager::onLayoutManagerError(const QString& error)
{
    emit errorOccurred(QString("Layout Manager error: %1").arg(error));
}

void UIManager::onThemeChanged(const QString& oldTheme, const QString& newTheme)
{
    // 适配ThemeManager的双参数信号到UIManager的单参数信号
    m_currentTheme = newTheme;
    emit themeChanged(newTheme);
}

void UIManager::setupManagers()
{
    // 创建主题管理器
    m_themeManager = std::make_unique<ThemeManager>(this);
    if (!m_themeManager->initialize()) {
        throw std::runtime_error("Failed to initialize Theme Manager");
    }
    
    // 创建布局管理器
    m_layoutManager = std::make_unique<LayoutManager>(this);
    if (!m_layoutManager->initialize()) {
        throw std::runtime_error("Failed to initialize Layout Manager");
    }
}

void UIManager::setupConnections()
{
    if (m_themeManager) {
        // 使用具体的ThemeManager类型进行连接
        ThemeManager* themeManager = static_cast<ThemeManager*>(m_themeManager.get());
        connect(themeManager, &ThemeManager::errorOccurred,
                this, &UIManager::onThemeManagerError);
        connect(themeManager, QOverload<const QString&, const QString&>::of(&ThemeManager::themeChanged),
                this, &UIManager::onThemeChanged);
    }

    if (m_layoutManager) {
        // 使用具体的LayoutManager类型进行连接
        LayoutManager* layoutManager = static_cast<LayoutManager*>(m_layoutManager.get());
        connect(layoutManager, &LayoutManager::errorOccurred,
                this, &UIManager::onLayoutManagerError);
        connect(layoutManager, &LayoutManager::layoutChanged,
                this, &UIManager::layoutChanged);
    }
}

void UIManager::applyDefaultConfiguration()
{
    if (m_config) {
        // 设置默认主题
        m_config->setTheme("default");
        
        // 设置默认布局
        m_config->setLayout("main");
        
        // 应用配置
        applyConfiguration(*m_config);
    }
}

bool UIManager::validateThemeName(const QString& themeName) const
{
    if (themeName.isEmpty()) {
        return false;
    }

    QStringList validThemes = availableThemes();
    return validThemes.contains(themeName);
}

bool UIManager::validateLayoutName(const QString& layoutName) const
{
    if (layoutName.isEmpty()) {
        return false;
    }

    QStringList validLayouts = availableLayouts();
    return validLayouts.contains(layoutName);
}
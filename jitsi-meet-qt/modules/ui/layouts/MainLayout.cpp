#include "MainLayout.h"
#include "../widgets/ToolBar.h"
#include "../widgets/StatusBar.h"
#include "../themes/BaseTheme.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QWidget>
#include <QApplication>

MainLayout::MainLayout(QWidget *parent)
    : BaseLayout(parent)
    , m_mainWindow(nullptr)
    , m_mainSplitter(nullptr)
    , m_contentSplitter(nullptr)
    , m_toolBar(nullptr)
    , m_statusBar(nullptr)
    , m_centralWidget(nullptr)
    , m_sideBar(nullptr)
    , m_headerWidget(nullptr)
    , m_footerWidget(nullptr)
    , m_toolBarVisible(true)
    , m_statusBarVisible(true)
    , m_sideBarVisible(false)
    , m_sideBarWidth(250)
    , m_responsive(true)
{
    // 初始化区域可见性
    m_areaVisibility[ToolBarArea] = true;
    m_areaVisibility[StatusBarArea] = true;
    m_areaVisibility[CentralArea] = true;
    m_areaVisibility[SideBarArea] = false;
    m_areaVisibility[HeaderArea] = false;
    m_areaVisibility[FooterArea] = false;
}

MainLayout::~MainLayout()
{
    cleanup();
}

QString MainLayout::layoutName() const
{
    return "main";
}

QString MainLayout::layoutDisplayName() const
{
    return tr("Main Layout");
}

QString MainLayout::layoutDescription() const
{
    return tr("Main application layout with toolbar, status bar, central area and optional sidebar.");
}

bool MainLayout::initialize()
{
    if (isInitialized()) {
        return true;
    }

    try {
        setupLayout();
        createAreas();
        connectSignals();
        
        setInitialized(true);
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to initialize main layout: %1").arg(e.what()));
        return false;
    }
}

bool MainLayout::apply(QWidget* widget)
{
    if (!widget) {
        emit errorOccurred("Cannot apply layout to null widget");
        return false;
    }

    if (!initialize()) {
        return false;
    }

    // 确保widget是QMainWindow类型
    m_mainWindow = qobject_cast<QMainWindow*>(widget);
    if (!m_mainWindow) {
        emit errorOccurred("MainLayout can only be applied to QMainWindow");
        return false;
    }

    try {
        arrangeAreas();
        updateLayout();
        
        setApplied(true);
        emit layoutApplied();
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to apply main layout: %1").arg(e.what()));
        return false;
    }
}

void MainLayout::cleanup()
{
    if (!isInitialized()) {
        return;
    }

    // 清理组件
    m_areaWidgets.clear();
    
    if (m_mainSplitter) {
        m_mainSplitter->deleteLater();
        m_mainSplitter = nullptr;
    }
    
    if (m_contentSplitter) {
        m_contentSplitter->deleteLater();
        m_contentSplitter = nullptr;
    }

    m_mainWindow = nullptr;
    m_toolBar = nullptr;
    m_statusBar = nullptr;
    m_centralWidget = nullptr;
    m_sideBar = nullptr;
    m_headerWidget = nullptr;
    m_footerWidget = nullptr;

    setInitialized(false);
    setApplied(false);
    emit layoutCleanedUp();
}

// 区域管理实现
bool MainLayout::setAreaWidget(LayoutArea area, QWidget* widget)
{
    if (!widget) {
        return removeAreaWidget(area);
    }

    QWidget* oldWidget = m_areaWidgets.value(area, nullptr);
    if (oldWidget == widget) {
        return true;
    }

    m_areaWidgets[area] = widget;

    // 根据区域类型设置特定组件
    switch (area) {
    case ToolBarArea:
        m_toolBar = qobject_cast<ToolBar*>(widget);
        break;
    case StatusBarArea:
        m_statusBar = qobject_cast<StatusBar*>(widget);
        break;
    case CentralArea:
        m_centralWidget = widget;
        break;
    case SideBarArea:
        m_sideBar = widget;
        break;
    case HeaderArea:
        m_headerWidget = widget;
        break;
    case FooterArea:
        m_footerWidget = widget;
        break;
    }

    if (isApplied()) {
        updateLayout();
    }

    emit areaWidgetChanged(area, widget);
    return true;
}

QWidget* MainLayout::getAreaWidget(LayoutArea area) const
{
    return m_areaWidgets.value(area, nullptr);
}

bool MainLayout::removeAreaWidget(LayoutArea area)
{
    if (!m_areaWidgets.contains(area)) {
        return false;
    }

    QWidget* widget = m_areaWidgets.take(area);

    // 清除特定组件引用
    switch (area) {
    case ToolBarArea:
        m_toolBar = nullptr;
        break;
    case StatusBarArea:
        m_statusBar = nullptr;
        break;
    case CentralArea:
        m_centralWidget = nullptr;
        break;
    case SideBarArea:
        m_sideBar = nullptr;
        break;
    case HeaderArea:
        m_headerWidget = nullptr;
        break;
    case FooterArea:
        m_footerWidget = nullptr;
        break;
    }

    if (isApplied()) {
        updateLayout();
    }

    emit areaWidgetChanged(area, nullptr);
    return true;
}

bool MainLayout::isAreaVisible(LayoutArea area) const
{
    return m_areaVisibility.value(area, false);
}

void MainLayout::setAreaVisible(LayoutArea area, bool visible)
{
    if (m_areaVisibility.value(area, false) == visible) {
        return;
    }

    m_areaVisibility[area] = visible;

    // 更新特定区域的可见性
    switch (area) {
    case ToolBarArea:
        setToolBarVisible(visible);
        break;
    case StatusBarArea:
        setStatusBarVisible(visible);
        break;
    case SideBarArea:
        setSideBarVisible(visible);
        break;
    default:
        if (isApplied()) {
            updateAreaVisibility();
        }
        break;
    }
}

// 工具栏管理实现
bool MainLayout::isToolBarVisible() const
{
    return m_toolBarVisible;
}

void MainLayout::setToolBarVisible(bool visible)
{
    if (m_toolBarVisible == visible) {
        return;
    }

    m_toolBarVisible = visible;
    m_areaVisibility[ToolBarArea] = visible;

    if (m_toolBar) {
        m_toolBar->setVisible(visible);
    }

    if (isApplied()) {
        updateLayout();
    }

    emit toolBarVisibleChanged(visible);
}

ToolBar* MainLayout::toolBar() const
{
    return m_toolBar;
}

void MainLayout::setToolBar(ToolBar* toolBar)
{
    setAreaWidget(ToolBarArea, toolBar);
}

// 状态栏管理实现
bool MainLayout::isStatusBarVisible() const
{
    return m_statusBarVisible;
}

void MainLayout::setStatusBarVisible(bool visible)
{
    if (m_statusBarVisible == visible) {
        return;
    }

    m_statusBarVisible = visible;
    m_areaVisibility[StatusBarArea] = visible;

    if (m_statusBar) {
        m_statusBar->setVisible(visible);
    }

    if (isApplied()) {
        updateLayout();
    }

    emit statusBarVisibleChanged(visible);
}

StatusBar* MainLayout::statusBar() const
{
    return m_statusBar;
}

void MainLayout::setStatusBar(StatusBar* statusBar)
{
    setAreaWidget(StatusBarArea, statusBar);
}

// 侧边栏管理实现
bool MainLayout::isSideBarVisible() const
{
    return m_sideBarVisible;
}

void MainLayout::setSideBarVisible(bool visible)
{
    if (m_sideBarVisible == visible) {
        return;
    }

    m_sideBarVisible = visible;
    m_areaVisibility[SideBarArea] = visible;

    if (m_sideBar) {
        m_sideBar->setVisible(visible);
    }

    if (isApplied()) {
        updateSplitterSizes();
    }

    emit sideBarVisibleChanged(visible);
}

int MainLayout::sideBarWidth() const
{
    return m_sideBarWidth;
}

void MainLayout::setSideBarWidth(int width)
{
    if (m_sideBarWidth == width || width < 0) {
        return;
    }

    m_sideBarWidth = width;

    if (isApplied() && m_sideBarVisible) {
        updateSplitterSizes();
    }

    emit sideBarWidthChanged(width);
}

QWidget* MainLayout::sideBar() const
{
    return m_sideBar;
}

void MainLayout::setSideBar(QWidget* sideBar)
{
    setAreaWidget(SideBarArea, sideBar);
}

// 中央区域管理实现
QWidget* MainLayout::centralWidget() const
{
    return m_centralWidget;
}

void MainLayout::setCentralWidget(QWidget* widget)
{
    setAreaWidget(CentralArea, widget);
    emit centralWidgetChanged(widget);
}

// 布局配置实现
QVariantMap MainLayout::getLayoutConfiguration() const
{
    QVariantMap config = BaseLayout::getLayoutConfiguration();
    
    config["toolBarVisible"] = m_toolBarVisible;
    config["statusBarVisible"] = m_statusBarVisible;
    config["sideBarVisible"] = m_sideBarVisible;
    config["sideBarWidth"] = m_sideBarWidth;
    config["responsive"] = m_responsive;

    // 保存区域可见性
    QVariantMap areaVisibility;
    for (auto it = m_areaVisibility.constBegin(); it != m_areaVisibility.constEnd(); ++it) {
        areaVisibility[QString::number(static_cast<int>(it.key()))] = it.value();
    }
    config["areaVisibility"] = areaVisibility;

    return config;
}

void MainLayout::setLayoutConfiguration(const QVariantMap& config)
{
    BaseLayout::setLayoutConfiguration(config);

    // 恢复布局设置
    setToolBarVisible(config.value("toolBarVisible", true).toBool());
    setStatusBarVisible(config.value("statusBarVisible", true).toBool());
    setSideBarVisible(config.value("sideBarVisible", false).toBool());
    setSideBarWidth(config.value("sideBarWidth", 250).toInt());
    setResponsive(config.value("responsive", true).toBool());

    // 恢复区域可见性
    QVariantMap areaVisibility = config.value("areaVisibility").toMap();
    for (auto it = areaVisibility.constBegin(); it != areaVisibility.constEnd(); ++it) {
        LayoutArea area = static_cast<LayoutArea>(it.key().toInt());
        m_areaVisibility[area] = it.value().toBool();
    }

    if (isApplied()) {
        updateLayout();
    }
}

// 响应式设计实现
bool MainLayout::adaptToSize(const QSize& size)
{
    if (!m_responsive || !isApplied()) {
        return false;
    }

    // 根据窗口大小调整布局
    bool changed = false;

    // 小屏幕时自动隐藏侧边栏
    if (size.width() < 800 && m_sideBarVisible) {
        setSideBarVisible(false);
        changed = true;
    }

    // 极小屏幕时隐藏工具栏
    if (size.width() < 600 && m_toolBarVisible) {
        setToolBarVisible(false);
        changed = true;
    }

    // 调整侧边栏宽度
    if (m_sideBarVisible && size.width() > 800) {
        int newWidth = qMin(m_sideBarWidth, size.width() / 3);
        if (newWidth != m_sideBarWidth) {
            setSideBarWidth(newWidth);
            changed = true;
        }
    }

    if (changed) {
        emit sizeAdapted(size);
    }

    return changed;
}

bool MainLayout::isResponsive() const
{
    return m_responsive;
}

void MainLayout::setResponsive(bool responsive)
{
    m_responsive = responsive;
}

// BaseLayout虚函数实现
void MainLayout::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    BaseLayout::onThemeChanged(theme);
    
    if (!theme || !isApplied()) {
        return;
    }

    // 应用主题到各个组件
    if (m_toolBar) {
        m_toolBar->setStyleSheet(theme->getToolBarStyleSheet());
    }
    
    if (m_statusBar) {
        m_statusBar->setStyleSheet(theme->getStatusBarStyleSheet());
    }
    
    if (m_mainSplitter) {
        m_mainSplitter->setStyleSheet(QString(
            "QSplitter::handle { background-color: %1; }"
        ).arg(theme->borderColor().name()));
    }
}

void MainLayout::onConfigurationChanged(const QVariantMap& config)
{
    BaseLayout::onConfigurationChanged(config);
    // 配置变化处理已在setLayoutConfiguration中实现
}

QVariantMap MainLayout::getDefaultConfiguration() const
{
    QVariantMap config;
    config["toolBarVisible"] = true;
    config["statusBarVisible"] = true;
    config["sideBarVisible"] = false;
    config["sideBarWidth"] = 250;
    config["responsive"] = true;
    return config;
}

bool MainLayout::validateConfiguration(const QVariantMap& config) const
{
    // 验证配置参数
    if (config.contains("sideBarWidth")) {
        int width = config["sideBarWidth"].toInt();
        if (width < 100 || width > 1000) {
            return false;
        }
    }
    
    return BaseLayout::validateConfiguration(config);
}

// 布局更新实现
void MainLayout::updateLayout()
{
    if (!isApplied() || !m_mainWindow) {
        return;
    }

    arrangeAreas();
    updateAreaVisibility();
    updateSplitterSizes();
    updateGeometry();
}

void MainLayout::updateGeometry()
{
    if (!m_mainWindow) {
        return;
    }

    // 更新几何布局
    if (m_mainSplitter) {
        m_mainSplitter->updateGeometry();
    }
    
    if (m_contentSplitter) {
        m_contentSplitter->updateGeometry();
    }
}

void MainLayout::updateSpacing()
{
    if (!currentTheme()) {
        return;
    }

    int spacing = currentTheme()->spacing();
    
    if (m_mainSplitter) {
        m_mainSplitter->setHandleWidth(spacing / 2);
    }
    
    if (m_contentSplitter) {
        m_contentSplitter->setHandleWidth(spacing / 2);
    }
}

void MainLayout::updateMargins()
{
    // 主布局通常不需要边距
}

// 私有槽函数实现
void MainLayout::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)
    
    // 更新侧边栏宽度
    if (m_contentSplitter && m_sideBarVisible) {
        QList<int> sizes = m_contentSplitter->sizes();
        if (sizes.size() >= 2) {
            m_sideBarWidth = sizes.first();
            emit sideBarWidthChanged(m_sideBarWidth);
        }
    }
}

void MainLayout::onAreaWidgetDestroyed()
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (!widget) {
        return;
    }

    // 查找并移除被销毁的组件
    for (auto it = m_areaWidgets.begin(); it != m_areaWidgets.end(); ++it) {
        if (it.value() == widget) {
            removeAreaWidget(it.key());
            break;
        }
    }
}

// 私有方法实现
void MainLayout::setupLayout()
{
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Vertical);
    m_mainSplitter->setChildrenCollapsible(false);
    
    // 创建内容分割器
    m_contentSplitter = new QSplitter(Qt::Horizontal);
    m_contentSplitter->setChildrenCollapsible(false);
}

void MainLayout::createAreas()
{
    // 区域将在arrangeAreas中创建和排列
}

void MainLayout::arrangeAreas()
{
    if (!m_mainWindow || !m_mainSplitter) {
        return;
    }

    // 清除现有布局
    m_mainSplitter->clear();
    m_contentSplitter->clear();

    // 添加头部区域
    if (m_headerWidget && m_areaVisibility.value(HeaderArea, false)) {
        m_mainSplitter->addWidget(m_headerWidget);
    }

    // 添加工具栏
    if (m_toolBar && m_toolBarVisible) {
        m_mainWindow->addToolBar(m_toolBar);
    }

    // 设置中央区域
    QWidget* centralArea = new QWidget;
    QHBoxLayout* centralLayout = new QHBoxLayout(centralArea);
    centralLayout->setContentsMargins(0, 0, 0, 0);
    centralLayout->setSpacing(0);

    // 添加侧边栏和中央组件到内容分割器
    if (m_sideBar && m_sideBarVisible) {
        m_contentSplitter->addWidget(m_sideBar);
    }
    
    if (m_centralWidget) {
        m_contentSplitter->addWidget(m_centralWidget);
    }

    centralLayout->addWidget(m_contentSplitter);
    m_mainSplitter->addWidget(centralArea);

    // 添加底部区域
    if (m_footerWidget && m_areaVisibility.value(FooterArea, false)) {
        m_mainSplitter->addWidget(m_footerWidget);
    }

    // 设置状态栏
    if (m_statusBar && m_statusBarVisible) {
        m_mainWindow->setStatusBar(m_statusBar);
    }

    // 设置主窗口的中央组件
    m_mainWindow->setCentralWidget(m_mainSplitter);
}

void MainLayout::updateAreaVisibility()
{
    // 更新各区域的可见性
    for (auto it = m_areaWidgets.constBegin(); it != m_areaWidgets.constEnd(); ++it) {
        LayoutArea area = it.key();
        QWidget* widget = it.value();
        bool visible = m_areaVisibility.value(area, false);
        
        if (widget) {
            widget->setVisible(visible);
        }
    }
}

void MainLayout::updateSplitterSizes()
{
    if (!m_contentSplitter || !m_sideBarVisible) {
        return;
    }

    QList<int> sizes;
    int totalWidth = m_contentSplitter->width();
    
    if (totalWidth > 0) {
        sizes << m_sideBarWidth;
        sizes << (totalWidth - m_sideBarWidth);
        m_contentSplitter->setSizes(sizes);
    }
}

void MainLayout::connectSignals()
{
    if (m_mainSplitter) {
        connect(m_mainSplitter, &QSplitter::splitterMoved,
                this, &MainLayout::onSplitterMoved);
    }
    
    if (m_contentSplitter) {
        connect(m_contentSplitter, &QSplitter::splitterMoved,
                this, &MainLayout::onSplitterMoved);
    }
}
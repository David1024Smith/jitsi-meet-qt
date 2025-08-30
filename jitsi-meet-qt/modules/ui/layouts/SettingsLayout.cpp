#include "SettingsLayout.h"
#include "../themes/BaseTheme.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QStackedWidget>
#include <QWidget>
#include <QTreeWidgetItem>
#include <QApplication>

SettingsLayout::SettingsLayout(QWidget *parent)
    : BaseLayout(parent)
    , m_layoutStyle(TreeAndPanelStyle)
    , m_responsive(true)
    , m_searchEnabled(false)
    , m_mainSplitter(nullptr)
    , m_mainLayout(nullptr)
    , m_contentLayout(nullptr)
    , m_categoryTree(nullptr)
    , m_settingsPanel(nullptr)
    , m_buttonArea(nullptr)
    , m_headerWidget(nullptr)
    , m_footerWidget(nullptr)
    , m_searchWidget(nullptr)
    , m_currentPanelIndex(0)
    , m_categoryTreeWidth(200)
    , m_buttonAreaHeight(60)
{
    // 初始化区域可见性
    m_regionVisibility[CategoryTreeRegion] = true;
    m_regionVisibility[SettingsPanelRegion] = true;
    m_regionVisibility[ButtonAreaRegion] = true;
    m_regionVisibility[HeaderRegion] = false;
    m_regionVisibility[FooterRegion] = false;
}

SettingsLayout::~SettingsLayout()
{
    cleanup();
}

QString SettingsLayout::layoutName() const
{
    return "settings";
}

QString SettingsLayout::layoutDisplayName() const
{
    return tr("Settings Layout");
}

QString SettingsLayout::layoutDescription() const
{
    return tr("Specialized layout for settings interface with category tree and settings panels.");
}

bool SettingsLayout::initialize()
{
    if (isInitialized()) {
        return true;
    }

    try {
        setupLayout();
        createRegions();
        connectSignals();
        
        setInitialized(true);
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to initialize settings layout: %1").arg(e.what()));
        return false;
    }
}

bool SettingsLayout::apply(QWidget* widget)
{
    if (!widget) {
        emit errorOccurred("Cannot apply layout to null widget");
        return false;
    }

    if (!initialize()) {
        return false;
    }

    try {
        arrangeRegions();
        updateLayout();
        
        setApplied(true);
        emit layoutApplied();
        return true;
    } catch (const std::exception& e) {
        emit errorOccurred(QString("Failed to apply settings layout: %1").arg(e.what()));
        return false;
    }
}

void SettingsLayout::cleanup()
{
    if (!isInitialized()) {
        return;
    }

    // 清理组件
    m_regionWidgets.clear();
    m_settingsPages.clear();
    m_categoryPages.clear();
    
    if (m_mainSplitter) {
        m_mainSplitter->deleteLater();
        m_mainSplitter = nullptr;
    }

    m_categoryTree = nullptr;
    m_settingsPanel = nullptr;
    m_buttonArea = nullptr;
    m_headerWidget = nullptr;
    m_footerWidget = nullptr;
    m_searchWidget = nullptr;

    setInitialized(false);
    setApplied(false);
    emit layoutCleanedUp();
}

// 布局样式管理
SettingsLayout::LayoutStyle SettingsLayout::layoutStyle() const
{
    return m_layoutStyle;
}

void SettingsLayout::setLayoutStyle(LayoutStyle style)
{
    if (m_layoutStyle == style) {
        return;
    }

    m_layoutStyle = style;
    
    if (isApplied()) {
        updateLayoutStyle();
    }

    emit layoutStyleChanged(style);
}

QStringList SettingsLayout::availableLayoutStyles() const
{
    return QStringList() << "TreeAndPanelStyle" << "TabsStyle" << "WizardStyle" << "CompactStyle";
}

// 区域管理实现
bool SettingsLayout::setRegionWidget(SettingsRegion region, QWidget* widget)
{
    if (!widget) {
        return removeRegionWidget(region);
    }

    QWidget* oldWidget = m_regionWidgets.value(region, nullptr);
    if (oldWidget == widget) {
        return true;
    }

    m_regionWidgets[region] = widget;

    // 根据区域类型设置特定组件
    switch (region) {
    case CategoryTreeRegion:
        m_categoryTree = qobject_cast<QTreeWidget*>(widget);
        break;
    case SettingsPanelRegion:
        m_settingsPanel = qobject_cast<QStackedWidget*>(widget);
        break;
    case ButtonAreaRegion:
        m_buttonArea = widget;
        break;
    case HeaderRegion:
        m_headerWidget = widget;
        break;
    case FooterRegion:
        m_footerWidget = widget;
        break;
    }

    if (isApplied()) {
        updateLayout();
    }

    return true;
}

QWidget* SettingsLayout::getRegionWidget(SettingsRegion region) const
{
    return m_regionWidgets.value(region, nullptr);
}

bool SettingsLayout::removeRegionWidget(SettingsRegion region)
{
    if (!m_regionWidgets.contains(region)) {
        return false;
    }

    QWidget* widget = m_regionWidgets.take(region);

    // 清除特定组件引用
    switch (region) {
    case CategoryTreeRegion:
        m_categoryTree = nullptr;
        break;
    case SettingsPanelRegion:
        m_settingsPanel = nullptr;
        break;
    case ButtonAreaRegion:
        m_buttonArea = nullptr;
        break;
    case HeaderRegion:
        m_headerWidget = nullptr;
        break;
    case FooterRegion:
        m_footerWidget = nullptr;
        break;
    }

    if (isApplied()) {
        updateLayout();
    }

    return true;
}

bool SettingsLayout::isRegionVisible(SettingsRegion region) const
{
    return m_regionVisibility.value(region, false);
}

void SettingsLayout::setRegionVisible(SettingsRegion region, bool visible)
{
    if (m_regionVisibility.value(region, false) == visible) {
        return;
    }

    m_regionVisibility[region] = visible;

    if (isApplied()) {
        updateRegionVisibility();
    }
}

// 分类树管理实现
bool SettingsLayout::isCategoryTreeVisible() const
{
    return isRegionVisible(CategoryTreeRegion);
}

void SettingsLayout::setCategoryTreeVisible(bool visible)
{
    setRegionVisible(CategoryTreeRegion, visible);
    emit categoryTreeVisibleChanged(visible);
}

int SettingsLayout::categoryTreeWidth() const
{
    return m_categoryTreeWidth;
}

void SettingsLayout::setCategoryTreeWidth(int width)
{
    if (m_categoryTreeWidth == width || width < 0) {
        return;
    }

    m_categoryTreeWidth = width;

    if (isApplied() && isCategoryTreeVisible()) {
        updateLayout();
    }

    emit categoryTreeWidthChanged(width);
}

QTreeWidget* SettingsLayout::categoryTree() const
{
    return m_categoryTree;
}

void SettingsLayout::setCategoryTree(QTreeWidget* tree)
{
    setRegionWidget(CategoryTreeRegion, tree);
}

// 设置面板管理实现
QStackedWidget* SettingsLayout::settingsPanel() const
{
    return m_settingsPanel;
}

void SettingsLayout::setSettingsPanel(QStackedWidget* panel)
{
    setRegionWidget(SettingsPanelRegion, panel);
}

int SettingsLayout::currentPanelIndex() const
{
    return m_currentPanelIndex;
}

void SettingsLayout::setCurrentPanelIndex(int index)
{
    if (m_currentPanelIndex == index || !m_settingsPanel) {
        return;
    }

    if (index < 0 || index >= m_settingsPanel->count()) {
        return;
    }

    m_currentPanelIndex = index;
    m_settingsPanel->setCurrentIndex(index);

    emit currentPanelChanged(index);
}

QWidget* SettingsLayout::currentPanel() const
{
    if (!m_settingsPanel) {
        return nullptr;
    }
    
    return m_settingsPanel->currentWidget();
}

void SettingsLayout::setCurrentPanel(QWidget* panel)
{
    if (!m_settingsPanel || !panel) {
        return;
    }

    int index = m_settingsPanel->indexOf(panel);
    if (index >= 0) {
        setCurrentPanelIndex(index);
    }
}// 设置页
面管理实现
bool SettingsLayout::addSettingsPage(const QString& category, const QString& title, QWidget* page)
{
    if (!page || category.isEmpty() || title.isEmpty()) {
        return false;
    }

    // 检查是否已存在
    for (const auto& pageInfo : m_settingsPages) {
        if (pageInfo.category == category && pageInfo.title == title) {
            return false;
        }
    }

    // 添加到堆栈组件
    int index = -1;
    if (m_settingsPanel) {
        index = m_settingsPanel->addWidget(page);
    }

    // 创建页面信息
    SettingsPageInfo pageInfo;
    pageInfo.category = category;
    pageInfo.title = title;
    pageInfo.widget = page;
    pageInfo.index = index;

    m_settingsPages.append(pageInfo);

    // 更新分类映射
    if (!m_categoryPages.contains(category)) {
        m_categoryPages[category] = QStringList();
    }
    m_categoryPages[category].append(title);

    // 更新分类树
    if (isApplied()) {
        updateCategoryTree();
    }

    emit settingsPageAdded(category, title);
    return true;
}

bool SettingsLayout::removeSettingsPage(const QString& category, const QString& title)
{
    // 查找页面
    int pageIndex = -1;
    for (int i = 0; i < m_settingsPages.size(); ++i) {
        const auto& pageInfo = m_settingsPages.at(i);
        if (pageInfo.category == category && pageInfo.title == title) {
            pageIndex = i;
            break;
        }
    }

    if (pageIndex < 0) {
        return false;
    }

    const SettingsPageInfo& pageInfo = m_settingsPages.at(pageIndex);

    // 从堆栈组件中移除
    if (m_settingsPanel && pageInfo.widget) {
        m_settingsPanel->removeWidget(pageInfo.widget);
    }

    // 从列表中移除
    m_settingsPages.removeAt(pageIndex);

    // 更新分类映射
    if (m_categoryPages.contains(category)) {
        m_categoryPages[category].removeAll(title);
        if (m_categoryPages[category].isEmpty()) {
            m_categoryPages.remove(category);
        }
    }

    // 更新分类树
    if (isApplied()) {
        updateCategoryTree();
    }

    emit settingsPageRemoved(category, title);
    return true;
}

QWidget* SettingsLayout::getSettingsPage(const QString& category, const QString& title) const
{
    for (const auto& pageInfo : m_settingsPages) {
        if (pageInfo.category == category && pageInfo.title == title) {
            return pageInfo.widget;
        }
    }
    return nullptr;
}

QStringList SettingsLayout::getSettingsCategories() const
{
    return m_categoryPages.keys();
}

QStringList SettingsLayout::getSettingsPages(const QString& category) const
{
    return m_categoryPages.value(category, QStringList());
}

// 按钮区域管理实现
bool SettingsLayout::isButtonAreaVisible() const
{
    return isRegionVisible(ButtonAreaRegion);
}

void SettingsLayout::setButtonAreaVisible(bool visible)
{
    setRegionVisible(ButtonAreaRegion, visible);
    emit buttonAreaVisibleChanged(visible);
}

int SettingsLayout::buttonAreaHeight() const
{
    return m_buttonAreaHeight;
}

void SettingsLayout::setButtonAreaHeight(int height)
{
    if (m_buttonAreaHeight == height || height < 0) {
        return;
    }

    m_buttonAreaHeight = height;

    if (isApplied() && isButtonAreaVisible()) {
        updateLayout();
    }

    emit buttonAreaHeightChanged(height);
}

QWidget* SettingsLayout::buttonArea() const
{
    return m_buttonArea;
}

void SettingsLayout::setButtonArea(QWidget* area)
{
    setRegionWidget(ButtonAreaRegion, area);
}

// 导航管理实现
bool SettingsLayout::navigateToCategory(const QString& category)
{
    if (!m_categoryPages.contains(category)) {
        return false;
    }

    QStringList pages = m_categoryPages[category];
    if (pages.isEmpty()) {
        return false;
    }

    return navigateToPage(category, pages.first());
}

bool SettingsLayout::navigateToPage(const QString& category, const QString& page)
{
    // 查找页面
    for (const auto& pageInfo : m_settingsPages) {
        if (pageInfo.category == category && pageInfo.title == page) {
            if (pageInfo.index >= 0) {
                setCurrentPanelIndex(pageInfo.index);
                m_currentCategory = category;
                m_currentPage = page;
                
                emit currentCategoryChanged(category);
                emit currentPageChanged(page);
                emit navigationRequested(category, page);
                return true;
            }
        }
    }
    return false;
}

QString SettingsLayout::currentCategory() const
{
    return m_currentCategory;
}

QString SettingsLayout::currentPage() const
{
    return m_currentPage;
}

// 搜索功能实现
void SettingsLayout::setSearchEnabled(bool enabled)
{
    m_searchEnabled = enabled;
    
    if (isApplied()) {
        updateLayout();
    }
}

bool SettingsLayout::isSearchEnabled() const
{
    return m_searchEnabled;
}

void SettingsLayout::setSearchWidget(QWidget* searchWidget)
{
    m_searchWidget = searchWidget;
    
    if (isApplied()) {
        updateLayout();
    }
}

QWidget* SettingsLayout::searchWidget() const
{
    return m_searchWidget;
}

// 布局配置实现
QVariantMap SettingsLayout::getLayoutConfiguration() const
{
    QVariantMap config = BaseLayout::getLayoutConfiguration();
    
    config["layoutStyle"] = static_cast<int>(m_layoutStyle);
    config["categoryTreeWidth"] = m_categoryTreeWidth;
    config["buttonAreaHeight"] = m_buttonAreaHeight;
    config["searchEnabled"] = m_searchEnabled;
    config["responsive"] = m_responsive;
    config["currentCategory"] = m_currentCategory;
    config["currentPage"] = m_currentPage;

    // 保存区域可见性
    QVariantMap regionVisibility;
    for (auto it = m_regionVisibility.constBegin(); it != m_regionVisibility.constEnd(); ++it) {
        regionVisibility[QString::number(static_cast<int>(it.key()))] = it.value();
    }
    config["regionVisibility"] = regionVisibility;

    return config;
}

void SettingsLayout::setLayoutConfiguration(const QVariantMap& config)
{
    BaseLayout::setLayoutConfiguration(config);

    // 恢复布局设置
    setLayoutStyle(static_cast<LayoutStyle>(config.value("layoutStyle", TreeAndPanelStyle).toInt()));
    setCategoryTreeWidth(config.value("categoryTreeWidth", 200).toInt());
    setButtonAreaHeight(config.value("buttonAreaHeight", 60).toInt());
    setSearchEnabled(config.value("searchEnabled", false).toBool());
    setResponsive(config.value("responsive", true).toBool());

    m_currentCategory = config.value("currentCategory").toString();
    m_currentPage = config.value("currentPage").toString();

    // 恢复区域可见性
    QVariantMap regionVisibility = config.value("regionVisibility").toMap();
    for (auto it = regionVisibility.constBegin(); it != regionVisibility.constEnd(); ++it) {
        SettingsRegion region = static_cast<SettingsRegion>(it.key().toInt());
        m_regionVisibility[region] = it.value().toBool();
    }

    if (isApplied()) {
        updateLayout();
    }
}

// 响应式设计实现
bool SettingsLayout::adaptToSize(const QSize& size)
{
    if (!m_responsive || !isApplied()) {
        return false;
    }

    bool changed = false;

    // 小屏幕时隐藏分类树，切换到紧凑模式
    if (size.width() < 800) {
        if (m_layoutStyle != CompactStyle) {
            setLayoutStyle(CompactStyle);
            changed = true;
        }
        
        if (isCategoryTreeVisible()) {
            setCategoryTreeVisible(false);
            changed = true;
        }
    } else {
        // 大屏幕时恢复树形布局
        if (m_layoutStyle == CompactStyle) {
            setLayoutStyle(TreeAndPanelStyle);
            changed = true;
        }
        
        if (!isCategoryTreeVisible()) {
            setCategoryTreeVisible(true);
            changed = true;
        }
    }

    // 调整分类树宽度
    if (isCategoryTreeVisible() && size.width() > 800) {
        int newWidth = qMin(m_categoryTreeWidth, size.width() / 4);
        if (newWidth != m_categoryTreeWidth) {
            setCategoryTreeWidth(newWidth);
            changed = true;
        }
    }

    if (changed) {
        emit sizeAdapted(size);
    }

    return changed;
}

bool SettingsLayout::isResponsive() const
{
    return m_responsive;
}

void SettingsLayout::setResponsive(bool responsive)
{
    m_responsive = responsive;
}/
/ BaseLayout虚函数实现
void SettingsLayout::onThemeChanged(std::shared_ptr<BaseTheme> theme)
{
    BaseLayout::onThemeChanged(theme);
    
    if (!theme || !isApplied()) {
        return;
    }

    // 应用主题到各个组件
    if (m_mainSplitter) {
        m_mainSplitter->setStyleSheet(QString(
            "QSplitter::handle { background-color: %1; }"
        ).arg(theme->borderColor().name()));
    }
    
    if (m_categoryTree) {
        m_categoryTree->setStyleSheet(QString(
            "QTreeWidget { background-color: %1; color: %2; border: 1px solid %3; }"
            "QTreeWidget::item:selected { background-color: %4; }"
        ).arg(theme->backgroundColor().name())
         .arg(theme->textColor().name())
         .arg(theme->borderColor().name())
         .arg(theme->accentColor().name()));
    }
}

void SettingsLayout::onConfigurationChanged(const QVariantMap& config)
{
    BaseLayout::onConfigurationChanged(config);
    // 配置变化处理已在setLayoutConfiguration中实现
}

QVariantMap SettingsLayout::getDefaultConfiguration() const
{
    QVariantMap config;
    config["layoutStyle"] = static_cast<int>(TreeAndPanelStyle);
    config["categoryTreeWidth"] = 200;
    config["buttonAreaHeight"] = 60;
    config["searchEnabled"] = false;
    config["responsive"] = true;
    return config;
}

bool SettingsLayout::validateConfiguration(const QVariantMap& config) const
{
    // 验证配置参数
    if (config.contains("categoryTreeWidth")) {
        int width = config["categoryTreeWidth"].toInt();
        if (width < 100 || width > 500) {
            return false;
        }
    }
    
    if (config.contains("buttonAreaHeight")) {
        int height = config["buttonAreaHeight"].toInt();
        if (height < 30 || height > 150) {
            return false;
        }
    }
    
    return BaseLayout::validateConfiguration(config);
}

// 布局更新实现
void SettingsLayout::updateLayout()
{
    if (!isApplied()) {
        return;
    }

    arrangeRegions();
    updateRegionVisibility();
    updateLayoutStyle();
    updateCategoryTree();
    updateSettingsPanel();
    updateGeometry();
}

void SettingsLayout::updateGeometry()
{
    if (m_mainSplitter) {
        m_mainSplitter->updateGeometry();
    }
}

void SettingsLayout::updateSpacing()
{
    if (!currentTheme()) {
        return;
    }

    int spacing = currentTheme()->spacing();
    
    if (m_mainSplitter) {
        m_mainSplitter->setHandleWidth(spacing / 2);
    }
    
    if (m_mainLayout) {
        m_mainLayout->setSpacing(spacing);
    }
    
    if (m_contentLayout) {
        m_contentLayout->setSpacing(spacing);
    }
}

void SettingsLayout::updateMargins()
{
    if (!currentTheme()) {
        return;
    }

    int margin = currentTheme()->margin();
    
    if (m_mainLayout) {
        m_mainLayout->setContentsMargins(margin, margin, margin, margin);
    }
    
    if (m_contentLayout) {
        m_contentLayout->setContentsMargins(margin, margin, margin, margin);
    }
}

// 私有槽函数实现
void SettingsLayout::onCategorySelectionChanged()
{
    if (!m_categoryTree) {
        return;
    }

    QTreeWidgetItem* currentItem = m_categoryTree->currentItem();
    if (!currentItem) {
        return;
    }

    QString category = currentItem->data(0, Qt::UserRole).toString();
    QString page = currentItem->text(0);

    if (!category.isEmpty() && !page.isEmpty()) {
        navigateToPage(category, page);
    }
}

void SettingsLayout::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)
    
    // 更新分类树宽度
    if (m_mainSplitter && isCategoryTreeVisible()) {
        QList<int> sizes = m_mainSplitter->sizes();
        if (sizes.size() >= 2) {
            m_categoryTreeWidth = sizes.first();
            emit categoryTreeWidthChanged(m_categoryTreeWidth);
        }
    }
}

void SettingsLayout::onRegionWidgetDestroyed()
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (!widget) {
        return;
    }

    // 查找并移除被销毁的组件
    for (auto it = m_regionWidgets.begin(); it != m_regionWidgets.end(); ++it) {
        if (it.value() == widget) {
            removeRegionWidget(it.key());
            break;
        }
    }
}

// 私有方法实现
void SettingsLayout::setupLayout()
{
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->setChildrenCollapsible(false);
    
    // 创建主布局
    m_mainLayout = new QHBoxLayout;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    // 创建内容布局
    m_contentLayout = new QVBoxLayout;
    m_contentLayout->setContentsMargins(0, 0, 0, 0);
    m_contentLayout->setSpacing(0);
}

void SettingsLayout::createRegions()
{
    // 创建分类树
    if (!m_categoryTree) {
        m_categoryTree = new QTreeWidget;
        m_categoryTree->setHeaderHidden(true);
        m_categoryTree->setRootIsDecorated(true);
        setRegionWidget(CategoryTreeRegion, m_categoryTree);
    }
    
    // 创建设置面板
    if (!m_settingsPanel) {
        m_settingsPanel = new QStackedWidget;
        setRegionWidget(SettingsPanelRegion, m_settingsPanel);
    }
}

void SettingsLayout::arrangeRegions()
{
    if (!m_mainSplitter) {
        return;
    }

    // 清除现有布局
    m_mainSplitter->clear();

    // 根据布局样式排列区域
    switch (m_layoutStyle) {
    case TreeAndPanelStyle:
        arrangeTreeAndPanelStyle();
        break;
    case TabsStyle:
        arrangeTabsStyle();
        break;
    case WizardStyle:
        arrangeWizardStyle();
        break;
    case CompactStyle:
        arrangeCompactStyle();
        break;
    }
}

void SettingsLayout::arrangeTreeAndPanelStyle()
{
    // 添加分类树
    if (m_categoryTree && isCategoryTreeVisible()) {
        m_mainSplitter->addWidget(m_categoryTree);
    }

    // 创建右侧内容区域
    QWidget* contentArea = new QWidget;
    QVBoxLayout* contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 添加头部区域
    if (m_headerWidget && isRegionVisible(HeaderRegion)) {
        contentLayout->addWidget(m_headerWidget);
    }

    // 添加搜索组件
    if (m_searchWidget && m_searchEnabled) {
        contentLayout->addWidget(m_searchWidget);
    }

    // 添加设置面板
    if (m_settingsPanel && isRegionVisible(SettingsPanelRegion)) {
        contentLayout->addWidget(m_settingsPanel, 1);
    }

    // 添加按钮区域
    if (m_buttonArea && isRegionVisible(ButtonAreaRegion)) {
        contentLayout->addWidget(m_buttonArea);
    }

    // 添加底部区域
    if (m_footerWidget && isRegionVisible(FooterRegion)) {
        contentLayout->addWidget(m_footerWidget);
    }

    m_mainSplitter->addWidget(contentArea);

    // 设置分割器比例
    if (isCategoryTreeVisible()) {
        QList<int> sizes;
        int totalWidth = m_mainSplitter->width();
        if (totalWidth > 0) {
            sizes << m_categoryTreeWidth << (totalWidth - m_categoryTreeWidth);
            m_mainSplitter->setSizes(sizes);
        }
    }
}

void SettingsLayout::arrangeTabsStyle()
{
    // 标签页样式 - 简化实现，主要显示设置面板
    QWidget* contentArea = new QWidget;
    QVBoxLayout* contentLayout = new QVBoxLayout(contentArea);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    if (m_headerWidget && isRegionVisible(HeaderRegion)) {
        contentLayout->addWidget(m_headerWidget);
    }

    if (m_settingsPanel && isRegionVisible(SettingsPanelRegion)) {
        contentLayout->addWidget(m_settingsPanel, 1);
    }

    if (m_buttonArea && isRegionVisible(ButtonAreaRegion)) {
        contentLayout->addWidget(m_buttonArea);
    }

    m_mainSplitter->addWidget(contentArea);
}

void SettingsLayout::arrangeWizardStyle()
{
    // 向导样式 - 类似标签页样式但可能有导航按钮
    arrangeTabsStyle();
}

void SettingsLayout::arrangeCompactStyle()
{
    // 紧凑样式 - 只显示设置面板
    if (m_settingsPanel && isRegionVisible(SettingsPanelRegion)) {
        m_mainSplitter->addWidget(m_settingsPanel);
    }
}

void SettingsLayout::updateLayoutStyle()
{
    if (!isApplied()) {
        return;
    }

    arrangeRegions();
}

void SettingsLayout::updateRegionVisibility()
{
    // 更新各区域的可见性
    for (auto it = m_regionWidgets.constBegin(); it != m_regionWidgets.constEnd(); ++it) {
        SettingsRegion region = it.key();
        QWidget* widget = it.value();
        bool visible = m_regionVisibility.value(region, false);
        
        if (widget) {
            widget->setVisible(visible);
        }
    }
}

void SettingsLayout::updateCategoryTree()
{
    if (!m_categoryTree) {
        return;
    }

    // 清除现有项目
    m_categoryTree->clear();

    // 重新构建分类树
    for (auto it = m_categoryPages.constBegin(); it != m_categoryPages.constEnd(); ++it) {
        const QString& category = it.key();
        const QStringList& pages = it.value();

        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(m_categoryTree);
        categoryItem->setText(0, category);
        categoryItem->setData(0, Qt::UserRole, category);
        categoryItem->setExpanded(true);

        for (const QString& page : pages) {
            QTreeWidgetItem* pageItem = new QTreeWidgetItem(categoryItem);
            pageItem->setText(0, page);
            pageItem->setData(0, Qt::UserRole, category);
        }
    }
}

void SettingsLayout::updateSettingsPanel()
{
    // 设置面板的更新主要通过页面管理函数处理
}

void SettingsLayout::connectSignals()
{
    if (m_mainSplitter) {
        connect(m_mainSplitter, &QSplitter::splitterMoved,
                this, &SettingsLayout::onSplitterMoved);
    }
    
    if (m_categoryTree) {
        connect(m_categoryTree, &QTreeWidget::currentItemChanged,
                this, &SettingsLayout::onCategorySelectionChanged);
    }
}
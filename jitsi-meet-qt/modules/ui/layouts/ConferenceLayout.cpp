#include "ConferenceLayout.h"
#include "../themes/BaseTheme.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QWidget>
#include <QApplication>
#include <cmath>

ConferenceLayout::ConferenceLayout(QWidget *parent)
    : BaseLayout(parent)
    , m_viewMode(GridView)
    , m_gridColumns(3)
    , m_gridRows(2)
    , m_responsive(true)
    , m_mainSplitter(nullptr)
    , m_videoSplitter(nullptr)
    , m_videoGridLayout(nullptr)
    , m_mainLayout(nullptr)
    , m_videoGridWidget(nullptr)
    , m_mainVideoWidget(nullptr)
    , m_chatPanel(nullptr)
    , m_controlPanel(nullptr)
    , m_participantList(nullptr)
    , m_screenShareWidget(nullptr)
    , m_toolbarWidget(nullptr)
    , m_chatPanelWidth(300)
    , m_controlPanelHeight(80)
{
    // 初始化区域可见性
    m_regionVisibility[VideoGridRegion] = true;
    m_regionVisibility[MainVideoRegion] = false;
    m_regionVisibility[ChatPanelRegion] = false;
    m_regionVisibility[ControlPanelRegion] = true;
    m_regionVisibility[ParticipantListRegion] = false;
    m_regionVisibility[ScreenShareRegion] = false;
    m_regionVisibility[ToolbarRegion] = true;
}

ConferenceLayout::~ConferenceLayout()
{
    cleanup();
}

QString ConferenceLayout::layoutName() const
{
    return "conference";
}

QString ConferenceLayout::layoutDisplayName() const
{
    return tr("Conference Layout");
}

QString ConferenceLayout::layoutDescription() const
{
    return tr("Specialized layout for video conferencing with grid view, chat panel, and controls.");
}

bool ConferenceLayout::initialize()
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
        emit errorOccurred(QString("Failed to initialize conference layout: %1").arg(e.what()));
        return false;
    }
}

bool ConferenceLayout::apply(QWidget* widget)
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
        emit errorOccurred(QString("Failed to apply conference layout: %1").arg(e.what()));
        return false;
    }
}

void ConferenceLayout::cleanup()
{
    if (!isInitialized()) {
        return;
    }

    // 清理组件
    m_regionWidgets.clear();
    m_videoWidgets.clear();
    
    if (m_mainSplitter) {
        m_mainSplitter->deleteLater();
        m_mainSplitter = nullptr;
    }
    
    if (m_videoSplitter) {
        m_videoSplitter->deleteLater();
        m_videoSplitter = nullptr;
    }

    if (m_videoGridLayout) {
        delete m_videoGridLayout;
        m_videoGridLayout = nullptr;
    }

    m_videoGridWidget = nullptr;
    m_mainVideoWidget = nullptr;
    m_chatPanel = nullptr;
    m_controlPanel = nullptr;
    m_participantList = nullptr;
    m_screenShareWidget = nullptr;
    m_toolbarWidget = nullptr;

    setInitialized(false);
    setApplied(false);
    emit layoutCleanedUp();
}

// 视图模式管理
ConferenceLayout::ViewMode ConferenceLayout::viewMode() const
{
    return m_viewMode;
}

void ConferenceLayout::setViewMode(ViewMode mode)
{
    if (m_viewMode == mode) {
        return;
    }

    m_viewMode = mode;
    
    if (isApplied()) {
        updateViewMode();
    }

    emit viewModeChanged(mode);
}

QStringList ConferenceLayout::availableViewModes() const
{
    return QStringList() << "GridView" << "SpeakerView" << "PresentationView" << "FullScreenView";
}

// 网格布局管理
int ConferenceLayout::gridColumns() const
{
    return m_gridColumns;
}

void ConferenceLayout::setGridColumns(int columns)
{
    if (m_gridColumns == columns || columns < 1) {
        return;
    }

    m_gridColumns = columns;
    
    if (isApplied()) {
        updateVideoGrid();
    }

    emit gridColumnsChanged(columns);
}

int ConferenceLayout::gridRows() const
{
    return m_gridRows;
}

void ConferenceLayout::setGridRows(int rows)
{
    if (m_gridRows == rows || rows < 1) {
        return;
    }

    m_gridRows = rows;
    
    if (isApplied()) {
        updateVideoGrid();
    }

    emit gridRowsChanged(rows);
}

void ConferenceLayout::setGridSize(int columns, int rows)
{
    if (columns < 1 || rows < 1) {
        return;
    }

    bool changed = false;
    
    if (m_gridColumns != columns) {
        m_gridColumns = columns;
        changed = true;
        emit gridColumnsChanged(columns);
    }
    
    if (m_gridRows != rows) {
        m_gridRows = rows;
        changed = true;
        emit gridRowsChanged(rows);
    }

    if (changed && isApplied()) {
        updateVideoGrid();
    }
}

int ConferenceLayout::maxGridItems() const
{
    return m_gridColumns * m_gridRows;
}// 区域管理实现

bool ConferenceLayout::setRegionWidget(LayoutRegion region, QWidget* widget)
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
    case VideoGridRegion:
        m_videoGridWidget = widget;
        break;
    case MainVideoRegion:
        m_mainVideoWidget = widget;
        break;
    case ChatPanelRegion:
        m_chatPanel = widget;
        break;
    case ControlPanelRegion:
        m_controlPanel = widget;
        break;
    case ParticipantListRegion:
        m_participantList = widget;
        break;
    case ScreenShareRegion:
        m_screenShareWidget = widget;
        break;
    case ToolbarRegion:
        m_toolbarWidget = widget;
        break;
    }

    if (isApplied()) {
        updateLayout();
    }

    emit regionWidgetChanged(region, widget);
    return true;
}

QWidget* ConferenceLayout::getRegionWidget(LayoutRegion region) const
{
    return m_regionWidgets.value(region, nullptr);
}

bool ConferenceLayout::removeRegionWidget(LayoutRegion region)
{
    if (!m_regionWidgets.contains(region)) {
        return false;
    }

    QWidget* widget = m_regionWidgets.take(region);

    // 清除特定组件引用
    switch (region) {
    case VideoGridRegion:
        m_videoGridWidget = nullptr;
        break;
    case MainVideoRegion:
        m_mainVideoWidget = nullptr;
        break;
    case ChatPanelRegion:
        m_chatPanel = nullptr;
        break;
    case ControlPanelRegion:
        m_controlPanel = nullptr;
        break;
    case ParticipantListRegion:
        m_participantList = nullptr;
        break;
    case ScreenShareRegion:
        m_screenShareWidget = nullptr;
        break;
    case ToolbarRegion:
        m_toolbarWidget = nullptr;
        break;
    }

    if (isApplied()) {
        updateLayout();
    }

    emit regionWidgetChanged(region, nullptr);
    return true;
}

bool ConferenceLayout::isRegionVisible(LayoutRegion region) const
{
    return m_regionVisibility.value(region, false);
}

void ConferenceLayout::setRegionVisible(LayoutRegion region, bool visible)
{
    if (m_regionVisibility.value(region, false) == visible) {
        return;
    }

    m_regionVisibility[region] = visible;

    if (isApplied()) {
        updateRegionVisibility();
    }
}

// 视频区域管理实现
bool ConferenceLayout::addVideoWidget(QWidget* videoWidget, int position)
{
    if (!videoWidget) {
        return false;
    }

    if (m_videoWidgets.contains(videoWidget)) {
        return false;
    }

    if (position < 0 || position > m_videoWidgets.size()) {
        position = m_videoWidgets.size();
    }

    m_videoWidgets.insert(position, videoWidget);

    if (isApplied()) {
        updateVideoGrid();
    }

    emit videoWidgetAdded(videoWidget, position);
    return true;
}

bool ConferenceLayout::removeVideoWidget(QWidget* videoWidget)
{
    int position = m_videoWidgets.indexOf(videoWidget);
    if (position < 0) {
        return false;
    }

    return removeVideoWidget(position);
}

bool ConferenceLayout::removeVideoWidget(int position)
{
    if (position < 0 || position >= m_videoWidgets.size()) {
        return false;
    }

    QWidget* widget = m_videoWidgets.takeAt(position);

    if (isApplied()) {
        updateVideoGrid();
    }

    emit videoWidgetRemoved(widget, position);
    return true;
}

QList<QWidget*> ConferenceLayout::videoWidgets() const
{
    return m_videoWidgets;
}

int ConferenceLayout::videoWidgetCount() const
{
    return m_videoWidgets.size();
}

void ConferenceLayout::clearVideoWidgets()
{
    if (m_videoWidgets.isEmpty()) {
        return;
    }

    m_videoWidgets.clear();

    if (isApplied()) {
        updateVideoGrid();
    }
}

// 主视频管理实现
QWidget* ConferenceLayout::mainVideoWidget() const
{
    return m_mainVideoWidget;
}

void ConferenceLayout::setMainVideoWidget(QWidget* widget)
{
    setRegionWidget(MainVideoRegion, widget);
    emit mainVideoChanged(widget);
}

bool ConferenceLayout::isMainVideoVisible() const
{
    return isRegionVisible(MainVideoRegion);
}

void ConferenceLayout::setMainVideoVisible(bool visible)
{
    setRegionVisible(MainVideoRegion, visible);
}

// 聊天面板管理实现
bool ConferenceLayout::isChatPanelVisible() const
{
    return isRegionVisible(ChatPanelRegion);
}

void ConferenceLayout::setChatPanelVisible(bool visible)
{
    setRegionVisible(ChatPanelRegion, visible);
    emit chatPanelVisibleChanged(visible);
}

QWidget* ConferenceLayout::chatPanel() const
{
    return m_chatPanel;
}

void ConferenceLayout::setChatPanel(QWidget* panel)
{
    setRegionWidget(ChatPanelRegion, panel);
}

int ConferenceLayout::chatPanelWidth() const
{
    return m_chatPanelWidth;
}

void ConferenceLayout::setChatPanelWidth(int width)
{
    if (m_chatPanelWidth == width || width < 0) {
        return;
    }

    m_chatPanelWidth = width;

    if (isApplied() && isChatPanelVisible()) {
        updateLayout();
    }
}

// 控制面板管理实现
bool ConferenceLayout::isControlPanelVisible() const
{
    return isRegionVisible(ControlPanelRegion);
}

void ConferenceLayout::setControlPanelVisible(bool visible)
{
    setRegionVisible(ControlPanelRegion, visible);
    emit controlPanelVisibleChanged(visible);
}

QWidget* ConferenceLayout::controlPanel() const
{
    return m_controlPanel;
}

void ConferenceLayout::setControlPanel(QWidget* panel)
{
    setRegionWidget(ControlPanelRegion, panel);
}

int ConferenceLayout::controlPanelHeight() const
{
    return m_controlPanelHeight;
}

void ConferenceLayout::setControlPanelHeight(int height)
{
    if (m_controlPanelHeight == height || height < 0) {
        return;
    }

    m_controlPanelHeight = height;

    if (isApplied() && isControlPanelVisible()) {
        updateLayout();
    }
}

// 参与者列表管理实现
bool ConferenceLayout::isParticipantListVisible() const
{
    return isRegionVisible(ParticipantListRegion);
}

void ConferenceLayout::setParticipantListVisible(bool visible)
{
    setRegionVisible(ParticipantListRegion, visible);
    emit participantListVisibleChanged(visible);
}

QWidget* ConferenceLayout::participantList() const
{
    return m_participantList;
}

void ConferenceLayout::setParticipantList(QWidget* list)
{
    setRegionWidget(ParticipantListRegion, list);
}

// 屏幕共享管理实现
bool ConferenceLayout::isScreenShareVisible() const
{
    return isRegionVisible(ScreenShareRegion);
}

void ConferenceLayout::setScreenShareVisible(bool visible)
{
    setRegionVisible(ScreenShareRegion, visible);
    emit screenShareVisibleChanged(visible);
}

QWidget* ConferenceLayout::screenShareWidget() const
{
    return m_screenShareWidget;
}

void ConferenceLayout::setScreenShareWidget(QWidget* widget)
{
    setRegionWidget(ScreenShareRegion, widget);
}// 布局配置实现
QVariantMap ConferenceLayout::getLayoutConfiguration() const
{
    QVariantMap config = BaseLayout::getLayoutConfiguration();
    
    config["viewMode"] = static_cast<int>(m_viewMode);
    config["gridColumns"] = m_gridColumns;
    config["gridRows"] = m_gridRows;
    config["chatPanelWidth"] = m_chatPanelWidth;
    config["controlPanelHeight"] = m_controlPanelHeight;
    config["responsive"] = m_responsive;

    // 保存区域可见性
    QVariantMap regionVisibility;
    for (auto it = m_regionVisibility.constBegin(); it != m_regionVisibility.constEnd(); ++it) {
        regionVisibility[QString::number(static_cast<int>(it.key()))] = it.value();
    }
    config["regionVisibility"] = regionVisibility;

    return config;
}

void ConferenceLayout::setLayoutConfiguration(const QVariantMap& config)
{
    BaseLayout::setLayoutConfiguration(config);

    // 恢复布局设置
    setViewMode(static_cast<ViewMode>(config.value("viewMode", GridView).toInt()));
    setGridColumns(config.value("gridColumns", 3).toInt());
    setGridRows(config.value("gridRows", 2).toInt());
    setChatPanelWidth(config.value("chatPanelWidth", 300).toInt());
    setControlPanelHeight(config.value("controlPanelHeight", 80).toInt());
    setResponsive(config.value("responsive", true).toBool());

    // 恢复区域可见性
    QVariantMap regionVisibility = config.value("regionVisibility").toMap();
    for (auto it = regionVisibility.constBegin(); it != regionVisibility.constEnd(); ++it) {
        LayoutRegion region = static_cast<LayoutRegion>(it.key().toInt());
        m_regionVisibility[region] = it.value().toBool();
    }

    if (isApplied()) {
        updateLayout();
    }
}

// 响应式设计实现
bool ConferenceLayout::adaptToSize(const QSize& size)
{
    if (!m_responsive || !isApplied()) {
        return false;
    }

    bool changed = false;

    // 根据窗口大小调整网格布局
    if (size.width() < 800) {
        // 小屏幕时使用较少的列数
        int newColumns = qMax(1, size.width() / 200);
        if (newColumns != m_gridColumns) {
            setGridColumns(newColumns);
            changed = true;
        }
        
        // 隐藏聊天面板
        if (isChatPanelVisible()) {
            setChatPanelVisible(false);
            changed = true;
        }
    } else {
        // 大屏幕时自动计算最优网格大小
        calculateOptimalGridSize();
        changed = true;
    }

    // 极小屏幕时切换到演讲者视图
    if (size.width() < 600 && m_viewMode != SpeakerView) {
        setViewMode(SpeakerView);
        changed = true;
    }

    if (changed) {
        emit sizeAdapted(size);
    }

    return changed;
}

bool ConferenceLayout::isResponsive() const
{
    return m_responsive;
}

void ConferenceLayout::setResponsive(bool responsive)
{
    m_responsive = responsive;
}

// BaseLayout虚函数实现
void ConferenceLayout::onThemeChanged(std::shared_ptr<BaseTheme> theme)
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
    
    if (m_videoSplitter) {
        m_videoSplitter->setStyleSheet(QString(
            "QSplitter::handle { background-color: %1; }"
        ).arg(theme->borderColor().name()));
    }
}

void ConferenceLayout::onConfigurationChanged(const QVariantMap& config)
{
    BaseLayout::onConfigurationChanged(config);
    // 配置变化处理已在setLayoutConfiguration中实现
}

QVariantMap ConferenceLayout::getDefaultConfiguration() const
{
    QVariantMap config;
    config["viewMode"] = static_cast<int>(GridView);
    config["gridColumns"] = 3;
    config["gridRows"] = 2;
    config["chatPanelWidth"] = 300;
    config["controlPanelHeight"] = 80;
    config["responsive"] = true;
    return config;
}

bool ConferenceLayout::validateConfiguration(const QVariantMap& config) const
{
    // 验证配置参数
    if (config.contains("gridColumns")) {
        int columns = config["gridColumns"].toInt();
        if (columns < 1 || columns > 10) {
            return false;
        }
    }
    
    if (config.contains("gridRows")) {
        int rows = config["gridRows"].toInt();
        if (rows < 1 || rows > 10) {
            return false;
        }
    }
    
    if (config.contains("chatPanelWidth")) {
        int width = config["chatPanelWidth"].toInt();
        if (width < 200 || width > 800) {
            return false;
        }
    }
    
    return BaseLayout::validateConfiguration(config);
}

// 布局更新实现
void ConferenceLayout::updateLayout()
{
    if (!isApplied()) {
        return;
    }

    arrangeRegions();
    updateRegionVisibility();
    updateVideoGrid();
    updateViewMode();
    updateGeometry();
}

void ConferenceLayout::updateGeometry()
{
    if (m_mainSplitter) {
        m_mainSplitter->updateGeometry();
    }
    
    if (m_videoSplitter) {
        m_videoSplitter->updateGeometry();
    }
}

void ConferenceLayout::updateSpacing()
{
    if (!currentTheme()) {
        return;
    }

    int spacing = currentTheme()->spacing();
    
    if (m_mainSplitter) {
        m_mainSplitter->setHandleWidth(spacing / 2);
    }
    
    if (m_videoSplitter) {
        m_videoSplitter->setHandleWidth(spacing / 2);
    }
    
    if (m_videoGridLayout) {
        m_videoGridLayout->setSpacing(spacing);
    }
}

void ConferenceLayout::updateMargins()
{
    if (!currentTheme()) {
        return;
    }

    int margin = currentTheme()->margin();
    
    if (m_videoGridLayout) {
        m_videoGridLayout->setContentsMargins(margin, margin, margin, margin);
    }
}

// 私有槽函数实现
void ConferenceLayout::onSplitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)
    
    // 更新面板尺寸
    if (m_mainSplitter && isChatPanelVisible()) {
        QList<int> sizes = m_mainSplitter->sizes();
        if (sizes.size() >= 2) {
            m_chatPanelWidth = sizes.last();
        }
    }
}

void ConferenceLayout::onVideoWidgetDestroyed()
{
    QWidget* widget = qobject_cast<QWidget*>(sender());
    if (widget) {
        removeVideoWidget(widget);
    }
}

void ConferenceLayout::onRegionWidgetDestroyed()
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
void ConferenceLayout::setupLayout()
{
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainSplitter->setChildrenCollapsible(false);
    
    // 创建视频分割器
    m_videoSplitter = new QSplitter(Qt::Vertical);
    m_videoSplitter->setChildrenCollapsible(false);
    
    // 创建主布局
    m_mainLayout = new QVBoxLayout;
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
}

void ConferenceLayout::createRegions()
{
    // 创建视频网格组件
    m_videoGridWidget = new QWidget;
    m_videoGridLayout = new QGridLayout(m_videoGridWidget);
    m_videoGridLayout->setSpacing(2);
    m_videoGridLayout->setContentsMargins(4, 4, 4, 4);
}

void ConferenceLayout::arrangeRegions()
{
    if (!m_mainSplitter || !m_videoSplitter) {
        return;
    }

    // 清除现有布局
    while (m_mainSplitter->count() > 0) {
        QWidget* widget = m_mainSplitter->widget(0);
        widget->setParent(nullptr);
    }
    
    while (m_videoSplitter->count() > 0) {
        QWidget* widget = m_videoSplitter->widget(0);
        widget->setParent(nullptr);
    }

    // 根据视图模式排列区域
    switch (m_viewMode) {
    case GridView:
        arrangeGridView();
        break;
    case SpeakerView:
        arrangeSpeakerView();
        break;
    case PresentationView:
        arrangePresentationView();
        break;
    case FullScreenView:
        arrangeFullScreenView();
        break;
    }
}

void ConferenceLayout::arrangeGridView()
{
    // 网格视图布局
    QWidget* mainContent = new QWidget;
    QVBoxLayout* contentLayout = new QVBoxLayout(mainContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 添加工具栏
    if (m_toolbarWidget && isRegionVisible(ToolbarRegion)) {
        contentLayout->addWidget(m_toolbarWidget);
    }

    // 添加视频区域
    if (m_videoGridWidget && isRegionVisible(VideoGridRegion)) {
        contentLayout->addWidget(m_videoGridWidget, 1);
    }

    // 添加控制面板
    if (m_controlPanel && isRegionVisible(ControlPanelRegion)) {
        contentLayout->addWidget(m_controlPanel);
    }

    m_mainSplitter->addWidget(mainContent);

    // 添加聊天面板
    if (m_chatPanel && isRegionVisible(ChatPanelRegion)) {
        m_mainSplitter->addWidget(m_chatPanel);
        
        // 设置分割器比例
        QList<int> sizes;
        int totalWidth = m_mainSplitter->width();
        if (totalWidth > 0) {
            sizes << (totalWidth - m_chatPanelWidth) << m_chatPanelWidth;
            m_mainSplitter->setSizes(sizes);
        }
    }
}

void ConferenceLayout::arrangeSpeakerView()
{
    // 演讲者视图布局
    QWidget* mainContent = new QWidget;
    QVBoxLayout* contentLayout = new QVBoxLayout(mainContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 添加工具栏
    if (m_toolbarWidget && isRegionVisible(ToolbarRegion)) {
        contentLayout->addWidget(m_toolbarWidget);
    }

    // 创建水平分割器用于主视频和参与者列表
    QSplitter* horizontalSplitter = new QSplitter(Qt::Horizontal);
    
    // 添加主视频
    if (m_mainVideoWidget && isRegionVisible(MainVideoRegion)) {
        horizontalSplitter->addWidget(m_mainVideoWidget);
    }
    
    // 添加参与者列表
    if (m_participantList && isRegionVisible(ParticipantListRegion)) {
        horizontalSplitter->addWidget(m_participantList);
    }
    
    contentLayout->addWidget(horizontalSplitter, 1);

    // 添加控制面板
    if (m_controlPanel && isRegionVisible(ControlPanelRegion)) {
        contentLayout->addWidget(m_controlPanel);
    }

    m_mainSplitter->addWidget(mainContent);

    // 添加聊天面板
    if (m_chatPanel && isRegionVisible(ChatPanelRegion)) {
        m_mainSplitter->addWidget(m_chatPanel);
    }
}

void ConferenceLayout::arrangePresentationView()
{
    // 演示视图布局
    QWidget* mainContent = new QWidget;
    QVBoxLayout* contentLayout = new QVBoxLayout(mainContent);
    contentLayout->setContentsMargins(0, 0, 0, 0);
    contentLayout->setSpacing(0);

    // 添加工具栏
    if (m_toolbarWidget && isRegionVisible(ToolbarRegion)) {
        contentLayout->addWidget(m_toolbarWidget);
    }

    // 创建水平分割器
    QSplitter* horizontalSplitter = new QSplitter(Qt::Horizontal);
    
    // 添加屏幕共享区域
    if (m_screenShareWidget && isRegionVisible(ScreenShareRegion)) {
        horizontalSplitter->addWidget(m_screenShareWidget);
    }
    
    // 添加视频网格（较小）
    if (m_videoGridWidget && isRegionVisible(VideoGridRegion)) {
        horizontalSplitter->addWidget(m_videoGridWidget);
    }
    
    contentLayout->addWidget(horizontalSplitter, 1);

    // 添加控制面板
    if (m_controlPanel && isRegionVisible(ControlPanelRegion)) {
        contentLayout->addWidget(m_controlPanel);
    }

    m_mainSplitter->addWidget(mainContent);

    // 添加聊天面板
    if (m_chatPanel && isRegionVisible(ChatPanelRegion)) {
        m_mainSplitter->addWidget(m_chatPanel);
    }
}

void ConferenceLayout::arrangeFullScreenView()
{
    // 全屏视图布局
    if (m_mainVideoWidget && isRegionVisible(MainVideoRegion)) {
        m_mainSplitter->addWidget(m_mainVideoWidget);
    } else if (m_screenShareWidget && isRegionVisible(ScreenShareRegion)) {
        m_mainSplitter->addWidget(m_screenShareWidget);
    }
}

void ConferenceLayout::updateViewMode()
{
    if (!isApplied()) {
        return;
    }

    arrangeRegions();
}

void ConferenceLayout::updateVideoGrid()
{
    if (!m_videoGridLayout || !isApplied()) {
        return;
    }

    // 清除现有布局
    QLayoutItem* item;
    while ((item = m_videoGridLayout->takeAt(0)) != nullptr) {
        delete item;
    }

    // 重新排列视频组件
    int row = 0, col = 0;
    for (int i = 0; i < m_videoWidgets.size() && i < maxGridItems(); ++i) {
        QWidget* videoWidget = m_videoWidgets.at(i);
        if (videoWidget) {
            m_videoGridLayout->addWidget(videoWidget, row, col);
            
            col++;
            if (col >= m_gridColumns) {
                col = 0;
                row++;
            }
        }
    }
}

void ConferenceLayout::updateRegionVisibility()
{
    // 更新各区域的可见性
    for (auto it = m_regionWidgets.constBegin(); it != m_regionWidgets.constEnd(); ++it) {
        LayoutRegion region = it.key();
        QWidget* widget = it.value();
        bool visible = m_regionVisibility.value(region, false);
        
        if (widget) {
            widget->setVisible(visible);
        }
    }
}

void ConferenceLayout::calculateOptimalGridSize()
{
    int videoCount = m_videoWidgets.size();
    if (videoCount <= 0) {
        return;
    }

    // 计算最优的网格尺寸
    int columns = static_cast<int>(std::ceil(std::sqrt(videoCount)));
    int rows = static_cast<int>(std::ceil(static_cast<double>(videoCount) / columns));
    
    setGridSize(columns, rows);
}

void ConferenceLayout::connectSignals()
{
    if (m_mainSplitter) {
        connect(m_mainSplitter, &QSplitter::splitterMoved,
                this, &ConferenceLayout::onSplitterMoved);
    }
    
    if (m_videoSplitter) {
        connect(m_videoSplitter, &QSplitter::splitterMoved,
                this, &ConferenceLayout::onSplitterMoved);
    }
}
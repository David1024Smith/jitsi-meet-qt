#include "ScreenSelector.h"
#include <QApplication>
#include <QScreen>
#include <QWindow>
#include <QPixmap>
#include <QPainter>
#include <QListWidgetItem>
#include <QHeaderView>
#include <QSplitter>
#include <QDebug>

class ScreenSelector::Private
{
public:
    Private()
        : selectionType(ScreenSelection)
        , thumbnailSize(200, 150)
        , showThumbnails(true)
        , allowMultipleSelection(false)
    {
    }

    SelectionType selectionType;
    QString selectedSource;
    QRect selectedRegion;
    
    // UI组件
    QVBoxLayout* mainLayout;
    QTabWidget* tabWidget;
    
    // 屏幕选择标签页
    QWidget* screenTab;
    QGridLayout* screenLayout;
    QListWidget* screenList;
    QPushButton* refreshScreensButton;
    
    // 窗口选择标签页
    QWidget* windowTab;
    QVBoxLayout* windowLayout;
    QListWidget* windowList;
    QLineEdit* windowFilter;
    QPushButton* refreshWindowsButton;
    
    // 区域选择标签页
    QWidget* regionTab;
    QVBoxLayout* regionLayout;
    QGroupBox* regionGroup;
    QPushButton* selectRegionButton;
    QPushButton* interactiveSelectButton;
    QLabel* regionInfoLabel;
    
    // 数据
    QList<ScreenInfo> availableScreens;
    QList<WindowInfo> availableWindows;
    QList<ScreenInfo> filteredScreens;
    QList<WindowInfo> filteredWindows;
    
    // 配置
    QSize thumbnailSize;
    bool showThumbnails;
    bool allowMultipleSelection;
};

ScreenSelector::ScreenSelector(QWidget *parent)
    : QWidget(parent)
    , d(new Private)
{
    setupUI();
    connectSignals();
    refreshScreens();
    refreshWindows();
}

ScreenSelector::~ScreenSelector()
{
    delete d;
}

ScreenSelector::SelectionType ScreenSelector::selectionType() const
{
    return d->selectionType;
}

void ScreenSelector::setSelectionType(SelectionType type)
{
    if (d->selectionType != type) {
        d->selectionType = type;
        d->tabWidget->setCurrentIndex(static_cast<int>(type));
        emit selectionTypeChanged(type);
    }
}

QString ScreenSelector::selectedSource() const
{
    return d->selectedSource;
}

QRect ScreenSelector::selectedRegion() const
{
    return d->selectedRegion;
}

bool ScreenSelector::hasSelection() const
{
    switch (d->selectionType) {
    case ScreenSelection:
        return !d->selectedSource.isEmpty() && !d->availableScreens.isEmpty();
    case WindowSelection:
        return !d->selectedSource.isEmpty() && !d->availableWindows.isEmpty();
    case RegionSelection:
        return !d->selectedRegion.isEmpty();
    }
    return false;
}

QList<ScreenSelector::ScreenInfo> ScreenSelector::availableScreens() const
{
    return d->availableScreens;
}

void ScreenSelector::setAvailableScreens(const QList<ScreenInfo>& screens)
{
    d->availableScreens = screens;
    updateScreenList();
}

ScreenSelector::ScreenInfo ScreenSelector::selectedScreen() const
{
    for (const auto& screen : d->availableScreens) {
        if (screen.id == d->selectedSource) {
            return screen;
        }
    }
    return ScreenInfo();
}

QList<ScreenSelector::WindowInfo> ScreenSelector::availableWindows() const
{
    return d->availableWindows;
}

void ScreenSelector::setAvailableWindows(const QList<WindowInfo>& windows)
{
    d->availableWindows = windows;
    updateWindowList();
}

ScreenSelector::WindowInfo ScreenSelector::selectedWindow() const
{
    for (const auto& window : d->availableWindows) {
        if (window.id == d->selectedSource) {
            return window;
        }
    }
    return WindowInfo();
}

void ScreenSelector::setCustomRegion(const QRect& region)
{
    d->selectedRegion = region;
    d->regionInfoLabel->setText(tr("选择区域: %1x%2 at (%3,%4)")
                               .arg(region.width())
                               .arg(region.height())
                               .arg(region.x())
                               .arg(region.y()));
    updateSelection();
}

QRect ScreenSelector::customRegion() const
{
    return d->selectedRegion;
}

void ScreenSelector::startInteractiveSelection()
{
    emit interactiveSelectionStarted();
    // 这里应该启动交互式区域选择
    // 实际实现需要创建一个覆盖整个屏幕的透明窗口
}

void ScreenSelector::cancelInteractiveSelection()
{
    emit selectionCancelled();
}

void ScreenSelector::setThumbnailSize(const QSize& size)
{
    d->thumbnailSize = size;
    updateScreenList();
    updateWindowList();
}

QSize ScreenSelector::thumbnailSize() const
{
    return d->thumbnailSize;
}

void ScreenSelector::setShowThumbnails(bool show)
{
    d->showThumbnails = show;
    updateScreenList();
    updateWindowList();
}

bool ScreenSelector::isShowThumbnails() const
{
    return d->showThumbnails;
}

void ScreenSelector::setAllowMultipleSelection(bool allow)
{
    d->allowMultipleSelection = allow;
    
    QAbstractItemView::SelectionMode mode = allow ? 
        QAbstractItemView::MultiSelection : 
        QAbstractItemView::SingleSelection;
    
    d->screenList->setSelectionMode(mode);
    d->windowList->setSelectionMode(mode);
}

bool ScreenSelector::isMultipleSelectionAllowed() const
{
    return d->allowMultipleSelection;
}

void ScreenSelector::refreshScreens()
{
    d->availableScreens.clear();
    
    QList<QScreen*> screens = QApplication::screens();
    for (int i = 0; i < screens.size(); ++i) {
        QScreen* screen = screens[i];
        
        ScreenInfo info;
        info.id = QString("screen_%1").arg(i);
        info.name = screen->name();
        info.geometry = screen->geometry();
        info.isPrimary = (screen == QApplication::primaryScreen());
        
        // 创建缩略图
        if (d->showThumbnails) {
            info.thumbnail = createScreenThumbnail(info);
        }
        
        d->availableScreens.append(info);
    }
    
    updateScreenList();
}

void ScreenSelector::refreshWindows()
{
    d->availableWindows.clear();
    
    // 这里应该使用平台特定的API来获取窗口列表
    // 为了演示，我们创建一些模拟数据
    
    WindowInfo info1;
    info1.id = "window_1";
    info1.title = "示例应用程序";
    info1.processName = "example.exe";
    info1.geometry = QRect(100, 100, 800, 600);
    info1.isVisible = true;
    
    WindowInfo info2;
    info2.id = "window_2";
    info2.title = "文本编辑器";
    info2.processName = "notepad.exe";
    info2.geometry = QRect(200, 200, 600, 400);
    info2.isVisible = true;
    
    d->availableWindows.append(info1);
    d->availableWindows.append(info2);
    
    updateWindowList();
}

void ScreenSelector::clearSelection()
{
    d->selectedSource.clear();
    d->selectedRegion = QRect();
    
    d->screenList->clearSelection();
    d->windowList->clearSelection();
    d->regionInfoLabel->setText(tr("未选择区域"));
    
    updateSelection();
}

void ScreenSelector::selectPrimaryScreen()
{
    for (const auto& screen : d->availableScreens) {
        if (screen.isPrimary) {
            d->selectedSource = screen.id;
            setSelectionType(ScreenSelection);
            
            // 在列表中选择对应项
            for (int i = 0; i < d->screenList->count(); ++i) {
                QListWidgetItem* item = d->screenList->item(i);
                if (item->data(Qt::UserRole).toString() == screen.id) {
                    d->screenList->setCurrentItem(item);
                    break;
                }
            }
            
            updateSelection();
            break;
        }
    }
}

void ScreenSelector::autoSelectBestSource()
{
    // 优先选择主屏幕
    selectPrimaryScreen();
}

void ScreenSelector::onTabChanged(int index)
{
    SelectionType newType = static_cast<SelectionType>(index);
    if (d->selectionType != newType) {
        d->selectionType = newType;
        clearSelection();
        emit selectionTypeChanged(newType);
    }
}

void ScreenSelector::onScreenItemClicked()
{
    QListWidgetItem* item = d->screenList->currentItem();
    if (item) {
        d->selectedSource = item->data(Qt::UserRole).toString();
        updateSelection();
        
        // 查找对应的屏幕信息
        for (const auto& screen : d->availableScreens) {
            if (screen.id == d->selectedSource) {
                emit screenSelected(screen);
                break;
            }
        }
    }
}

void ScreenSelector::onWindowItemClicked()
{
    QListWidgetItem* item = d->windowList->currentItem();
    if (item) {
        d->selectedSource = item->data(Qt::UserRole).toString();
        updateSelection();
        
        // 查找对应的窗口信息
        for (const auto& window : d->availableWindows) {
            if (window.id == d->selectedSource) {
                emit windowSelected(window);
                break;
            }
        }
    }
}

void ScreenSelector::onRegionButtonClicked()
{
    // 显示区域选择对话框或开始交互式选择
    startInteractiveSelection();
}

void ScreenSelector::onInteractiveSelectionClicked()
{
    startInteractiveSelection();
}

void ScreenSelector::onRefreshScreensClicked()
{
    refreshScreens();
}

void ScreenSelector::onRefreshWindowsClicked()
{
    refreshWindows();
}

void ScreenSelector::onFilterTextChanged(const QString& text)
{
    filterWindowList(text);
}

void ScreenSelector::setupUI()
{
    d->mainLayout = new QVBoxLayout(this);
    d->tabWidget = new QTabWidget(this);
    
    setupScreenTab();
    setupWindowTab();
    setupRegionTab();
    
    d->tabWidget->addTab(d->screenTab, tr("屏幕"));
    d->tabWidget->addTab(d->windowTab, tr("窗口"));
    d->tabWidget->addTab(d->regionTab, tr("区域"));
    
    d->mainLayout->addWidget(d->tabWidget);
}

void ScreenSelector::setupScreenTab()
{
    d->screenTab = new QWidget;
    d->screenLayout = new QGridLayout(d->screenTab);
    
    d->screenList = new QListWidget(d->screenTab);
    d->screenList->setViewMode(QListWidget::IconMode);
    d->screenList->setResizeMode(QListWidget::Adjust);
    d->screenList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    d->refreshScreensButton = new QPushButton(tr("刷新屏幕"), d->screenTab);
    
    d->screenLayout->addWidget(d->screenList, 0, 0, 1, 2);
    d->screenLayout->addWidget(d->refreshScreensButton, 1, 1);
    d->screenLayout->setColumnStretch(0, 1);
}

void ScreenSelector::setupWindowTab()
{
    d->windowTab = new QWidget;
    d->windowLayout = new QVBoxLayout(d->windowTab);
    
    // 过滤器
    QHBoxLayout* filterLayout = new QHBoxLayout;
    filterLayout->addWidget(new QLabel(tr("过滤:")));
    d->windowFilter = new QLineEdit(d->windowTab);
    d->windowFilter->setPlaceholderText(tr("输入窗口标题或进程名"));
    filterLayout->addWidget(d->windowFilter);
    
    d->windowList = new QListWidget(d->windowTab);
    d->windowList->setViewMode(QListWidget::ListMode);
    d->windowList->setSelectionMode(QAbstractItemView::SingleSelection);
    
    d->refreshWindowsButton = new QPushButton(tr("刷新窗口"), d->windowTab);
    
    d->windowLayout->addLayout(filterLayout);
    d->windowLayout->addWidget(d->windowList);
    d->windowLayout->addWidget(d->refreshWindowsButton);
}

void ScreenSelector::setupRegionTab()
{
    d->regionTab = new QWidget;
    d->regionLayout = new QVBoxLayout(d->regionTab);
    
    d->regionGroup = new QGroupBox(tr("区域选择"), d->regionTab);
    QVBoxLayout* regionGroupLayout = new QVBoxLayout(d->regionGroup);
    
    d->selectRegionButton = new QPushButton(tr("选择区域"), d->regionGroup);
    d->interactiveSelectButton = new QPushButton(tr("交互式选择"), d->regionGroup);
    d->regionInfoLabel = new QLabel(tr("未选择区域"), d->regionGroup);
    
    regionGroupLayout->addWidget(d->selectRegionButton);
    regionGroupLayout->addWidget(d->interactiveSelectButton);
    regionGroupLayout->addWidget(d->regionInfoLabel);
    regionGroupLayout->addStretch();
    
    d->regionLayout->addWidget(d->regionGroup);
    d->regionLayout->addStretch();
}

void ScreenSelector::connectSignals()
{
    connect(d->tabWidget, &QTabWidget::currentChanged,
            this, &ScreenSelector::onTabChanged);
    
    connect(d->screenList, &QListWidget::itemClicked,
            this, &ScreenSelector::onScreenItemClicked);
    connect(d->windowList, &QListWidget::itemClicked,
            this, &ScreenSelector::onWindowItemClicked);
    
    connect(d->refreshScreensButton, &QPushButton::clicked,
            this, &ScreenSelector::onRefreshScreensClicked);
    connect(d->refreshWindowsButton, &QPushButton::clicked,
            this, &ScreenSelector::onRefreshWindowsClicked);
    
    connect(d->selectRegionButton, &QPushButton::clicked,
            this, &ScreenSelector::onRegionButtonClicked);
    connect(d->interactiveSelectButton, &QPushButton::clicked,
            this, &ScreenSelector::onInteractiveSelectionClicked);
    
    connect(d->windowFilter, &QLineEdit::textChanged,
            this, &ScreenSelector::onFilterTextChanged);
}

void ScreenSelector::updateScreenList()
{
    d->screenList->clear();
    
    for (const auto& screen : d->availableScreens) {
        QListWidgetItem* item = new QListWidgetItem;
        
        QString text = screen.name;
        if (screen.isPrimary) {
            text += tr(" (主屏幕)");
        }
        text += QString("\n%1x%2").arg(screen.geometry.width()).arg(screen.geometry.height());
        
        item->setText(text);
        item->setData(Qt::UserRole, screen.id);
        
        if (d->showThumbnails && !screen.thumbnail.isNull()) {
            item->setIcon(QIcon(screen.thumbnail));
        }
        
        d->screenList->addItem(item);
    }
}

void ScreenSelector::updateWindowList()
{
    d->windowList->clear();
    
    for (const auto& window : d->availableWindows) {
        QListWidgetItem* item = new QListWidgetItem;
        
        QString text = window.title;
        text += QString("\n%1").arg(window.processName);
        text += QString("\n%1x%2").arg(window.geometry.width()).arg(window.geometry.height());
        
        item->setText(text);
        item->setData(Qt::UserRole, window.id);
        
        if (d->showThumbnails && !window.thumbnail.isNull()) {
            item->setIcon(QIcon(window.thumbnail));
        }
        
        d->windowList->addItem(item);
    }
}

void ScreenSelector::filterWindowList(const QString& filter)
{
    for (int i = 0; i < d->windowList->count(); ++i) {
        QListWidgetItem* item = d->windowList->item(i);
        bool visible = filter.isEmpty() || 
                      item->text().contains(filter, Qt::CaseInsensitive);
        item->setHidden(!visible);
    }
}

QPixmap ScreenSelector::createScreenThumbnail(const ScreenInfo& screen)
{
    // 创建屏幕缩略图
    QScreen* qscreen = nullptr;
    QList<QScreen*> screens = QApplication::screens();
    
    for (QScreen* s : screens) {
        if (s->name() == screen.name) {
            qscreen = s;
            break;
        }
    }
    
    if (!qscreen) {
        return QPixmap();
    }
    
    QPixmap screenshot = qscreen->grabWindow(0);
    return screenshot.scaled(d->thumbnailSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

QPixmap ScreenSelector::createWindowThumbnail(const WindowInfo& window)
{
    // 创建窗口缩略图
    // 这里需要平台特定的实现来捕获窗口内容
    // 暂时返回一个占位符
    
    QPixmap placeholder(d->thumbnailSize);
    placeholder.fill(Qt::lightGray);
    
    QPainter painter(&placeholder);
    painter.setPen(Qt::black);
    painter.drawRect(placeholder.rect().adjusted(1, 1, -1, -1));
    painter.drawText(placeholder.rect(), Qt::AlignCenter, window.title);
    
    return placeholder;
}

void ScreenSelector::updateSelection()
{
    emit selectedSourceChanged(d->selectedSource);
    
    switch (d->selectionType) {
    case ScreenSelection:
        if (!d->selectedSource.isEmpty()) {
            ScreenInfo screen = selectedScreen();
            if (!screen.id.isEmpty()) {
                emit screenSelected(screen);
            }
        }
        break;
        
    case WindowSelection:
        if (!d->selectedSource.isEmpty()) {
            WindowInfo window = selectedWindow();
            if (!window.id.isEmpty()) {
                emit windowSelected(window);
            }
        }
        break;
        
    case RegionSelection:
        if (!d->selectedRegion.isEmpty()) {
            emit regionSelected(d->selectedRegion);
        }
        break;
    }
}
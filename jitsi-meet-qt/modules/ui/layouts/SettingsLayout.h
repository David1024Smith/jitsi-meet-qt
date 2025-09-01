#ifndef SETTINGSLAYOUT_H
#define SETTINGSLAYOUT_H

#include "BaseLayout.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QStackedWidget>

/**
 * @brief 设置布局管理器
 * 
 * SettingsLayout专门用于管理设置界面的布局，
 * 包括设置分类树、设置面板和按钮区域的布局。
 */
class SettingsLayout : public BaseLayout
{
    Q_OBJECT
    Q_PROPERTY(bool categoryTreeVisible READ isCategoryTreeVisible WRITE setCategoryTreeVisible NOTIFY categoryTreeVisibleChanged)
    Q_PROPERTY(int categoryTreeWidth READ categoryTreeWidth WRITE setCategoryTreeWidth NOTIFY categoryTreeWidthChanged)
    Q_PROPERTY(bool buttonAreaVisible READ isButtonAreaVisible WRITE setButtonAreaVisible NOTIFY buttonAreaVisibleChanged)
    Q_PROPERTY(int buttonAreaHeight READ buttonAreaHeight WRITE setButtonAreaHeight NOTIFY buttonAreaHeightChanged)

public:
    enum SettingsRegion {
        CategoryTreeRegion,
        SettingsPanelRegion,
        ButtonAreaRegion,
        HeaderRegion,
        FooterRegion
    };
    Q_ENUM(SettingsRegion)

    enum LayoutStyle {
        TreeAndPanelStyle,
        TabsStyle,
        WizardStyle,
        CompactStyle
    };
    Q_ENUM(LayoutStyle)

    explicit SettingsLayout(QWidget *parent = nullptr);
    ~SettingsLayout() override;

    // BaseLayout接口实现
    QString layoutName() const override;
    QString layoutDisplayName() const override;
    QString layoutDescription() const override;
    bool initialize() override;
    bool apply(QWidget* widget) override;
    void cleanup() override;

    // 布局样式管理
    LayoutStyle layoutStyle() const;
    void setLayoutStyle(LayoutStyle style);
    QStringList availableLayoutStyles() const;

    // 区域管理
    bool setRegionWidget(SettingsRegion region, QWidget* widget);
    QWidget* getRegionWidget(SettingsRegion region) const;
    bool removeRegionWidget(SettingsRegion region);
    bool isRegionVisible(SettingsRegion region) const;
    void setRegionVisible(SettingsRegion region, bool visible);

    // 分类树管理
    bool isCategoryTreeVisible() const;
    void setCategoryTreeVisible(bool visible);
    int categoryTreeWidth() const;
    void setCategoryTreeWidth(int width);
    QTreeWidget* categoryTree() const;
    void setCategoryTree(QTreeWidget* tree);

    // 设置面板管理
    QStackedWidget* settingsPanel() const;
    void setSettingsPanel(QStackedWidget* panel);
    int currentPanelIndex() const;
    void setCurrentPanelIndex(int index);
    QWidget* currentPanel() const;
    void setCurrentPanel(QWidget* panel);

    // 设置页面管理
    bool addSettingsPage(const QString& category, const QString& title, QWidget* page);
    bool removeSettingsPage(const QString& category, const QString& title);
    QWidget* getSettingsPage(const QString& category, const QString& title) const;
    QStringList getSettingsCategories() const;
    QStringList getSettingsPages(const QString& category) const;

    // 按钮区域管理
    bool isButtonAreaVisible() const;
    void setButtonAreaVisible(bool visible);
    int buttonAreaHeight() const;
    void setButtonAreaHeight(int height);
    QWidget* buttonArea() const;
    void setButtonArea(QWidget* area);

    // 导航管理
    bool navigateToCategory(const QString& category);
    bool navigateToPage(const QString& category, const QString& page);
    QString currentCategory() const;
    QString currentPage() const;

    // 搜索功能
    void setSearchEnabled(bool enabled);
    bool isSearchEnabled() const;
    void setSearchWidget(QWidget* searchWidget);
    QWidget* searchWidget() const;

    // 布局配置
    QVariantMap getLayoutConfiguration() const override;
    void setLayoutConfiguration(const QVariantMap& config) override;

    // 响应式设计
    bool adaptToSize(const QSize& size) override;
    bool isResponsive() const override;
    void setResponsive(bool responsive) override;

signals:
    void categoryTreeVisibleChanged(bool visible);
    void categoryTreeWidthChanged(int width);
    void buttonAreaVisibleChanged(bool visible);
    void buttonAreaHeightChanged(int height);
    void layoutStyleChanged(LayoutStyle style);
    void currentPanelChanged(int index);
    void currentCategoryChanged(const QString& category);
    void currentPageChanged(const QString& page);
    void settingsPageAdded(const QString& category, const QString& title);
    void settingsPageRemoved(const QString& category, const QString& title);
    void navigationRequested(const QString& category, const QString& page);

protected:
    // BaseLayout虚函数实现
    void onThemeChanged(std::shared_ptr<BaseTheme> theme) override;
    void onConfigurationChanged(const QVariantMap& config) override;
    QVariantMap getDefaultConfiguration() const override;
    bool validateConfiguration(const QVariantMap& config) const override;

    // 布局更新
    void updateLayout() override;
    void updateGeometry() override;
    void updateSpacing() override;
    void updateMargins() override;

private slots:
    void onCategorySelectionChanged();
    void onSplitterMoved(int pos, int index);
    void onRegionWidgetDestroyed();

private:
    void setupLayout();
    void createRegions();
    void arrangeRegions();
    void arrangeTreeAndPanelStyle();
    void arrangeTabsStyle();
    void arrangeWizardStyle();
    void arrangeCompactStyle();
    void updateLayoutStyle();
    void updateRegionVisibility();
    void updateCategoryTree();
    void updateSettingsPanel();
    void connectSignals();

    LayoutStyle m_layoutStyle;
    bool m_responsive;
    bool m_searchEnabled;

    // 布局组件
    QSplitter* m_mainSplitter;
    QHBoxLayout* m_mainLayout;
    QVBoxLayout* m_contentLayout;

    // 区域组件
    QTreeWidget* m_categoryTree;
    QStackedWidget* m_settingsPanel;
    QWidget* m_buttonArea;
    QWidget* m_headerWidget;
    QWidget* m_footerWidget;
    QWidget* m_searchWidget;

    // 区域状态
    QMap<SettingsRegion, QWidget*> m_regionWidgets;
    QMap<SettingsRegion, bool> m_regionVisibility;

    // 设置页面管理
    struct SettingsPageInfo {
        QString category;
        QString title;
        QWidget* widget;
        int index;
    };
    QList<SettingsPageInfo> m_settingsPages;
    QMap<QString, QStringList> m_categoryPages;

    // 导航状态
    QString m_currentCategory;
    QString m_currentPage;
    int m_currentPanelIndex;

    // 尺寸设置
    int m_categoryTreeWidth;
    int m_buttonAreaHeight;
};

#endif // SETTINGSLAYOUT_H
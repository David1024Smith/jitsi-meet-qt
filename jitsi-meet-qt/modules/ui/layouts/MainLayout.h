#ifndef MAINLAYOUT_H
#define MAINLAYOUT_H

#include "BaseLayout.h"
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

class ToolBar;
class StatusBar;

/**
 * @brief 主布局管理器
 * 
 * MainLayout负责管理应用程序的主窗口布局，包括工具栏、
 * 状态栏、中央区域和侧边栏的布局管理。
 */
class MainLayout : public BaseLayout
{
    Q_OBJECT
    Q_PROPERTY(bool toolBarVisible READ isToolBarVisible WRITE setToolBarVisible NOTIFY toolBarVisibleChanged)
    Q_PROPERTY(bool statusBarVisible READ isStatusBarVisible WRITE setStatusBarVisible NOTIFY statusBarVisibleChanged)
    Q_PROPERTY(bool sideBarVisible READ isSideBarVisible WRITE setSideBarVisible NOTIFY sideBarVisibleChanged)
    Q_PROPERTY(int sideBarWidth READ sideBarWidth WRITE setSideBarWidth NOTIFY sideBarWidthChanged)

public:
    enum LayoutArea {
        ToolBarArea,
        StatusBarArea,
        CentralArea,
        SideBarArea,
        HeaderArea,
        FooterArea
    };
    Q_ENUM(LayoutArea)

    explicit MainLayout(QWidget *parent = nullptr);
    ~MainLayout() override;

    // BaseLayout接口实现
    QString layoutName() const override;
    QString layoutDisplayName() const override;
    QString layoutDescription() const override;
    bool initialize() override;
    bool apply(QWidget* widget) override;
    void cleanup() override;

    // 区域管理
    bool setAreaWidget(LayoutArea area, QWidget* widget);
    QWidget* getAreaWidget(LayoutArea area) const;
    bool removeAreaWidget(LayoutArea area);
    bool isAreaVisible(LayoutArea area) const;
    void setAreaVisible(LayoutArea area, bool visible);

    // 工具栏管理
    bool isToolBarVisible() const;
    void setToolBarVisible(bool visible);
    ToolBar* toolBar() const;
    void setToolBar(ToolBar* toolBar);

    // 状态栏管理
    bool isStatusBarVisible() const;
    void setStatusBarVisible(bool visible);
    StatusBar* statusBar() const;
    void setStatusBar(StatusBar* statusBar);

    // 侧边栏管理
    bool isSideBarVisible() const;
    void setSideBarVisible(bool visible);
    int sideBarWidth() const;
    void setSideBarWidth(int width);
    QWidget* sideBar() const;
    void setSideBar(QWidget* sideBar);

    // 中央区域管理
    QWidget* centralWidget() const;
    void setCentralWidget(QWidget* widget);

    // 布局配置
    QVariantMap getLayoutConfiguration() const override;
    void setLayoutConfiguration(const QVariantMap& config) override;

    // 响应式设计
    bool adaptToSize(const QSize& size) override;
    bool isResponsive() const override;
    void setResponsive(bool responsive) override;

signals:
    void toolBarVisibleChanged(bool visible);
    void statusBarVisibleChanged(bool visible);
    void sideBarVisibleChanged(bool visible);
    void sideBarWidthChanged(int width);
    void areaWidgetChanged(LayoutArea area, QWidget* widget);
    void centralWidgetChanged(QWidget* widget);

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
    void onSplitterMoved(int pos, int index);
    void onAreaWidgetDestroyed();

private:
    void setupLayout();
    void createAreas();
    void arrangeAreas();
    void updateAreaVisibility();
    void updateSplitterSizes();
    void connectSignals();

    // 布局组件
    QMainWindow* m_mainWindow;
    QSplitter* m_mainSplitter;
    QSplitter* m_contentSplitter;

    // 区域组件
    ToolBar* m_toolBar;
    StatusBar* m_statusBar;
    QWidget* m_centralWidget;
    QWidget* m_sideBar;
    QWidget* m_headerWidget;
    QWidget* m_footerWidget;

    // 布局状态
    bool m_toolBarVisible;
    bool m_statusBarVisible;
    bool m_sideBarVisible;
    int m_sideBarWidth;
    bool m_responsive;

    // 区域映射
    QMap<LayoutArea, QWidget*> m_areaWidgets;
    QMap<LayoutArea, bool> m_areaVisibility;
};

#endif // MAINLAYOUT_H
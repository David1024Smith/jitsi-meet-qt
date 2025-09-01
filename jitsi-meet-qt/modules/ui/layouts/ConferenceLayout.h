#ifndef CONFERENCELAYOUT_H
#define CONFERENCELAYOUT_H

#include "BaseLayout.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>

/**
 * @brief 会议布局管理器
 * 
 * ConferenceLayout专门用于管理视频会议界面的布局，
 * 包括视频网格、聊天面板、控制面板等区域的布局。
 */
class ConferenceLayout : public BaseLayout
{
    Q_OBJECT
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)
    Q_PROPERTY(int gridColumns READ gridColumns WRITE setGridColumns NOTIFY gridColumnsChanged)
    Q_PROPERTY(int gridRows READ gridRows WRITE setGridRows NOTIFY gridRowsChanged)
    Q_PROPERTY(bool chatPanelVisible READ isChatPanelVisible WRITE setChatPanelVisible NOTIFY chatPanelVisibleChanged)
    Q_PROPERTY(bool controlPanelVisible READ isControlPanelVisible WRITE setControlPanelVisible NOTIFY controlPanelVisibleChanged)

public:
    enum ViewMode {
        GridView,
        SpeakerView,
        PresentationView,
        FullScreenView
    };
    Q_ENUM(ViewMode)

    enum LayoutRegion {
        VideoGridRegion,
        MainVideoRegion,
        ChatPanelRegion,
        ControlPanelRegion,
        ParticipantListRegion,
        ScreenShareRegion,
        ToolbarRegion
    };
    Q_ENUM(LayoutRegion)

    explicit ConferenceLayout(QWidget *parent = nullptr);
    ~ConferenceLayout() override;

    // BaseLayout接口实现
    QString layoutName() const override;
    QString layoutDisplayName() const override;
    QString layoutDescription() const override;
    bool initialize() override;
    bool apply(QWidget* widget) override;
    void cleanup() override;

    // 视图模式管理
    ViewMode viewMode() const;
    void setViewMode(ViewMode mode);
    QStringList availableViewModes() const;

    // 网格布局管理
    int gridColumns() const;
    void setGridColumns(int columns);
    int gridRows() const;
    void setGridRows(int rows);
    void setGridSize(int columns, int rows);
    int maxGridItems() const;

    // 区域管理
    bool setRegionWidget(LayoutRegion region, QWidget* widget);
    QWidget* getRegionWidget(LayoutRegion region) const;
    bool removeRegionWidget(LayoutRegion region);
    bool isRegionVisible(LayoutRegion region) const;
    void setRegionVisible(LayoutRegion region, bool visible);

    // 视频区域管理
    bool addVideoWidget(QWidget* videoWidget, int position = -1);
    bool removeVideoWidget(QWidget* videoWidget);
    bool removeVideoWidget(int position);
    QList<QWidget*> videoWidgets() const;
    int videoWidgetCount() const;
    void clearVideoWidgets();

    // 主视频管理
    QWidget* mainVideoWidget() const;
    void setMainVideoWidget(QWidget* widget);
    bool isMainVideoVisible() const;
    void setMainVideoVisible(bool visible);

    // 聊天面板管理
    bool isChatPanelVisible() const;
    void setChatPanelVisible(bool visible);
    QWidget* chatPanel() const;
    void setChatPanel(QWidget* panel);
    int chatPanelWidth() const;
    void setChatPanelWidth(int width);

    // 控制面板管理
    bool isControlPanelVisible() const;
    void setControlPanelVisible(bool visible);
    QWidget* controlPanel() const;
    void setControlPanel(QWidget* panel);
    int controlPanelHeight() const;
    void setControlPanelHeight(int height);

    // 参与者列表管理
    bool isParticipantListVisible() const;
    void setParticipantListVisible(bool visible);
    QWidget* participantList() const;
    void setParticipantList(QWidget* list);

    // 屏幕共享管理
    bool isScreenShareVisible() const;
    void setScreenShareVisible(bool visible);
    QWidget* screenShareWidget() const;
    void setScreenShareWidget(QWidget* widget);

    // 布局配置
    QVariantMap getLayoutConfiguration() const override;
    void setLayoutConfiguration(const QVariantMap& config) override;

    // 响应式设计
    bool adaptToSize(const QSize& size) override;
    bool isResponsive() const override;
    void setResponsive(bool responsive) override;

signals:
    void viewModeChanged(ViewMode mode);
    void gridColumnsChanged(int columns);
    void gridRowsChanged(int rows);
    void chatPanelVisibleChanged(bool visible);
    void controlPanelVisibleChanged(bool visible);
    void participantListVisibleChanged(bool visible);
    void screenShareVisibleChanged(bool visible);
    void videoWidgetAdded(QWidget* widget, int position);
    void videoWidgetRemoved(QWidget* widget, int position);
    void mainVideoChanged(QWidget* widget);
    void regionWidgetChanged(LayoutRegion region, QWidget* widget);

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
    void onVideoWidgetDestroyed();
    void onRegionWidgetDestroyed();

private:
    void setupLayout();
    void createRegions();
    void arrangeRegions();
    void arrangeGridView();
    void arrangeSpeakerView();
    void arrangePresentationView();
    void arrangeFullScreenView();
    void updateViewMode();
    void updateVideoGrid();
    void updateRegionVisibility();
    void calculateOptimalGridSize();
    void connectSignals();

    ViewMode m_viewMode;
    int m_gridColumns;
    int m_gridRows;
    bool m_responsive;

    // 布局组件
    QSplitter* m_mainSplitter;
    QSplitter* m_videoSplitter;
    QGridLayout* m_videoGridLayout;
    QVBoxLayout* m_mainLayout;

    // 区域组件
    QWidget* m_videoGridWidget;
    QWidget* m_mainVideoWidget;
    QWidget* m_chatPanel;
    QWidget* m_controlPanel;
    QWidget* m_participantList;
    QWidget* m_screenShareWidget;
    QWidget* m_toolbarWidget;

    // 视频组件列表
    QList<QWidget*> m_videoWidgets;

    // 区域状态
    QMap<LayoutRegion, QWidget*> m_regionWidgets;
    QMap<LayoutRegion, bool> m_regionVisibility;

    // 面板尺寸
    int m_chatPanelWidth;
    int m_controlPanelHeight;
};

#endif // CONFERENCELAYOUT_H
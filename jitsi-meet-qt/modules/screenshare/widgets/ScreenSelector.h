#ifndef SCREENSELECTOR_H
#define SCREENSELECTOR_H

#include <QWidget>
#include <QListWidget>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTabWidget>

/**
 * @brief 屏幕选择器组件
 * 
 * 提供屏幕、窗口和区域选择功能
 */
class ScreenSelector : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(SelectionType selectionType READ selectionType WRITE setSelectionType NOTIFY selectionTypeChanged)
    Q_PROPERTY(QString selectedSource READ selectedSource NOTIFY selectedSourceChanged)

public:
    /**
     * @brief 选择类型枚举
     */
    enum SelectionType {
        ScreenSelection,    ///< 屏幕选择
        WindowSelection,    ///< 窗口选择
        RegionSelection     ///< 区域选择
    };
    Q_ENUM(SelectionType)

    /**
     * @brief 屏幕信息结构
     */
    struct ScreenInfo {
        QString id;
        QString name;
        QRect geometry;
        bool isPrimary;
        QPixmap thumbnail;
    };

    /**
     * @brief 窗口信息结构
     */
    struct WindowInfo {
        QString id;
        QString title;
        QString processName;
        QRect geometry;
        bool isVisible;
        QPixmap thumbnail;
    };

    explicit ScreenSelector(QWidget *parent = nullptr);
    virtual ~ScreenSelector();

    // 选择类型接口
    SelectionType selectionType() const;
    void setSelectionType(SelectionType type);

    // 选择结果接口
    QString selectedSource() const;
    QRect selectedRegion() const;
    bool hasSelection() const;

    // 屏幕管理接口
    QList<ScreenInfo> availableScreens() const;
    void setAvailableScreens(const QList<ScreenInfo>& screens);
    ScreenInfo selectedScreen() const;

    // 窗口管理接口
    QList<WindowInfo> availableWindows() const;
    void setAvailableWindows(const QList<WindowInfo>& windows);
    WindowInfo selectedWindow() const;

    // 区域选择接口
    void setCustomRegion(const QRect& region);
    QRect customRegion() const;
    void startInteractiveSelection();
    void cancelInteractiveSelection();

    // UI配置接口
    void setThumbnailSize(const QSize& size);
    QSize thumbnailSize() const;
    void setShowThumbnails(bool show);
    bool isShowThumbnails() const;
    void setAllowMultipleSelection(bool allow);
    bool isMultipleSelectionAllowed() const;

public slots:
    /**
     * @brief 刷新屏幕列表
     */
    void refreshScreens();

    /**
     * @brief 刷新窗口列表
     */
    void refreshWindows();

    /**
     * @brief 清除选择
     */
    void clearSelection();

    /**
     * @brief 选择主屏幕
     */
    void selectPrimaryScreen();

    /**
     * @brief 自动选择最佳源
     */
    void autoSelectBestSource();

signals:
    /**
     * @brief 选择类型改变信号
     * @param type 新的选择类型
     */
    void selectionTypeChanged(SelectionType type);

    /**
     * @brief 选择源改变信号
     * @param source 选择的源标识
     */
    void selectedSourceChanged(const QString& source);

    /**
     * @brief 屏幕选择信号
     * @param screen 选择的屏幕信息
     */
    void screenSelected(const ScreenInfo& screen);

    /**
     * @brief 窗口选择信号
     * @param window 选择的窗口信息
     */
    void windowSelected(const WindowInfo& window);

    /**
     * @brief 区域选择信号
     * @param region 选择的区域
     */
    void regionSelected(const QRect& region);

    /**
     * @brief 交互式选择开始信号
     */
    void interactiveSelectionStarted();

    /**
     * @brief 交互式选择完成信号
     * @param region 选择的区域
     */
    void interactiveSelectionFinished(const QRect& region);

    /**
     * @brief 选择取消信号
     */
    void selectionCancelled();

private slots:
    void onTabChanged(int index);
    void onScreenItemClicked();
    void onWindowItemClicked();
    void onRegionButtonClicked();
    void onInteractiveSelectionClicked();
    void onRefreshScreensClicked();
    void onRefreshWindowsClicked();
    void onFilterTextChanged(const QString& text);

private:
    void setupUI();
    void setupScreenTab();
    void setupWindowTab();
    void setupRegionTab();
    void connectSignals();
    void updateScreenList();
    void updateWindowList();
    void filterWindowList(const QString& filter);
    QPixmap createScreenThumbnail(const ScreenInfo& screen);
    QPixmap createWindowThumbnail(const WindowInfo& window);
    void updateSelection();

    class Private;
    Private* d;
};

// 注册元类型
Q_DECLARE_METATYPE(ScreenSelector::ScreenInfo)
Q_DECLARE_METATYPE(ScreenSelector::WindowInfo)

#endif // SCREENSELECTOR_H
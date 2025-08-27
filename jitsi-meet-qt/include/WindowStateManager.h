#ifndef WINDOWSTATEMANAGER_H
#define WINDOWSTATEMANAGER_H

#include <QObject>
#include <QRect>
#include <QScreen>
#include <QWidget>

class ConfigurationManager;

/**
 * @brief 窗口状态管理器，负责管理窗口几何状态
 * 
 * 该类提供窗口大小、位置、最大化状态的保存和恢复功能，
 * 并处理多显示器环境下的窗口状态验证。
 */
class WindowStateManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 窗口状态结构体
     */
    struct WindowState {
        QRect geometry;          ///< 窗口几何信息
        bool maximized;          ///< 是否最大化
        bool valid;              ///< 状态是否有效
        QString screenName;      ///< 关联的屏幕名称
        
        WindowState() : maximized(false), valid(false) {}
        
        bool operator==(const WindowState& other) const {
            return geometry == other.geometry && 
                   maximized == other.maximized && 
                   valid == other.valid &&
                   screenName == other.screenName;
        }
    };

    /**
     * @brief 构造函数
     * @param configManager 配置管理器实例
     * @param parent 父对象
     */
    explicit WindowStateManager(ConfigurationManager* configManager, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~WindowStateManager();

    /**
     * @brief 保存窗口状态
     * @param widget 要保存状态的窗口
     * @return 是否保存成功
     */
    bool saveWindowState(QWidget* widget);

    /**
     * @brief 恢复窗口状态
     * @param widget 要恢复状态的窗口
     * @return 是否恢复成功
     */
    bool restoreWindowState(QWidget* widget);

    /**
     * @brief 获取当前窗口状态
     * @param widget 窗口对象
     * @return 窗口状态
     */
    WindowState getCurrentWindowState(QWidget* widget) const;

    /**
     * @brief 验证窗口状态
     * @param state 要验证的窗口状态
     * @return 验证后的窗口状态
     */
    WindowState validateWindowState(const WindowState& state) const;

    /**
     * @brief 检查窗口是否在可见区域内
     * @param geometry 窗口几何信息
     * @return 是否在可见区域内
     */
    bool isWindowVisible(const QRect& geometry) const;

    /**
     * @brief 获取最佳屏幕
     * @param geometry 窗口几何信息
     * @return 最佳屏幕对象，如果没有找到则返回nullptr
     */
    QScreen* getBestScreen(const QRect& geometry) const;

    /**
     * @brief 调整窗口到屏幕范围内
     * @param geometry 原始窗口几何信息
     * @param screen 目标屏幕，如果为nullptr则使用主屏幕
     * @return 调整后的窗口几何信息
     */
    QRect adjustToScreen(const QRect& geometry, QScreen* screen = nullptr) const;

    /**
     * @brief 获取默认窗口状态
     * @return 默认窗口状态
     */
    WindowState getDefaultWindowState() const;

    /**
     * @brief 检查是否启用窗口状态记忆功能
     * @return 是否启用
     */
    bool isRememberWindowStateEnabled() const;

    /**
     * @brief 设置是否启用窗口状态记忆功能
     * @param enabled 是否启用
     */
    void setRememberWindowStateEnabled(bool enabled);

signals:
    /**
     * @brief 窗口状态已保存信号
     * @param state 保存的窗口状态
     */
    void windowStateSaved(const WindowState& state);

    /**
     * @brief 窗口状态已恢复信号
     * @param state 恢复的窗口状态
     */
    void windowStateRestored(const WindowState& state);

    /**
     * @brief 窗口状态验证失败信号
     * @param originalState 原始状态
     * @param correctedState 修正后的状态
     */
    void windowStateValidationFailed(const WindowState& originalState, const WindowState& correctedState);

private slots:
    /**
     * @brief 屏幕配置改变处理
     */
    void onScreenConfigurationChanged();

private:
    /**
     * @brief 验证窗口几何信息
     * @param geometry 要验证的几何信息
     * @return 验证后的几何信息
     */
    QRect validateGeometry(const QRect& geometry) const;

    /**
     * @brief 获取屏幕名称
     * @param screen 屏幕对象
     * @return 屏幕名称
     */
    QString getScreenName(QScreen* screen) const;

    /**
     * @brief 根据名称查找屏幕
     * @param screenName 屏幕名称
     * @return 屏幕对象，如果没有找到则返回nullptr
     */
    QScreen* findScreenByName(const QString& screenName) const;

    /**
     * @brief 计算窗口与屏幕的重叠面积
     * @param geometry 窗口几何信息
     * @param screen 屏幕对象
     * @return 重叠面积
     */
    int calculateOverlapArea(const QRect& geometry, QScreen* screen) const;

    /**
     * @brief 确保窗口大小在有效范围内
     * @param size 窗口大小
     * @return 调整后的窗口大小
     */
    QSize ensureValidSize(const QSize& size) const;

    /**
     * @brief 确保窗口位置在屏幕范围内
     * @param geometry 窗口几何信息
     * @param screen 屏幕对象
     * @return 调整后的窗口几何信息
     */
    QRect ensureOnScreen(const QRect& geometry, QScreen* screen) const;

private:
    ConfigurationManager* m_configManager;  ///< 配置管理器
    bool m_rememberWindowState;             ///< 是否记住窗口状态
};

Q_DECLARE_METATYPE(WindowStateManager::WindowState)

#endif // WINDOWSTATEMANAGER_H
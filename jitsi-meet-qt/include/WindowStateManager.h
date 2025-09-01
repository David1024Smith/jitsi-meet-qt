#ifndef WINDOWSTATEMANAGER_H
#define WINDOWSTATEMANAGER_H

#include <QObject>
#include <QWidget>
#include <QRect>
#include <QSettings>

/**
 * @brief 窗口状态管理器
 * 
 * 负责保存和恢复窗口的位置、大小等状态信息
 */
class WindowStateManager : public QObject
{
    Q_OBJECT

public:
    explicit WindowStateManager(QObject* parent = nullptr);
    ~WindowStateManager();

    /**
     * @brief 保存窗口状态
     * @param widget 窗口控件
     * @param key 保存键名
     */
    void saveWindowState(QWidget* widget, const QString& key);

    /**
     * @brief 恢复窗口状态
     * @param widget 窗口控件
     * @param key 保存键名
     */
    void restoreWindowState(QWidget* widget, const QString& key);

    /**
     * @brief 保存窗口几何信息
     * @param widget 窗口控件
     * @param key 保存键名
     */
    void saveGeometry(QWidget* widget, const QString& key);

    /**
     * @brief 恢复窗口几何信息
     * @param widget 窗口控件
     * @param key 保存键名
     */
    void restoreGeometry(QWidget* widget, const QString& key);

private:
    QSettings* m_settings;
};

#endif // WINDOWSTATEMANAGER_H
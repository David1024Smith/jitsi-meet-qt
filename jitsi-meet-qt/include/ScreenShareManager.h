#ifndef SCREENSHAREMANAGER_H
#define SCREENSHAREMANAGER_H

#include <QObject>
#include <QString>
#include <QRect>
#include <QVideoWidget>

/**
 * @brief 屏幕共享管理器
 * 
 * 负责管理屏幕共享功能
 */
class ScreenShareManager : public QObject
{
    Q_OBJECT

public:
    explicit ScreenShareManager(QObject* parent = nullptr);
    ~ScreenShareManager();

    /**
     * @brief 开始屏幕共享
     * @param screenId 屏幕ID（可选）
     * @return 是否成功开始
     */
    bool startScreenShare(int screenId = -1);

    /**
     * @brief 停止屏幕共享
     */
    void stopScreenShare();

    /**
     * @brief 检查是否正在共享屏幕
     * @return 是否正在共享
     */
    bool isScreenSharing() const;

    /**
     * @brief 获取可用屏幕列表
     * @return 屏幕信息列表
     */
    QStringList getAvailableScreens() const;

    /**
     * @brief 显示屏幕选择对话框
     * @return 是否选择了屏幕
     */
    bool showScreenSelectionDialog();

    /**
     * @brief 获取本地屏幕共享控件
     * @return 视频控件
     */
    QVideoWidget* localScreenShareWidget() const;

signals:
    /**
     * @brief 屏幕共享开始信号
     */
    void screenShareStarted();

    /**
     * @brief 屏幕共享停止信号
     */
    void screenShareStopped();

    /**
     * @brief 屏幕共享错误信号
     * @param error 错误信息
     */
    void screenShareError(const QString& error);

    /**
     * @brief 远程屏幕共享接收信号
     * @param participantId 参与者ID
     * @param widget 视频控件
     */
    void remoteScreenShareReceived(const QString& participantId, QVideoWidget* widget);

    /**
     * @brief 远程屏幕共享移除信号
     * @param participantId 参与者ID
     */
    void remoteScreenShareRemoved(const QString& participantId);

private:
    bool m_isSharing;
    int m_currentScreenId;
};

#endif // SCREENSHAREMANAGER_H
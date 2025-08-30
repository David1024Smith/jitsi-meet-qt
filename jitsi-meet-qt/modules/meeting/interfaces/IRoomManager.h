#ifndef IROOMMANAGER_H
#define IROOMMANAGER_H

#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVariantList>

/**
 * @brief 房间管理器接口
 * 
 * 定义会议房间管理的核心功能，包括房间创建、配置和参与者管理
 */
class IRoomManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 房间状态枚举
     */
    enum RoomState {
        Inactive,        ///< 未激活
        Active,          ///< 激活
        Locked,          ///< 锁定
        Full,            ///< 已满
        Closed           ///< 已关闭
    };
    Q_ENUM(RoomState)

    /**
     * @brief 房间类型枚举
     */
    enum RoomType {
        PublicRoom,      ///< 公开房间
        PrivateRoom,     ///< 私人房间
        PasswordRoom,    ///< 密码房间
        InviteOnlyRoom   ///< 仅邀请房间
    };
    Q_ENUM(RoomType)

    /**
     * @brief 权限级别枚举
     */
    enum PermissionLevel {
        NoPermission,    ///< 无权限
        ViewOnly,        ///< 仅查看
        Participate,     ///< 参与
        Moderate,        ///< 主持
        Administrate     ///< 管理
    };
    Q_ENUM(PermissionLevel)

    explicit IRoomManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~IRoomManager() = default;

    /**
     * @brief 创建房间
     * @param roomName 房间名称
     * @param roomType 房间类型
     * @param settings 房间设置
     * @return 创建是否成功
     */
    virtual bool createRoom(const QString& roomName, 
                          RoomType roomType = PublicRoom,
                          const QVariantMap& settings = QVariantMap()) = 0;

    /**
     * @brief 加入房间
     * @param roomId 房间ID
     * @param password 密码（如需要）
     * @return 加入是否成功
     */
    virtual bool joinRoom(const QString& roomId, const QString& password = QString()) = 0;

    /**
     * @brief 离开房间
     * @param roomId 房间ID
     * @return 离开是否成功
     */
    virtual bool leaveRoom(const QString& roomId) = 0;

    /**
     * @brief 获取房间信息
     * @param roomId 房间ID
     * @return 房间信息映射
     */
    virtual QVariantMap getRoomInfo(const QString& roomId) = 0;

    /**
     * @brief 设置房间配置
     * @param roomId 房间ID
     * @param settings 配置映射
     * @return 设置是否成功
     */
    virtual bool setRoomSettings(const QString& roomId, const QVariantMap& settings) = 0;

    /**
     * @brief 获取房间配置
     * @param roomId 房间ID
     * @return 配置映射
     */
    virtual QVariantMap getRoomSettings(const QString& roomId) = 0;

    /**
     * @brief 锁定房间
     * @param roomId 房间ID
     * @param password 密码
     * @return 锁定是否成功
     */
    virtual bool lockRoom(const QString& roomId, const QString& password) = 0;

    /**
     * @brief 解锁房间
     * @param roomId 房间ID
     * @return 解锁是否成功
     */
    virtual bool unlockRoom(const QString& roomId) = 0;

    /**
     * @brief 获取参与者列表
     * @param roomId 房间ID
     * @return 参与者列表
     */
    virtual QVariantList getParticipants(const QString& roomId) = 0;

    /**
     * @brief 邀请参与者
     * @param roomId 房间ID
     * @param participantId 参与者ID
     * @return 邀请是否成功
     */
    virtual bool inviteParticipant(const QString& roomId, const QString& participantId) = 0;

    /**
     * @brief 移除参与者
     * @param roomId 房间ID
     * @param participantId 参与者ID
     * @return 移除是否成功
     */
    virtual bool removeParticipant(const QString& roomId, const QString& participantId) = 0;

    /**
     * @brief 设置参与者权限
     * @param roomId 房间ID
     * @param participantId 参与者ID
     * @param permission 权限级别
     * @return 设置是否成功
     */
    virtual bool setParticipantPermission(const QString& roomId, 
                                        const QString& participantId, 
                                        PermissionLevel permission) = 0;

    /**
     * @brief 获取参与者权限
     * @param roomId 房间ID
     * @param participantId 参与者ID
     * @return 权限级别
     */
    virtual PermissionLevel getParticipantPermission(const QString& roomId, 
                                                   const QString& participantId) = 0;

    /**
     * @brief 静音参与者
     * @param roomId 房间ID
     * @param participantId 参与者ID
     * @param muted 是否静音
     * @return 操作是否成功
     */
    virtual bool muteParticipant(const QString& roomId, 
                               const QString& participantId, 
                               bool muted) = 0;

    /**
     * @brief 获取房间统计信息
     * @param roomId 房间ID
     * @return 统计信息映射
     */
    virtual QVariantMap getRoomStatistics(const QString& roomId) = 0;

signals:
    /**
     * @brief 房间创建信号
     * @param roomId 房间ID
     * @param roomInfo 房间信息
     */
    void roomCreated(const QString& roomId, const QVariantMap& roomInfo);

    /**
     * @brief 房间状态改变信号
     * @param roomId 房间ID
     * @param state 新状态
     */
    void roomStateChanged(const QString& roomId, RoomState state);

    /**
     * @brief 参与者加入房间信号
     * @param roomId 房间ID
     * @param participantInfo 参与者信息
     */
    void participantJoinedRoom(const QString& roomId, const QVariantMap& participantInfo);

    /**
     * @brief 参与者离开房间信号
     * @param roomId 房间ID
     * @param participantId 参与者ID
     */
    void participantLeftRoom(const QString& roomId, const QString& participantId);

    /**
     * @brief 参与者权限改变信号
     * @param roomId 房间ID
     * @param participantId 参与者ID
     * @param permission 新权限
     */
    void participantPermissionChanged(const QString& roomId, 
                                    const QString& participantId, 
                                    PermissionLevel permission);

    /**
     * @brief 房间设置改变信号
     * @param roomId 房间ID
     * @param settings 新设置
     */
    void roomSettingsChanged(const QString& roomId, const QVariantMap& settings);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);
};

#endif // IROOMMANAGER_H
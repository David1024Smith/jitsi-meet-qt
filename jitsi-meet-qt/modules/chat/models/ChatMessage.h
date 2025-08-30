#ifndef CHATMESSAGE_H
#define CHATMESSAGE_H

#include <QObject>
#include <QString>
#include <QDateTime>
#include <QVariantMap>
#include <QUrl>
#include <QUuid>

/**
 * @brief 聊天消息数据模型
 * 
 * ChatMessage表示一条聊天消息，包含消息的所有相关信息
 * 如内容、发送者、时间戳、类型等。
 */
class ChatMessage : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString content READ content WRITE setContent NOTIFY contentChanged)
    Q_PROPERTY(MessageType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString senderId READ senderId WRITE setSenderId NOTIFY senderIdChanged)
    Q_PROPERTY(QString senderName READ senderName WRITE setSenderName NOTIFY senderNameChanged)
    Q_PROPERTY(QString roomId READ roomId WRITE setRoomId NOTIFY roomIdChanged)
    Q_PROPERTY(QDateTime timestamp READ timestamp WRITE setTimestamp NOTIFY timestampChanged)
    Q_PROPERTY(MessageStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(bool isRead READ isRead WRITE setRead NOTIFY readChanged)
    Q_PROPERTY(bool isEdited READ isEdited NOTIFY editedChanged)

public:
    /**
     * @brief 消息类型枚举
     */
    enum MessageType {
        TextMessage,        ///< 文本消息
        EmojiMessage,       ///< 表情消息
        FileMessage,        ///< 文件消息
        ImageMessage,       ///< 图片消息
        VideoMessage,       ///< 视频消息
        AudioMessage,       ///< 音频消息
        SystemMessage,      ///< 系统消息
        NotificationMessage,///< 通知消息
        JoinMessage,        ///< 加入消息
        LeaveMessage        ///< 离开消息
    };
    Q_ENUM(MessageType)

    /**
     * @brief 消息状态枚举
     */
    enum MessageStatus {
        Pending,            ///< 待发送
        Sending,            ///< 发送中
        Sent,               ///< 已发送
        Delivered,          ///< 已送达
        Read,               ///< 已读
        Failed,             ///< 发送失败
        Deleted             ///< 已删除
    };
    Q_ENUM(MessageStatus)

    /**
     * @brief 消息优先级枚举
     */
    enum MessagePriority {
        Low = 0,            ///< 低优先级
        Normal = 1,         ///< 普通优先级
        High = 2,           ///< 高优先级
        Critical = 3        ///< 关键优先级
    };
    Q_ENUM(MessagePriority)

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit ChatMessage(QObject *parent = nullptr);

    /**
     * @brief 构造函数
     * @param content 消息内容
     * @param senderId 发送者ID
     * @param roomId 房间ID
     * @param type 消息类型
     * @param parent 父对象
     */
    explicit ChatMessage(const QString& content, const QString& senderId, const QString& roomId, MessageType type = TextMessage, QObject *parent = nullptr);

    /**
     * @brief 拷贝构造函数
     * @param other 其他消息对象
     */
    ChatMessage(const ChatMessage& other);

    /**
     * @brief 赋值操作符
     * @param other 其他消息对象
     * @return 当前对象引用
     */
    ChatMessage& operator=(const ChatMessage& other);

    /**
     * @brief 析构函数
     */
    ~ChatMessage();

    /**
     * @brief 获取消息ID
     * @return 消息ID
     */
    QString id() const;

    /**
     * @brief 获取消息内容
     * @return 消息内容
     */
    QString content() const;

    /**
     * @brief 设置消息内容
     * @param content 消息内容
     */
    void setContent(const QString& content);

    /**
     * @brief 获取消息类型
     * @return 消息类型
     */
    MessageType type() const;

    /**
     * @brief 设置消息类型
     * @param type 消息类型
     */
    void setType(MessageType type);

    /**
     * @brief 获取发送者ID
     * @return 发送者ID
     */
    QString senderId() const;

    /**
     * @brief 设置发送者ID
     * @param senderId 发送者ID
     */
    void setSenderId(const QString& senderId);

    /**
     * @brief 获取发送者名称
     * @return 发送者名称
     */
    QString senderName() const;

    /**
     * @brief 设置发送者名称
     * @param senderName 发送者名称
     */
    void setSenderName(const QString& senderName);

    /**
     * @brief 获取房间ID
     * @return 房间ID
     */
    QString roomId() const;

    /**
     * @brief 设置房间ID
     * @param roomId 房间ID
     */
    void setRoomId(const QString& roomId);

    /**
     * @brief 获取时间戳
     * @return 时间戳
     */
    QDateTime timestamp() const;

    /**
     * @brief 设置时间戳
     * @param timestamp 时间戳
     */
    void setTimestamp(const QDateTime& timestamp);

    /**
     * @brief 获取消息状态
     * @return 消息状态
     */
    MessageStatus status() const;

    /**
     * @brief 设置消息状态
     * @param status 消息状态
     */
    void setStatus(MessageStatus status);

    /**
     * @brief 获取消息优先级
     * @return 消息优先级
     */
    MessagePriority priority() const;

    /**
     * @brief 设置消息优先级
     * @param priority 消息优先级
     */
    void setPriority(MessagePriority priority);

    /**
     * @brief 检查消息是否已读
     * @return 是否已读
     */
    bool isRead() const;

    /**
     * @brief 设置消息已读状态
     * @param read 是否已读
     */
    void setRead(bool read);

    /**
     * @brief 检查消息是否已编辑
     * @return 是否已编辑
     */
    bool isEdited() const;

    /**
     * @brief 获取编辑时间
     * @return 编辑时间
     */
    QDateTime editedTimestamp() const;

    /**
     * @brief 编辑消息内容
     * @param newContent 新内容
     */
    void editContent(const QString& newContent);

    /**
     * @brief 获取文件信息（仅文件消息）
     * @return 文件信息映射
     */
    QVariantMap fileInfo() const;

    /**
     * @brief 设置文件信息（仅文件消息）
     * @param fileInfo 文件信息
     */
    void setFileInfo(const QVariantMap& fileInfo);

    /**
     * @brief 获取文件URL（仅文件消息）
     * @return 文件URL
     */
    QUrl fileUrl() const;

    /**
     * @brief 设置文件URL（仅文件消息）
     * @param url 文件URL
     */
    void setFileUrl(const QUrl& url);

    /**
     * @brief 获取文件大小（仅文件消息）
     * @return 文件大小（字节）
     */
    qint64 fileSize() const;

    /**
     * @brief 设置文件大小（仅文件消息）
     * @param size 文件大小（字节）
     */
    void setFileSize(qint64 size);

    /**
     * @brief 获取MIME类型（仅文件消息）
     * @return MIME类型
     */
    QString mimeType() const;

    /**
     * @brief 设置MIME类型（仅文件消息）
     * @param mimeType MIME类型
     */
    void setMimeType(const QString& mimeType);

    /**
     * @brief 获取扩展属性
     * @param key 属性键
     * @param defaultValue 默认值
     * @return 属性值
     */
    QVariant property(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 设置扩展属性
     * @param key 属性键
     * @param value 属性值
     */
    void setProperty(const QString& key, const QVariant& value);

    /**
     * @brief 获取所有扩展属性
     * @return 属性映射
     */
    QVariantMap properties() const;

    /**
     * @brief 设置扩展属性
     * @param properties 属性映射
     */
    void setProperties(const QVariantMap& properties);

    /**
     * @brief 转换为变体映射
     * @return 变体映射
     */
    QVariantMap toVariantMap() const;

    /**
     * @brief 从变体映射创建
     * @param map 变体映射
     * @return 消息对象
     */
    static ChatMessage* fromVariantMap(const QVariantMap& map, QObject* parent = nullptr);

    /**
     * @brief 转换为JSON字符串
     * @return JSON字符串
     */
    QString toJson() const;

    /**
     * @brief 从JSON字符串创建
     * @param json JSON字符串
     * @return 消息对象
     */
    static ChatMessage* fromJson(const QString& json, QObject* parent = nullptr);

    /**
     * @brief 克隆消息
     * @param parent 父对象
     * @return 克隆的消息对象
     */
    ChatMessage* clone(QObject* parent = nullptr) const;

    /**
     * @brief 验证消息数据
     * @return 验证是否通过
     */
    bool validate() const;

    /**
     * @brief 获取消息摘要
     * @param maxLength 最大长度
     * @return 消息摘要
     */
    QString summary(int maxLength = 50) const;

    /**
     * @brief 比较消息
     * @param other 其他消息
     * @return 是否相等
     */
    bool equals(const ChatMessage* other) const;

    /**
     * @brief 获取消息大小（字节）
     * @return 消息大小
     */
    qint64 size() const;

public slots:
    /**
     * @brief 标记为已读
     */
    void markAsRead();

    /**
     * @brief 标记为未读
     */
    void markAsUnread();

    /**
     * @brief 重试发送
     */
    void retrySend();

signals:
    /**
     * @brief 内容改变信号
     * @param content 新内容
     */
    void contentChanged(const QString& content);

    /**
     * @brief 类型改变信号
     * @param type 新类型
     */
    void typeChanged(MessageType type);

    /**
     * @brief 发送者ID改变信号
     * @param senderId 新发送者ID
     */
    void senderIdChanged(const QString& senderId);

    /**
     * @brief 发送者名称改变信号
     * @param senderName 新发送者名称
     */
    void senderNameChanged(const QString& senderName);

    /**
     * @brief 房间ID改变信号
     * @param roomId 新房间ID
     */
    void roomIdChanged(const QString& roomId);

    /**
     * @brief 时间戳改变信号
     * @param timestamp 新时间戳
     */
    void timestampChanged(const QDateTime& timestamp);

    /**
     * @brief 状态改变信号
     * @param status 新状态
     */
    void statusChanged(MessageStatus status);

    /**
     * @brief 已读状态改变信号
     * @param read 是否已读
     */
    void readChanged(bool read);

    /**
     * @brief 编辑状态改变信号
     * @param edited 是否已编辑
     */
    void editedChanged(bool edited);

    /**
     * @brief 属性改变信号
     * @param key 属性键
     * @param value 新值
     */
    void propertyChanged(const QString& key, const QVariant& value);

private:
    /**
     * @brief 生成唯一ID
     * @return 唯一ID
     */
    QString generateId() const;

    /**
     * @brief 验证内容
     * @param content 内容
     * @return 是否有效
     */
    bool validateContent(const QString& content) const;

private:
    class Private;
    Private* d;
};

Q_DECLARE_METATYPE(ChatMessage*)
Q_DECLARE_METATYPE(ChatMessage::MessageType)
Q_DECLARE_METATYPE(ChatMessage::MessageStatus)
Q_DECLARE_METATYPE(ChatMessage::MessagePriority)

#endif // CHATMESSAGE_H
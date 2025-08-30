#include "ChatMessage.h"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRegularExpression>
#include <QMimeDatabase>
#include <QFileInfo>

class ChatMessage::Private
{
public:
    QString id;
    QString content;
    MessageType type;
    QString senderId;
    QString senderName;
    QString roomId;
    QDateTime timestamp;
    MessageStatus status;
    MessagePriority priority;
    bool isRead;
    bool isEdited;
    QDateTime editedTimestamp;
    
    // 文件相关属性
    QVariantMap fileInfo;
    QUrl fileUrl;
    qint64 fileSize;
    QString mimeType;
    
    // 扩展属性
    QVariantMap properties;
    
    Private() 
        : type(TextMessage)
        , status(Pending)
        , priority(Normal)
        , isRead(false)
        , isEdited(false)
        , fileSize(0)
    {
        timestamp = QDateTime::currentDateTime();
    }
};

ChatMessage::ChatMessage(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = generateId();
}

ChatMessage::ChatMessage(const QString& content, const QString& senderId, const QString& roomId, MessageType type, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = generateId();
    d->content = content;
    d->senderId = senderId;
    d->roomId = roomId;
    d->type = type;
}

ChatMessage::ChatMessage(const ChatMessage& other)
    : QObject(other.parent())
    , d(new Private(*other.d))
{
}

ChatMessage& ChatMessage::operator=(const ChatMessage& other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

ChatMessage::~ChatMessage()
{
    delete d;
}

QString ChatMessage::id() const
{
    return d->id;
}

QString ChatMessage::content() const
{
    return d->content;
}

void ChatMessage::setContent(const QString& content)
{
    if (d->content != content) {
        d->content = content;
        emit contentChanged(content);
    }
}

ChatMessage::MessageType ChatMessage::type() const
{
    return d->type;
}

void ChatMessage::setType(MessageType type)
{
    if (d->type != type) {
        d->type = type;
        emit typeChanged(type);
    }
}

QString ChatMessage::senderId() const
{
    return d->senderId;
}

void ChatMessage::setSenderId(const QString& senderId)
{
    if (d->senderId != senderId) {
        d->senderId = senderId;
        emit senderIdChanged(senderId);
    }
}

QString ChatMessage::senderName() const
{
    return d->senderName;
}

void ChatMessage::setSenderName(const QString& senderName)
{
    if (d->senderName != senderName) {
        d->senderName = senderName;
        emit senderNameChanged(senderName);
    }
}

QString ChatMessage::roomId() const
{
    return d->roomId;
}

void ChatMessage::setRoomId(const QString& roomId)
{
    if (d->roomId != roomId) {
        d->roomId = roomId;
        emit roomIdChanged(roomId);
    }
}

QDateTime ChatMessage::timestamp() const
{
    return d->timestamp;
}

void ChatMessage::setTimestamp(const QDateTime& timestamp)
{
    if (d->timestamp != timestamp) {
        d->timestamp = timestamp;
        emit timestampChanged(timestamp);
    }
}

ChatMessage::MessageStatus ChatMessage::status() const
{
    return d->status;
}

void ChatMessage::setStatus(MessageStatus status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged(status);
    }
}

ChatMessage::MessagePriority ChatMessage::priority() const
{
    return d->priority;
}

void ChatMessage::setPriority(MessagePriority priority)
{
    d->priority = priority;
}

bool ChatMessage::isRead() const
{
    return d->isRead;
}

void ChatMessage::setRead(bool read)
{
    if (d->isRead != read) {
        d->isRead = read;
        emit readChanged(read);
    }
}

bool ChatMessage::isEdited() const
{
    return d->isEdited;
}

QDateTime ChatMessage::editedTimestamp() const
{
    return d->editedTimestamp;
}

void ChatMessage::editContent(const QString& newContent)
{
    if (d->content != newContent) {
        d->content = newContent;
        d->isEdited = true;
        d->editedTimestamp = QDateTime::currentDateTime();
        emit contentChanged(newContent);
        emit editedChanged(true);
    }
}

QVariantMap ChatMessage::fileInfo() const
{
    return d->fileInfo;
}

void ChatMessage::setFileInfo(const QVariantMap& fileInfo)
{
    d->fileInfo = fileInfo;
}

QUrl ChatMessage::fileUrl() const
{
    return d->fileUrl;
}

void ChatMessage::setFileUrl(const QUrl& url)
{
    d->fileUrl = url;
}

qint64 ChatMessage::fileSize() const
{
    return d->fileSize;
}

void ChatMessage::setFileSize(qint64 size)
{
    d->fileSize = size;
}

QString ChatMessage::mimeType() const
{
    return d->mimeType;
}

void ChatMessage::setMimeType(const QString& mimeType)
{
    d->mimeType = mimeType;
}

QVariant ChatMessage::property(const QString& key, const QVariant& defaultValue) const
{
    return d->properties.value(key, defaultValue);
}

void ChatMessage::setProperty(const QString& key, const QVariant& value)
{
    if (d->properties.value(key) != value) {
        d->properties[key] = value;
        emit propertyChanged(key, value);
    }
}

QVariantMap ChatMessage::properties() const
{
    return d->properties;
}

void ChatMessage::setProperties(const QVariantMap& properties)
{
    d->properties = properties;
}

QVariantMap ChatMessage::toVariantMap() const
{
    QVariantMap map;
    map["id"] = d->id;
    map["content"] = d->content;
    map["type"] = static_cast<int>(d->type);
    map["senderId"] = d->senderId;
    map["senderName"] = d->senderName;
    map["roomId"] = d->roomId;
    map["timestamp"] = d->timestamp;
    map["status"] = static_cast<int>(d->status);
    map["priority"] = static_cast<int>(d->priority);
    map["isRead"] = d->isRead;
    map["isEdited"] = d->isEdited;
    map["editedTimestamp"] = d->editedTimestamp;
    map["fileInfo"] = d->fileInfo;
    map["fileUrl"] = d->fileUrl;
    map["fileSize"] = d->fileSize;
    map["mimeType"] = d->mimeType;
    map["properties"] = d->properties;
    return map;
}

ChatMessage* ChatMessage::fromVariantMap(const QVariantMap& map, QObject* parent)
{
    ChatMessage* message = new ChatMessage(parent);
    message->d->id = map.value("id").toString();
    message->d->content = map.value("content").toString();
    message->d->type = static_cast<MessageType>(map.value("type").toInt());
    message->d->senderId = map.value("senderId").toString();
    message->d->senderName = map.value("senderName").toString();
    message->d->roomId = map.value("roomId").toString();
    message->d->timestamp = map.value("timestamp").toDateTime();
    message->d->status = static_cast<MessageStatus>(map.value("status").toInt());
    message->d->priority = static_cast<MessagePriority>(map.value("priority").toInt());
    message->d->isRead = map.value("isRead").toBool();
    message->d->isEdited = map.value("isEdited").toBool();
    message->d->editedTimestamp = map.value("editedTimestamp").toDateTime();
    message->d->fileInfo = map.value("fileInfo").toMap();
    message->d->fileUrl = map.value("fileUrl").toUrl();
    message->d->fileSize = map.value("fileSize").toLongLong();
    message->d->mimeType = map.value("mimeType").toString();
    message->d->properties = map.value("properties").toMap();
    return message;
}

QString ChatMessage::toJson() const
{
    QJsonDocument doc = QJsonDocument::fromVariant(toVariantMap());
    return doc.toJson(QJsonDocument::Compact);
}

ChatMessage* ChatMessage::fromJson(const QString& json, QObject* parent)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return nullptr;
    }
    return fromVariantMap(doc.toVariant().toMap(), parent);
}

ChatMessage* ChatMessage::clone(QObject* parent) const
{
    return fromVariantMap(toVariantMap(), parent);
}

bool ChatMessage::validate() const
{
    if (d->id.isEmpty() || d->senderId.isEmpty() || d->roomId.isEmpty()) {
        return false;
    }
    
    if (!validateContent(d->content)) {
        return false;
    }
    
    if (!d->timestamp.isValid()) {
        return false;
    }
    
    return true;
}

QString ChatMessage::summary(int maxLength) const
{
    QString text = d->content;
    if (text.length() > maxLength) {
        text = text.left(maxLength - 3) + "...";
    }
    return text;
}

bool ChatMessage::equals(const ChatMessage* other) const
{
    if (!other) {
        return false;
    }
    
    return d->id == other->d->id &&
           d->content == other->d->content &&
           d->type == other->d->type &&
           d->senderId == other->d->senderId &&
           d->roomId == other->d->roomId &&
           d->timestamp == other->d->timestamp;
}

qint64 ChatMessage::size() const
{
    qint64 size = 0;
    size += d->content.toUtf8().size();
    size += d->senderId.toUtf8().size();
    size += d->senderName.toUtf8().size();
    size += d->roomId.toUtf8().size();
    size += d->mimeType.toUtf8().size();
    size += d->fileSize;
    return size;
}

void ChatMessage::markAsRead()
{
    setRead(true);
}

void ChatMessage::markAsUnread()
{
    setRead(false);
}

void ChatMessage::retrySend()
{
    if (d->status == Failed) {
        setStatus(Pending);
    }
}

QString ChatMessage::generateId() const
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

bool ChatMessage::validateContent(const QString& content) const
{
    // 基本验证：内容不能为空（除了某些特殊类型的消息）
    if (content.isEmpty() && d->type == TextMessage) {
        return false;
    }
    
    // 长度限制
    const int maxContentLength = 10000; // 10KB
    if (content.length() > maxContentLength) {
        return false;
    }
    
    return true;
}
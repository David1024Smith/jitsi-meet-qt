#include "../models/ChatMessage.h"
#include <QDateTime>
#include <QUuid>

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
    QMap<QString, QVariant> fileInfo;
    QUrl fileUrl;
    qint64 fileSize;
    QString mimeType;
    QMap<QString, QVariant> properties;
};

ChatMessage::ChatMessage(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    d->type = TextMessage;
    d->timestamp = QDateTime::currentDateTime();
    d->status = Pending;
    d->priority = Normal;
    d->isRead = false;
    d->isEdited = false;
    d->fileSize = 0;
}

ChatMessage::ChatMessage(const QString& content, const QString& senderId, 
                        const QString& roomId, MessageType type, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    d->content = content;
    d->senderId = senderId;
    d->senderName = QString(); // 初始化为空，可以后续设置
    d->roomId = roomId;
    d->type = type;
    d->timestamp = QDateTime::currentDateTime();
    d->status = Pending;
    d->priority = Normal;
    d->isRead = false;
    d->isEdited = false;
    d->fileSize = 0;
}

ChatMessage::ChatMessage(const ChatMessage& other)
    : QObject(other.parent())
    , d(new Private(*other.d))
{
}

ChatMessage::~ChatMessage() = default;

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
    if (d->priority != priority) {
        d->priority = priority;
        // Note: priorityChanged signal not declared in header, using propertyChanged instead
        emit propertyChanged("priority", static_cast<int>(priority));
    }
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

QMap<QString, QVariant> ChatMessage::fileInfo() const
{
    return d->fileInfo;
}

void ChatMessage::setFileInfo(const QMap<QString, QVariant>& fileInfo)
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

QMap<QString, QVariant> ChatMessage::properties() const
{
    return d->properties;
}

void ChatMessage::setProperties(const QMap<QString, QVariant>& properties)
{
    d->properties = properties;
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
        // Note: retryRequested signal not declared in header, using propertyChanged instead
        emit propertyChanged("retryRequested", true);
    }
}
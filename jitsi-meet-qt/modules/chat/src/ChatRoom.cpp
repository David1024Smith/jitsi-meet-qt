#include "../models/ChatRoom.h"
#include <QDateTime>
#include <QUuid>

class ChatRoom::Private
{
public:
    QString id;
    QString name;
    QString description;
    RoomType type;
    RoomStatus status;
    int participantCount;
    int maxParticipants;
    bool isPrivate;
    bool hasPassword;
    QDateTime createdTime;
    QDateTime lastActivity;
    QStringList participants;
    QMap<QString, QVariant> properties;
};

ChatRoom::ChatRoom(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    d->type = PublicRoom;
    d->status = Inactive;
    d->participantCount = 0;
    d->maxParticipants = 100;
    d->isPrivate = false;
    d->hasPassword = false;
    d->createdTime = QDateTime::currentDateTime();
    d->lastActivity = QDateTime::currentDateTime();
}

ChatRoom::ChatRoom(const QString& id, const QString& name, RoomType type, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = id;
    d->name = name;
    d->type = type;
    d->status = Inactive;
    d->participantCount = 0;
    d->maxParticipants = 100;
    d->isPrivate = (type == PrivateRoom);
    d->hasPassword = false;
    d->createdTime = QDateTime::currentDateTime();
    d->lastActivity = QDateTime::currentDateTime();
}

ChatRoom::ChatRoom(const ChatRoom& other)
    : QObject(other.parent())
    , d(new Private(*other.d))
{
}

ChatRoom::~ChatRoom() = default;

QString ChatRoom::id() const
{
    return d->id;
}

QString ChatRoom::name() const
{
    return d->name;
}

void ChatRoom::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged(name);
    }
}

QString ChatRoom::description() const
{
    return d->description;
}

void ChatRoom::setDescription(const QString& description)
{
    if (d->description != description) {
        d->description = description;
        emit descriptionChanged(description);
    }
}

ChatRoom::RoomType ChatRoom::type() const
{
    return d->type;
}

void ChatRoom::setType(RoomType type)
{
    if (d->type != type) {
        d->type = type;
        d->isPrivate = (type == PrivateRoom);
        emit typeChanged(type);
    }
}

ChatRoom::RoomStatus ChatRoom::status() const
{
    return d->status;
}

void ChatRoom::setStatus(RoomStatus status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged(status);
    }
}

int ChatRoom::participantCount() const
{
    return d->participantCount;
}

int ChatRoom::maxParticipants() const
{
    return d->maxParticipants;
}

void ChatRoom::setMaxParticipants(int max)
{
    if (d->maxParticipants != max) {
        d->maxParticipants = max;
        emit maxParticipantsChanged(max);
    }
}

bool ChatRoom::isPrivate() const
{
    return d->isPrivate;
}

void ChatRoom::setPrivate(bool isPrivate)
{
    if (d->isPrivate != isPrivate) {
        d->isPrivate = isPrivate;
        if (isPrivate) {
            d->type = PrivateRoom;
        } else {
            d->type = PublicRoom;
        }
        emit privateChanged(isPrivate);
    }
}

bool ChatRoom::hasPassword() const
{
    return d->hasPassword;
}

QDateTime ChatRoom::createdTime() const
{
    return d->createdTime;
}

QDateTime ChatRoom::lastActivity() const
{
    return d->lastActivity;
}

QList<Participant*> ChatRoom::participants() const
{
    // 注意：这里需要返回实际的Participant对象列表
    // 当前实现返回空列表，实际应用中需要维护真实的参与者对象
    QList<Participant*> participantList;
    // TODO: 实现从d->participants字符串列表转换为Participant对象列表
    return participantList;
}

void ChatRoom::activate()
{
    setStatus(Active);
}

void ChatRoom::deactivate()
{
    setStatus(Inactive);
}

void ChatRoom::archive()
{
    setStatus(Archived);
}

void ChatRoom::lock()
{
    d->hasPassword = true;
    emit passwordChanged(true);
}

void ChatRoom::unlock()
{
    d->hasPassword = false;
    emit passwordChanged(false);
}

void ChatRoom::clearParticipants()
{
    if (!d->participants.isEmpty()) {
        d->participants.clear();
        d->participantCount = 0;
        emit participantCountChanged(d->participantCount);
    }
}
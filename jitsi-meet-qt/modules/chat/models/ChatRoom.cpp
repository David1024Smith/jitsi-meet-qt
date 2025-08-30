#include "ChatRoom.h"
#include "Participant.h"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QCryptographicHash>
#include <QRegularExpression>

class ChatRoom::Private
{
public:
    QString id;
    QString name;
    QString description;
    RoomType type;
    RoomStatus status;
    QDateTime createdTime;
    QDateTime lastActivity;
    int maxParticipants;
    bool isPrivate;
    QString passwordHash;
    QString topic;
    QUrl avatarUrl;
    QString ownerId;
    
    QList<Participant*> participants;
    QStringList administrators;
    QMap<QString, Permissions> userPermissions;
    QVariantMap settings;
    QVariantMap properties;
    
    Private() 
        : type(PublicRoom)
        , status(Active)
        , maxParticipants(100)
        , isPrivate(false)
    {
        createdTime = QDateTime::currentDateTime();
        lastActivity = createdTime;
    }
};

ChatRoom::ChatRoom(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

ChatRoom::ChatRoom(const QString& id, const QString& name, RoomType type, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = id;
    d->name = name;
    d->type = type;
}

ChatRoom::ChatRoom(const ChatRoom& other)
    : QObject(other.parent())
    , d(new Private(*other.d))
{
    // 深拷贝参与者列表
    d->participants.clear();
    for (Participant* participant : other.d->participants) {
        d->participants.append(participant->clone(this));
    }
}

ChatRoom& ChatRoom::operator=(const ChatRoom& other)
{
    if (this != &other) {
        // 清理现有参与者
        qDeleteAll(d->participants);
        d->participants.clear();
        
        *d = *other.d;
        
        // 深拷贝参与者列表
        for (Participant* participant : other.d->participants) {
            d->participants.append(participant->clone(this));
        }
    }
    return *this;
}

ChatRoom::~ChatRoom()
{
    qDeleteAll(d->participants);
    delete d;
}

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
        updateLastActivity();
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

QDateTime ChatRoom::createdTime() const
{
    return d->createdTime;
}

QDateTime ChatRoom::lastActivity() const
{
    return d->lastActivity;
}

void ChatRoom::updateLastActivity()
{
    QDateTime now = QDateTime::currentDateTime();
    if (d->lastActivity != now) {
        d->lastActivity = now;
        emit lastActivityChanged(now);
    }
}

int ChatRoom::participantCount() const
{
    return d->participants.size();
}

int ChatRoom::maxParticipants() const
{
    return d->maxParticipants;
}

void ChatRoom::setMaxParticipants(int maxParticipants)
{
    if (d->maxParticipants != maxParticipants) {
        d->maxParticipants = maxParticipants;
        emit maxParticipantsChanged(maxParticipants);
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
        emit privateChanged(isPrivate);
    }
}

bool ChatRoom::hasPassword() const
{
    return !d->passwordHash.isEmpty();
}

void ChatRoom::setPassword(const QString& password)
{
    QString newHash;
    if (!password.isEmpty()) {
        QCryptographicHash hash(QCryptographicHash::Sha256);
        hash.addData(password.toUtf8());
        newHash = hash.result().toHex();
    }
    
    bool hadPassword = hasPassword();
    d->passwordHash = newHash;
    bool hasPasswordNow = hasPassword();
    
    if (hadPassword != hasPasswordNow) {
        emit passwordChanged(hasPasswordNow);
    }
}

bool ChatRoom::validatePassword(const QString& password) const
{
    if (!hasPassword()) {
        return password.isEmpty();
    }
    
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(password.toUtf8());
    QString inputHash = hash.result().toHex();
    
    return inputHash == d->passwordHash;
}

void ChatRoom::clearPassword()
{
    if (hasPassword()) {
        d->passwordHash.clear();
        emit passwordChanged(false);
    }
}

QString ChatRoom::topic() const
{
    return d->topic;
}

void ChatRoom::setTopic(const QString& topic)
{
    d->topic = topic;
}

QUrl ChatRoom::avatarUrl() const
{
    return d->avatarUrl;
}

void ChatRoom::setAvatarUrl(const QUrl& url)
{
    d->avatarUrl = url;
}

QList<Participant*> ChatRoom::participants() const
{
    return d->participants;
}

bool ChatRoom::addParticipant(Participant* participant)
{
    if (!participant || hasParticipant(participant->id())) {
        return false;
    }
    
    if (d->participants.size() >= d->maxParticipants) {
        return false;
    }
    
    participant->setParent(this);
    d->participants.append(participant);
    updateLastActivity();
    emit participantCountChanged(d->participants.size());
    emit participantJoined(participant);
    
    return true;
}

bool ChatRoom::removeParticipant(const QString& participantId)
{
    for (int i = 0; i < d->participants.size(); ++i) {
        if (d->participants[i]->id() == participantId) {
            Participant* participant = d->participants.takeAt(i);
            
            // 从管理员列表中移除
            d->administrators.removeAll(participantId);
            
            // 移除权限设置
            d->userPermissions.remove(participantId);
            
            // 如果是拥有者，清除拥有者设置
            if (d->ownerId == participantId) {
                d->ownerId.clear();
            }
            
            updateLastActivity();
            emit participantCountChanged(d->participants.size());
            emit participantLeft(participantId);
            
            participant->deleteLater();
            return true;
        }
    }
    return false;
}

Participant* ChatRoom::getParticipant(const QString& participantId) const
{
    for (Participant* participant : d->participants) {
        if (participant->id() == participantId) {
            return participant;
        }
    }
    return nullptr;
}

bool ChatRoom::hasParticipant(const QString& participantId) const
{
    return getParticipant(participantId) != nullptr;
}

QStringList ChatRoom::participantIds() const
{
    QStringList ids;
    for (Participant* participant : d->participants) {
        ids.append(participant->id());
    }
    return ids;
}

QStringList ChatRoom::administrators() const
{
    return d->administrators;
}

bool ChatRoom::addAdministrator(const QString& participantId)
{
    if (!hasParticipant(participantId) || d->administrators.contains(participantId)) {
        return false;
    }
    
    d->administrators.append(participantId);
    emit administratorAdded(participantId);
    return true;
}

bool ChatRoom::removeAdministrator(const QString& participantId)
{
    if (d->administrators.removeAll(participantId) > 0) {
        emit administratorRemoved(participantId);
        return true;
    }
    return false;
}

bool ChatRoom::isAdministrator(const QString& participantId) const
{
    return d->administrators.contains(participantId);
}

QString ChatRoom::owner() const
{
    return d->ownerId;
}

void ChatRoom::setOwner(const QString& ownerId)
{
    if (d->ownerId != ownerId) {
        d->ownerId = ownerId;
        emit ownerChanged(ownerId);
    }
}

ChatRoom::Permissions ChatRoom::getUserPermissions(const QString& participantId) const
{
    return d->userPermissions.value(participantId, Permissions());
}

void ChatRoom::setUserPermissions(const QString& participantId, Permissions permissions)
{
    if (d->userPermissions.value(participantId) != permissions) {
        d->userPermissions[participantId] = permissions;
        emit permissionsChanged(participantId, permissions);
    }
}

QVariantMap ChatRoom::settings() const
{
    return d->settings;
}

void ChatRoom::setSettings(const QVariantMap& settings)
{
    d->settings = settings;
}

QVariant ChatRoom::getSetting(const QString& key, const QVariant& defaultValue) const
{
    return d->settings.value(key, defaultValue);
}

void ChatRoom::setSetting(const QString& key, const QVariant& value)
{
    if (d->settings.value(key) != value) {
        d->settings[key] = value;
        emit settingChanged(key, value);
    }
}

QVariant ChatRoom::property(const QString& key, const QVariant& defaultValue) const
{
    return d->properties.value(key, defaultValue);
}

void ChatRoom::setProperty(const QString& key, const QVariant& value)
{
    if (d->properties.value(key) != value) {
        d->properties[key] = value;
        emit propertyChanged(key, value);
    }
}

QVariantMap ChatRoom::properties() const
{
    return d->properties;
}

void ChatRoom::setProperties(const QVariantMap& properties)
{
    d->properties = properties;
}

QVariantMap ChatRoom::toVariantMap() const
{
    QVariantMap map;
    map["id"] = d->id;
    map["name"] = d->name;
    map["description"] = d->description;
    map["type"] = static_cast<int>(d->type);
    map["status"] = static_cast<int>(d->status);
    map["createdTime"] = d->createdTime;
    map["lastActivity"] = d->lastActivity;
    map["maxParticipants"] = d->maxParticipants;
    map["isPrivate"] = d->isPrivate;
    map["hasPassword"] = hasPassword();
    map["topic"] = d->topic;
    map["avatarUrl"] = d->avatarUrl;
    map["ownerId"] = d->ownerId;
    map["administrators"] = d->administrators;
    map["settings"] = d->settings;
    map["properties"] = d->properties;
    
    // 参与者列表
    QVariantList participantList;
    for (Participant* participant : d->participants) {
        participantList.append(participant->toVariantMap());
    }
    map["participants"] = participantList;
    
    return map;
}

ChatRoom* ChatRoom::fromVariantMap(const QVariantMap& map, QObject* parent)
{
    ChatRoom* room = new ChatRoom(parent);
    room->d->id = map.value("id").toString();
    room->d->name = map.value("name").toString();
    room->d->description = map.value("description").toString();
    room->d->type = static_cast<RoomType>(map.value("type").toInt());
    room->d->status = static_cast<RoomStatus>(map.value("status").toInt());
    room->d->createdTime = map.value("createdTime").toDateTime();
    room->d->lastActivity = map.value("lastActivity").toDateTime();
    room->d->maxParticipants = map.value("maxParticipants").toInt();
    room->d->isPrivate = map.value("isPrivate").toBool();
    room->d->topic = map.value("topic").toString();
    room->d->avatarUrl = map.value("avatarUrl").toUrl();
    room->d->ownerId = map.value("ownerId").toString();
    room->d->administrators = map.value("administrators").toStringList();
    room->d->settings = map.value("settings").toMap();
    room->d->properties = map.value("properties").toMap();
    
    // 加载参与者
    QVariantList participantList = map.value("participants").toList();
    for (const QVariant& participantData : participantList) {
        Participant* participant = Participant::fromVariantMap(participantData.toMap(), room);
        if (participant) {
            room->d->participants.append(participant);
        }
    }
    
    return room;
}

QString ChatRoom::toJson() const
{
    QJsonDocument doc = QJsonDocument::fromVariant(toVariantMap());
    return doc.toJson(QJsonDocument::Compact);
}

ChatRoom* ChatRoom::fromJson(const QString& json, QObject* parent)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return nullptr;
    }
    return fromVariantMap(doc.toVariant().toMap(), parent);
}

ChatRoom* ChatRoom::clone(QObject* parent) const
{
    return fromVariantMap(toVariantMap(), parent);
}

bool ChatRoom::validate() const
{
    if (!validateId(d->id) || !validateName(d->name)) {
        return false;
    }
    
    if (d->maxParticipants <= 0) {
        return false;
    }
    
    if (!d->createdTime.isValid()) {
        return false;
    }
    
    return true;
}

bool ChatRoom::equals(const ChatRoom* other) const
{
    if (!other) {
        return false;
    }
    
    return d->id == other->d->id &&
           d->name == other->d->name &&
           d->type == other->d->type &&
           d->status == other->d->status;
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
    setStatus(Locked);
}

void ChatRoom::unlock()
{
    if (d->status == Locked) {
        setStatus(Active);
    }
}

void ChatRoom::clearParticipants()
{
    if (!d->participants.isEmpty()) {
        qDeleteAll(d->participants);
        d->participants.clear();
        d->administrators.clear();
        d->userPermissions.clear();
        d->ownerId.clear();
        
        emit participantCountChanged(0);
    }
}

bool ChatRoom::validateId(const QString& id) const
{
    if (id.isEmpty()) {
        return false;
    }
    
    // 检查ID格式（UUID或自定义格式）
    QRegularExpression uuidRegex("^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$");
    QRegularExpression customIdRegex("^[a-zA-Z0-9_-]+$");
    
    return uuidRegex.match(id).hasMatch() || customIdRegex.match(id).hasMatch();
}

bool ChatRoom::validateName(const QString& name) const
{
    if (name.isEmpty()) {
        return false;
    }
    
    // 名称长度限制
    const int maxNameLength = 100;
    if (name.length() > maxNameLength) {
        return false;
    }
    
    return true;
}
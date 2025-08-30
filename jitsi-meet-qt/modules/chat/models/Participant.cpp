#include "Participant.h"
#include <QUuid>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>

class Participant::Private
{
public:
    QString id;
    QString name;
    QString displayName;
    QString email;
    ParticipantStatus status;
    ParticipantRole role;
    bool isMuted;
    bool isVideoEnabled;
    QDateTime joinTime;
    QDateTime lastActivity;
    QUrl avatarUrl;
    QString statusMessage;
    Permissions permissions;
    QString clientInfo;
    QString ipAddress;
    QVariantMap location;
    QVariantMap properties;
    QVariantMap statistics;
    
    Private() 
        : status(Online)
        , role(Member)
        , isMuted(false)
        , isVideoEnabled(true)
        , permissions(Read | Write)
    {
        joinTime = QDateTime::currentDateTime();
        lastActivity = joinTime;
    }
};

Participant::Participant(QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = QUuid::createUuid().toString(QUuid::WithoutBraces);
}

Participant::Participant(const QString& id, const QString& name, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = id;
    d->name = name;
    d->displayName = name;
}

Participant::Participant(const Participant& other)
    : QObject(other.parent())
    , d(new Private(*other.d))
{
}

Participant& Participant::operator=(const Participant& other)
{
    if (this != &other) {
        *d = *other.d;
    }
    return *this;
}

Participant::~Participant()
{
    delete d;
}

QString Participant::id() const
{
    return d->id;
}

QString Participant::name() const
{
    return d->name;
}

void Participant::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        updateLastActivity();
        emit nameChanged(name);
    }
}

QString Participant::displayName() const
{
    return d->displayName.isEmpty() ? d->name : d->displayName;
}

void Participant::setDisplayName(const QString& displayName)
{
    if (d->displayName != displayName) {
        d->displayName = displayName;
        emit displayNameChanged(displayName);
    }
}

QString Participant::email() const
{
    return d->email;
}

void Participant::setEmail(const QString& email)
{
    if (d->email != email) {
        d->email = email;
        emit emailChanged(email);
    }
}

Participant::ParticipantStatus Participant::status() const
{
    return d->status;
}

void Participant::setStatus(ParticipantStatus status)
{
    if (d->status != status) {
        d->status = status;
        updateLastActivity();
        emit statusChanged(status);
        emit onlineStatusChanged(status != Offline);
    }
}

Participant::ParticipantRole Participant::role() const
{
    return d->role;
}

void Participant::setRole(ParticipantRole role)
{
    if (d->role != role) {
        d->role = role;
        updateLastActivity();
        emit roleChanged(role);
    }
}

bool Participant::isOnline() const
{
    return d->status != Offline;
}

bool Participant::isMuted() const
{
    return d->isMuted;
}

void Participant::setMuted(bool muted)
{
    if (d->isMuted != muted) {
        d->isMuted = muted;
        updateLastActivity();
        emit mutedChanged(muted);
    }
}

bool Participant::isVideoEnabled() const
{
    return d->isVideoEnabled;
}

void Participant::setVideoEnabled(bool enabled)
{
    if (d->isVideoEnabled != enabled) {
        d->isVideoEnabled = enabled;
        updateLastActivity();
        emit videoEnabledChanged(enabled);
    }
}

QDateTime Participant::joinTime() const
{
    return d->joinTime;
}

QDateTime Participant::lastActivity() const
{
    return d->lastActivity;
}

void Participant::updateLastActivity()
{
    QDateTime now = QDateTime::currentDateTime();
    if (d->lastActivity != now) {
        d->lastActivity = now;
        emit lastActivityChanged(now);
    }
}

QUrl Participant::avatarUrl() const
{
    return d->avatarUrl;
}

void Participant::setAvatarUrl(const QUrl& url)
{
    if (d->avatarUrl != url) {
        d->avatarUrl = url;
        emit avatarChanged(url);
    }
}

QString Participant::statusMessage() const
{
    return d->statusMessage;
}

void Participant::setStatusMessage(const QString& message)
{
    if (d->statusMessage != message) {
        d->statusMessage = message;
        emit statusMessageChanged(message);
    }
}

Participant::Permissions Participant::permissions() const
{
    return d->permissions;
}

void Participant::setPermissions(Permissions permissions)
{
    if (d->permissions != permissions) {
        d->permissions = permissions;
        emit permissionsChanged(permissions);
    }
}

bool Participant::hasPermission(Permission permission) const
{
    return d->permissions.testFlag(permission);
}

void Participant::addPermission(Permission permission)
{
    if (!hasPermission(permission)) {
        d->permissions |= permission;
        emit permissionsChanged(d->permissions);
    }
}

void Participant::removePermission(Permission permission)
{
    if (hasPermission(permission)) {
        d->permissions &= ~permission;
        emit permissionsChanged(d->permissions);
    }
}

QString Participant::clientInfo() const
{
    return d->clientInfo;
}

void Participant::setClientInfo(const QString& clientInfo)
{
    d->clientInfo = clientInfo;
}

QString Participant::ipAddress() const
{
    return d->ipAddress;
}

void Participant::setIpAddress(const QString& ipAddress)
{
    d->ipAddress = ipAddress;
}

QVariantMap Participant::location() const
{
    return d->location;
}

void Participant::setLocation(const QVariantMap& location)
{
    d->location = location;
}

QVariant Participant::property(const QString& key, const QVariant& defaultValue) const
{
    return d->properties.value(key, defaultValue);
}

void Participant::setProperty(const QString& key, const QVariant& value)
{
    if (d->properties.value(key) != value) {
        d->properties[key] = value;
        emit propertyChanged(key, value);
    }
}

QVariantMap Participant::properties() const
{
    return d->properties;
}

void Participant::setProperties(const QVariantMap& properties)
{
    d->properties = properties;
}

QVariantMap Participant::statistics() const
{
    return d->statistics;
}

void Participant::updateStatistics(const QVariantMap& stats)
{
    d->statistics = stats;
    emit statisticsUpdated(stats);
}

QVariantMap Participant::toVariantMap() const
{
    QVariantMap map;
    map["id"] = d->id;
    map["name"] = d->name;
    map["displayName"] = d->displayName;
    map["email"] = d->email;
    map["status"] = static_cast<int>(d->status);
    map["role"] = static_cast<int>(d->role);
    map["isMuted"] = d->isMuted;
    map["isVideoEnabled"] = d->isVideoEnabled;
    map["joinTime"] = d->joinTime;
    map["lastActivity"] = d->lastActivity;
    map["avatarUrl"] = d->avatarUrl;
    map["statusMessage"] = d->statusMessage;
    map["permissions"] = static_cast<int>(d->permissions);
    map["clientInfo"] = d->clientInfo;
    map["ipAddress"] = d->ipAddress;
    map["location"] = d->location;
    map["properties"] = d->properties;
    map["statistics"] = d->statistics;
    return map;
}

Participant* Participant::fromVariantMap(const QVariantMap& map, QObject* parent)
{
    Participant* participant = new Participant(parent);
    participant->d->id = map.value("id").toString();
    participant->d->name = map.value("name").toString();
    participant->d->displayName = map.value("displayName").toString();
    participant->d->email = map.value("email").toString();
    participant->d->status = static_cast<ParticipantStatus>(map.value("status").toInt());
    participant->d->role = static_cast<ParticipantRole>(map.value("role").toInt());
    participant->d->isMuted = map.value("isMuted").toBool();
    participant->d->isVideoEnabled = map.value("isVideoEnabled").toBool();
    participant->d->joinTime = map.value("joinTime").toDateTime();
    participant->d->lastActivity = map.value("lastActivity").toDateTime();
    participant->d->avatarUrl = map.value("avatarUrl").toUrl();
    participant->d->statusMessage = map.value("statusMessage").toString();
    participant->d->permissions = static_cast<Permissions>(map.value("permissions").toInt());
    participant->d->clientInfo = map.value("clientInfo").toString();
    participant->d->ipAddress = map.value("ipAddress").toString();
    participant->d->location = map.value("location").toMap();
    participant->d->properties = map.value("properties").toMap();
    participant->d->statistics = map.value("statistics").toMap();
    return participant;
}

QString Participant::toJson() const
{
    QJsonDocument doc = QJsonDocument::fromVariant(toVariantMap());
    return doc.toJson(QJsonDocument::Compact);
}

Participant* Participant::fromJson(const QString& json, QObject* parent)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &error);
    if (error.error != QJsonParseError::NoError) {
        return nullptr;
    }
    return fromVariantMap(doc.toVariant().toMap(), parent);
}

Participant* Participant::clone(QObject* parent) const
{
    return fromVariantMap(toVariantMap(), parent);
}

bool Participant::validate() const
{
    if (!validateId(d->id) || !validateName(d->name)) {
        return false;
    }
    
    if (!d->email.isEmpty() && !validateEmail(d->email)) {
        return false;
    }
    
    if (!d->joinTime.isValid()) {
        return false;
    }
    
    return true;
}

bool Participant::equals(const Participant* other) const
{
    if (!other) {
        return false;
    }
    
    return d->id == other->d->id &&
           d->name == other->d->name &&
           d->email == other->d->email &&
           d->role == other->d->role;
}

qint64 Participant::onlineDuration() const
{
    if (!isOnline()) {
        return 0;
    }
    
    return d->joinTime.secsTo(QDateTime::currentDateTime());
}

bool Participant::isAdministrator() const
{
    return d->role == Administrator;
}

bool Participant::isModerator() const
{
    return d->role == Moderator;
}

bool Participant::isOwner() const
{
    return d->role == Owner;
}

bool Participant::isGuest() const
{
    return d->role == Guest;
}

void Participant::setOnline()
{
    setStatus(Online);
}

void Participant::setOffline()
{
    setStatus(Offline);
}

void Participant::toggleMute()
{
    setMuted(!d->isMuted);
}

void Participant::toggleVideo()
{
    setVideoEnabled(!d->isVideoEnabled);
}

void Participant::promoteToModerator()
{
    setRole(Moderator);
}

void Participant::demoteToMember()
{
    setRole(Member);
}

void Participant::kick()
{
    // 这里应该触发踢出逻辑，由房间管理器处理
    setStatus(Offline);
}

void Participant::ban()
{
    // 这里应该触发封禁逻辑，由房间管理器处理
    setStatus(Offline);
}

bool Participant::validateId(const QString& id) const
{
    if (id.isEmpty()) {
        return false;
    }
    
    // 检查ID格式
    QRegularExpression idRegex("^[a-zA-Z0-9_-]+$");
    return idRegex.match(id).hasMatch();
}

bool Participant::validateName(const QString& name) const
{
    if (name.isEmpty()) {
        return false;
    }
    
    // 名称长度限制
    const int maxNameLength = 50;
    if (name.length() > maxNameLength) {
        return false;
    }
    
    return true;
}

bool Participant::validateEmail(const QString& email) const
{
    if (email.isEmpty()) {
        return true; // 邮箱是可选的
    }
    
    // 简单的邮箱格式验证
    QRegularExpression emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return emailRegex.match(email).hasMatch();
}
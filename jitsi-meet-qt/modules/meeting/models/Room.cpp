#include "Room.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QRegularExpression>
#include <QCryptographicHash>
#include <QDebug>

class Room::Private
{
public:
    QString id;
    QString name;
    QString displayName;
    QString server;
    RoomType type;
    RoomStatus status;
    QDateTime createdAt;
    QString ownerId;
    int maxParticipants;
    bool locked;
    bool isPublic;
    bool allowGuests;
    QString password;
    QString description;
    QString subject;
    
    QStringList participants;
    QMap<QString, QString> participantRoles; // participantId -> role
    QVariantMap configuration;
    QVariantMap permissions;
    QVariantMap statistics;
    
    Private()
        : type(PublicRoom)
        , status(Inactive)
        , maxParticipants(100)
        , locked(false)
        , isPublic(true)
        , allowGuests(true)
    {
        createdAt = QDateTime::currentDateTime();
    }
};

Room::Room(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = generateRoomId();
    initializeDefaultConfiguration();
    initializeDefaultPermissions();
}

Room::Room(const QString& name, const QString& server, QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = generateRoomId();
    d->name = name;
    d->displayName = name;
    d->server = server;
    initializeDefaultConfiguration();
    initializeDefaultPermissions();
}

Room::~Room() = default;

QString Room::id() const
{
    return d->id;
}

void Room::setId(const QString& id)
{
    if (d->id != id) {
        d->id = id;
        emit idChanged(id);
    }
}

QString Room::name() const
{
    return d->name;
}

void Room::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged(name);
        
        // Update display name if it matches the old name
        if (d->displayName == d->name || d->displayName.isEmpty()) {
            setDisplayName(name);
        }
    }
}

QString Room::displayName() const
{
    return d->displayName;
}

void Room::setDisplayName(const QString& displayName)
{
    if (d->displayName != displayName) {
        d->displayName = displayName;
        emit displayNameChanged(displayName);
    }
}

QString Room::server() const
{
    return d->server;
}

void Room::setServer(const QString& server)
{
    if (d->server != server) {
        d->server = server;
        emit serverChanged(server);
    }
}

Room::RoomType Room::type() const
{
    return d->type;
}

void Room::setType(RoomType type)
{
    if (d->type != type) {
        d->type = type;
        emit typeChanged(type);
        
        // Update related settings based on type
        switch (type) {
        case PublicRoom:
            setPublic(true);
            setAllowGuests(true);
            break;
        case PrivateRoom:
            setPublic(false);
            setAllowGuests(false);
            break;
        case PasswordRoom:
            setPublic(true);
            setAllowGuests(true);
            break;
        case InviteOnlyRoom:
            setPublic(false);
            setAllowGuests(false);
            break;
        case TemporaryRoom:
            setPublic(true);
            setAllowGuests(true);
            break;
        }
    }
}

Room::RoomStatus Room::status() const
{
    return d->status;
}

void Room::setStatus(RoomStatus status)
{
    if (d->status != status) {
        d->status = status;
        emit statusChanged(status);
    }
}

QDateTime Room::createdAt() const
{
    return d->createdAt;
}

void Room::setCreatedAt(const QDateTime& createdAt)
{
    if (d->createdAt != createdAt) {
        d->createdAt = createdAt;
        emit createdAtChanged(createdAt);
    }
}

QString Room::ownerId() const
{
    return d->ownerId;
}

void Room::setOwnerId(const QString& ownerId)
{
    if (d->ownerId != ownerId) {
        d->ownerId = ownerId;
        emit ownerIdChanged(ownerId);
    }
}

int Room::participantCount() const
{
    return d->participants.size();
}

int Room::maxParticipants() const
{
    return d->maxParticipants;
}

void Room::setMaxParticipants(int maxParticipants)
{
    if (d->maxParticipants != maxParticipants) {
        d->maxParticipants = maxParticipants;
        emit maxParticipantsChanged(maxParticipants);
    }
}

QStringList Room::participants() const
{
    return d->participants;
}

void Room::addParticipant(const QString& participantId, const QString& role)
{
    if (!participantId.isEmpty() && !d->participants.contains(participantId)) {
        d->participants.append(participantId);
        d->participantRoles[participantId] = role;
        updateParticipantCount();
        emit participantAdded(participantId, role);
    }
}

void Room::removeParticipant(const QString& participantId)
{
    if (d->participants.removeOne(participantId)) {
        d->participantRoles.remove(participantId);
        updateParticipantCount();
        emit participantRemoved(participantId);
    }
}

bool Room::hasParticipant(const QString& participantId) const
{
    return d->participants.contains(participantId);
}

QString Room::getParticipantRole(const QString& participantId) const
{
    return d->participantRoles.value(participantId, "participant");
}

void Room::setParticipantRole(const QString& participantId, const QString& role)
{
    if (d->participants.contains(participantId) && d->participantRoles.value(participantId) != role) {
        d->participantRoles[participantId] = role;
        emit participantRoleChanged(participantId, role);
    }
}

QStringList Room::moderators() const
{
    QStringList mods;
    for (auto it = d->participantRoles.begin(); it != d->participantRoles.end(); ++it) {
        if (it.value() == "moderator" || it.value() == "owner") {
            mods.append(it.key());
        }
    }
    return mods;
}

bool Room::isModerator(const QString& participantId) const
{
    QString role = getParticipantRole(participantId);
    return role == "moderator" || role == "owner";
}

bool Room::isLocked() const
{
    return d->locked;
}

void Room::setLocked(bool locked)
{
    if (d->locked != locked) {
        d->locked = locked;
        emit lockedChanged(locked);
        
        // Update status based on lock state
        if (locked && d->status == Active) {
            setStatus(Locked);
        } else if (!locked && d->status == Locked) {
            setStatus(Active);
        }
    }
}

bool Room::isPublic() const
{
    return d->isPublic;
}

void Room::setPublic(bool isPublic)
{
    if (d->isPublic != isPublic) {
        d->isPublic = isPublic;
        emit publicChanged(isPublic);
    }
}

bool Room::allowGuests() const
{
    return d->allowGuests;
}

void Room::setAllowGuests(bool allowGuests)
{
    if (d->allowGuests != allowGuests) {
        d->allowGuests = allowGuests;
        emit allowGuestsChanged(allowGuests);
    }
}

QString Room::password() const
{
    return d->password;
}

void Room::setPassword(const QString& password)
{
    d->password = password;
    
    // Update room type based on password
    if (!password.isEmpty() && d->type != PasswordRoom) {
        setType(PasswordRoom);
    }
}

bool Room::requiresPassword() const
{
    return !d->password.isEmpty();
}

bool Room::validatePassword(const QString& password) const
{
    if (d->password.isEmpty()) {
        return true; // No password required
    }
    
    return d->password == password;
}

QString Room::description() const
{
    return d->description;
}

void Room::setDescription(const QString& description)
{
    d->description = description;
}

QString Room::subject() const
{
    return d->subject;
}

void Room::setSubject(const QString& subject)
{
    d->subject = subject;
}

QVariantMap Room::configuration() const
{
    return d->configuration;
}

void Room::setConfiguration(const QVariantMap& config)
{
    if (d->configuration != config) {
        d->configuration = config;
        emit configurationChanged(config);
    }
}

QVariant Room::getConfigValue(const QString& key, const QVariant& defaultValue) const
{
    return d->configuration.value(key, defaultValue);
}

void Room::setConfigValue(const QString& key, const QVariant& value)
{
    if (d->configuration.value(key) != value) {
        d->configuration[key] = value;
        emit configValueChanged(key, value);
        emit configurationChanged(d->configuration);
    }
}

QVariantMap Room::permissions() const
{
    return d->permissions;
}

void Room::setPermissions(const QVariantMap& permissions)
{
    if (d->permissions != permissions) {
        d->permissions = permissions;
        emit permissionsChanged(permissions);
    }
}

bool Room::hasPermission(const QString& participantId, const QString& permission) const
{
    QString role = getParticipantRole(participantId);
    
    // Owner has all permissions
    if (role == "owner" || participantId == d->ownerId) {
        return true;
    }
    
    // Moderator has most permissions
    if (role == "moderator") {
        QStringList restrictedPermissions = {"delete_room", "change_owner"};
        return !restrictedPermissions.contains(permission);
    }
    
    // Check specific permissions
    QVariantMap userPermissions = d->permissions.value(participantId).toMap();
    if (userPermissions.contains(permission)) {
        return userPermissions.value(permission).toBool();
    }
    
    // Check role-based permissions
    QVariantMap rolePermissions = d->permissions.value("roles").toMap().value(role).toMap();
    return rolePermissions.value(permission, false).toBool();
}

void Room::grantPermission(const QString& participantId, const QString& permission)
{
    QVariantMap userPermissions = d->permissions.value(participantId).toMap();
    userPermissions[permission] = true;
    d->permissions[participantId] = userPermissions;
    
    emit permissionGranted(participantId, permission);
    emit permissionsChanged(d->permissions);
}

void Room::revokePermission(const QString& participantId, const QString& permission)
{
    QVariantMap userPermissions = d->permissions.value(participantId).toMap();
    userPermissions[permission] = false;
    d->permissions[participantId] = userPermissions;
    
    emit permissionRevoked(participantId, permission);
    emit permissionsChanged(d->permissions);
}

QVariantMap Room::statistics() const
{
    return d->statistics;
}

void Room::updateStatistics(const QVariantMap& stats)
{
    d->statistics = stats;
    emit statisticsUpdated(stats);
}

qint64 Room::getUsageDuration() const
{
    if (!d->createdAt.isValid()) {
        return 0;
    }
    
    return d->createdAt.secsTo(QDateTime::currentDateTime());
}

QVariantMap Room::toVariantMap() const
{
    QVariantMap map;
    
    map["id"] = d->id;
    map["name"] = d->name;
    map["displayName"] = d->displayName;
    map["server"] = d->server;
    map["type"] = static_cast<int>(d->type);
    map["status"] = static_cast<int>(d->status);
    map["createdAt"] = d->createdAt;
    map["ownerId"] = d->ownerId;
    map["maxParticipants"] = d->maxParticipants;
    map["locked"] = d->locked;
    map["isPublic"] = d->isPublic;
    map["allowGuests"] = d->allowGuests;
    map["password"] = d->password;
    map["description"] = d->description;
    map["subject"] = d->subject;
    map["participants"] = d->participants;
    map["participantRoles"] = QVariantMap(d->participantRoles);
    map["configuration"] = d->configuration;
    map["permissions"] = d->permissions;
    map["statistics"] = d->statistics;
    
    return map;
}

void Room::fromVariantMap(const QVariantMap& map)
{
    setId(map.value("id").toString());
    setName(map.value("name").toString());
    setDisplayName(map.value("displayName").toString());
    setServer(map.value("server").toString());
    setType(static_cast<RoomType>(map.value("type", PublicRoom).toInt()));
    setStatus(static_cast<RoomStatus>(map.value("status", Inactive).toInt()));
    setCreatedAt(map.value("createdAt").toDateTime());
    setOwnerId(map.value("ownerId").toString());
    setMaxParticipants(map.value("maxParticipants", 100).toInt());
    setLocked(map.value("locked", false).toBool());
    setPublic(map.value("isPublic", true).toBool());
    setAllowGuests(map.value("allowGuests", true).toBool());
    setPassword(map.value("password").toString());
    setDescription(map.value("description").toString());
    setSubject(map.value("subject").toString());
    
    d->participants = map.value("participants").toStringList();
    
    QVariantMap roles = map.value("participantRoles").toMap();
    d->participantRoles.clear();
    for (auto it = roles.begin(); it != roles.end(); ++it) {
        d->participantRoles[it.key()] = it.value().toString();
    }
    
    updateParticipantCount();
    
    setConfiguration(map.value("configuration").toMap());
    setPermissions(map.value("permissions").toMap());
    d->statistics = map.value("statistics").toMap();
}

QString Room::toJson() const
{
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    return doc.toJson(QJsonDocument::Compact);
}

bool Room::fromJson(const QString& json)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse room JSON:" << parseError.errorString();
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool Room::isValid() const
{
    return !d->id.isEmpty() && 
           validateRoomName(d->name) && 
           !d->server.isEmpty();
}

QStringList Room::validationErrors() const
{
    QStringList errors;
    
    if (d->id.isEmpty()) {
        errors << "Room ID is required";
    }
    
    if (!validateRoomName(d->name)) {
        errors << "Invalid room name";
    }
    
    if (d->server.isEmpty()) {
        errors << "Server is required";
    }
    
    if (d->maxParticipants <= 0) {
        errors << "Maximum participants must be greater than 0";
    }
    
    return errors;
}

QString Room::generateRoomId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Room::generateRoomUrl(const QString& server, const QString& roomName)
{
    if (server.isEmpty() || roomName.isEmpty()) {
        return QString();
    }
    
    return QString("https://%1/%2").arg(server, roomName);
}

bool Room::validateRoomName(const QString& name)
{
    if (name.isEmpty() || name.length() > 100) {
        return false;
    }
    
    // Check for valid characters (alphanumeric, dash, underscore)
    QRegularExpression validPattern("^[a-zA-Z0-9_-]+$");
    return validPattern.match(name).hasMatch();
}

void Room::updateParticipantCount()
{
    emit participantCountChanged(d->participants.size());
}

void Room::initializeDefaultConfiguration()
{
    d->configuration["enableChat"] = true;
    d->configuration["enableScreenShare"] = true;
    d->configuration["enableRecording"] = false;
    d->configuration["muteOnJoin"] = false;
    d->configuration["videoOnJoin"] = true;
    d->configuration["lobbyEnabled"] = false;
    d->configuration["waitingRoomEnabled"] = false;
}

void Room::initializeDefaultPermissions()
{
    // Default role permissions
    QVariantMap rolePermissions;
    
    // Owner permissions
    QVariantMap ownerPerms;
    ownerPerms["join"] = true;
    ownerPerms["speak"] = true;
    ownerPerms["video"] = true;
    ownerPerms["screen_share"] = true;
    ownerPerms["chat"] = true;
    ownerPerms["mute_others"] = true;
    ownerPerms["kick_participants"] = true;
    ownerPerms["lock_room"] = true;
    ownerPerms["record"] = true;
    ownerPerms["change_settings"] = true;
    ownerPerms["delete_room"] = true;
    ownerPerms["change_owner"] = true;
    rolePermissions["owner"] = ownerPerms;
    
    // Moderator permissions
    QVariantMap modPerms;
    modPerms["join"] = true;
    modPerms["speak"] = true;
    modPerms["video"] = true;
    modPerms["screen_share"] = true;
    modPerms["chat"] = true;
    modPerms["mute_others"] = true;
    modPerms["kick_participants"] = true;
    modPerms["lock_room"] = true;
    modPerms["record"] = true;
    modPerms["change_settings"] = false;
    modPerms["delete_room"] = false;
    modPerms["change_owner"] = false;
    rolePermissions["moderator"] = modPerms;
    
    // Participant permissions
    QVariantMap participantPerms;
    participantPerms["join"] = true;
    participantPerms["speak"] = true;
    participantPerms["video"] = true;
    participantPerms["screen_share"] = true;
    participantPerms["chat"] = true;
    participantPerms["mute_others"] = false;
    participantPerms["kick_participants"] = false;
    participantPerms["lock_room"] = false;
    participantPerms["record"] = false;
    participantPerms["change_settings"] = false;
    participantPerms["delete_room"] = false;
    participantPerms["change_owner"] = false;
    rolePermissions["participant"] = participantPerms;
    
    // Guest permissions
    QVariantMap guestPerms;
    guestPerms["join"] = true;
    guestPerms["speak"] = true;
    guestPerms["video"] = true;
    guestPerms["screen_share"] = false;
    guestPerms["chat"] = true;
    guestPerms["mute_others"] = false;
    guestPerms["kick_participants"] = false;
    guestPerms["lock_room"] = false;
    guestPerms["record"] = false;
    guestPerms["change_settings"] = false;
    guestPerms["delete_room"] = false;
    guestPerms["change_owner"] = false;
    rolePermissions["guest"] = guestPerms;
    
    d->permissions["roles"] = rolePermissions;
}
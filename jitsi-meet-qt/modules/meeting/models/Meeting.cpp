#include "Meeting.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QUuid>
#include <QRegularExpression>
#include <QDebug>

class Meeting::Private
{
public:
    QString id;
    QString name;
    QString url;
    QString server;
    QString roomName;
    MeetingType type;
    MeetingStatus status;
    QDateTime createdAt;
    QDateTime startedAt;
    QDateTime endedAt;
    QString creatorId;
    int maxParticipants;
    bool locked;
    bool recording;
    QString password;
    QString description;
    QStringList tags;
    QStringList participants;
    QVariantMap settings;
    QVariantMap statistics;
    
    Private()
        : type(InstantMeeting)
        , status(Created)
        , maxParticipants(100)
        , locked(false)
        , recording(false)
    {
        createdAt = QDateTime::currentDateTime();
    }
};

Meeting::Meeting(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = generateMeetingId();
}

Meeting::Meeting(const QString& name, const QString& url, QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = generateMeetingId();
    d->name = name;
    d->url = url;
    
    // Extract server and room name from URL
    QUrl qurl(url);
    if (qurl.isValid()) {
        d->server = qurl.host();
        QString path = qurl.path();
        if (path.startsWith('/')) {
            path = path.mid(1);
        }
        int slashIndex = path.indexOf('/');
        if (slashIndex > 0) {
            d->roomName = path.left(slashIndex);
        } else {
            d->roomName = path;
        }
    }
}

Meeting::~Meeting() = default;

QString Meeting::id() const
{
    return d->id;
}

void Meeting::setId(const QString& id)
{
    if (d->id != id) {
        d->id = id;
        emit idChanged(id);
    }
}

QString Meeting::name() const
{
    return d->name;
}

void Meeting::setName(const QString& name)
{
    if (d->name != name) {
        d->name = name;
        emit nameChanged(name);
    }
}

QString Meeting::url() const
{
    return d->url;
}

void Meeting::setUrl(const QString& url)
{
    if (d->url != url) {
        d->url = url;
        emit urlChanged(url);
        
        // Update server and room name from URL
        QUrl qurl(url);
        if (qurl.isValid()) {
            setServer(qurl.host());
            QString path = qurl.path();
            if (path.startsWith('/')) {
                path = path.mid(1);
            }
            int slashIndex = path.indexOf('/');
            if (slashIndex > 0) {
                setRoomName(path.left(slashIndex));
            } else {
                setRoomName(path);
            }
        }
    }
}

QString Meeting::server() const
{
    return d->server;
}

void Meeting::setServer(const QString& server)
{
    if (d->server != server) {
        d->server = server;
        emit serverChanged(server);
    }
}

QString Meeting::roomName() const
{
    return d->roomName;
}

void Meeting::setRoomName(const QString& roomName)
{
    if (d->roomName != roomName) {
        d->roomName = roomName;
        emit roomNameChanged(roomName);
    }
}

Meeting::MeetingType Meeting::type() const
{
    return d->type;
}

void Meeting::setType(MeetingType type)
{
    if (d->type != type) {
        d->type = type;
        emit typeChanged(type);
    }
}

Meeting::MeetingStatus Meeting::status() const
{
    return d->status;
}

void Meeting::setStatus(MeetingStatus status)
{
    if (d->status != status) {
        MeetingStatus oldStatus = d->status;
        d->status = status;
        
        // Update timestamps based on status changes
        if (status == Active && oldStatus != Active) {
            d->startedAt = QDateTime::currentDateTime();
            emit startedAtChanged(d->startedAt);
        } else if (status == Ended && oldStatus == Active) {
            d->endedAt = QDateTime::currentDateTime();
            emit endedAtChanged(d->endedAt);
        }
        
        emit statusChanged(status);
    }
}

QDateTime Meeting::createdAt() const
{
    return d->createdAt;
}

void Meeting::setCreatedAt(const QDateTime& createdAt)
{
    if (d->createdAt != createdAt) {
        d->createdAt = createdAt;
        emit createdAtChanged(createdAt);
    }
}

QDateTime Meeting::startedAt() const
{
    return d->startedAt;
}

void Meeting::setStartedAt(const QDateTime& startedAt)
{
    if (d->startedAt != startedAt) {
        d->startedAt = startedAt;
        emit startedAtChanged(startedAt);
    }
}

QDateTime Meeting::endedAt() const
{
    return d->endedAt;
}

void Meeting::setEndedAt(const QDateTime& endedAt)
{
    if (d->endedAt != endedAt) {
        d->endedAt = endedAt;
        emit endedAtChanged(endedAt);
    }
}

qint64 Meeting::duration() const
{
    if (!d->startedAt.isValid()) {
        return 0;
    }
    
    QDateTime endTime = d->endedAt.isValid() ? d->endedAt : QDateTime::currentDateTime();
    return d->startedAt.secsTo(endTime);
}

QString Meeting::creatorId() const
{
    return d->creatorId;
}

void Meeting::setCreatorId(const QString& creatorId)
{
    if (d->creatorId != creatorId) {
        d->creatorId = creatorId;
        emit creatorIdChanged(creatorId);
    }
}

int Meeting::participantCount() const
{
    return d->participants.size();
}

int Meeting::maxParticipants() const
{
    return d->maxParticipants;
}

void Meeting::setMaxParticipants(int maxParticipants)
{
    if (d->maxParticipants != maxParticipants) {
        d->maxParticipants = maxParticipants;
        emit maxParticipantsChanged(maxParticipants);
    }
}

QStringList Meeting::participants() const
{
    return d->participants;
}

void Meeting::addParticipant(const QString& participantId)
{
    if (!participantId.isEmpty() && !d->participants.contains(participantId)) {
        d->participants.append(participantId);
        updateParticipantCount();
        emit participantAdded(participantId);
    }
}

void Meeting::removeParticipant(const QString& participantId)
{
    if (d->participants.removeOne(participantId)) {
        updateParticipantCount();
        emit participantRemoved(participantId);
    }
}

bool Meeting::hasParticipant(const QString& participantId) const
{
    return d->participants.contains(participantId);
}

bool Meeting::isLocked() const
{
    return d->locked;
}

void Meeting::setLocked(bool locked)
{
    if (d->locked != locked) {
        d->locked = locked;
        emit lockedChanged(locked);
    }
}

bool Meeting::isRecording() const
{
    return d->recording;
}

void Meeting::setRecording(bool recording)
{
    if (d->recording != recording) {
        d->recording = recording;
        emit recordingChanged(recording);
    }
}

QString Meeting::password() const
{
    return d->password;
}

void Meeting::setPassword(const QString& password)
{
    d->password = password;
}

QString Meeting::description() const
{
    return d->description;
}

void Meeting::setDescription(const QString& description)
{
    d->description = description;
}

QStringList Meeting::tags() const
{
    return d->tags;
}

void Meeting::setTags(const QStringList& tags)
{
    d->tags = tags;
}

void Meeting::addTag(const QString& tag)
{
    if (!tag.isEmpty() && !d->tags.contains(tag)) {
        d->tags.append(tag);
    }
}

void Meeting::removeTag(const QString& tag)
{
    d->tags.removeOne(tag);
}

QVariantMap Meeting::settings() const
{
    return d->settings;
}

void Meeting::setSettings(const QVariantMap& settings)
{
    if (d->settings != settings) {
        d->settings = settings;
        emit settingsChanged(settings);
    }
}

QVariant Meeting::getSetting(const QString& key, const QVariant& defaultValue) const
{
    return d->settings.value(key, defaultValue);
}

void Meeting::setSetting(const QString& key, const QVariant& value)
{
    if (d->settings.value(key) != value) {
        d->settings[key] = value;
        emit settingChanged(key, value);
        emit settingsChanged(d->settings);
    }
}

QVariantMap Meeting::statistics() const
{
    return d->statistics;
}

void Meeting::updateStatistics(const QVariantMap& stats)
{
    d->statistics = stats;
    emit statisticsUpdated(stats);
}

QVariantMap Meeting::toVariantMap() const
{
    QVariantMap map;
    
    map["id"] = d->id;
    map["name"] = d->name;
    map["url"] = d->url;
    map["server"] = d->server;
    map["roomName"] = d->roomName;
    map["type"] = static_cast<int>(d->type);
    map["status"] = static_cast<int>(d->status);
    map["createdAt"] = d->createdAt;
    map["startedAt"] = d->startedAt;
    map["endedAt"] = d->endedAt;
    map["creatorId"] = d->creatorId;
    map["maxParticipants"] = d->maxParticipants;
    map["locked"] = d->locked;
    map["recording"] = d->recording;
    map["password"] = d->password;
    map["description"] = d->description;
    map["tags"] = d->tags;
    map["participants"] = d->participants;
    map["settings"] = d->settings;
    map["statistics"] = d->statistics;
    
    return map;
}

void Meeting::fromVariantMap(const QVariantMap& map)
{
    setId(map.value("id").toString());
    setName(map.value("name").toString());
    setUrl(map.value("url").toString());
    setServer(map.value("server").toString());
    setRoomName(map.value("roomName").toString());
    setType(static_cast<MeetingType>(map.value("type", InstantMeeting).toInt()));
    setStatus(static_cast<MeetingStatus>(map.value("status", Created).toInt()));
    setCreatedAt(map.value("createdAt").toDateTime());
    setStartedAt(map.value("startedAt").toDateTime());
    setEndedAt(map.value("endedAt").toDateTime());
    setCreatorId(map.value("creatorId").toString());
    setMaxParticipants(map.value("maxParticipants", 100).toInt());
    setLocked(map.value("locked", false).toBool());
    setRecording(map.value("recording", false).toBool());
    setPassword(map.value("password").toString());
    setDescription(map.value("description").toString());
    setTags(map.value("tags").toStringList());
    
    d->participants = map.value("participants").toStringList();
    updateParticipantCount();
    
    setSettings(map.value("settings").toMap());
    d->statistics = map.value("statistics").toMap();
}

QString Meeting::toJson() const
{
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    return doc.toJson(QJsonDocument::Compact);
}

bool Meeting::fromJson(const QString& json)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse meeting JSON:" << parseError.errorString();
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool Meeting::isValid() const
{
    return validateName(d->name) && validateUrl(d->url) && !d->id.isEmpty();
}

QStringList Meeting::validationErrors() const
{
    QStringList errors;
    
    if (d->id.isEmpty()) {
        errors << "Meeting ID is required";
    }
    
    if (!validateName(d->name)) {
        errors << "Invalid meeting name";
    }
    
    if (!validateUrl(d->url)) {
        errors << "Invalid meeting URL";
    }
    
    if (d->server.isEmpty()) {
        errors << "Server is required";
    }
    
    if (d->roomName.isEmpty()) {
        errors << "Room name is required";
    }
    
    if (d->maxParticipants <= 0) {
        errors << "Maximum participants must be greater than 0";
    }
    
    return errors;
}

QString Meeting::generateMeetingId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Meeting::generateMeetingUrl(const QString& server, const QString& roomName)
{
    if (server.isEmpty() || roomName.isEmpty()) {
        return QString();
    }
    
    return QString("https://%1/%2").arg(server, roomName);
}

void Meeting::updateParticipantCount()
{
    emit participantCountChanged(d->participants.size());
}

bool Meeting::validateName(const QString& name) const
{
    if (name.isEmpty() || name.length() > 200) {
        return false;
    }
    
    // Check for valid characters
    QRegularExpression validPattern("^[\\w\\s\\-_.,!?()]+$");
    return validPattern.match(name).hasMatch();
}

bool Meeting::validateUrl(const QString& url) const
{
    if (url.isEmpty()) {
        return false;
    }
    
    QUrl qurl(url);
    return qurl.isValid() && 
           (qurl.scheme() == "https" || qurl.scheme() == "http") &&
           !qurl.host().isEmpty();
}
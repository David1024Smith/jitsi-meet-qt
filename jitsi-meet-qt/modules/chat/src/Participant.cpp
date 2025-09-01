#include "../models/Participant.h"
#include <QDateTime>

class Participant::Private
{
public:
    QString id;
    QString name;
    QString displayName;
    QString email;
    ParticipantStatus status;
    ParticipantRole role;
    bool isOnline;
    bool isMuted;
    bool isVideoEnabled;
    QDateTime joinTime;
    QDateTime lastActivity;
    QMap<QString, QVariant> properties;
};

Participant::Participant(QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->status = Offline;
    d->role = Member;
    d->isOnline = false;
    d->isMuted = false;
    d->isVideoEnabled = false;
    d->joinTime = QDateTime::currentDateTime();
    d->lastActivity = QDateTime::currentDateTime();
}

Participant::Participant(const QString& id, const QString& name, QObject* parent)
    : QObject(parent)
    , d(new Private)
{
    d->id = id;
    d->name = name;
    d->displayName = name;
    d->status = Offline;
    d->role = Member;
    d->isOnline = false;
    d->isMuted = false;
    d->isVideoEnabled = false;
    d->joinTime = QDateTime::currentDateTime();
    d->lastActivity = QDateTime::currentDateTime();
}

Participant::~Participant() = default;

QString Participant::id() const
{
    return d->id;
}

QString Participant::name() const
{
    return d->name;
}

QString Participant::displayName() const
{
    return d->displayName;
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
        emit statusChanged(status);
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
        emit roleChanged(role);
    }
}

bool Participant::isOnline() const
{
    return d->isOnline;
}

bool Participant::isMuted() const
{
    return d->isMuted;
}

void Participant::setMuted(bool muted)
{
    if (d->isMuted != muted) {
        d->isMuted = muted;
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

void Participant::setOnline()
{
    if (!d->isOnline) {
        d->isOnline = true;
        setStatus(Online);
        d->lastActivity = QDateTime::currentDateTime();
        emit onlineStatusChanged(true);
    }
}

void Participant::setOffline()
{
    if (d->isOnline) {
        d->isOnline = false;
        setStatus(Offline);
        emit onlineStatusChanged(false);
    }
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
    emit kickRequested();
}

void Participant::ban()
{
    emit banRequested();
}
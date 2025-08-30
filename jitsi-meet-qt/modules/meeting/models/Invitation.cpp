#include "Invitation.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUuid>
#include <QRegularExpression>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDebug>

class Invitation::Private
{
public:
    QString id;
    QString meetingId;
    QString senderId;
    QString recipientId;
    QString recipientEmail;
    InvitationType type;
    InvitationStatus status;
    QDateTime createdAt;
    QDateTime sentAt;
    QDateTime respondedAt;
    QDateTime expiresAt;
    QString message;
    QString subject;
    QString meetingUrl;
    QString invitationUrl;
    QString senderName;
    QString senderEmail;
    QString recipientName;
    QVariantMap settings;
    QList<int> reminderTimes;
    bool reminderEnabled;
    QVariantMap trackingInfo;
    QVariantList eventHistory;
    
    Private()
        : type(EmailInvitation)
        , status(Pending)
        , reminderEnabled(true)
    {
        createdAt = QDateTime::currentDateTime();
        // Default expiration: 7 days from creation
        expiresAt = createdAt.addDays(7);
        // Default reminder times: 15 minutes and 1 hour before
        reminderTimes << 15 << 60;
    }
};

Invitation::Invitation(QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = generateInvitationId();
    initializeDefaultSettings();
}

Invitation::Invitation(const QString& meetingId, const QString& recipientEmail, QObject* parent)
    : QObject(parent)
    , d(std::make_unique<Private>())
{
    d->id = generateInvitationId();
    d->meetingId = meetingId;
    d->recipientEmail = recipientEmail;
    initializeDefaultSettings();
}

Invitation::~Invitation() = default;

QString Invitation::id() const
{
    return d->id;
}

void Invitation::setId(const QString& id)
{
    if (d->id != id) {
        d->id = id;
        emit idChanged(id);
    }
}

QString Invitation::meetingId() const
{
    return d->meetingId;
}

void Invitation::setMeetingId(const QString& meetingId)
{
    if (d->meetingId != meetingId) {
        d->meetingId = meetingId;
        emit meetingIdChanged(meetingId);
    }
}

QString Invitation::senderId() const
{
    return d->senderId;
}

void Invitation::setSenderId(const QString& senderId)
{
    if (d->senderId != senderId) {
        d->senderId = senderId;
        emit senderIdChanged(senderId);
    }
}

QString Invitation::recipientId() const
{
    return d->recipientId;
}

void Invitation::setRecipientId(const QString& recipientId)
{
    if (d->recipientId != recipientId) {
        d->recipientId = recipientId;
        emit recipientIdChanged(recipientId);
    }
}

QString Invitation::recipientEmail() const
{
    return d->recipientEmail;
}

void Invitation::setRecipientEmail(const QString& recipientEmail)
{
    if (d->recipientEmail != recipientEmail) {
        d->recipientEmail = recipientEmail;
        emit recipientEmailChanged(recipientEmail);
    }
}

Invitation::InvitationType Invitation::type() const
{
    return d->type;
}

void Invitation::setType(InvitationType type)
{
    if (d->type != type) {
        d->type = type;
        emit typeChanged(type);
    }
}

Invitation::InvitationStatus Invitation::status() const
{
    return d->status;
}

void Invitation::setStatus(InvitationStatus status)
{
    if (d->status != status) {
        InvitationStatus oldStatus = d->status;
        d->status = status;
        
        // Update timestamps based on status changes
        QDateTime now = QDateTime::currentDateTime();
        switch (status) {
        case Sent:
            if (oldStatus == Pending) {
                d->sentAt = now;
                emit sentAtChanged(d->sentAt);
                recordEvent("sent", {{"timestamp", now}});
                emit invitationSent();
            }
            break;
        case Delivered:
            recordEvent("delivered", {{"timestamp", now}});
            emit invitationDelivered();
            break;
        case Opened:
            recordEvent("opened", {{"timestamp", now}});
            emit invitationOpened();
            break;
        case Accepted:
        case Declined:
            if (oldStatus != Accepted && oldStatus != Declined) {
                d->respondedAt = now;
                emit respondedAtChanged(d->respondedAt);
            }
            break;
        default:
            break;
        }
        
        emit statusChanged(status);
    }
}

QDateTime Invitation::createdAt() const
{
    return d->createdAt;
}

void Invitation::setCreatedAt(const QDateTime& createdAt)
{
    if (d->createdAt != createdAt) {
        d->createdAt = createdAt;
        emit createdAtChanged(createdAt);
    }
}

QDateTime Invitation::sentAt() const
{
    return d->sentAt;
}

void Invitation::setSentAt(const QDateTime& sentAt)
{
    if (d->sentAt != sentAt) {
        d->sentAt = sentAt;
        emit sentAtChanged(sentAt);
    }
}

QDateTime Invitation::respondedAt() const
{
    return d->respondedAt;
}

void Invitation::setRespondedAt(const QDateTime& respondedAt)
{
    if (d->respondedAt != respondedAt) {
        d->respondedAt = respondedAt;
        emit respondedAtChanged(respondedAt);
    }
}

QDateTime Invitation::expiresAt() const
{
    return d->expiresAt;
}

void Invitation::setExpiresAt(const QDateTime& expiresAt)
{
    if (d->expiresAt != expiresAt) {
        d->expiresAt = expiresAt;
        emit expiresAtChanged(expiresAt);
    }
}

bool Invitation::isExpired() const
{
    return d->expiresAt.isValid() && QDateTime::currentDateTime() > d->expiresAt;
}

qint64 Invitation::timeRemaining() const
{
    if (!d->expiresAt.isValid()) {
        return -1; // No expiration
    }
    
    qint64 remaining = QDateTime::currentDateTime().secsTo(d->expiresAt);
    return qMax(0LL, remaining);
}

QString Invitation::message() const
{
    return d->message;
}

void Invitation::setMessage(const QString& message)
{
    if (d->message != message) {
        d->message = message;
        emit messageChanged(message);
    }
}

QString Invitation::subject() const
{
    return d->subject;
}

void Invitation::setSubject(const QString& subject)
{
    d->subject = subject;
}

QString Invitation::meetingUrl() const
{
    return d->meetingUrl;
}

void Invitation::setMeetingUrl(const QString& meetingUrl)
{
    d->meetingUrl = meetingUrl;
}

QString Invitation::invitationUrl() const
{
    return d->invitationUrl;
}

void Invitation::setInvitationUrl(const QString& invitationUrl)
{
    d->invitationUrl = invitationUrl;
}

QString Invitation::senderName() const
{
    return d->senderName;
}

void Invitation::setSenderName(const QString& senderName)
{
    d->senderName = senderName;
}

QString Invitation::senderEmail() const
{
    return d->senderEmail;
}

void Invitation::setSenderEmail(const QString& senderEmail)
{
    d->senderEmail = senderEmail;
}

QString Invitation::recipientName() const
{
    return d->recipientName;
}

void Invitation::setRecipientName(const QString& recipientName)
{
    d->recipientName = recipientName;
}

QVariantMap Invitation::settings() const
{
    return d->settings;
}

void Invitation::setSettings(const QVariantMap& settings)
{
    if (d->settings != settings) {
        d->settings = settings;
        emit settingsChanged(settings);
    }
}

QVariant Invitation::getSetting(const QString& key, const QVariant& defaultValue) const
{
    return d->settings.value(key, defaultValue);
}

void Invitation::setSetting(const QString& key, const QVariant& value)
{
    if (d->settings.value(key) != value) {
        d->settings[key] = value;
        emit settingChanged(key, value);
        emit settingsChanged(d->settings);
    }
}

QList<int> Invitation::reminderTimes() const
{
    return d->reminderTimes;
}

void Invitation::setReminderTimes(const QList<int>& reminderTimes)
{
    d->reminderTimes = reminderTimes;
}

void Invitation::addReminderTime(int minutes)
{
    if (!d->reminderTimes.contains(minutes)) {
        d->reminderTimes.append(minutes);
        std::sort(d->reminderTimes.begin(), d->reminderTimes.end());
    }
}

void Invitation::removeReminderTime(int minutes)
{
    d->reminderTimes.removeOne(minutes);
}

bool Invitation::isReminderEnabled() const
{
    return d->reminderEnabled;
}

void Invitation::setReminderEnabled(bool enabled)
{
    d->reminderEnabled = enabled;
}

QVariantMap Invitation::trackingInfo() const
{
    return d->trackingInfo;
}

void Invitation::updateTrackingInfo(const QVariantMap& info)
{
    d->trackingInfo = info;
    emit trackingInfoUpdated(info);
}

void Invitation::recordEvent(const QString& event, const QVariantMap& data)
{
    QVariantMap eventData = data;
    eventData["event"] = event;
    eventData["timestamp"] = QDateTime::currentDateTime();
    
    d->eventHistory.append(eventData);
    emit eventRecorded(event, eventData);
}

QVariantList Invitation::eventHistory() const
{
    return d->eventHistory;
}

bool Invitation::send()
{
    if (d->status != Pending) {
        return false;
    }
    
    if (!isValid()) {
        return false;
    }
    
    // Generate invitation URL if not set
    if (d->invitationUrl.isEmpty() && !d->meetingUrl.isEmpty()) {
        d->invitationUrl = generateInvitationUrl(d->meetingUrl, d->id);
    }
    
    // Generate default subject and message if not set
    if (d->subject.isEmpty()) {
        d->subject = generateDefaultSubject();
    }
    
    if (d->message.isEmpty()) {
        d->message = generateDefaultMessage();
    }
    
    setStatus(Sent);
    return true;
}

bool Invitation::resend()
{
    if (d->status == Cancelled || d->status == Expired) {
        return false;
    }
    
    recordEvent("resent", {{"previousStatus", static_cast<int>(d->status)}});
    setStatus(Sent);
    return true;
}

bool Invitation::cancel()
{
    if (d->status == Accepted || d->status == Declined || d->status == Cancelled) {
        return false;
    }
    
    recordEvent("cancelled", {{"previousStatus", static_cast<int>(d->status)}});
    setStatus(Cancelled);
    emit invitationCancelled();
    return true;
}

bool Invitation::accept(const QString& response)
{
    if (d->status == Cancelled || d->status == Expired || isExpired()) {
        return false;
    }
    
    recordEvent("accepted", {{"response", response}});
    setStatus(Accepted);
    emit invitationAccepted(response);
    return true;
}

bool Invitation::decline(const QString& reason)
{
    if (d->status == Cancelled || d->status == Expired || isExpired()) {
        return false;
    }
    
    recordEvent("declined", {{"reason", reason}});
    setStatus(Declined);
    emit invitationDeclined(reason);
    return true;
}

QVariantMap Invitation::toVariantMap() const
{
    QVariantMap map;
    
    map["id"] = d->id;
    map["meetingId"] = d->meetingId;
    map["senderId"] = d->senderId;
    map["recipientId"] = d->recipientId;
    map["recipientEmail"] = d->recipientEmail;
    map["type"] = static_cast<int>(d->type);
    map["status"] = static_cast<int>(d->status);
    map["createdAt"] = d->createdAt;
    map["sentAt"] = d->sentAt;
    map["respondedAt"] = d->respondedAt;
    map["expiresAt"] = d->expiresAt;
    map["message"] = d->message;
    map["subject"] = d->subject;
    map["meetingUrl"] = d->meetingUrl;
    map["invitationUrl"] = d->invitationUrl;
    map["senderName"] = d->senderName;
    map["senderEmail"] = d->senderEmail;
    map["recipientName"] = d->recipientName;
    map["settings"] = d->settings;
    
    QVariantList reminderList;
    for (int time : d->reminderTimes) {
        reminderList.append(time);
    }
    map["reminderTimes"] = reminderList;
    map["reminderEnabled"] = d->reminderEnabled;
    map["trackingInfo"] = d->trackingInfo;
    map["eventHistory"] = d->eventHistory;
    
    return map;
}

void Invitation::fromVariantMap(const QVariantMap& map)
{
    setId(map.value("id").toString());
    setMeetingId(map.value("meetingId").toString());
    setSenderId(map.value("senderId").toString());
    setRecipientId(map.value("recipientId").toString());
    setRecipientEmail(map.value("recipientEmail").toString());
    setType(static_cast<InvitationType>(map.value("type", EmailInvitation).toInt()));
    setStatus(static_cast<InvitationStatus>(map.value("status", Pending).toInt()));
    setCreatedAt(map.value("createdAt").toDateTime());
    setSentAt(map.value("sentAt").toDateTime());
    setRespondedAt(map.value("respondedAt").toDateTime());
    setExpiresAt(map.value("expiresAt").toDateTime());
    setMessage(map.value("message").toString());
    setSubject(map.value("subject").toString());
    setMeetingUrl(map.value("meetingUrl").toString());
    setInvitationUrl(map.value("invitationUrl").toString());
    setSenderName(map.value("senderName").toString());
    setSenderEmail(map.value("senderEmail").toString());
    setRecipientName(map.value("recipientName").toString());
    setSettings(map.value("settings").toMap());
    
    QVariantList reminderList = map.value("reminderTimes").toList();
    d->reminderTimes.clear();
    for (const QVariant& time : reminderList) {
        d->reminderTimes.append(time.toInt());
    }
    
    setReminderEnabled(map.value("reminderEnabled", true).toBool());
    d->trackingInfo = map.value("trackingInfo").toMap();
    d->eventHistory = map.value("eventHistory").toList();
}

QString Invitation::toJson() const
{
    QJsonDocument doc(QJsonObject::fromVariantMap(toVariantMap()));
    return doc.toJson(QJsonDocument::Compact);
}

bool Invitation::fromJson(const QString& json)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse invitation JSON:" << parseError.errorString();
        return false;
    }
    
    fromVariantMap(doc.object().toVariantMap());
    return true;
}

bool Invitation::isValid() const
{
    return !d->id.isEmpty() && 
           !d->meetingId.isEmpty() && 
           validateEmail(d->recipientEmail);
}

QStringList Invitation::validationErrors() const
{
    QStringList errors;
    
    if (d->id.isEmpty()) {
        errors << "Invitation ID is required";
    }
    
    if (d->meetingId.isEmpty()) {
        errors << "Meeting ID is required";
    }
    
    if (!validateEmail(d->recipientEmail)) {
        errors << "Valid recipient email is required";
    }
    
    if (d->senderEmail.isEmpty() || !validateEmail(d->senderEmail)) {
        errors << "Valid sender email is required";
    }
    
    if (isExpired()) {
        errors << "Invitation has expired";
    }
    
    return errors;
}

QString Invitation::generateInvitationId()
{
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QString Invitation::generateInvitationUrl(const QString& meetingUrl, const QString& invitationId)
{
    if (meetingUrl.isEmpty() || invitationId.isEmpty()) {
        return QString();
    }
    
    QUrl url(meetingUrl);
    if (!url.isValid()) {
        return QString();
    }
    
    // Add invitation ID as a query parameter
    QString query = url.query();
    if (!query.isEmpty()) {
        query += "&";
    }
    query += QString("invitation=%1").arg(invitationId);
    url.setQuery(query);
    
    return url.toString();
}

bool Invitation::validateEmail(const QString& email)
{
    if (email.isEmpty()) {
        return false;
    }
    
    // Basic email validation regex
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

QString Invitation::formatMessage(const QString& templateStr, const QVariantMap& variables)
{
    QString result = templateStr;
    
    for (auto it = variables.begin(); it != variables.end(); ++it) {
        QString placeholder = QString("{%1}").arg(it.key());
        result.replace(placeholder, it.value().toString());
    }
    
    return result;
}

void Invitation::initializeDefaultSettings()
{
    d->settings["autoReminder"] = true;
    d->settings["allowResponse"] = true;
    d->settings["trackOpening"] = true;
    d->settings["includeCalendar"] = true;
    d->settings["language"] = "en";
}

bool Invitation::validateEmailFormat(const QString& email) const
{
    return validateEmail(email);
}

QString Invitation::generateDefaultSubject() const
{
    QString senderName = d->senderName.isEmpty() ? "Someone" : d->senderName;
    return QString("Meeting Invitation from %1").arg(senderName);
}

QString Invitation::generateDefaultMessage() const
{
    QVariantMap variables;
    variables["senderName"] = d->senderName.isEmpty() ? "Someone" : d->senderName;
    variables["recipientName"] = d->recipientName.isEmpty() ? "there" : d->recipientName;
    variables["meetingUrl"] = d->meetingUrl;
    variables["invitationUrl"] = d->invitationUrl;
    
    QString templateStr = "Hi {recipientName},\n\n"
                         "{senderName} has invited you to join a meeting.\n\n"
                         "Join the meeting: {meetingUrl}\n\n"
                         "Best regards,\n"
                         "Jitsi Meet Qt";
    
    return formatMessage(templateStr, variables);
}
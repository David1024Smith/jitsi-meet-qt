#include "models/RecentItem.h"
#include <QJsonObject>
#include <QUrl>
#include <QRegularExpression>

QJsonObject RecentItem::toJson() const {
    QJsonObject json;
    json["url"] = url;
    json["displayName"] = displayName;
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["accessCount"] = accessCount;
    return json;
}

RecentItem RecentItem::fromJson(const QJsonObject& json) {
    RecentItem item;
    item.url = json["url"].toString();
    item.displayName = json["displayName"].toString();
    item.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    item.accessCount = json["accessCount"].toInt();
    
    // Ensure timestamp is valid
    if (!item.timestamp.isValid()) {
        item.timestamp = QDateTime::currentDateTime();
    }
    
    return item;
}

QString RecentItem::extractRoomNameFromUrl(const QString& url) const {
    if (url.isEmpty()) {
        return QString();
    }
    
    QUrl qurl(url);
    QString path = qurl.path();
    
    // Remove leading slash
    if (path.startsWith('/')) {
        path = path.mid(1);
    }
    
    // For Jitsi Meet URLs, the room name is typically the last part of the path
    QStringList pathParts = path.split('/', Qt::SkipEmptyParts);
    if (!pathParts.isEmpty()) {
        return pathParts.last();
    }
    
    // If we can't extract a room name, return the host
    return qurl.host();
}
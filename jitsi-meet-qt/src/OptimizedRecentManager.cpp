#include "OptimizedRecentManager.h"
#include <QSettings>
#include <QStandardPaths>
#include <QDir>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

OptimizedRecentManager* OptimizedRecentManager::s_instance = nullptr;

OptimizedRecentManager::OptimizedRecentManager(QObject *parent)
    : QObject(parent)
    , m_maxListSize(50)
    , m_initialized(false)
{
    s_instance = this;
}

OptimizedRecentManager::~OptimizedRecentManager()
{
    if (m_initialized) {
        saveRecentMeetings();
    }
    s_instance = nullptr;
}

OptimizedRecentManager* OptimizedRecentManager::instance()
{
    return s_instance;
}

bool OptimizedRecentManager::initialize()
{
    if (m_initialized) {
        return true;
    }

    qDebug() << "Initializing OptimizedRecentManager...";
    
    // Load existing recent meetings
    bool success = loadRecentMeetings();
    
    m_initialized = true;
    qDebug() << "OptimizedRecentManager initialized successfully";
    
    return success;
}

void OptimizedRecentManager::shutdown()
{
    if (m_initialized) {
        saveRecentMeetings();
    }
    m_initialized = false;
}

bool OptimizedRecentManager::addRecentMeeting(const QString& meetingId, const QString& displayName, 
                                            const QString& url, const QVariantMap& metadata)
{
    if (!validateMeetingId(meetingId)) {
        return false;
    }

    // Check if meeting already exists
    for (int i = 0; i < m_recentMeetings.size(); ++i) {
        if (m_recentMeetings[i].meetingId == meetingId) {
            // Update existing meeting
            m_recentMeetings[i].displayName = displayName;
            m_recentMeetings[i].url = url;
            m_recentMeetings[i].lastJoined = QDateTime::currentDateTime();
            m_recentMeetings[i].joinCount++;
            m_recentMeetings[i].metadata = metadata;
            
            sortRecentMeetings();
            emit recentMeetingUpdated(meetingId);
            emit recentMeetingsChanged();
            return true;
        }
    }

    // Add new meeting
    RecentItem item;
    item.meetingId = meetingId;
    item.displayName = displayName;
    item.url = url;
    item.lastJoined = QDateTime::currentDateTime();
    item.joinCount = 1;
    item.favorite = false;
    item.metadata = metadata;

    m_recentMeetings.append(item);
    sortRecentMeetings();
    pruneRecentMeetings();

    emit recentMeetingAdded(meetingId);
    emit recentMeetingsChanged();
    
    return true;
}

bool OptimizedRecentManager::updateRecentMeeting(const QString& meetingId, const QString& displayName, 
                                                const QString& url, const QVariantMap& metadata)
{
    for (int i = 0; i < m_recentMeetings.size(); ++i) {
        if (m_recentMeetings[i].meetingId == meetingId) {
            m_recentMeetings[i].displayName = displayName;
            m_recentMeetings[i].url = url;
            m_recentMeetings[i].metadata = metadata;
            
            emit recentMeetingUpdated(meetingId);
            emit recentMeetingsChanged();
            return true;
        }
    }
    
    return false;
}

bool OptimizedRecentManager::removeRecentMeeting(const QString& meetingId)
{
    for (int i = 0; i < m_recentMeetings.size(); ++i) {
        if (m_recentMeetings[i].meetingId == meetingId) {
            m_recentMeetings.removeAt(i);
            emit recentMeetingRemoved(meetingId);
            emit recentMeetingsChanged();
            return true;
        }
    }
    
    return false;
}

void OptimizedRecentManager::clearRecentMeetings()
{
    m_recentMeetings.clear();
    emit recentMeetingsChanged();
}

QList<OptimizedRecentManager::RecentItem> OptimizedRecentManager::getRecentMeetings(int limit) const
{
    if (limit < 0 || limit >= m_recentMeetings.size()) {
        return m_recentMeetings;
    }
    
    return m_recentMeetings.mid(0, limit);
}

QList<OptimizedRecentManager::RecentItem> OptimizedRecentManager::getFavoriteMeetings() const
{
    QList<RecentItem> favorites;
    for (const auto& item : m_recentMeetings) {
        if (item.favorite) {
            favorites.append(item);
        }
    }
    return favorites;
}

bool OptimizedRecentManager::setMeetingFavorite(const QString& meetingId, bool favorite)
{
    for (int i = 0; i < m_recentMeetings.size(); ++i) {
        if (m_recentMeetings[i].meetingId == meetingId) {
            m_recentMeetings[i].favorite = favorite;
            emit favoriteStatusChanged(meetingId, favorite);
            emit recentMeetingsChanged();
            return true;
        }
    }
    
    return false;
}

bool OptimizedRecentManager::containsMeeting(const QString& meetingId) const
{
    for (const auto& item : m_recentMeetings) {
        if (item.meetingId == meetingId) {
            return true;
        }
    }
    return false;
}

OptimizedRecentManager::RecentItem OptimizedRecentManager::getMeetingDetails(const QString& meetingId) const
{
    for (const auto& item : m_recentMeetings) {
        if (item.meetingId == meetingId) {
            return item;
        }
    }
    return RecentItem();
}

void OptimizedRecentManager::setMaxListSize(int size)
{
    m_maxListSize = qMax(1, size);
    pruneRecentMeetings();
}

int OptimizedRecentManager::maxListSize() const
{
    return m_maxListSize;
}

bool OptimizedRecentManager::saveRecentMeetings()
{
    QString filePath = getStorageFilePath();
    QDir().mkpath(QFileInfo(filePath).absolutePath());

    QJsonArray jsonArray;
    for (const auto& item : m_recentMeetings) {
        QJsonObject jsonObj;
        jsonObj["meetingId"] = item.meetingId;
        jsonObj["displayName"] = item.displayName;
        jsonObj["url"] = item.url;
        jsonObj["lastJoined"] = item.lastJoined.toString(Qt::ISODate);
        jsonObj["joinCount"] = item.joinCount;
        jsonObj["favorite"] = item.favorite;
        
        // Convert metadata to JSON
        QJsonObject metadataObj;
        for (auto it = item.metadata.constBegin(); it != item.metadata.constEnd(); ++it) {
            metadataObj[it.key()] = QJsonValue::fromVariant(it.value());
        }
        jsonObj["metadata"] = metadataObj;
        
        jsonArray.append(jsonObj);
    }

    QJsonDocument doc(jsonArray);
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        return true;
    }

    qWarning() << "Failed to save recent meetings to:" << filePath;
    return false;
}

bool OptimizedRecentManager::loadRecentMeetings()
{
    QString filePath = getStorageFilePath();
    QFile file(filePath);
    
    if (!file.exists()) {
        qDebug() << "Recent meetings file does not exist, starting with empty list";
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open recent meetings file:" << filePath;
        return false;
    }

    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "Failed to parse recent meetings JSON:" << error.errorString();
        return false;
    }

    m_recentMeetings.clear();
    QJsonArray jsonArray = doc.array();
    
    for (const auto& value : jsonArray) {
        QJsonObject jsonObj = value.toObject();
        
        RecentItem item;
        item.meetingId = jsonObj["meetingId"].toString();
        item.displayName = jsonObj["displayName"].toString();
        item.url = jsonObj["url"].toString();
        item.lastJoined = QDateTime::fromString(jsonObj["lastJoined"].toString(), Qt::ISODate);
        item.joinCount = jsonObj["joinCount"].toInt();
        item.favorite = jsonObj["favorite"].toBool();
        
        // Convert metadata from JSON
        QJsonObject metadataObj = jsonObj["metadata"].toObject();
        for (auto it = metadataObj.constBegin(); it != metadataObj.constEnd(); ++it) {
            item.metadata[it.key()] = it.value().toVariant();
        }
        
        m_recentMeetings.append(item);
    }

    sortRecentMeetings();
    qDebug() << "Loaded" << m_recentMeetings.size() << "recent meetings";
    return true;
}

void OptimizedRecentManager::sortRecentMeetings()
{
    std::sort(m_recentMeetings.begin(), m_recentMeetings.end(), 
              [](const RecentItem& a, const RecentItem& b) {
                  // Favorites first, then by last joined time
                  if (a.favorite != b.favorite) {
                      return a.favorite > b.favorite;
                  }
                  return a.lastJoined > b.lastJoined;
              });
}

bool OptimizedRecentManager::validateMeetingId(const QString& meetingId) const
{
    return !meetingId.isEmpty() && meetingId.length() <= 255;
}

void OptimizedRecentManager::pruneRecentMeetings()
{
    if (m_recentMeetings.size() > m_maxListSize) {
        // Keep favorites and remove oldest non-favorites
        QList<RecentItem> favorites;
        QList<RecentItem> nonFavorites;
        
        for (const auto& item : m_recentMeetings) {
            if (item.favorite) {
                favorites.append(item);
            } else {
                nonFavorites.append(item);
            }
        }
        
        // Sort non-favorites by last joined time
        std::sort(nonFavorites.begin(), nonFavorites.end(), 
                  [](const RecentItem& a, const RecentItem& b) {
                      return a.lastJoined > b.lastJoined;
                  });
        
        m_recentMeetings = favorites;
        int remainingSlots = m_maxListSize - favorites.size();
        if (remainingSlots > 0 && !nonFavorites.isEmpty()) {
            m_recentMeetings.append(nonFavorites.mid(0, remainingSlots));
        }
        
        sortRecentMeetings();
    }
}

QString OptimizedRecentManager::getStorageFilePath() const
{
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataPath + "/recent_meetings.json";
}
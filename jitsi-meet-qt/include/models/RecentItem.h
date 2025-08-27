#ifndef RECENTITEM_H
#define RECENTITEM_H

#include <QString>
#include <QDateTime>
#include <QJsonObject>

/**
 * @brief Data model for storing recent meeting information
 * 
 * This class represents a recent meeting item with URL, display name,
 * timestamp, and access count information.
 */
struct RecentItem {
    QString url;
    QString displayName;
    QDateTime timestamp;
    int accessCount;
    
    /**
     * @brief Default constructor
     */
    RecentItem() : accessCount(0) {}
    
    /**
     * @brief Constructor with parameters
     */
    RecentItem(const QString& url, const QString& displayName = QString())
        : url(url), displayName(displayName), timestamp(QDateTime::currentDateTime()), accessCount(1) {
        if (displayName.isEmpty()) {
            this->displayName = extractRoomNameFromUrl(url);
        }
    }
    
    /**
     * @brief Check if the item is valid
     */
    bool isValid() const {
        return !url.isEmpty() && timestamp.isValid();
    }
    
    /**
     * @brief Get display text for the item
     */
    QString getDisplayText() const {
        return displayName.isEmpty() ? url : displayName;
    }
    
    /**
     * @brief Comparison operator for sorting (newest first)
     */
    bool operator<(const RecentItem& other) const {
        return timestamp > other.timestamp; // 最新的在前
    }
    
    /**
     * @brief Equality operator
     */
    bool operator==(const RecentItem& other) const {
        return url == other.url;
    }
    
    /**
     * @brief Convert to JSON object for serialization
     */
    QJsonObject toJson() const;
    
    /**
     * @brief Create from JSON object for deserialization
     */
    static RecentItem fromJson(const QJsonObject& json);
    
    /**
     * @brief Update access information
     */
    void updateAccess() {
        timestamp = QDateTime::currentDateTime();
        accessCount++;
    }

private:
    /**
     * @brief Extract room name from URL
     */
    QString extractRoomNameFromUrl(const QString& url) const;
};

#endif // RECENTITEM_H
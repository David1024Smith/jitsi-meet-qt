#include "../include/PerformanceMetrics.h"
#include <QJsonDocument>

QVariantMap PerformanceMetrics::toVariantMap() const
{
    QVariantMap map;
    
    map["timestamp"] = timestamp;
    map["cpuUsage"] = cpuUsage;
    map["cpuTemperature"] = cpuTemperature;
    map["cpuCores"] = cpuCores;
    map["memoryUsed"] = static_cast<qulonglong>(memoryUsed);
    map["memoryTotal"] = static_cast<qulonglong>(memoryTotal);
    map["memoryUsage"] = memoryUsage;
    map["networkBytesReceived"] = static_cast<qulonglong>(networkBytesReceived);
    map["networkBytesSent"] = static_cast<qulonglong>(networkBytesSent);
    map["networkLatency"] = networkLatency;
    map["threadCount"] = threadCount;
    map["handleCount"] = static_cast<qulonglong>(handleCount);
    map["frameRate"] = frameRate;
    map["diskReadBytes"] = static_cast<qulonglong>(diskReadBytes);
    map["diskWriteBytes"] = static_cast<qulonglong>(diskWriteBytes);
    map["diskUsage"] = diskUsage;
    
    return map;
}

PerformanceMetrics PerformanceMetrics::fromVariantMap(const QVariantMap& map)
{
    PerformanceMetrics metrics;
    
    metrics.timestamp = map.value("timestamp").toDateTime();
    metrics.cpuUsage = map.value("cpuUsage").toDouble();
    metrics.cpuTemperature = map.value("cpuTemperature").toDouble();
    metrics.cpuCores = map.value("cpuCores").toInt();
    metrics.memoryUsed = map.value("memoryUsed").toULongLong();
    metrics.memoryTotal = map.value("memoryTotal").toULongLong();
    metrics.memoryUsage = map.value("memoryUsage").toDouble();
    metrics.networkBytesReceived = map.value("networkBytesReceived").toULongLong();
    metrics.networkBytesSent = map.value("networkBytesSent").toULongLong();
    metrics.networkLatency = map.value("networkLatency").toDouble();
    metrics.threadCount = map.value("threadCount").toInt();
    metrics.handleCount = map.value("handleCount").toULongLong();
    metrics.frameRate = map.value("frameRate").toDouble();
    metrics.diskReadBytes = map.value("diskReadBytes").toULongLong();
    metrics.diskWriteBytes = map.value("diskWriteBytes").toULongLong();
    metrics.diskUsage = map.value("diskUsage").toDouble();
    
    return metrics;
}

QJsonObject PerformanceMetrics::toJson() const
{
    QJsonObject json;
    
    json["timestamp"] = timestamp.toString(Qt::ISODate);
    json["cpuUsage"] = cpuUsage;
    json["cpuTemperature"] = cpuTemperature;
    json["cpuCores"] = cpuCores;
    json["memoryUsed"] = static_cast<qint64>(memoryUsed);
    json["memoryTotal"] = static_cast<qint64>(memoryTotal);
    json["memoryUsage"] = memoryUsage;
    json["networkBytesReceived"] = static_cast<qint64>(networkBytesReceived);
    json["networkBytesSent"] = static_cast<qint64>(networkBytesSent);
    json["networkLatency"] = networkLatency;
    json["threadCount"] = threadCount;
    json["handleCount"] = static_cast<qint64>(handleCount);
    json["frameRate"] = frameRate;
    json["diskReadBytes"] = static_cast<qint64>(diskReadBytes);
    json["diskWriteBytes"] = static_cast<qint64>(diskWriteBytes);
    json["diskUsage"] = diskUsage;
    
    return json;
}

PerformanceMetrics PerformanceMetrics::fromJson(const QJsonObject& json)
{
    PerformanceMetrics metrics;
    
    metrics.timestamp = QDateTime::fromString(json["timestamp"].toString(), Qt::ISODate);
    metrics.cpuUsage = json["cpuUsage"].toDouble();
    metrics.cpuTemperature = json["cpuTemperature"].toDouble();
    metrics.cpuCores = json["cpuCores"].toInt();
    metrics.memoryUsed = static_cast<quint64>(json["memoryUsed"].toVariant().toLongLong());
    metrics.memoryTotal = static_cast<quint64>(json["memoryTotal"].toVariant().toLongLong());
    metrics.memoryUsage = json["memoryUsage"].toDouble();
    metrics.networkBytesReceived = static_cast<quint64>(json["networkBytesReceived"].toVariant().toLongLong());
    metrics.networkBytesSent = static_cast<quint64>(json["networkBytesSent"].toVariant().toLongLong());
    metrics.networkLatency = json["networkLatency"].toDouble();
    metrics.threadCount = json["threadCount"].toInt();
    metrics.handleCount = static_cast<quint64>(json["handleCount"].toVariant().toLongLong());
    metrics.frameRate = json["frameRate"].toDouble();
    metrics.diskReadBytes = static_cast<quint64>(json["diskReadBytes"].toVariant().toLongLong());
    metrics.diskWriteBytes = static_cast<quint64>(json["diskWriteBytes"].toVariant().toLongLong());
    metrics.diskUsage = json["diskUsage"].toDouble();
    
    return metrics;
}

void PerformanceMetrics::reset()
{
    timestamp = QDateTime();
    cpuUsage = 0.0;
    cpuTemperature = 0.0;
    cpuCores = 0;
    memoryUsed = 0;
    memoryTotal = 0;
    memoryUsage = 0.0;
    networkBytesReceived = 0;
    networkBytesSent = 0;
    networkLatency = 0.0;
    threadCount = 0;
    handleCount = 0;
    frameRate = 0.0;
    diskReadBytes = 0;
    diskWriteBytes = 0;
    diskUsage = 0.0;
}

bool PerformanceMetrics::isValid() const
{
    return timestamp.isValid() && 
           cpuUsage >= 0.0 && cpuUsage <= 100.0 &&
           memoryUsage >= 0.0 && memoryUsage <= 100.0 &&
           diskUsage >= 0.0 && diskUsage <= 100.0;
}
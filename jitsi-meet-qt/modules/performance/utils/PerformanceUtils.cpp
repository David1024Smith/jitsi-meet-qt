#include "PerformanceUtils.h"
#include <QDebug>

// Implementation of PerformanceUtils methods would go here
// For now, this is a placeholder to satisfy the build system

namespace PerformanceUtils {

QString formatBytes(qint64 bytes)
{
    const qint64 KB = 1024;
    const qint64 MB = KB * 1024;
    const qint64 GB = MB * 1024;
    
    if (bytes >= GB) {
        return QString("%1 GB").arg(bytes / (double)GB, 0, 'f', 2);
    } else if (bytes >= MB) {
        return QString("%1 MB").arg(bytes / (double)MB, 0, 'f', 1);
    } else if (bytes >= KB) {
        return QString("%1 KB").arg(bytes / (double)KB, 0, 'f', 1);
    } else {
        return QString("%1 B").arg(bytes);
    }
}

QString formatPercentage(double percentage)
{
    return QString("%1%").arg(percentage, 0, 'f', 1);
}

QString formatTime(double milliseconds)
{
    if (milliseconds >= 1000) {
        return QString("%1 s").arg(milliseconds / 1000.0, 0, 'f', 2);
    } else {
        return QString("%1 ms").arg(milliseconds, 0, 'f', 1);
    }
}

} // namespace PerformanceUtils
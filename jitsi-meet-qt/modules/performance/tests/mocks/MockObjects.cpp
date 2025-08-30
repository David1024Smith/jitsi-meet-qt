#include "MockObjects.h"
#include <QThread>
#include <QElapsedTimer>
#include <QDir>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
#include <QCoreApplication>

// MockMonitor Implementation
MockMonitor::MockMonitor(const QString& name, QObject* parent)
    : BaseMonitor(name, parent)
{
}

bool MockMonitor::initializeMonitor()
{
    if (m_simulateError) {
        addError(m_errorMessage);
        return false;
    }
    
    if (m_simulateDelayMs > 0) {
        QThread::msleep(m_simulateDelayMs);
    }
    
    return true;
}

ResourceUsage MockMonitor::collectResourceUsage()
{
    m_callCount++;
    
    if (m_simulateError && m_callCount % 5 == 0) {
        addError(m_errorMessage);
        return ResourceUsage();
    }
    
    if (m_simulateDelayMs > 0) {
        QThread::msleep(m_simulateDelayMs);
    }
    
    ResourceUsage usage;
    usage.timestamp = QDateTime::currentDateTime();
    usage.cpuUsage = m_mockCpuUsage + (QRandomGenerator::global()->bounded(10) - 5); // ±5% variation
    usage.memoryUsage = m_mockMemoryUsage + (QRandomGenerator::global()->bounded(100 * 1024 * 1024)); // ±100MB variation
    usage.networkLatency = m_mockNetworkLatency + (QRandomGenerator::global()->bounded(20) - 10); // ±10ms variation
    usage.networkBandwidth = m_mockBandwidth + (QRandomGenerator::glo
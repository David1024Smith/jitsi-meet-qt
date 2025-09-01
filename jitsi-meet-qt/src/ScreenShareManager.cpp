#include "../include/ScreenShareManager.h"
#include <QDebug>
#include <QVideoWidget>

ScreenShareManager::ScreenShareManager(QObject *parent)
    : QObject(parent)
    , m_isSharing(false)
    , m_currentScreenId(-1)
{
    qDebug() << "ScreenShareManager created";
}

ScreenShareManager::~ScreenShareManager()
{
    stopScreenShare();
    qDebug() << "ScreenShareManager destroyed";
}

bool ScreenShareManager::startScreenShare(int screenId)
{
    m_currentScreenId = screenId;
    m_isSharing = true;
    emit screenShareStarted();
    qDebug() << "Screen sharing started for screen:" << screenId;
    return true;
}

void ScreenShareManager::stopScreenShare()
{
    if (m_isSharing) {
        m_isSharing = false;
        emit screenShareStopped();
        qDebug() << "Screen sharing stopped";
    }
}

bool ScreenShareManager::isScreenSharing() const
{
    return m_isSharing;
}

QStringList ScreenShareManager::getAvailableScreens() const
{
    return QStringList() << "Screen 1" << "Screen 2";
}

bool ScreenShareManager::showScreenSelectionDialog()
{
    // Stub implementation
    return true;
}

QVideoWidget* ScreenShareManager::localScreenShareWidget() const
{
    // Stub implementation
    return nullptr;
}
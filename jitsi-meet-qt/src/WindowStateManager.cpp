#include "../include/WindowStateManager.h"
#include <QDebug>

WindowStateManager::WindowStateManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(this))
{
    qDebug() << "WindowStateManager created";
}

WindowStateManager::~WindowStateManager()
{
    qDebug() << "WindowStateManager destroyed";
}

void WindowStateManager::saveWindowState(QWidget* widget, const QString& key)
{
    if (!widget) return;
    
    m_settings->beginGroup("WindowStates");
    m_settings->setValue(key + "/geometry", widget->saveGeometry());
    if (widget->isWindow()) {
        m_settings->setValue(key + "/windowState", static_cast<int>(widget->windowState()));
    }
    m_settings->endGroup();
    
    qDebug() << "Window state saved for key:" << key;
}

void WindowStateManager::restoreWindowState(QWidget* widget, const QString& key)
{
    if (!widget) return;
    
    m_settings->beginGroup("WindowStates");
    QByteArray geometry = m_settings->value(key + "/geometry").toByteArray();
    if (!geometry.isEmpty()) {
        widget->restoreGeometry(geometry);
    }
    
    if (widget->isWindow()) {
        Qt::WindowState state = static_cast<Qt::WindowState>(
            m_settings->value(key + "/windowState", Qt::WindowNoState).toInt());
        widget->setWindowState(state);
    }
    m_settings->endGroup();
    
    qDebug() << "Window state restored for key:" << key;
}

void WindowStateManager::saveGeometry(QWidget* widget, const QString& key)
{
    if (!widget) return;
    
    m_settings->setValue("Geometry/" + key, widget->saveGeometry());
    qDebug() << "Geometry saved for key:" << key;
}

void WindowStateManager::restoreGeometry(QWidget* widget, const QString& key)
{
    if (!widget) return;
    
    QByteArray geometry = m_settings->value("Geometry/" + key).toByteArray();
    if (!geometry.isEmpty()) {
        widget->restoreGeometry(geometry);
    }
    qDebug() << "Geometry restored for key:" << key;
}
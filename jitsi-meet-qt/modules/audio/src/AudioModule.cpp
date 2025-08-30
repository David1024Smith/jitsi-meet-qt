#include "AudioModule.h"
#include <QDebug>

AudioModule::AudioModule(QObject *parent)
    : QObject(parent)
    , m_status(Uninitialized)
    , m_initialized(false)
{
    qDebug() << "AudioModule created";
}

AudioModule::~AudioModule()
{
    if (m_initialized) {
        shutdown();
    }
    qDebug() << "AudioModule destroyed";
}

bool AudioModule::initialize()
{
    if (m_status == Ready || m_status == Active) {
        return true;
    }

    setStatus(Initializing);
    
    if (doInitialize()) {
        m_initialized = true;
        setStatus(Ready);
        emit initialized();
        return true;
    } else {
        setStatus(Error);
        return false;
    }
}

void AudioModule::shutdown()
{
    if (m_status == Shutdown || m_status == Uninitialized) {
        return;
    }

    doCleanup();
    m_initialized = false;
    setStatus(Shutdown);
    emit shutdownCompleted();
}

AudioModule::ModuleStatus AudioModule::status() const
{
    return m_status;
}

QString AudioModule::version()
{
    return QStringLiteral("1.0.0");
}

QString AudioModule::moduleName()
{
    return QStringLiteral("Audio Module");
}

bool AudioModule::isAvailable() const
{
    return m_status == Ready || m_status == Active;
}

void AudioModule::setStatus(ModuleStatus status)
{
    if (m_status != status) {
        m_status = status;
        emit statusChanged(status);
    }
}

void AudioModule::reportError(const QString &error)
{
    m_lastError = error;
    qWarning() << "AudioModule error:" << error;
    emit errorOccurred(error);
}

bool AudioModule::doInitialize()
{
    // TODO: 实现具体的初始化逻辑
    qDebug() << "AudioModule initializing...";
    return true;
}

void AudioModule::doCleanup()
{
    // TODO: 实现具体的清理逻辑
    qDebug() << "AudioModule cleaning up...";
}
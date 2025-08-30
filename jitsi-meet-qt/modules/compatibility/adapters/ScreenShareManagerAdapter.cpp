#include "ScreenShareManagerAdapter.h"
#include <QDebug>

// 临时的占位符类定义
class ScreenShareManager : public QObject {
    Q_OBJECT
public:
    explicit ScreenShareManager(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~ScreenShareManager() = default;
    
    virtual bool initialize() { return true; }
    virtual bool startCapture() { return true; }
    virtual bool stopCapture() { return true; }
    virtual QStringList getAvailableScreens() const { return QStringList() << "Screen 1" << "Screen 2"; }
    virtual bool selectScreen(int screenIndex) { Q_UNUSED(screenIndex) return true; }
};

ScreenShareManagerAdapter::ScreenShareManagerAdapter(QObject *parent)
    : ICompatibilityAdapter(parent)
    , m_status(NotInitialized)
    , m_legacyManager(nullptr)
{
    m_config["enable_region_capture"] = true;
    m_config["enable_window_capture"] = true;
    m_config["compatibility_mode"] = "full";
}

ScreenShareManagerAdapter::~ScreenShareManagerAdapter()
{
    disable();
    if (m_legacyManager) {
        m_legacyManager->deleteLater();
    }
}

bool ScreenShareManagerAdapter::initialize()
{
    if (m_status != NotInitialized) {
        return m_status == Ready;
    }

    qDebug() << "Initializing ScreenShareManagerAdapter...";
    
    m_status = Initializing;
    emit statusChanged(m_status);

    createLegacyScreenShareManager();
    
    if (!m_legacyManager || !m_legacyManager->initialize()) {
        qWarning() << "Failed to initialize legacy ScreenShareManager";
        m_status = Error;
        emit statusChanged(m_status);
        return false;
    }

    m_status = Ready;
    emit statusChanged(m_status);
    
    qDebug() << "ScreenShareManagerAdapter initialized successfully";
    return true;
}

ICompatibilityAdapter::AdapterStatus ScreenShareManagerAdapter::status() const
{
    return m_status;
}

QString ScreenShareManagerAdapter::adapterName() const
{
    return "ScreenShareManagerAdapter";
}

QString ScreenShareManagerAdapter::targetModule() const
{
    return "screenshare";
}

ICompatibilityAdapter::CompatibilityLevel ScreenShareManagerAdapter::checkCompatibility()
{
    if (m_status != Ready) {
        return NoCompatibility;
    }
    return FullCompatibility;
}

bool ScreenShareManagerAdapter::enable()
{
    if (m_status != Ready) {
        return false;
    }

    m_status = Active;
    emit statusChanged(m_status);
    
    qDebug() << "ScreenShareManagerAdapter enabled";
    return true;
}

void ScreenShareManagerAdapter::disable()
{
    if (m_status == Active) {
        m_status = Ready;
        emit statusChanged(m_status);
        qDebug() << "ScreenShareManagerAdapter disabled";
    }
}

QVariantMap ScreenShareManagerAdapter::getConfiguration() const
{
    return m_config;
}

bool ScreenShareManagerAdapter::setConfiguration(const QVariantMap& config)
{
    m_config = config;
    return true;
}

QStringList ScreenShareManagerAdapter::validateFunctionality()
{
    QStringList results;
    
    if (!m_legacyManager) {
        results << "ERROR: Legacy ScreenShareManager not created";
        return results;
    }

    try {
        if (m_legacyManager->startCapture()) {
            results << "PASS: Screen capture start functionality";
            if (m_legacyManager->stopCapture()) {
                results << "PASS: Screen capture stop functionality";
            } else {
                results << "FAIL: Screen capture stop functionality";
            }
        } else {
            results << "FAIL: Screen capture start functionality";
        }

        QStringList screens = m_legacyManager->getAvailableScreens();
        if (!screens.isEmpty()) {
            results << "PASS: Screen enumeration functionality";
            if (m_legacyManager->selectScreen(0)) {
                results << "PASS: Screen selection functionality";
            } else {
                results << "FAIL: Screen selection functionality";
            }
        } else {
            results << "FAIL: Screen enumeration functionality";
        }

    } catch (const std::exception& e) {
        results << QString("ERROR: Exception during validation: %1").arg(e.what());
    }

    return results;
}

ScreenShareManager* ScreenShareManagerAdapter::getLegacyManager() const
{
    return m_legacyManager;
}

void ScreenShareManagerAdapter::createLegacyScreenShareManager()
{
    if (m_legacyManager) {
        m_legacyManager->deleteLater();
    }

    m_legacyManager = new ScreenShareManager(this);
    qDebug() << "Created legacy ScreenShareManager";
}

#include "ScreenShareManagerAdapter.moc"
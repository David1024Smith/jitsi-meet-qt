#include "CameraFactory.h"
#include "../include/CameraManager.h"
#include "../include/CameraModule.h"
#include <QDebug>
#include <QUuid>

// 静态成员初始化
CameraFactory* CameraFactory::s_instance = nullptr;

CameraFactory* CameraFactory::instance()
{
    if (!s_instance) {
        s_instance = new CameraFactory();
    }
    return s_instance;
}

void CameraFactory::destroyInstance()
{
    if (s_instance) {
        delete s_instance;
        s_instance = nullptr;
    }
}

CameraFactory::CameraFactory(QObject *parent)
    : QObject(parent)
    , m_defaultType(LocalCamera)
    , m_globalAutoRecovery(true)
    , m_globalMonitoring(false)
    , m_instanceCounter(0)
{
    qDebug() << "CameraFactory: Initializing...";
    initializeDefaultTemplates();
    qDebug() << "CameraFactory: Initialization completed";
}

CameraFactory::~CameraFactory()
{
    qDebug() << "CameraFactory: Destroying...";
    
    // 销毁所有管理器
    for (auto it = m_managers.begin(); it != m_managers.end(); ++it) {
        delete it.value();
    }
    m_managers.clear();
    
    qDebug() << "CameraFactory: Destruction completed";
}

CameraManager* CameraFactory::createManager(CameraType type, const QString& name)
{
    QString managerName = name.isEmpty() ? generateUniqueName("camera") : name;
    
    qDebug() << "CameraFactory: Creating manager" << managerName << "of type" << type;
    
    if (m_managers.contains(managerName)) {
        qWarning() << "CameraFactory: Manager with name" << managerName << "already exists";
        return m_managers[managerName];
    }
    
    // 创建管理器
    CameraManager* manager = new CameraManager(this);
    
    // 应用全局设置
    applyGlobalSettings(manager);
    
    // 根据类型应用特定配置
    switch (type) {
        case LocalCamera:
            manager->setPreset(CameraManager::StandardQuality);
            break;
        case RemoteCamera:
            manager->setPreset(CameraManager::StandardQuality);
            manager->enableAutoRecovery(false); // 远程摄像头不自动恢复
            break;
        case ScreenShareCamera:
            manager->setPreset(CameraManager::HighQuality);
            break;
        case VirtualCamera:
            manager->setPreset(CameraManager::LowQuality);
            break;
    }
    
    // 初始化管理器
    if (!manager->initialize()) {
        qWarning() << "CameraFactory: Failed to initialize manager" << managerName;
        delete manager;
        return nullptr;
    }
    
    // 存储管理器
    m_managers[managerName] = manager;
    
    emit managerCreated(managerName, manager);
    qDebug() << "CameraFactory: Manager" << managerName << "created successfully";
    
    return manager;
}

CameraManager* CameraFactory::createManagerWithTemplate(const QString& templateName, const QString& name)
{
    qDebug() << "CameraFactory: Creating manager with template" << templateName;
    
    if (!m_templates.contains(templateName)) {
        qWarning() << "CameraFactory: Template" << templateName << "not found";
        return nullptr;
    }
    
    CameraTemplate tmpl = m_templates[templateName];
    QString managerName = name.isEmpty() ? generateUniqueName(templateName) : name;
    
    if (m_managers.contains(managerName)) {
        qWarning() << "CameraFactory: Manager with name" << managerName << "already exists";
        return m_managers[managerName];
    }
    
    // 创建管理器
    CameraManager* manager = new CameraManager(this);
    
    // 应用模板配置
    manager->setPreset(CameraManager::StandardQuality);
    manager->enableAutoRecovery(tmpl.enableRecovery);
    
    // 应用全局设置
    applyGlobalSettings(manager);
    
    // 初始化管理器
    if (!manager->initialize()) {
        qWarning() << "CameraFactory: Failed to initialize manager" << managerName;
        delete manager;
        return nullptr;
    }
    
    // 自动启动（如果模板配置了）
    if (tmpl.autoStart) {
        manager->startDefault();
    }
    
    // 存储管理器
    m_managers[managerName] = manager;
    
    emit managerCreated(managerName, manager);
    qDebug() << "CameraFactory: Manager" << managerName << "created with template" << templateName;
    
    return manager;
}

CameraManager* CameraFactory::getManager(const QString& name)
{
    return m_managers.value(name, nullptr);
}

void CameraFactory::destroyManager(const QString& name)
{
    qDebug() << "CameraFactory: Destroying manager" << name;
    
    if (!m_managers.contains(name)) {
        qWarning() << "CameraFactory: Manager" << name << "not found";
        return;
    }
    
    CameraManager* manager = m_managers.take(name);
    if (manager) {
        manager->stopCamera();
        manager->deleteLater();
    }
    
    emit managerDestroyed(name);
    qDebug() << "CameraFactory: Manager" << name << "destroyed";
}

QStringList CameraFactory::getManagerNames() const
{
    return m_managers.keys();
}

void CameraFactory::registerTemplate(const QString& name, const CameraTemplate& tmpl)
{
    qDebug() << "CameraFactory: Registering template" << name;
    
    m_templates[name] = tmpl;
    emit templateRegistered(name);
    
    qDebug() << "CameraFactory: Template" << name << "registered";
}

CameraFactory::CameraTemplate CameraFactory::getTemplate(const QString& name) const
{
    return m_templates.value(name, CameraTemplate());
}

QStringList CameraFactory::getTemplateNames() const
{
    return m_templates.keys();
}

void CameraFactory::removeTemplate(const QString& name)
{
    qDebug() << "CameraFactory: Removing template" << name;
    
    if (m_templates.remove(name) > 0) {
        emit templateRemoved(name);
        qDebug() << "CameraFactory: Template" << name << "removed";
    }
}

void CameraFactory::setDefaultCameraType(CameraType type)
{
    qDebug() << "CameraFactory: Setting default camera type to" << type;
    m_defaultType = type;
}

CameraFactory::CameraType CameraFactory::defaultCameraType() const
{
    return m_defaultType;
}

void CameraFactory::setGlobalAutoRecovery(bool enable)
{
    qDebug() << "CameraFactory: Setting global auto recovery to" << enable;
    
    m_globalAutoRecovery = enable;
    
    // 应用到所有现有管理器
    for (auto manager : m_managers) {
        manager->enableAutoRecovery(enable);
    }
}

bool CameraFactory::globalAutoRecovery() const
{
    return m_globalAutoRecovery;
}

void CameraFactory::setGlobalMonitoring(bool enable)
{
    qDebug() << "CameraFactory: Setting global monitoring to" << enable;
    
    m_globalMonitoring = enable;
    
    // 应用到所有现有管理器
    for (auto manager : m_managers) {
        manager->enableMonitoring(enable);
    }
}

bool CameraFactory::globalMonitoring() const
{
    return m_globalMonitoring;
}

CameraManager* CameraFactory::createLocalCamera(const QString& name)
{
    return createManager(LocalCamera, name);
}

CameraManager* CameraFactory::createRemoteCamera(const QString& name)
{
    return createManager(RemoteCamera, name);
}

CameraManager* CameraFactory::createScreenShareCamera(const QString& name)
{
    return createManager(ScreenShareCamera, name);
}

CameraManager* CameraFactory::localCamera()
{
    const QString localName = "local";
    
    if (!m_managers.contains(localName)) {
        return createLocalCamera(localName);
    }
    
    return m_managers[localName];
}

CameraManager* CameraFactory::screenShareCamera()
{
    const QString screenShareName = "screenshare";
    
    if (!m_managers.contains(screenShareName)) {
        return createScreenShareCamera(screenShareName);
    }
    
    return m_managers[screenShareName];
}

// === ICameraManager接口便捷方法 ===

ICameraManager* CameraFactory::createLocalCameraInterface(const QString& name)
{
    return createLocalCamera(name);
}

ICameraManager* CameraFactory::createRemoteCameraInterface(const QString& name)
{
    return createRemoteCamera(name);
}

void CameraFactory::destroyCamera(ICameraManager* camera)
{
    if (!camera) {
        return;
    }
    
    // 查找对应的管理器名称
    QString managerName;
    for (auto it = m_managers.begin(); it != m_managers.end(); ++it) {
        if (it.value() == camera) {
            managerName = it.key();
            break;
        }
    }
    
    if (!managerName.isEmpty()) {
        destroyManager(managerName);
    }
}

// === 私有方法 ===

void CameraFactory::initializeDefaultTemplates()
{
    qDebug() << "CameraFactory: Initializing default templates";
    
    // 低质量模板
    CameraTemplate lowQuality;
    lowQuality.name = "Low Quality";
    lowQuality.description = "Low quality camera for basic video calls";
    lowQuality.autoStart = false;
    lowQuality.enableRecovery = true;
    registerTemplate("low_quality", lowQuality);
    
    // 标准质量模板
    CameraTemplate standardQuality;
    standardQuality.name = "Standard Quality";
    standardQuality.description = "Standard quality camera for normal video calls";
    standardQuality.autoStart = true;
    standardQuality.enableRecovery = true;
    registerTemplate("standard_quality", standardQuality);
    
    // 高质量模板
    CameraTemplate highQuality;
    highQuality.name = "High Quality";
    highQuality.description = "High quality camera for professional video calls";
    highQuality.autoStart = false;
    highQuality.enableRecovery = true;
    registerTemplate("high_quality", highQuality);
    
    // 会议模板
    CameraTemplate conference;
    conference.name = "Conference";
    conference.description = "Optimized for conference calls";
    conference.autoStart = true;
    conference.enableRecovery = true;
    registerTemplate("conference", conference);
    
    // 屏幕共享模板
    CameraTemplate screenShare;
    screenShare.name = "Screen Share";
    screenShare.description = "Optimized for screen sharing";
    screenShare.autoStart = false;
    screenShare.enableRecovery = false;
    registerTemplate("screen_share", screenShare);
    
    qDebug() << "CameraFactory: Default templates initialized";
}

QString CameraFactory::generateUniqueName(const QString& prefix) const
{
    QString baseName = prefix.isEmpty() ? "camera" : prefix;
    QString name;
    
    do {
        const_cast<CameraFactory*>(this)->m_instanceCounter++;
        name = QString("%1_%2").arg(baseName).arg(m_instanceCounter);
    } while (m_managers.contains(name));
    
    return name;
}

void CameraFactory::applyGlobalSettings(CameraManager* manager)
{
    if (!manager) {
        return;
    }
    
    manager->enableAutoRecovery(m_globalAutoRecovery);
    manager->enableMonitoring(m_globalMonitoring);
}
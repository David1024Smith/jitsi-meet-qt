#include "StartupOptimizer.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QThread>
#include <QtConcurrent>
#include <QDebug>

StartupOptimizer::StartupOptimizer(QObject *parent)
    : QObject(parent)
    , m_optimizationLevel(Moderate)
    , m_preloadEnabled(true)
    , m_deferredInitEnabled(true)
    , m_fastStartupEnabled(false)
    , m_resourcesPreloaded(false)
    , m_preloadProgress(0)
{
    // 设置关键资源路径
    m_criticalResourcePaths = {
        ":/styles/default.qss",
        ":/styles/dark.qss",
        ":/icons/settings.svg",
        ":/icons/about.svg",
        ":/icons/back.svg",
        ":/icons/recent.svg",
        ":/images/logo.svg"
    };
    
    qDebug() << "StartupOptimizer: Initialized";
}

StartupOptimizer::~StartupOptimizer()
{
    logStartupMetrics();
}

void StartupOptimizer::setOptimizationLevel(OptimizationLevel level)
{
    m_optimizationLevel = level;
    
    switch (level) {
    case Basic:
        initializeBasicOptimizations();
        break;
    case Moderate:
        initializeModerateOptimizations();
        break;
    case Aggressive:
        initializeAggressiveOptimizations();
        break;
    }
    
    qDebug() << "StartupOptimizer: Optimization level set to" << level;
}

void StartupOptimizer::enableFastStartup()
{
    if (m_fastStartupEnabled) {
        return;
    }
    
    startPhaseTimer("FastStartup");
    
    // 设置应用程序属性以加快启动
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings, true);
    
    // 预加载关键资源
    if (m_preloadEnabled) {
        preloadCriticalResources();
    }
    
    // 延迟非关键初始化
    if (m_deferredInitEnabled) {
        deferNonCriticalInitialization();
    }
    
    m_fastStartupEnabled = true;
    endPhaseTimer("FastStartup");
    
    qDebug() << "StartupOptimizer: Fast startup enabled";
}

void StartupOptimizer::preloadCriticalResources()
{
    if (m_resourcesPreloaded) {
        return;
    }
    
    startPhaseTimer("ResourcePreload");
    
    // 在后台线程中预加载资源
    QtConcurrent::run([this]() {
        m_preloadProgress = 0;
        int totalResources = m_criticalResourcePaths.size();
        
        for (int i = 0; i < totalResources; ++i) {
            const QString& path = m_criticalResourcePaths[i];
            loadResourceInBackground(path);
            
            m_preloadProgress = ((i + 1) * 100) / totalResources;
        }
        
        m_resourcesPreloaded = true;
        emit allResourcesPreloaded();
        
        endPhaseTimer("ResourcePreload");
        qDebug() << "StartupOptimizer: All critical resources preloaded";
    });
}

void StartupOptimizer::deferNonCriticalInitialization()
{
    // 延迟翻译系统初始化
    scheduleDelayedInitialization("TranslationSystem", []() {
        qDebug() << "StartupOptimizer: Initializing translation system (deferred)";
        // 翻译系统初始化代码
    });
    
    // 延迟主题系统初始化
    scheduleDelayedInitialization("ThemeSystem", []() {
        qDebug() << "StartupOptimizer: Initializing theme system (deferred)";
        // 主题系统初始化代码
    });
    
    // 延迟错误处理系统初始化
    scheduleDelayedInitialization("ErrorHandling", []() {
        qDebug() << "StartupOptimizer: Initializing error handling system (deferred)";
        // 错误处理系统初始化代码
    });
    
    // 1秒后执行延迟初始化
    QTimer::singleShot(1000, this, &StartupOptimizer::executeDelayedInitializations);
    
    qDebug() << "StartupOptimizer: Non-critical initialization deferred";
}

void StartupOptimizer::preloadStyleSheets()
{
    startPhaseTimer("StyleSheetPreload");
    
    QStringList styleSheets = {
        ":/styles/default.qss",
        ":/styles/dark.qss"
    };
    
    for (const QString& path : styleSheets) {
        loadResourceInBackground(path);
    }
    
    endPhaseTimer("StyleSheetPreload");
}

void StartupOptimizer::preloadTranslations()
{
    startPhaseTimer("TranslationPreload");
    
    // 预加载当前语言的翻译文件
    QString currentLocale = QLocale::system().name();
    QString translationPath = QString(":/translations/jitsi_%1.qm").arg(currentLocale);
    
    loadResourceInBackground(translationPath);
    
    endPhaseTimer("TranslationPreload");
}

void StartupOptimizer::preloadIcons()
{
    startPhaseTimer("IconPreload");
    
    QStringList icons = {
        ":/icons/settings.svg",
        ":/icons/about.svg",
        ":/icons/back.svg",
        ":/icons/recent.svg",
        ":/icons/close.svg",
        ":/icons/warning.svg"
    };
    
    for (const QString& path : icons) {
        loadResourceInBackground(path);
    }
    
    endPhaseTimer("IconPreload");
}

void StartupOptimizer::optimizeResourceLoading()
{
    // 并行预加载不同类型的资源
    QtConcurrent::run([this]() { preloadStyleSheets(); });
    QtConcurrent::run([this]() { preloadTranslations(); });
    QtConcurrent::run([this]() { preloadIcons(); });
}

void StartupOptimizer::scheduleDelayedInitialization(const QString& component, std::function<void()> initFunc)
{
    m_delayedInitializations[component] = initFunc;
    qDebug() << "StartupOptimizer: Scheduled delayed initialization for" << component;
}

void StartupOptimizer::executeDelayedInitializations()
{
    startPhaseTimer("DelayedInitialization");
    
    for (auto it = m_delayedInitializations.begin(); it != m_delayedInitializations.end(); ++it) {
        const QString& component = it.key();
        const std::function<void()>& initFunc = it.value();
        
        qDebug() << "StartupOptimizer: Executing delayed initialization for" << component;
        initFunc();
    }
    
    m_delayedInitializations.clear();
    
    endPhaseTimer("DelayedInitialization");
    emit delayedInitializationCompleted();
    
    qDebug() << "StartupOptimizer: All delayed initializations completed";
}

void StartupOptimizer::startPhaseTimer(const QString& phase)
{
    m_phaseTimers[phase].start();
}

void StartupOptimizer::endPhaseTimer(const QString& phase)
{
    if (m_phaseTimers.contains(phase)) {
        qint64 elapsed = m_phaseTimers[phase].elapsed();
        m_phaseTimes[phase] = elapsed;
        emit startupPhaseCompleted(phase, elapsed);
        
        qDebug() << "StartupOptimizer: Phase" << phase << "completed in" << elapsed << "ms";
    }
}

qint64 StartupOptimizer::getPhaseTime(const QString& phase) const
{
    return m_phaseTimes.value(phase, 0);
}

void StartupOptimizer::logStartupMetrics()
{
    qDebug() << "=== Startup Optimization Metrics ===";
    
    qint64 totalTime = 0;
    for (auto it = m_phaseTimes.begin(); it != m_phaseTimes.end(); ++it) {
        qDebug() << "Phase" << it.key() << ":" << it.value() << "ms";
        totalTime += it.value();
    }
    
    qDebug() << "Total optimization time:" << totalTime << "ms";
    qDebug() << "Resources preloaded:" << m_preloadedResources.size();
    qDebug() << "Optimization level:" << m_optimizationLevel;
    qDebug() << "Fast startup enabled:" << m_fastStartupEnabled;
    qDebug() << "===================================";
}

void StartupOptimizer::setPreloadEnabled(bool enabled)
{
    m_preloadEnabled = enabled;
    qDebug() << "StartupOptimizer: Preload" << (enabled ? "enabled" : "disabled");
}

void StartupOptimizer::setDeferredInitEnabled(bool enabled)
{
    m_deferredInitEnabled = enabled;
    qDebug() << "StartupOptimizer: Deferred initialization" << (enabled ? "enabled" : "disabled");
}

void StartupOptimizer::setCriticalResourcePaths(const QStringList& paths)
{
    m_criticalResourcePaths = paths;
    qDebug() << "StartupOptimizer: Critical resource paths updated, count:" << paths.size();
}

void StartupOptimizer::initializeBasicOptimizations()
{
    // 基本优化：只启用必要的功能
    m_preloadEnabled = false;
    m_deferredInitEnabled = false;
    
    qDebug() << "StartupOptimizer: Basic optimizations initialized";
}

void StartupOptimizer::initializeModerateOptimizations()
{
    // 中等优化：平衡启动速度和功能
    m_preloadEnabled = true;
    m_deferredInitEnabled = true;
    
    // 减少预加载的资源数量
    m_criticalResourcePaths = m_criticalResourcePaths.mid(0, 3);
    
    qDebug() << "StartupOptimizer: Moderate optimizations initialized";
}

void StartupOptimizer::initializeAggressiveOptimizations()
{
    // 激进优化：最大化启动速度
    m_preloadEnabled = true;
    m_deferredInitEnabled = true;
    
    // 设置更多的应用程序属性
    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus, true);
    QCoreApplication::setAttribute(Qt::AA_DisableShaderDiskCache, true);
    
    qDebug() << "StartupOptimizer: Aggressive optimizations initialized";
}

void StartupOptimizer::loadResourceInBackground(const QString& path)
{
    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        m_preloadedResources[path] = data;
        
        qDebug() << "StartupOptimizer: Preloaded resource" << path << "(" << data.size() << "bytes)";
    } else {
        qWarning() << "StartupOptimizer: Failed to preload resource" << path;
    }
}
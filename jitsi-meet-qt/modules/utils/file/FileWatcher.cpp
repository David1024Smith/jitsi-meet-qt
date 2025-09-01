#include "FileWatcher.h"
#include <QDir>
#include <QDirIterator>
#include <QRegularExpression>
#include <QDebug>

FileWatcher::FileWatcher(QObject* parent)
    : QObject(parent)
    , m_fsWatcher(new QFileSystemWatcher(this))
    , m_pollTimer(new QTimer(this))
    , m_batchTimer(new QTimer(this))
    , m_enabled(true)
    , m_paused(false)
{
    // 设置默认配置
    m_globalConfig.recursive = false;
    m_globalConfig.mode = WatchBoth;
    m_globalConfig.pollInterval = 1000;
    m_globalConfig.enableBatching = false;
    m_globalConfig.batchInterval = 500;
    m_globalConfig.maxBatchSize = 100;
    m_globalConfig.followSymlinks = false;
    
    // 连接文件系统监控器信号
    connect(m_fsWatcher, &QFileSystemWatcher::fileChanged,
            this, &FileWatcher::onFileChanged);
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged,
            this, &FileWatcher::onDirectoryChanged);
    
    // 设置轮询定时器
    m_pollTimer->setSingleShot(false);
    connect(m_pollTimer, &QTimer::timeout, this, &FileWatcher::onPollTimer);
    
    // 设置批量处理定时器
    m_batchTimer->setSingleShot(false);
    connect(m_batchTimer, &QTimer::timeout, this, &FileWatcher::onBatchTimer);
}

FileWatcher::~FileWatcher()
{
    cleanup();
}

bool FileWatcher::initialize()
{
    QMutexLocker locker(&m_mutex);
    
    // 启动轮询定时器（如果需要）
    if (m_globalConfig.pollInterval > 0) {
        m_pollTimer->start(m_globalConfig.pollInterval);
    }
    
    // 启动批量处理定时器（如果需要）
    if (m_globalConfig.enableBatching && m_globalConfig.batchInterval > 0) {
        m_batchTimer->start(m_globalConfig.batchInterval);
    }
    
    return true;
}

void FileWatcher::cleanup()
{
    QMutexLocker locker(&m_mutex);
    
    // 停止定时器
    if (m_pollTimer) {
        m_pollTimer->stop();
    }
    
    if (m_batchTimer) {
        m_batchTimer->stop();
    }
    
    // 清理文件系统监控器
    if (m_fsWatcher) {
        m_fsWatcher->removePaths(m_fsWatcher->files());
        m_fsWatcher->removePaths(m_fsWatcher->directories());
    }
    
    // 处理剩余的批量事件
    if (!m_batchQueue.isEmpty()) {
        processBatchEvents();
    }
    
    // 清理缓存
    m_fileStatus.clear();
    m_lastModified.clear();
    m_pathConfigs.clear();
    m_eventQueue.clear();
    m_batchQueue.clear();
}

bool FileWatcher::addWatch(const QString& path, const WatchConfig& config)
{
    if (!QFile::exists(path)) {
        return false;
    }
    
    QMutexLocker locker(&m_mutex);
    
    QFileInfo fileInfo(path);
    
    // 存储路径特定配置
    m_pathConfigs[path] = config;
    
    // 更新文件状态缓存
    updateFileStatus(path);
    
    if (fileInfo.isFile()) {
        // 监控文件
        if (config.mode == WatchFiles || config.mode == WatchBoth) {
            if (matchesFilters(path, config)) {
                m_fsWatcher->addPath(path);
            }
        }
    } else if (fileInfo.isDir()) {
        // 监控目录
        if (config.mode == WatchDirectories || config.mode == WatchBoth) {
            m_fsWatcher->addPath(path);
            
            // 递归添加子目录（如果启用）
            if (config.recursive) {
                addDirectoryRecursive(path, config);
            }
        }
    }
    
    return true;
}

bool FileWatcher::removeWatch(const QString& path)
{
    QMutexLocker locker(&m_mutex);
    
    // 从文件系统监控器中移除
    m_fsWatcher->removePath(path);
    
    // 递归移除子目录
    QFileInfo fileInfo(path);
    if (fileInfo.isDir()) {
        removeDirectoryRecursive(path);
    }
    
    // 清理配置和缓存
    m_pathConfigs.remove(path);
    m_fileStatus.remove(path);
    m_lastModified.remove(path);
    
    return true;
}

bool FileWatcher::isWatched(const QString& path) const
{
    QMutexLocker locker(&m_mutex);
    
    return m_fsWatcher->files().contains(path) || 
           m_fsWatcher->directories().contains(path);
}

QStringList FileWatcher::watchedPaths() const
{
    QMutexLocker locker(&m_mutex);
    
    QStringList paths;
    paths << m_fsWatcher->files();
    paths << m_fsWatcher->directories();
    
    return paths;
}

void FileWatcher::setGlobalConfig(const WatchConfig& config)
{
    QMutexLocker locker(&m_mutex);
    
    m_globalConfig = config;
    
    // 更新定时器
    if (config.pollInterval > 0) {
        m_pollTimer->start(config.pollInterval);
    } else {
        m_pollTimer->stop();
    }
    
    if (config.enableBatching && config.batchInterval > 0) {
        m_batchTimer->start(config.batchInterval);
    } else {
        m_batchTimer->stop();
    }
}

FileWatcher::WatchConfig FileWatcher::globalConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalConfig;
}

void FileWatcher::setPathConfig(const QString& path, const WatchConfig& config)
{
    QMutexLocker locker(&m_mutex);
    m_pathConfigs[path] = config;
}

FileWatcher::WatchConfig FileWatcher::pathConfig(const QString& path) const
{
    QMutexLocker locker(&m_mutex);
    return m_pathConfigs.value(path, m_globalConfig);
}

void FileWatcher::setEnabled(bool enabled)
{
    QMutexLocker locker(&m_mutex);
    m_enabled = enabled;
}

bool FileWatcher::isEnabled() const
{
    QMutexLocker locker(&m_mutex);
    return m_enabled;
}

void FileWatcher::pause()
{
    QMutexLocker locker(&m_mutex);
    m_paused = true;
}

void FileWatcher::resume()
{
    QMutexLocker locker(&m_mutex);
    m_paused = false;
}

bool FileWatcher::isPaused() const
{
    QMutexLocker locker(&m_mutex);
    return m_paused;
}

void FileWatcher::forceCheck()
{
    QMutexLocker locker(&m_mutex);
    
    // 强制检查所有监控的路径
    QStringList paths = watchedPaths();
    for (const QString& path : paths) {
        updateFileStatus(path);
    }
}

void FileWatcher::clearEventQueue()
{
    QMutexLocker locker(&m_mutex);
    
    m_eventQueue.clear();
    m_batchQueue.clear();
}

int FileWatcher::eventQueueSize() const
{
    QMutexLocker locker(&m_mutex);
    return m_eventQueue.size() + m_batchQueue.size();
}

QVariantMap FileWatcher::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    
    QVariantMap stats;
    stats["totalEvents"] = static_cast<qint64>(m_statistics.totalEvents);
    stats["filteredEvents"] = static_cast<qint64>(m_statistics.filteredEvents);
    stats["batchedEvents"] = static_cast<qint64>(m_statistics.batchedEvents);
    stats["startTime"] = m_statistics.startTime.toString(Qt::ISODate);
    stats["watchedFiles"] = m_fsWatcher->files().size();
    stats["watchedDirectories"] = m_fsWatcher->directories().size();
    stats["queuedEvents"] = eventQueueSize();
    stats["enabled"] = m_enabled;
    stats["paused"] = m_paused;
    
    return stats;
}

void FileWatcher::setEventFilter(std::function<bool(const FileEvent&)> filter)
{
    QMutexLocker locker(&m_mutex);
    m_eventFilter = filter;
}

void FileWatcher::addNameFilter(const QString& pattern)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_globalConfig.nameFilters.contains(pattern)) {
        m_globalConfig.nameFilters.append(pattern);
    }
}

void FileWatcher::removeNameFilter(const QString& pattern)
{
    QMutexLocker locker(&m_mutex);
    m_globalConfig.nameFilters.removeAll(pattern);
}

void FileWatcher::clearNameFilters()
{
    QMutexLocker locker(&m_mutex);
    m_globalConfig.nameFilters.clear();
}

QStringList FileWatcher::nameFilters() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalConfig.nameFilters;
}

void FileWatcher::addExcludeFilter(const QString& pattern)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_globalConfig.excludeFilters.contains(pattern)) {
        m_globalConfig.excludeFilters.append(pattern);
    }
}

void FileWatcher::removeExcludeFilter(const QString& pattern)
{
    QMutexLocker locker(&m_mutex);
    m_globalConfig.excludeFilters.removeAll(pattern);
}

void FileWatcher::clearExcludeFilters()
{
    QMutexLocker locker(&m_mutex);
    m_globalConfig.excludeFilters.clear();
}

QStringList FileWatcher::excludeFilters() const
{
    QMutexLocker locker(&m_mutex);
    return m_globalConfig.excludeFilters;
}

void FileWatcher::onFileChanged(const QString& path)
{
    if (!m_enabled || m_paused) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    EventType eventType = detectChangeType(path);
    FileEvent event(eventType, path);
    
    processFileEvent(event);
}

void FileWatcher::onDirectoryChanged(const QString& path)
{
    if (!m_enabled || m_paused) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    FileEvent event(DirectoryModified, path);
    processFileEvent(event);
}

void FileWatcher::onPollTimer()
{
    if (!m_enabled || m_paused) {
        return;
    }
    
    QMutexLocker locker(&m_mutex);
    
    // 轮询检查所有监控的文件
    QStringList files = m_fsWatcher->files();
    for (const QString& file : files) {
        QFileInfo currentInfo(file);
        QFileInfo cachedInfo = m_fileStatus.value(file);
        
        if (currentInfo.exists()) {
            if (!cachedInfo.exists() || 
                currentInfo.lastModified() != cachedInfo.lastModified() ||
                currentInfo.size() != cachedInfo.size()) {
                
                EventType eventType = cachedInfo.exists() ? FileModified : FileCreated;
                FileEvent event(eventType, file);
                processFileEvent(event);
                
                updateFileStatus(file);
            }
        } else if (cachedInfo.exists()) {
            // 文件被删除
            FileEvent event(FileDeleted, file);
            processFileEvent(event);
            
            m_fileStatus.remove(file);
            m_lastModified.remove(file);
        }
    }
}

void FileWatcher::onBatchTimer()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_batchQueue.isEmpty()) {
        processBatchEvents();
    }
}

void FileWatcher::processFileEvent(const FileEvent& event)
{
    // 应用事件过滤器
    if (m_eventFilter && !m_eventFilter(event)) {
        m_statistics.filteredEvents++;
        return;
    }
    
    // 检查路径过滤器
    WatchConfig config = pathConfig(event.path);
    if (!matchesFilters(event.path, config)) {
        m_statistics.filteredEvents++;
        return;
    }
    
    m_statistics.totalEvents++;
    
    // 添加到事件队列
    m_eventQueue.append(event);
    
    // 批量处理或立即处理
    if (m_globalConfig.enableBatching) {
        m_batchQueue.append(event);
        
        if (m_batchQueue.size() >= m_globalConfig.maxBatchSize) {
            processBatchEvents();
        }
    } else {
        // 立即发出信号
        emitEventSignals(event);
    }
}

bool FileWatcher::matchesFilters(const QString& path, const WatchConfig& config) const
{
    QFileInfo fileInfo(path);
    QString fileName = fileInfo.fileName();
    
    // 检查名称过滤器
    if (!config.nameFilters.isEmpty()) {
        bool matches = false;
        for (const QString& filter : config.nameFilters) {
            QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(filter), QRegularExpression::CaseInsensitiveOption);
            if (regex.match(fileName).hasMatch()) {
                matches = true;
                break;
            }
        }
        if (!matches) {
            return false;
        }
    }
    
    // 检查排除过滤器
    for (const QString& filter : config.excludeFilters) {
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(filter), QRegularExpression::CaseInsensitiveOption);
        if (regex.match(fileName).hasMatch()) {
            return false;
        }
    }
    
    return true;
}

void FileWatcher::addDirectoryRecursive(const QString& dirPath, const WatchConfig& config)
{
    QDirIterator it(dirPath, QDir::Dirs | QDir::NoDotAndDotDot, 
                   config.recursive ? QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags);
    
    while (it.hasNext()) {
        QString subDir = it.next();
        
        if (config.followSymlinks || !QFileInfo(subDir).isSymLink()) {
            m_fsWatcher->addPath(subDir);
            updateFileStatus(subDir);
        }
    }
}

void FileWatcher::removeDirectoryRecursive(const QString& dirPath)
{
    QStringList directories = m_fsWatcher->directories();
    
    for (const QString& dir : directories) {
        if (dir.startsWith(dirPath + "/")) {
            m_fsWatcher->removePath(dir);
            m_fileStatus.remove(dir);
            m_lastModified.remove(dir);
        }
    }
}

FileWatcher::EventType FileWatcher::detectChangeType(const QString& path)
{
    QFileInfo currentInfo(path);
    QFileInfo cachedInfo = m_fileStatus.value(path);
    
    if (!currentInfo.exists()) {
        return FileDeleted;
    } else if (!cachedInfo.exists()) {
        return FileCreated;
    } else if (currentInfo.lastModified() != cachedInfo.lastModified() ||
               currentInfo.size() != cachedInfo.size()) {
        return FileModified;
    }
    
    return FileModified; // 默认为修改
}

void FileWatcher::updateFileStatus(const QString& path)
{
    QFileInfo fileInfo(path);
    if (fileInfo.exists()) {
        m_fileStatus[path] = fileInfo;
        m_lastModified[path] = fileInfo.lastModified();
    }
}

void FileWatcher::processBatchEvents()
{
    if (m_batchQueue.isEmpty()) {
        return;
    }
    
    QList<FileEvent> events = m_batchQueue;
    m_batchQueue.clear();
    
    m_statistics.batchedEvents += events.size();
    
    // 发出批量事件信号
    emit batchFileEvents(events);
    
    // 发出单个事件信号
    for (const FileEvent& event : events) {
        emitEventSignals(event);
    }
}

void FileWatcher::emitEventSignals(const FileEvent& event)
{
    // 发出通用事件信号
    emit fileEvent(event);
    
    // 发出特定类型的信号
    switch (event.type) {
        case FileCreated:
            emit fileCreated(event.path);
            break;
        case FileModified:
            emit fileModified(event.path);
            break;
        case FileDeleted:
            emit fileDeleted(event.path);
            break;
        case FileRenamed:
            emit fileRenamed(event.oldPath, event.path);
            break;
        case DirectoryCreated:
            emit directoryCreated(event.path);
            break;
        case DirectoryModified:
            emit directoryModified(event.path);
            break;
        case DirectoryDeleted:
            emit directoryDeleted(event.path);
            break;
        default:
            break;
    }
}
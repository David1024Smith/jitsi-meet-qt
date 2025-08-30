#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QMutex>
#include <QHash>
#include <QDateTime>
#include <QFileInfo>

/**
 * @brief 文件监控器
 * 
 * FileWatcher提供高级文件和目录监控功能，支持递归监控、
 * 过滤器、批量事件处理和自定义监控策略。
 */
class FileWatcher : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 监控事件类型枚举
     */
    enum EventType {
        FileCreated,        ///< 文件创建
        FileModified,       ///< 文件修改
        FileDeleted,        ///< 文件删除
        FileRenamed,        ///< 文件重命名
        DirectoryCreated,   ///< 目录创建
        DirectoryModified,  ///< 目录修改
        DirectoryDeleted,   ///< 目录删除
        AttributeChanged    ///< 属性改变
    };
    Q_ENUM(EventType)

    /**
     * @brief 监控模式枚举
     */
    enum WatchMode {
        WatchFiles,         ///< 只监控文件
        WatchDirectories,   ///< 只监控目录
        WatchBoth          ///< 监控文件和目录
    };
    Q_ENUM(WatchMode)

    /**
     * @brief 文件事件结构
     */
    struct FileEvent {
        EventType type;         ///< 事件类型
        QString path;           ///< 文件路径
        QString oldPath;        ///< 旧路径（重命名事件）
        QDateTime timestamp;    ///< 事件时间戳
        qint64 size;            ///< 文件大小
        QFileInfo fileInfo;     ///< 文件信息
        
        FileEvent() : type(FileModified), size(0) {}
        
        FileEvent(EventType t, const QString& p) 
            : type(t), path(p), size(0), timestamp(QDateTime::currentDateTime()) {
            if (QFile::exists(p)) {
                fileInfo = QFileInfo(p);
                size = fileInfo.size();
            }
        }
    };

    /**
     * @brief 监控配置结构
     */
    struct WatchConfig {
        bool recursive;             ///< 是否递归监控
        WatchMode mode;             ///< 监控模式
        QStringList nameFilters;   ///< 文件名过滤器
        QStringList excludeFilters; ///< 排除过滤器
        int pollInterval;           ///< 轮询间隔（毫秒）
        bool enableBatching;        ///< 是否启用批量处理
        int batchInterval;          ///< 批量处理间隔（毫秒）
        int maxBatchSize;           ///< 最大批量大小
        bool followSymlinks;        ///< 是否跟随符号链接
        
        WatchConfig() 
            : recursive(false), mode(WatchBoth), pollInterval(1000)
            , enableBatching(false), batchInterval(500), maxBatchSize(100)
            , followSymlinks(false) {}
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit FileWatcher(QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~FileWatcher();

    /**
     * @brief 初始化文件监控器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理文件监控器
     */
    void cleanup();

    /**
     * @brief 添加文件监控
     * @param path 文件路径
     * @param config 监控配置
     * @return 添加是否成功
     */
    bool addWatch(const QString& path, const WatchConfig& config = WatchConfig());

    /**
     * @brief 移除文件监控
     * @param path 文件路径
     * @return 移除是否成功
     */
    bool removeWatch(const QString& path);

    /**
     * @brief 检查路径是否被监控
     * @param path 文件路径
     * @return 是否被监控
     */
    bool isWatched(const QString& path) const;

    /**
     * @brief 获取所有监控路径
     * @return 监控路径列表
     */
    QStringList watchedPaths() const;

    /**
     * @brief 设置全局监控配置
     * @param config 监控配置
     */
    void setGlobalConfig(const WatchConfig& config);

    /**
     * @brief 获取全局监控配置
     * @return 监控配置
     */
    WatchConfig globalConfig() const;

    /**
     * @brief 设置路径特定配置
     * @param path 文件路径
     * @param config 监控配置
     */
    void setPathConfig(const QString& path, const WatchConfig& config);

    /**
     * @brief 获取路径特定配置
     * @param path 文件路径
     * @return 监控配置
     */
    WatchConfig pathConfig(const QString& path) const;

    /**
     * @brief 启用/禁用监控
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

    /**
     * @brief 检查监控是否启用
     * @return 是否启用
     */
    bool isEnabled() const;

    /**
     * @brief 暂停监控
     */
    void pause();

    /**
     * @brief 恢复监控
     */
    void resume();

    /**
     * @brief 检查监控是否暂停
     * @return 是否暂停
     */
    bool isPaused() const;

    /**
     * @brief 强制检查所有监控路径
     */
    void forceCheck();

    /**
     * @brief 清空事件队列
     */
    void clearEventQueue();

    /**
     * @brief 获取事件队列大小
     * @return 队列大小
     */
    int eventQueueSize() const;

    /**
     * @brief 获取监控统计信息
     * @return 统计信息
     */
    QVariantMap getStatistics() const;

    /**
     * @brief 设置事件过滤器
     * @param filter 过滤器函数
     */
    void setEventFilter(std::function<bool(const FileEvent&)> filter);

    /**
     * @brief 添加文件名过滤器
     * @param pattern 过滤模式
     */
    void addNameFilter(const QString& pattern);

    /**
     * @brief 移除文件名过滤器
     * @param pattern 过滤模式
     */
    void removeNameFilter(const QString& pattern);

    /**
     * @brief 清空文件名过滤器
     */
    void clearNameFilters();

    /**
     * @brief 获取文件名过滤器
     * @return 过滤器列表
     */
    QStringList nameFilters() const;

    /**
     * @brief 添加排除过滤器
     * @param pattern 排除模式
     */
    void addExcludeFilter(const QString& pattern);

    /**
     * @brief 移除排除过滤器
     * @param pattern 排除模式
     */
    void removeExcludeFilter(const QString& pattern);

    /**
     * @brief 清空排除过滤器
     */
    void clearExcludeFilters();

    /**
     * @brief 获取排除过滤器
     * @return 排除过滤器列表
     */
    QStringList excludeFilters() const;

signals:
    /**
     * @brief 文件事件信号
     * @param event 文件事件
     */
    void fileEvent(const FileEvent& event);

    /**
     * @brief 批量文件事件信号
     * @param events 文件事件列表
     */
    void batchFileEvents(const QList<FileEvent>& events);

    /**
     * @brief 文件创建信号
     * @param path 文件路径
     */
    void fileCreated(const QString& path);

    /**
     * @brief 文件修改信号
     * @param path 文件路径
     */
    void fileModified(const QString& path);

    /**
     * @brief 文件删除信号
     * @param path 文件路径
     */
    void fileDeleted(const QString& path);

    /**
     * @brief 文件重命名信号
     * @param oldPath 旧路径
     * @param newPath 新路径
     */
    void fileRenamed(const QString& oldPath, const QString& newPath);

    /**
     * @brief 目录创建信号
     * @param path 目录路径
     */
    void directoryCreated(const QString& path);

    /**
     * @brief 目录修改信号
     * @param path 目录路径
     */
    void directoryModified(const QString& path);

    /**
     * @brief 目录删除信号
     * @param path 目录路径
     */
    void directoryDeleted(const QString& path);

    /**
     * @brief 监控错误信号
     * @param error 错误信息
     */
    void watchError(const QString& error);

private slots:
    /**
     * @brief 文件系统监控器文件改变槽函数
     * @param path 文件路径
     */
    void onFileChanged(const QString& path);

    /**
     * @brief 文件系统监控器目录改变槽函数
     * @param path 目录路径
     */
    void onDirectoryChanged(const QString& path);

    /**
     * @brief 轮询定时器槽函数
     */
    void onPollTimer();

    /**
     * @brief 批量处理定时器槽函数
     */
    void onBatchTimer();

private:
    /**
     * @brief 处理文件事件
     * @param event 文件事件
     */
    void processFileEvent(const FileEvent& event);

    /**
     * @brief 检查文件是否匹配过滤器
     * @param path 文件路径
     * @param config 监控配置
     * @return 是否匹配
     */
    bool matchesFilters(const QString& path, const WatchConfig& config) const;

    /**
     * @brief 递归添加目录监控
     * @param dirPath 目录路径
     * @param config 监控配置
     */
    void addDirectoryRecursive(const QString& dirPath, const WatchConfig& config);

    /**
     * @brief 递归移除目录监控
     * @param dirPath 目录路径
     */
    void removeDirectoryRecursive(const QString& dirPath);

    /**
     * @brief 检测文件变化类型
     * @param path 文件路径
     * @return 事件类型
     */
    EventType detectChangeType(const QString& path);

    /**
     * @brief 更新文件状态缓存
     * @param path 文件路径
     */
    void updateFileStatus(const QString& path);

    /**
     * @brief 处理批量事件
     */
    void processBatchEvents();

    /**
     * @brief 发出事件信号
     * @param event 文件事件
     */
    void emitEventSignals(const FileEvent& event);

private:
    QFileSystemWatcher* m_fsWatcher;                    ///< Qt文件系统监控器
    
    WatchConfig m_globalConfig;                         ///< 全局监控配置
    QHash<QString, WatchConfig> m_pathConfigs;          ///< 路径特定配置
    
    QHash<QString, QFileInfo> m_fileStatus;             ///< 文件状态缓存
    QHash<QString, QDateTime> m_lastModified;           ///< 最后修改时间缓存
    
    QTimer* m_pollTimer;                                ///< 轮询定时器
    QTimer* m_batchTimer;                               ///< 批量处理定时器
    
    QList<FileEvent> m_eventQueue;                      ///< 事件队列
    QList<FileEvent> m_batchQueue;                      ///< 批量事件队列
    
    bool m_enabled;                                     ///< 是否启用
    bool m_paused;                                      ///< 是否暂停
    
    std::function<bool(const FileEvent&)> m_eventFilter; ///< 事件过滤器
    
    // 统计信息
    struct Statistics {
        qint64 totalEvents;         ///< 总事件数
        qint64 filteredEvents;      ///< 过滤的事件数
        qint64 batchedEvents;       ///< 批量处理的事件数
        QDateTime startTime;        ///< 开始时间
        
        Statistics() : totalEvents(0), filteredEvents(0), batchedEvents(0) {
            startTime = QDateTime::currentDateTime();
        }
    } m_statistics;
    
    mutable QMutex m_mutex;                             ///< 线程安全互斥锁
};

// 声明元类型以支持信号槽
Q_DECLARE_METATYPE(FileWatcher::EventType)
Q_DECLARE_METATYPE(FileWatcher::WatchMode)
Q_DECLARE_METATYPE(FileWatcher::FileEvent)
Q_DECLARE_METATYPE(FileWatcher::WatchConfig)

#endif // FILEWATCHER_H
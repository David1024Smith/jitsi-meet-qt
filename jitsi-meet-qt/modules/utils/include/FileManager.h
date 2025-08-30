#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QFileInfo>
#include <QDir>
#include <QMutex>
#include <QHash>
#include <memory>

// 前向声明
class IFileHandler;
class FileWatcher;

/**
 * @brief 文件管理器类
 * 
 * FileManager提供统一的文件操作接口，包括文件读写、目录管理、
 * 文件监控等功能。支持多种文件处理器和缓存机制。
 */
class FileManager : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 文件操作结果枚举
     */
    enum OperationResult {
        Success,            ///< 操作成功
        FileNotFound,       ///< 文件未找到
        PermissionDenied,   ///< 权限被拒绝
        DiskFull,           ///< 磁盘空间不足
        InvalidPath,        ///< 无效路径
        UnknownError        ///< 未知错误
    };
    Q_ENUM(OperationResult)

    /**
     * @brief 文件类型枚举
     */
    enum FileType {
        RegularFile,        ///< 普通文件
        Directory,          ///< 目录
        SymbolicLink,       ///< 符号链接
        Unknown             ///< 未知类型
    };
    Q_ENUM(FileType)

    /**
     * @brief 文件信息结构
     */
    struct FileInfo {
        QString path;           ///< 文件路径
        QString name;           ///< 文件名
        FileType type;          ///< 文件类型
        qint64 size;            ///< 文件大小
        QDateTime created;      ///< 创建时间
        QDateTime modified;     ///< 修改时间
        QDateTime accessed;     ///< 访问时间
        bool readable;          ///< 是否可读
        bool writable;          ///< 是否可写
        bool executable;        ///< 是否可执行
    };

    /**
     * @brief 获取FileManager单例实例
     * @return FileManager实例指针
     */
    static FileManager* instance();

    /**
     * @brief 析构函数
     */
    ~FileManager();

    /**
     * @brief 初始化文件管理器
     * @return 初始化是否成功
     */
    bool initialize();

    /**
     * @brief 清理文件管理器
     */
    void cleanup();

    /**
     * @brief 注册文件处理器
     * @param extension 文件扩展名
     * @param handler 文件处理器
     */
    void registerFileHandler(const QString& extension, std::shared_ptr<IFileHandler> handler);

    /**
     * @brief 注销文件处理器
     * @param extension 文件扩展名
     */
    void unregisterFileHandler(const QString& extension);

    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return 文件是否存在
     */
    bool exists(const QString& path) const;

    /**
     * @brief 获取文件信息
     * @param path 文件路径
     * @return 文件信息
     */
    FileInfo getFileInfo(const QString& path) const;

    /**
     * @brief 读取文件内容
     * @param path 文件路径
     * @param data 输出数据
     * @return 操作结果
     */
    OperationResult readFile(const QString& path, QByteArray& data);

    /**
     * @brief 写入文件内容
     * @param path 文件路径
     * @param data 输入数据
     * @param append 是否追加模式
     * @return 操作结果
     */
    OperationResult writeFile(const QString& path, const QByteArray& data, bool append = false);

    /**
     * @brief 复制文件
     * @param sourcePath 源文件路径
     * @param destPath 目标文件路径
     * @param overwrite 是否覆盖已存在文件
     * @return 操作结果
     */
    OperationResult copyFile(const QString& sourcePath, const QString& destPath, bool overwrite = false);

    /**
     * @brief 移动文件
     * @param sourcePath 源文件路径
     * @param destPath 目标文件路径
     * @param overwrite 是否覆盖已存在文件
     * @return 操作结果
     */
    OperationResult moveFile(const QString& sourcePath, const QString& destPath, bool overwrite = false);

    /**
     * @brief 删除文件
     * @param path 文件路径
     * @return 操作结果
     */
    OperationResult deleteFile(const QString& path);

    /**
     * @brief 创建目录
     * @param path 目录路径
     * @param recursive 是否递归创建
     * @return 操作结果
     */
    OperationResult createDirectory(const QString& path, bool recursive = true);

    /**
     * @brief 删除目录
     * @param path 目录路径
     * @param recursive 是否递归删除
     * @return 操作结果
     */
    OperationResult removeDirectory(const QString& path, bool recursive = false);

    /**
     * @brief 列出目录内容
     * @param path 目录路径
     * @param nameFilters 名称过滤器
     * @param recursive 是否递归列出
     * @return 文件列表
     */
    QStringList listDirectory(const QString& path, const QStringList& nameFilters = QStringList(), 
                             bool recursive = false) const;

    /**
     * @brief 获取临时目录路径
     * @return 临时目录路径
     */
    QString tempPath() const;

    /**
     * @brief 获取应用数据目录路径
     * @return 应用数据目录路径
     */
    QString appDataPath() const;

    /**
     * @brief 获取用户文档目录路径
     * @return 用户文档目录路径
     */
    QString documentsPath() const;

    /**
     * @brief 启用文件缓存
     * @param enabled 是否启用
     */
    void setCacheEnabled(bool enabled);

    /**
     * @brief 检查文件缓存是否启用
     * @return 是否启用缓存
     */
    bool isCacheEnabled() const;

    /**
     * @brief 清理文件缓存
     */
    void clearCache();

    /**
     * @brief 添加文件监控
     * @param path 监控路径
     * @return 是否添加成功
     */
    bool addFileWatch(const QString& path);

    /**
     * @brief 移除文件监控
     * @param path 监控路径
     * @return 是否移除成功
     */
    bool removeFileWatch(const QString& path);

    /**
     * @brief 操作结果转字符串
     * @param result 操作结果
     * @return 结果字符串
     */
    static QString resultToString(OperationResult result);

signals:
    /**
     * @brief 文件改变信号
     * @param path 文件路径
     */
    void fileChanged(const QString& path);

    /**
     * @brief 目录改变信号
     * @param path 目录路径
     */
    void directoryChanged(const QString& path);

    /**
     * @brief 文件操作完成信号
     * @param operation 操作类型
     * @param path 文件路径
     * @param result 操作结果
     */
    void operationCompleted(const QString& operation, const QString& path, OperationResult result);

private:
    /**
     * @brief 私有构造函数（单例模式）
     * @param parent 父对象
     */
    explicit FileManager(QObject* parent = nullptr);

    /**
     * @brief 获取文件处理器
     * @param path 文件路径
     * @return 文件处理器
     */
    std::shared_ptr<IFileHandler> getFileHandler(const QString& path) const;

    /**
     * @brief 验证文件路径
     * @param path 文件路径
     * @return 路径是否有效
     */
    bool validatePath(const QString& path) const;

    /**
     * @brief 从缓存获取文件数据
     * @param path 文件路径
     * @param data 输出数据
     * @return 是否从缓存获取成功
     */
    bool getFromCache(const QString& path, QByteArray& data) const;

    /**
     * @brief 将文件数据存入缓存
     * @param path 文件路径
     * @param data 文件数据
     */
    void putToCache(const QString& path, const QByteArray& data);

private:
    static FileManager* s_instance;                                     ///< 单例实例
    static QMutex s_mutex;                                              ///< 线程安全互斥锁

    QHash<QString, std::shared_ptr<IFileHandler>> m_fileHandlers;       ///< 文件处理器映射
    QHash<QString, QByteArray> m_fileCache;                             ///< 文件缓存
    std::unique_ptr<FileWatcher> m_fileWatcher;                         ///< 文件监控器
    
    bool m_cacheEnabled;                                                ///< 缓存是否启用
    mutable QMutex m_cacheMutex;                                        ///< 缓存互斥锁
    mutable QMutex m_operationMutex;                                    ///< 操作互斥锁

    // 禁用拷贝构造和赋值操作
    Q_DISABLE_COPY(FileManager)
};

#endif // FILEMANAGER_H
#ifndef TEMPFILE_H
#define TEMPFILE_H

#include "../interfaces/IFileHandler.h"
#include <QTemporaryFile>
#include <QMutex>
#include <QTimer>

/**
 * @brief 临时文件处理器
 * 
 * TempFile提供临时文件的创建、管理和自动清理功能，
 * 支持自定义生命周期、安全删除和批量管理。
 */
class TempFile : public IFileHandler
{
    Q_OBJECT

public:
    /**
     * @brief 临时文件类型枚举
     */
    enum TempFileType {
        AutoDelete,     ///< 自动删除
        ManualDelete,   ///< 手动删除
        SessionDelete   ///< 会话结束时删除
    };
    Q_ENUM(TempFileType)

    /**
     * @brief 清理策略枚举
     */
    enum CleanupPolicy {
        Immediate,      ///< 立即清理
        Delayed,        ///< 延迟清理
        OnExit,         ///< 程序退出时清理
        Never           ///< 永不清理
    };
    Q_ENUM(CleanupPolicy)

    /**
     * @brief 构造函数
     * @param nameTemplate 文件名模板
     * @param type 临时文件类型
     * @param parent 父对象
     */
    explicit TempFile(const QString& nameTemplate = QString(), 
                     TempFileType type = AutoDelete, 
                     QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~TempFile() override;

    // IFileHandler接口实现
    bool initialize() override;
    void cleanup() override;
    bool exists(const QString& path) const override;
    OperationResult read(const QString& path, QByteArray& data) override;
    OperationResult write(const QString& path, const QByteArray& data, bool append = false) override;
    OperationResult remove(const QString& path) override;
    OperationResult copy(const QString& sourcePath, const QString& destPath, bool overwrite = false) override;
    OperationResult move(const QString& sourcePath, const QString& destPath, bool overwrite = false) override;
    OperationResult getAttributes(const QString& path, FileAttributes& attributes) override;
    OperationResult setAttributes(const QString& path, const FileAttributes& attributes) override;
    qint64 size(const QString& path) const override;
    bool isReadable(const QString& path) const override;
    bool isWritable(const QString& path) const override;
    bool isExecutable(const QString& path) const override;
    QStringList supportedExtensions() const override;
    bool supports(const QString& path) const override;
    QString name() const override;
    QString version() const override;

    /**
     * @brief 创建临时文件
     * @return 创建是否成功
     */
    bool create();

    /**
     * @brief 打开临时文件
     * @param mode 打开模式
     * @return 打开是否成功
     */
    bool open(QIODevice::OpenMode mode = QIODevice::ReadWrite);

    /**
     * @brief 关闭临时文件
     */
    void close();

    /**
     * @brief 检查文件是否已打开
     * @return 是否已打开
     */
    bool isOpen() const;

    /**
     * @brief 写入数据
     * @param data 数据
     * @return 写入的字节数
     */
    qint64 write(const QByteArray& data);

    /**
     * @brief 写入字符串
     * @param text 字符串
     * @return 写入的字节数
     */
    qint64 write(const QString& text);

    /**
     * @brief 读取所有数据
     * @return 文件内容
     */
    QByteArray readAll();

    /**
     * @brief 读取指定字节数
     * @param maxSize 最大字节数
     * @return 读取的数据
     */
    QByteArray read(qint64 maxSize);

    /**
     * @brief 读取一行
     * @param maxSize 最大字节数
     * @return 读取的行
     */
    QByteArray readLine(qint64 maxSize = 0);

    /**
     * @brief 刷新缓冲区
     * @return 刷新是否成功
     */
    bool flush();

    /**
     * @brief 获取文件路径
     * @return 文件路径
     */
    QString fileName() const;

    /**
     * @brief 获取文件大小
     * @return 文件大小
     */
    qint64 size() const;

    /**
     * @brief 设置文件权限
     * @param permissions 权限
     * @return 设置是否成功
     */
    bool setPermissions(QFile::Permissions permissions);

    /**
     * @brief 获取文件权限
     * @return 文件权限
     */
    QFile::Permissions permissions() const;

    /**
     * @brief 设置临时文件类型
     * @param type 文件类型
     */
    void setTempFileType(TempFileType type);

    /**
     * @brief 获取临时文件类型
     * @return 文件类型
     */
    TempFileType tempFileType() const;

    /**
     * @brief 设置清理策略
     * @param policy 清理策略
     */
    void setCleanupPolicy(CleanupPolicy policy);

    /**
     * @brief 获取清理策略
     * @return 清理策略
     */
    CleanupPolicy cleanupPolicy() const;

    /**
     * @brief 设置生存时间（毫秒）
     * @param ttl 生存时间
     */
    void setTimeToLive(int ttl);

    /**
     * @brief 获取生存时间
     * @return 生存时间
     */
    int timeToLive() const;

    /**
     * @brief 设置最大文件大小
     * @param maxSize 最大大小
     */
    void setMaxSize(qint64 maxSize);

    /**
     * @brief 获取最大文件大小
     * @return 最大大小
     */
    qint64 maxSize() const;

    /**
     * @brief 重命名临时文件
     * @param newName 新文件名
     * @return 重命名是否成功
     */
    bool rename(const QString& newName);

    /**
     * @brief 复制到指定路径
     * @param destPath 目标路径
     * @param keepOriginal 是否保留原文件
     * @return 复制是否成功
     */
    bool copyTo(const QString& destPath, bool keepOriginal = true);

    /**
     * @brief 移动到指定路径
     * @param destPath 目标路径
     * @return 移动是否成功
     */
    bool moveTo(const QString& destPath);

    /**
     * @brief 设置自动删除
     * @param autoDelete 是否自动删除
     */
    void setAutoRemove(bool autoDelete);

    /**
     * @brief 检查是否自动删除
     * @return 是否自动删除
     */
    bool autoRemove() const;

    /**
     * @brief 手动删除文件
     * @return 删除是否成功
     */
    bool remove();

    /**
     * @brief 获取创建时间
     * @return 创建时间
     */
    QDateTime creationTime() const;

    /**
     * @brief 获取最后访问时间
     * @return 最后访问时间
     */
    QDateTime lastAccessTime() const;

    /**
     * @brief 获取最后修改时间
     * @return 最后修改时间
     */
    QDateTime lastModifiedTime() const;

    /**
     * @brief 检查文件是否过期
     * @return 是否过期
     */
    bool isExpired() const;

    // 静态方法
    /**
     * @brief 创建临时文件
     * @param nameTemplate 文件名模板
     * @param data 初始数据
     * @return 临时文件路径
     */
    static QString createTempFile(const QString& nameTemplate = QString(), 
                                 const QByteArray& data = QByteArray());

    /**
     * @brief 创建临时目录
     * @param nameTemplate 目录名模板
     * @return 临时目录路径
     */
    static QString createTempDir(const QString& nameTemplate = QString());

    /**
     * @brief 获取系统临时目录
     * @return 临时目录路径
     */
    static QString tempPath();

    /**
     * @brief 清理过期的临时文件
     * @param directory 目录路径
     * @param maxAge 最大年龄（秒）
     * @return 清理的文件数量
     */
    static int cleanupExpiredFiles(const QString& directory, int maxAge = 3600);

    /**
     * @brief 获取临时文件统计信息
     * @return 统计信息
     */
    static QVariantMap getStatistics();

signals:
    /**
     * @brief 文件创建信号
     * @param filePath 文件路径
     */
    void fileCreated(const QString& filePath);

    /**
     * @brief 文件删除信号
     * @param filePath 文件路径
     */
    void fileRemoved(const QString& filePath);

    /**
     * @brief 文件过期信号
     * @param filePath 文件路径
     */
    void fileExpired(const QString& filePath);

    /**
     * @brief 大小限制超出信号
     * @param filePath 文件路径
     * @param currentSize 当前大小
     * @param maxSize 最大大小
     */
    void sizeLimitExceeded(const QString& filePath, qint64 currentSize, qint64 maxSize);

private slots:
    /**
     * @brief TTL定时器槽函数
     */
    void onTTLTimer();

    /**
     * @brief 清理定时器槽函数
     */
    void onCleanupTimer();

private:
    /**
     * @brief 初始化TTL定时器
     */
    void initializeTTLTimer();

    /**
     * @brief 检查文件大小限制
     * @return 是否超出限制
     */
    bool checkSizeLimit();

    /**
     * @brief 执行清理操作
     */
    void performCleanup();

    /**
     * @brief 注册临时文件（用于全局管理）
     */
    void registerTempFile();

    /**
     * @brief 注销临时文件
     */
    void unregisterTempFile();

private:
    QTemporaryFile* m_tempFile;         ///< Qt临时文件对象
    QString m_nameTemplate;             ///< 文件名模板
    TempFileType m_type;                ///< 临时文件类型
    CleanupPolicy m_cleanupPolicy;      ///< 清理策略
    
    int m_timeToLive;                   ///< 生存时间（毫秒）
    qint64 m_maxSize;                   ///< 最大文件大小
    QDateTime m_creationTime;           ///< 创建时间
    
    QTimer* m_ttlTimer;                 ///< TTL定时器
    
    mutable QMutex m_mutex;             ///< 线程安全互斥锁
    
    // 全局临时文件管理
    static QList<TempFile*> s_tempFiles;    ///< 全局临时文件列表
    static QMutex s_globalMutex;            ///< 全局互斥锁
};

#endif // TEMPFILE_H
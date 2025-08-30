#ifndef IFILEHANDLER_H
#define IFILEHANDLER_H

#include <QObject>
#include <QString>
#include <QByteArray>
#include <QStringList>
#include <QDateTime>

/**
 * @brief 文件处理器接口
 * 
 * IFileHandler定义了文件处理器的标准接口，用于处理不同类型的文件操作。
 * 支持基本的文件读写、属性查询和批量操作。
 */
class IFileHandler : public QObject
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
     * @brief 文件属性结构
     */
    struct FileAttributes {
        qint64 size;            ///< 文件大小
        QDateTime created;      ///< 创建时间
        QDateTime modified;     ///< 修改时间
        QDateTime accessed;     ///< 访问时间
        bool readable;          ///< 是否可读
        bool writable;          ///< 是否可写
        bool executable;        ///< 是否可执行
        bool hidden;            ///< 是否隐藏
        
        FileAttributes() 
            : size(0), readable(false), writable(false), 
              executable(false), hidden(false) {}
    };

    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit IFileHandler(QObject* parent = nullptr) : QObject(parent) {}

    /**
     * @brief 虚析构函数
     */
    virtual ~IFileHandler() = default;

    /**
     * @brief 初始化文件处理器
     * @return 初始化是否成功
     */
    virtual bool initialize() = 0;

    /**
     * @brief 清理文件处理器
     */
    virtual void cleanup() = 0;

    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return 文件是否存在
     */
    virtual bool exists(const QString& path) const = 0;

    /**
     * @brief 读取文件内容
     * @param path 文件路径
     * @param data 输出数据
     * @return 操作结果
     */
    virtual OperationResult read(const QString& path, QByteArray& data) = 0;

    /**
     * @brief 写入文件内容
     * @param path 文件路径
     * @param data 输入数据
     * @param append 是否追加模式
     * @return 操作结果
     */
    virtual OperationResult write(const QString& path, const QByteArray& data, bool append = false) = 0;

    /**
     * @brief 删除文件
     * @param path 文件路径
     * @return 操作结果
     */
    virtual OperationResult remove(const QString& path) = 0;

    /**
     * @brief 复制文件
     * @param sourcePath 源文件路径
     * @param destPath 目标文件路径
     * @param overwrite 是否覆盖已存在文件
     * @return 操作结果
     */
    virtual OperationResult copy(const QString& sourcePath, const QString& destPath, bool overwrite = false) = 0;

    /**
     * @brief 移动文件
     * @param sourcePath 源文件路径
     * @param destPath 目标文件路径
     * @param overwrite 是否覆盖已存在文件
     * @return 操作结果
     */
    virtual OperationResult move(const QString& sourcePath, const QString& destPath, bool overwrite = false) = 0;

    /**
     * @brief 获取文件属性
     * @param path 文件路径
     * @param attributes 输出属性
     * @return 操作结果
     */
    virtual OperationResult getAttributes(const QString& path, FileAttributes& attributes) = 0;

    /**
     * @brief 设置文件属性
     * @param path 文件路径
     * @param attributes 输入属性
     * @return 操作结果
     */
    virtual OperationResult setAttributes(const QString& path, const FileAttributes& attributes) = 0;

    /**
     * @brief 获取文件大小
     * @param path 文件路径
     * @return 文件大小（-1表示错误）
     */
    virtual qint64 size(const QString& path) const = 0;

    /**
     * @brief 检查文件是否可读
     * @param path 文件路径
     * @return 是否可读
     */
    virtual bool isReadable(const QString& path) const = 0;

    /**
     * @brief 检查文件是否可写
     * @param path 文件路径
     * @return 是否可写
     */
    virtual bool isWritable(const QString& path) const = 0;

    /**
     * @brief 检查文件是否可执行
     * @param path 文件路径
     * @return 是否可执行
     */
    virtual bool isExecutable(const QString& path) const = 0;

    /**
     * @brief 获取支持的文件扩展名
     * @return 支持的扩展名列表
     */
    virtual QStringList supportedExtensions() const = 0;

    /**
     * @brief 检查是否支持指定文件
     * @param path 文件路径
     * @return 是否支持
     */
    virtual bool supports(const QString& path) const = 0;

    /**
     * @brief 获取处理器名称
     * @return 处理器名称
     */
    virtual QString name() const = 0;

    /**
     * @brief 获取处理器版本
     * @return 处理器版本
     */
    virtual QString version() const = 0;

    /**
     * @brief 批量读取文件
     * @param paths 文件路径列表
     * @param dataList 输出数据列表
     * @return 操作结果列表
     */
    virtual QList<OperationResult> readBatch(const QStringList& paths, QList<QByteArray>& dataList) {
        QList<OperationResult> results;
        dataList.clear();
        
        for (const QString& path : paths) {
            QByteArray data;
            OperationResult result = read(path, data);
            results.append(result);
            dataList.append(data);
        }
        
        return results;
    }

    /**
     * @brief 批量写入文件
     * @param paths 文件路径列表
     * @param dataList 输入数据列表
     * @param append 是否追加模式
     * @return 操作结果列表
     */
    virtual QList<OperationResult> writeBatch(const QStringList& paths, const QList<QByteArray>& dataList, bool append = false) {
        QList<OperationResult> results;
        
        int count = qMin(paths.size(), dataList.size());
        for (int i = 0; i < count; ++i) {
            OperationResult result = write(paths[i], dataList[i], append);
            results.append(result);
        }
        
        return results;
    }

    /**
     * @brief 操作结果转字符串
     * @param result 操作结果
     * @return 结果字符串
     */
    static QString resultToString(OperationResult result) {
        switch (result) {
            case Success: return "Success";
            case FileNotFound: return "File not found";
            case PermissionDenied: return "Permission denied";
            case DiskFull: return "Disk full";
            case InvalidPath: return "Invalid path";
            case UnknownError: return "Unknown error";
            default: return "Unknown result";
        }
    }

signals:
    /**
     * @brief 文件操作完成信号
     * @param operation 操作类型
     * @param path 文件路径
     * @param result 操作结果
     */
    void operationCompleted(const QString& operation, const QString& path, OperationResult result);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 进度更新信号（用于大文件操作）
     * @param path 文件路径
     * @param bytesProcessed 已处理字节数
     * @param totalBytes 总字节数
     */
    void progressUpdated(const QString& path, qint64 bytesProcessed, qint64 totalBytes);

protected:
    /**
     * @brief 验证文件路径
     * @param path 文件路径
     * @return 路径是否有效
     */
    virtual bool validatePath(const QString& path) const {
        return !path.isEmpty() && !path.contains("..") && !path.contains("//");
    }

    /**
     * @brief 发出操作完成信号
     * @param operation 操作类型
     * @param path 文件路径
     * @param result 操作结果
     */
    void emitOperationCompleted(const QString& operation, const QString& path, OperationResult result) {
        emit operationCompleted(operation, path, result);
    }
};

// 声明元类型以支持信号槽
Q_DECLARE_METATYPE(IFileHandler::OperationResult)
Q_DECLARE_METATYPE(IFileHandler::FileAttributes)

#endif // IFILEHANDLER_H
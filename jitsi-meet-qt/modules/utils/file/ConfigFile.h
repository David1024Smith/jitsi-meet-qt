#ifndef CONFIGFILE_H
#define CONFIGFILE_H

#include "../interfaces/IFileHandler.h"
#include <QSettings>
#include <QVariant>
#include <QStringList>
#include <QMutex>
#include <QFileSystemWatcher>
#include <QTimer>

/**
 * @brief 配置文件处理器
 * 
 * ConfigFile提供统一的配置文件管理功能，支持多种配置文件格式
 * （INI、JSON、XML），自动保存、变更监控和配置验证。
 */
class ConfigFile : public IFileHandler
{
    Q_OBJECT

public:
    /**
     * @brief 配置文件格式枚举
     */
    enum Format {
        IniFormat,      ///< INI格式
        JsonFormat,     ///< JSON格式
        XmlFormat,      ///< XML格式
        AutoDetect      ///< 自动检测格式
    };
    Q_ENUM(Format)

    /**
     * @brief 配置访问模式枚举
     */
    enum AccessMode {
        ReadOnly,       ///< 只读模式
        WriteOnly,      ///< 只写模式
        ReadWrite       ///< 读写模式
    };
    Q_ENUM(AccessMode)

    /**
     * @brief 构造函数
     * @param filePath 配置文件路径
     * @param format 文件格式
     * @param parent 父对象
     */
    explicit ConfigFile(const QString& filePath, Format format = AutoDetect, QObject* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ConfigFile() override;

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
     * @brief 加载配置文件
     * @return 加载是否成功
     */
    bool load();

    /**
     * @brief 保存配置文件
     * @return 保存是否成功
     */
    bool save();

    /**
     * @brief 重新加载配置文件
     * @return 重新加载是否成功
     */
    bool reload();

    /**
     * @brief 设置配置值
     * @param key 配置键
     * @param value 配置值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 获取配置值
     * @param key 配置键
     * @param defaultValue 默认值
     * @return 配置值
     */
    QVariant value(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 检查配置键是否存在
     * @param key 配置键
     * @return 是否存在
     */
    bool contains(const QString& key) const;

    /**
     * @brief 移除配置项
     * @param key 配置键
     */
    void removeKey(const QString& key);

    /**
     * @brief 获取所有配置键
     * @return 配置键列表
     */
    QStringList allKeys() const;

    /**
     * @brief 获取子键列表
     * @param prefix 前缀
     * @return 子键列表
     */
    QStringList childKeys(const QString& prefix = QString()) const;

    /**
     * @brief 获取子组列表
     * @param prefix 前缀
     * @return 子组列表
     */
    QStringList childGroups(const QString& prefix = QString()) const;

    /**
     * @brief 开始配置组
     * @param group 组名
     */
    void beginGroup(const QString& group);

    /**
     * @brief 结束配置组
     */
    void endGroup();

    /**
     * @brief 获取当前组
     * @return 当前组名
     */
    QString group() const;

    /**
     * @brief 清空所有配置
     */
    void clear();

    /**
     * @brief 设置文件格式
     * @param format 文件格式
     */
    void setFormat(Format format);

    /**
     * @brief 获取文件格式
     * @return 文件格式
     */
    Format format() const;

    /**
     * @brief 设置访问模式
     * @param mode 访问模式
     */
    void setAccessMode(AccessMode mode);

    /**
     * @brief 获取访问模式
     * @return 访问模式
     */
    AccessMode accessMode() const;

    /**
     * @brief 启用/禁用自动保存
     * @param enabled 是否启用
     */
    void setAutoSave(bool enabled);

    /**
     * @brief 检查自动保存是否启用
     * @return 是否启用
     */
    bool isAutoSave() const;

    /**
     * @brief 设置自动保存间隔（毫秒）
     * @param interval 保存间隔
     */
    void setAutoSaveInterval(int interval);

    /**
     * @brief 获取自动保存间隔
     * @return 保存间隔
     */
    int autoSaveInterval() const;

    /**
     * @brief 启用/禁用文件监控
     * @param enabled 是否启用
     */
    void setFileWatchEnabled(bool enabled);

    /**
     * @brief 检查文件监控是否启用
     * @return 是否启用
     */
    bool isFileWatchEnabled() const;

    /**
     * @brief 设置配置验证器
     * @param validator 验证器函数
     */
    void setValidator(std::function<bool(const QString&, const QVariant&)> validator);

    /**
     * @brief 验证配置
     * @return 验证是否通过
     */
    bool validate() const;

    /**
     * @brief 获取配置文件路径
     * @return 文件路径
     */
    QString filePath() const;

    /**
     * @brief 检查配置是否已修改
     * @return 是否已修改
     */
    bool isModified() const;

    /**
     * @brief 获取最后修改时间
     * @return 最后修改时间
     */
    QDateTime lastModified() const;

    /**
     * @brief 创建配置备份
     * @param backupPath 备份路径
     * @return 备份是否成功
     */
    bool createBackup(const QString& backupPath = QString());

    /**
     * @brief 从备份恢复配置
     * @param backupPath 备份路径
     * @return 恢复是否成功
     */
    bool restoreFromBackup(const QString& backupPath);

    /**
     * @brief 导出配置到指定格式
     * @param exportPath 导出路径
     * @param exportFormat 导出格式
     * @return 导出是否成功
     */
    bool exportTo(const QString& exportPath, Format exportFormat);

    /**
     * @brief 从指定文件导入配置
     * @param importPath 导入路径
     * @param merge 是否合并（否则替换）
     * @return 导入是否成功
     */
    bool importFrom(const QString& importPath, bool merge = true);

signals:
    /**
     * @brief 配置值改变信号
     * @param key 配置键
     * @param value 新值
     */
    void valueChanged(const QString& key, const QVariant& value);

    /**
     * @brief 配置文件改变信号
     */
    void fileChanged();

    /**
     * @brief 配置保存信号
     */
    void saved();

    /**
     * @brief 配置加载信号
     */
    void loaded();

private slots:
    /**
     * @brief 文件系统监控槽函数
     * @param path 文件路径
     */
    void onFileChanged(const QString& path);

    /**
     * @brief 自动保存定时器槽函数
     */
    void onAutoSaveTimer();

private:
    /**
     * @brief 检测文件格式
     * @param filePath 文件路径
     * @return 检测到的格式
     */
    Format detectFormat(const QString& filePath) const;

    /**
     * @brief 加载INI格式配置
     * @return 加载是否成功
     */
    bool loadIniFormat();

    /**
     * @brief 保存INI格式配置
     * @return 保存是否成功
     */
    bool saveIniFormat();

    /**
     * @brief 加载JSON格式配置
     * @return 加载是否成功
     */
    bool loadJsonFormat();

    /**
     * @brief 保存JSON格式配置
     * @return 保存是否成功
     */
    bool saveJsonFormat();

    /**
     * @brief 加载XML格式配置
     * @return 加载是否成功
     */
    bool loadXmlFormat();

    /**
     * @brief 保存XML格式配置
     * @return 保存是否成功
     */
    bool saveXmlFormat();

    /**
     * @brief 创建目录（如果不存在）
     * @param filePath 文件路径
     * @return 创建是否成功
     */
    bool ensureDirectoryExists(const QString& filePath);

    /**
     * @brief 标记为已修改
     */
    void markAsModified();

private:
    QString m_filePath;                                                 ///< 配置文件路径
    Format m_format;                                                    ///< 文件格式
    AccessMode m_accessMode;                                            ///< 访问模式
    
    QVariantMap m_data;                                                 ///< 配置数据
    QStringList m_groupStack;                                           ///< 组栈
    
    bool m_autoSave;                                                    ///< 是否自动保存
    int m_autoSaveInterval;                                             ///< 自动保存间隔
    QTimer* m_autoSaveTimer;                                            ///< 自动保存定时器
    
    bool m_fileWatchEnabled;                                            ///< 是否启用文件监控
    QFileSystemWatcher* m_fileWatcher;                                  ///< 文件系统监控器
    
    bool m_modified;                                                    ///< 是否已修改
    QDateTime m_lastModified;                                           ///< 最后修改时间
    
    std::function<bool(const QString&, const QVariant&)> m_validator;   ///< 配置验证器
    
    mutable QMutex m_mutex;                                             ///< 线程安全互斥锁
};

#endif // CONFIGFILE_H
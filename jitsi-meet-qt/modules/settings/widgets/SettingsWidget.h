#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <memory>

// Forward declarations
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTabWidget;
class QScrollArea;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QSlider;
class QPushButton;
class QTextEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QSplitter;
class SettingsManager;
class PreferencesHandler;

/**
 * @brief 设置管理组件
 * 
 * 提供完整的设置管理用户界面，支持分类显示、实时编辑、验证反馈等功能。
 * 可以自动生成设置界面或使用自定义布局。
 */
class SettingsWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QString currentCategory READ currentCategory WRITE setCurrentCategory NOTIFY currentCategoryChanged)
    Q_PROPERTY(bool autoSave READ isAutoSaveEnabled WRITE setAutoSaveEnabled NOTIFY autoSaveChanged)
    Q_PROPERTY(bool showAdvanced READ isShowAdvancedEnabled WRITE setShowAdvancedEnabled NOTIFY showAdvancedChanged)
    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)

public:
    /**
     * @brief 视图模式枚举
     */
    enum ViewMode {
        TabView,        ///< 标签页视图
        TreeView,       ///< 树形视图
        ListView,       ///< 列表视图
        WizardView      ///< 向导视图
    };
    Q_ENUM(ViewMode)

    /**
     * @brief 设置项类型枚举
     */
    enum SettingType {
        StringSetting,      ///< 字符串设置
        IntegerSetting,     ///< 整数设置
        DoubleSetting,      ///< 浮点数设置
        BooleanSetting,     ///< 布尔设置
        EnumSetting,        ///< 枚举设置
        PathSetting,        ///< 路径设置
        ColorSetting,       ///< 颜色设置
        FontSetting,        ///< 字体设置
        DateTimeSetting,    ///< 日期时间设置
        ListSetting,        ///< 列表设置
        CustomSetting       ///< 自定义设置
    };
    Q_ENUM(SettingType)

    /**
     * @brief 设置项描述结构
     */
    struct SettingDescriptor {
        QString key;                ///< 设置键
        QString displayName;        ///< 显示名称
        QString description;        ///< 描述信息
        SettingType type;           ///< 设置类型
        QVariant defaultValue;      ///< 默认值
        QVariant minValue;          ///< 最小值
        QVariant maxValue;          ///< 最大值
        QStringList enumValues;     ///< 枚举值列表
        QString category;           ///< 所属类别
        bool isAdvanced;            ///< 是否为高级设置
        bool isReadOnly;            ///< 是否只读
        QString tooltip;            ///< 工具提示
        QString placeholder;        ///< 占位符文本
        
        SettingDescriptor() 
            : type(StringSetting), isAdvanced(false), isReadOnly(false) {}
    };

    explicit SettingsWidget(QWidget* parent = nullptr);
    ~SettingsWidget();

    /**
     * @brief 获取当前类别
     * @return 当前类别名称
     */
    QString currentCategory() const;

    /**
     * @brief 设置当前类别
     * @param category 类别名称
     */
    void setCurrentCategory(const QString& category);

    /**
     * @brief 检查是否启用自动保存
     * @return 是否启用自动保存
     */
    bool isAutoSaveEnabled() const;

    /**
     * @brief 启用/禁用自动保存
     * @param enabled 是否启用
     */
    void setAutoSaveEnabled(bool enabled);

    /**
     * @brief 检查是否显示高级设置
     * @return 是否显示高级设置
     */
    bool isShowAdvancedEnabled() const;

    /**
     * @brief 启用/禁用显示高级设置
     * @param enabled 是否启用
     */
    void setShowAdvancedEnabled(bool enabled);

    /**
     * @brief 获取视图模式
     * @return 视图模式
     */
    ViewMode viewMode() const;

    /**
     * @brief 设置视图模式
     * @param mode 视图模式
     */
    void setViewMode(ViewMode mode);

    // 设置管理器
    /**
     * @brief 设置设置管理器
     * @param manager 设置管理器实例
     */
    void setSettingsManager(SettingsManager* manager);

    /**
     * @brief 获取设置管理器
     * @return 设置管理器实例
     */
    SettingsManager* settingsManager() const;

    /**
     * @brief 设置偏好处理器
     * @param handler 偏好处理器实例
     */
    void setPreferencesHandler(PreferencesHandler* handler);

    /**
     * @brief 获取偏好处理器
     * @return 偏好处理器实例
     */
    PreferencesHandler* preferencesHandler() const;

    // 设置项管理
    /**
     * @brief 添加设置项
     * @param descriptor 设置项描述
     */
    void addSetting(const SettingDescriptor& descriptor);

    /**
     * @brief 移除设置项
     * @param key 设置键
     */
    void removeSetting(const QString& key);

    /**
     * @brief 获取设置项描述
     * @param key 设置键
     * @return 设置项描述
     */
    SettingDescriptor getSettingDescriptor(const QString& key) const;

    /**
     * @brief 获取所有设置项
     * @return 设置项描述列表
     */
    QList<SettingDescriptor> getAllSettings() const;

    /**
     * @brief 获取类别的设置项
     * @param category 类别名称
     * @return 设置项描述列表
     */
    QList<SettingDescriptor> getCategorySettings(const QString& category) const;

    /**
     * @brief 批量添加设置项
     * @param descriptors 设置项描述列表
     */
    void addSettings(const QList<SettingDescriptor>& descriptors);

    /**
     * @brief 从JSON加载设置项
     * @param json JSON对象
     * @return 加载是否成功
     */
    bool loadSettingsFromJson(const QJsonObject& json);

    /**
     * @brief 导出设置项到JSON
     * @return JSON对象
     */
    QJsonObject exportSettingsToJson() const;

    // 类别管理
    /**
     * @brief 添加类别
     * @param category 类别名称
     * @param displayName 显示名称
     * @param icon 图标路径
     */
    void addCategory(const QString& category, const QString& displayName = QString(), const QString& icon = QString());

    /**
     * @brief 移除类别
     * @param category 类别名称
     */
    void removeCategory(const QString& category);

    /**
     * @brief 获取所有类别
     * @return 类别列表
     */
    QStringList categories() const;

    /**
     * @brief 设置类别显示名称
     * @param category 类别名称
     * @param displayName 显示名称
     */
    void setCategoryDisplayName(const QString& category, const QString& displayName);

    /**
     * @brief 获取类别显示名称
     * @param category 类别名称
     * @return 显示名称
     */
    QString getCategoryDisplayName(const QString& category) const;

    /**
     * @brief 设置类别图标
     * @param category 类别名称
     * @param icon 图标路径
     */
    void setCategoryIcon(const QString& category, const QString& icon);

    /**
     * @brief 获取类别图标
     * @param category 类别名称
     * @return 图标路径
     */
    QString getCategoryIcon(const QString& category) const;

    // 值操作
    /**
     * @brief 设置值
     * @param key 设置键
     * @param value 值
     */
    void setValue(const QString& key, const QVariant& value);

    /**
     * @brief 获取值
     * @param key 设置键
     * @return 值
     */
    QVariant value(const QString& key) const;

    /**
     * @brief 重置值为默认值
     * @param key 设置键
     */
    void resetValue(const QString& key);

    /**
     * @brief 重置类别的所有值
     * @param category 类别名称
     */
    void resetCategory(const QString& category);

    /**
     * @brief 重置所有值
     */
    void resetAll();

    // 验证和保存
    /**
     * @brief 验证所有设置
     * @return 验证是否通过
     */
    bool validateSettings();

    /**
     * @brief 获取验证错误
     * @return 错误列表
     */
    QStringList validationErrors() const;

    /**
     * @brief 保存设置
     * @return 保存是否成功
     */
    bool saveSettings();

    /**
     * @brief 加载设置
     * @return 加载是否成功
     */
    bool loadSettings();

    /**
     * @brief 检查是否有未保存的更改
     * @return 是否有更改
     */
    bool hasUnsavedChanges() const;

    // 搜索和过滤
    /**
     * @brief 设置搜索文本
     * @param text 搜索文本
     */
    void setSearchText(const QString& text);

    /**
     * @brief 获取搜索文本
     * @return 搜索文本
     */
    QString searchText() const;

    /**
     * @brief 设置过滤器
     * @param filter 过滤器函数
     */
    void setFilter(std::function<bool(const SettingDescriptor&)> filter);

    /**
     * @brief 清除过滤器
     */
    void clearFilter();

    // 主题和样式
    /**
     * @brief 设置主题
     * @param theme 主题名称
     */
    void setTheme(const QString& theme);

    /**
     * @brief 获取主题
     * @return 主题名称
     */
    QString theme() const;

    /**
     * @brief 设置自定义样式表
     * @param styleSheet 样式表
     */
    void setCustomStyleSheet(const QString& styleSheet);

    // 导入导出
    /**
     * @brief 导出设置到文件
     * @param filePath 文件路径
     * @param format 格式（json, ini, xml）
     * @return 导出是否成功
     */
    bool exportToFile(const QString& filePath, const QString& format = "json") const;

    /**
     * @brief 从文件导入设置
     * @param filePath 文件路径
     * @param merge 是否合并
     * @return 导入是否成功
     */
    bool importFromFile(const QString& filePath, bool merge = false);

public slots:
    /**
     * @brief 刷新界面
     */
    void refresh();

    /**
     * @brief 应用设置
     */
    void applySettings();

    /**
     * @brief 取消更改
     */
    void cancelChanges();

    /**
     * @brief 恢复默认设置
     */
    void restoreDefaults();

    /**
     * @brief 展开所有类别
     */
    void expandAll();

    /**
     * @brief 折叠所有类别
     */
    void collapseAll();

    /**
     * @brief 显示帮助
     */
    void showHelp();

signals:
    /**
     * @brief 当前类别变化信号
     * @param category 新类别
     */
    void currentCategoryChanged(const QString& category);

    /**
     * @brief 自动保存设置变化信号
     * @param enabled 是否启用
     */
    void autoSaveChanged(bool enabled);

    /**
     * @brief 显示高级设置变化信号
     * @param enabled 是否显示
     */
    void showAdvancedChanged(bool enabled);

    /**
     * @brief 视图模式变化信号
     * @param mode 新模式
     */
    void viewModeChanged(ViewMode mode);

    /**
     * @brief 设置值变化信号
     * @param key 设置键
     * @param value 新值
     */
    void settingChanged(const QString& key, const QVariant& value);

    /**
     * @brief 设置保存信号
     * @param success 是否成功
     */
    void settingsSaved(bool success);

    /**
     * @brief 设置加载信号
     * @param success 是否成功
     */
    void settingsLoaded(bool success);

    /**
     * @brief 验证完成信号
     * @param success 是否成功
     * @param errors 错误列表
     */
    void validationCompleted(bool success, const QStringList& errors);

    /**
     * @brief 搜索文本变化信号
     * @param text 搜索文本
     */
    void searchTextChanged(const QString& text);

    /**
     * @brief 帮助请求信号
     * @param key 设置键
     */
    void helpRequested(const QString& key);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;

private slots:
    void onSettingValueChanged();
    void onCategorySelectionChanged();
    void onSearchTextChanged(const QString& text);
    void onResetButtonClicked();
    void onApplyButtonClicked();
    void onCancelButtonClicked();

private:
    void setupUI();
    void setupTabView();
    void setupTreeView();
    void setupListView();
    void setupWizardView();
    void createSettingWidget(const SettingDescriptor& descriptor, QWidget* parent);
    QWidget* createStringWidget(const SettingDescriptor& descriptor);
    QWidget* createIntegerWidget(const SettingDescriptor& descriptor);
    QWidget* createDoubleWidget(const SettingDescriptor& descriptor);
    QWidget* createBooleanWidget(const SettingDescriptor& descriptor);
    QWidget* createEnumWidget(const SettingDescriptor& descriptor);
    QWidget* createPathWidget(const SettingDescriptor& descriptor);
    QWidget* createColorWidget(const SettingDescriptor& descriptor);
    QWidget* createFontWidget(const SettingDescriptor& descriptor);
    QWidget* createDateTimeWidget(const SettingDescriptor& descriptor);
    QWidget* createListWidget(const SettingDescriptor& descriptor);
    
    void updateWidgetValue(const QString& key, const QVariant& value);
    QVariant getWidgetValue(const QString& key) const;
    void connectWidgetSignals(const QString& key, QWidget* widget);
    void applyFilter();
    void updateSearchHighlight();
    void loadTheme(const QString& theme);
    void saveWindowState();
    void restoreWindowState();

    class Private;
    std::unique_ptr<Private> d;
};

#endif // SETTINGSWIDGET_H
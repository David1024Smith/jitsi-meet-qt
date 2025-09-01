#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <memory>

// Forward declarations
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTabWidget;
class QListWidget;
class QStackedWidget;
class QGroupBox;
class QLabel;
class QPushButton;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QTextEdit;
class QSpinBox;
class QSlider;
class QProgressBar;
class QSplitter;
class SettingsWidget;
class PreferencesHandler;

/**
 * @brief 偏好设置对话框
 * 
 * 提供用户友好的偏好设置界面，支持配置文件管理、实时预览、导入导出等功能。
 * 集成了设置验证、帮助系统和多语言支持。
 */
class PreferencesDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString currentProfile READ currentProfile WRITE setCurrentProfile NOTIFY currentProfileChanged)
    Q_PROPERTY(bool previewMode READ isPreviewMode WRITE setPreviewMode NOTIFY previewModeChanged)
    Q_PROPERTY(DialogMode dialogMode READ dialogMode WRITE setDialogMode NOTIFY dialogModeChanged)

public:
    /**
     * @brief 对话框模式枚举
     */
    enum DialogMode {
        StandardMode,   ///< 标准模式
        CompactMode,    ///< 紧凑模式
        WizardMode,     ///< 向导模式
        ExpertMode      ///< 专家模式
    };
    Q_ENUM(DialogMode)

    /**
     * @brief 对话框结果枚举
     */
    enum DialogResult {
        Applied = QDialog::Accepted + 1,    ///< 已应用
        Reset,                              ///< 已重置
        Imported,                           ///< 已导入
        Exported                            ///< 已导出
    };
    Q_ENUM(DialogResult)

    /**
     * @brief 页面信息结构
     */
    struct PageInfo {
        QString id;             ///< 页面ID
        QString title;          ///< 页面标题
        QString description;    ///< 页面描述
        QString icon;           ///< 页面图标
        QWidget* widget;        ///< 页面组件
        bool isVisible;         ///< 是否可见
        int order;              ///< 显示顺序
        
        PageInfo() : widget(nullptr), isVisible(true), order(0) {}
    };

    explicit PreferencesDialog(QWidget* parent = nullptr);
    ~PreferencesDialog();

    /**
     * @brief 获取当前配置文件
     * @return 配置文件名称
     */
    QString currentProfile() const;

    /**
     * @brief 设置当前配置文件
     * @param profile 配置文件名称
     */
    void setCurrentProfile(const QString& profile);

    /**
     * @brief 检查是否为预览模式
     * @return 是否为预览模式
     */
    bool isPreviewMode() const;

    /**
     * @brief 启用/禁用预览模式
     * @param enabled 是否启用
     */
    void setPreviewMode(bool enabled);

    /**
     * @brief 获取对话框模式
     * @return 对话框模式
     */
    DialogMode dialogMode() const;

    /**
     * @brief 设置对话框模式
     * @param mode 对话框模式
     */
    void setDialogMode(DialogMode mode);

    // 偏好处理器
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

    // 页面管理
    /**
     * @brief 添加页面
     * @param pageInfo 页面信息
     */
    void addPage(const PageInfo& pageInfo);

    /**
     * @brief 移除页面
     * @param pageId 页面ID
     */
    void removePage(const QString& pageId);

    /**
     * @brief 获取页面信息
     * @param pageId 页面ID
     * @return 页面信息
     */
    PageInfo getPageInfo(const QString& pageId) const;

    /**
     * @brief 获取所有页面
     * @return 页面信息列表
     */
    QList<PageInfo> getAllPages() const;

    /**
     * @brief 设置当前页面
     * @param pageId 页面ID
     */
    void setCurrentPage(const QString& pageId);

    /**
     * @brief 获取当前页面ID
     * @return 页面ID
     */
    QString currentPageId() const;

    /**
     * @brief 设置页面可见性
     * @param pageId 页面ID
     * @param visible 是否可见
     */
    void setPageVisible(const QString& pageId, bool visible);

    /**
     * @brief 检查页面是否可见
     * @param pageId 页面ID
     * @return 是否可见
     */
    bool isPageVisible(const QString& pageId) const;

    // 配置文件管理
    /**
     * @brief 创建新配置文件
     * @param profileName 配置文件名称
     * @param copyFrom 复制源配置文件（可选）
     * @return 创建是否成功
     */
    bool createProfile(const QString& profileName, const QString& copyFrom = QString());

    /**
     * @brief 删除配置文件
     * @param profileName 配置文件名称
     * @return 删除是否成功
     */
    bool deleteProfile(const QString& profileName);

    /**
     * @brief 重命名配置文件
     * @param oldName 旧名称
     * @param newName 新名称
     * @return 重命名是否成功
     */
    bool renameProfile(const QString& oldName, const QString& newName);

    /**
     * @brief 复制配置文件
     * @param sourceName 源配置文件
     * @param targetName 目标配置文件
     * @return 复制是否成功
     */
    bool copyProfile(const QString& sourceName, const QString& targetName);

    /**
     * @brief 获取可用配置文件列表
     * @return 配置文件列表
     */
    QStringList availableProfiles() const;

    /**
     * @brief 设置默认配置文件
     * @param profileName 配置文件名称
     */
    void setDefaultProfile(const QString& profileName);

    /**
     * @brief 获取默认配置文件
     * @return 默认配置文件名称
     */
    QString defaultProfile() const;

    // 导入导出
    /**
     * @brief 导出配置文件
     * @param profileName 配置文件名称
     * @param filePath 导出路径
     * @param format 导出格式
     * @return 导出是否成功
     */
    bool exportProfile(const QString& profileName, const QString& filePath, const QString& format = "json");

    /**
     * @brief 导入配置文件
     * @param filePath 导入路径
     * @param profileName 目标配置文件名称
     * @param merge 是否合并
     * @return 导入是否成功
     */
    bool importProfile(const QString& filePath, const QString& profileName, bool merge = false);

    /**
     * @brief 导出当前设置
     * @param filePath 导出路径
     * @param format 导出格式
     * @return 导出是否成功
     */
    bool exportCurrentSettings(const QString& filePath, const QString& format = "json");

    /**
     * @brief 导入设置
     * @param filePath 导入路径
     * @param merge 是否合并
     * @return 导入是否成功
     */
    bool importSettings(const QString& filePath, bool merge = false);

    // 验证和应用
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
     * @brief 应用设置
     * @return 应用是否成功
     */
    bool applySettings();

    /**
     * @brief 重置设置
     * @param category 类别（空表示全部）
     */
    void resetSettings(const QString& category = QString());

    /**
     * @brief 检查是否有未保存的更改
     * @return 是否有更改
     */
    bool hasUnsavedChanges() const;

    // 搜索和帮助
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
     * @brief 显示帮助
     * @param topic 帮助主题
     */
    void showHelp(const QString& topic = QString());

    /**
     * @brief 设置帮助URL
     * @param url 帮助URL
     */
    void setHelpUrl(const QString& url);

    /**
     * @brief 获取帮助URL
     * @return 帮助URL
     */
    QString helpUrl() const;

    // 主题和本地化
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
     * @brief 设置语言
     * @param language 语言代码
     */
    void setLanguage(const QString& language);

    /**
     * @brief 获取语言
     * @return 语言代码
     */
    QString language() const;

    // 静态便利方法
    /**
     * @brief 显示偏好设置对话框
     * @param parent 父组件
     * @param handler 偏好处理器
     * @return 对话框结果
     */
    static int showPreferences(QWidget* parent, PreferencesHandler* handler);

    /**
     * @brief 快速设置对话框
     * @param parent 父组件
     * @param settings 设置映射
     * @return 对话框结果和设置
     */
    static QPair<int, QVariantMap> quickSettings(QWidget* parent, const QVariantMap& settings);

public slots:
    /**
     * @brief 刷新界面
     */
    void refresh();

    /**
     * @brief 应用并关闭
     */
    void applyAndClose();

    /**
     * @brief 重置为默认值
     */
    void resetToDefaults();

    /**
     * @brief 显示导入对话框
     */
    void showImportDialog();

    /**
     * @brief 显示导出对话框
     */
    void showExportDialog();

    /**
     * @brief 显示配置文件管理器
     */
    void showProfileManager();

    /**
     * @brief 切换预览模式
     */
    void togglePreviewMode();

    /**
     * @brief 显示关于对话框
     */
    void showAbout();

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

signals:
    /**
     * @brief 当前配置文件变化信号
     * @param profile 新配置文件
     */
    void currentProfileChanged(const QString& profile);

    /**
     * @brief 预览模式变化信号
     * @param enabled 是否启用
     */
    void previewModeChanged(bool enabled);

    /**
     * @brief 对话框模式变化信号
     * @param mode 新模式
     */
    void dialogModeChanged(DialogMode mode);

    /**
     * @brief 当前页面变化信号
     * @param pageId 新页面ID
     */
    void currentPageChanged(const QString& pageId);

    /**
     * @brief 设置应用信号
     * @param success 是否成功
     */
    void settingsApplied(bool success);

    /**
     * @brief 设置重置信号
     * @param category 重置的类别
     */
    void settingsReset(const QString& category);

    /**
     * @brief 配置文件变化信号
     * @param oldProfile 旧配置文件
     * @param newProfile 新配置文件
     */
    void profileChanged(const QString& oldProfile, const QString& newProfile);

    /**
     * @brief 配置文件创建信号
     * @param profileName 配置文件名称
     */
    void profileCreated(const QString& profileName);

    /**
     * @brief 配置文件删除信号
     * @param profileName 配置文件名称
     */
    void profileDeleted(const QString& profileName);

    /**
     * @brief 导入完成信号
     * @param success 是否成功
     * @param filePath 文件路径
     */
    void importCompleted(bool success, const QString& filePath);

    /**
     * @brief 导出完成信号
     * @param success 是否成功
     * @param filePath 文件路径
     */
    void exportCompleted(bool success, const QString& filePath);

    /**
     * @brief 验证完成信号
     * @param success 是否成功
     * @param errors 错误列表
     */
    void validationCompleted(bool success, const QStringList& errors);

    /**
     * @brief 帮助请求信号
     * @param topic 帮助主题
     */
    void helpRequested(const QString& topic);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

    /**
     * @brief 对话框显示信号
     */
    void dialogShown();

    /**
     * @brief 对话框关闭信号
     */
    void dialogClosed();

    /**
     * @brief 设置变化信号
     * @param key 设置键
     * @param value 设置值
     */
    void settingChanged(const QString& key, const QVariant& value);

    /**
     * @brief 刷新完成信号
     */
    void refreshCompleted();

    /**
     * @brief 主题变化信号
     * @param theme 新主题名称
     */
    void themeChanged(const QString& theme);

private slots:
    void onPageSelectionChanged();
    void onProfileChanged(const QString& profile);
    void onSettingChanged(const QString& key, const QVariant& value);
    void onApplyButtonClicked();
    void onCancelButtonClicked();
    void onResetButtonClicked();
    void onOkButtonClicked();
    void onHelpButtonClicked();
    void onImportButtonClicked();
    void onExportButtonClicked();
    void onProfileManagerButtonClicked();
    void onSearchTextChanged(const QString& text);
    void onPreviewModeToggled(bool enabled);

private:
    void setupUI();
    void setupStandardMode();
    void loadProfileSettings();
    void applyPreviewSettings();
    void setupCompactMode();
    void setupWizardMode();
    void setupExpertMode();
    void createDefaultPages();
    void createGeneralPage();
    void createAudioPage();
    void createVideoPage();
    void createNetworkPage();
    void createUIPage();
    void createAdvancedPage();
    void setupButtonBox();
    void setupProfileSelector();
    void setupSearchBox();
    void connectSignals();
    void updateUI();
    void updatePageList();
    void updateButtonStates();
    void applyTheme(const QString& theme);
    void retranslateUI();
    bool confirmUnsavedChanges();
    void saveDialogState();
    void restoreDialogState();
    QString getPageIconPath(const QString& pageId) const;
    void showValidationErrors(const QStringList& errors);
    void showProgressDialog(const QString& title, const QString& message);
    void hideProgressDialog();

    class Private;
    std::unique_ptr<Private> d;
};

#endif // PREFERENCESDIALOG_H
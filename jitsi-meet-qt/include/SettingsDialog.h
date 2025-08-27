#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>

class ConfigurationManager;
class TranslationManager;
struct ApplicationSettings;

/**
 * @brief 设置对话框类，提供应用程序配置界面
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param configManager 配置管理器
     * @param translationManager 翻译管理器
     * @param parent 父窗口
     */
    explicit SettingsDialog(ConfigurationManager* configManager,
                           TranslationManager* translationManager,
                           QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~SettingsDialog();

public slots:
    /**
     * @brief 显示设置对话框
     */
    void showSettings();

    /**
     * @brief 重置所有设置为默认值
     */
    void resetToDefaults();

signals:
    /**
     * @brief 设置已保存信号
     */
    void settingsSaved();

    /**
     * @brief 语言改变信号
     * @param language 新的语言代码
     */
    void languageChanged(const QString& language);

private slots:
    /**
     * @brief 服务器URL输入改变时的处理
     */
    void onServerUrlChanged();

    /**
     * @brief 语言选择改变时的处理
     */
    void onLanguageChanged();

    /**
     * @brief 深色模式切换时的处理
     */
    void onDarkModeToggled();

    /**
     * @brief 保存设置
     */
    void saveSettings();

    /**
     * @brief 取消设置更改
     */
    void cancelSettings();

    /**
     * @brief 应用设置（不关闭对话框）
     */
    void applySettings();

    /**
     * @brief 恢复默认设置
     */
    void restoreDefaults();

    /**
     * @brief 验证服务器URL
     */
    void validateServerUrl();

    /**
     * @brief 当翻译管理器语言改变时更新界面
     */
    void onTranslationChanged();

private:
    /**
     * @brief 初始化用户界面
     */
    void setupUI();

    /**
     * @brief 创建服务器设置组
     * @return 服务器设置组框
     */
    QGroupBox* createServerGroup();

    /**
     * @brief 创建界面设置组
     * @return 界面设置组框
     */
    QGroupBox* createInterfaceGroup();

    /**
     * @brief 创建会议设置组
     * @return 会议设置组框
     */
    QGroupBox* createConferenceGroup();

    /**
     * @brief 创建高级设置组
     * @return 高级设置组框
     */
    QGroupBox* createAdvancedGroup();

    /**
     * @brief 设置信号连接
     */
    void setupConnections();

    /**
     * @brief 加载当前设置到界面
     */
    void loadSettings();

    /**
     * @brief 从界面获取设置
     * @return 应用程序设置
     */
    ApplicationSettings getSettingsFromUI() const;

    /**
     * @brief 更新界面文本（用于多语言支持）
     */
    void updateUIText();

    /**
     * @brief 验证设置输入
     * @return 是否有效
     */
    bool validateInput();

    /**
     * @brief 显示验证错误
     * @param message 错误消息
     */
    void showValidationError(const QString& message);

    /**
     * @brief 设置控件启用状态
     * @param enabled 是否启用
     */
    void setControlsEnabled(bool enabled);

    /**
     * @brief 检查设置是否有更改
     * @return 是否有更改
     */
    bool hasChanges() const;

private:
    // 管理器
    ConfigurationManager* m_configManager;
    TranslationManager* m_translationManager;

    // 服务器设置
    QGroupBox* m_serverGroup;
    QLineEdit* m_serverUrlEdit;
    QSpinBox* m_serverTimeoutSpin;
    QLabel* m_serverUrlLabel;
    QLabel* m_serverTimeoutLabel;
    QLabel* m_serverUrlStatusLabel;

    // 界面设置
    QGroupBox* m_interfaceGroup;
    QComboBox* m_languageCombo;
    QCheckBox* m_darkModeCheck;
    QCheckBox* m_rememberWindowStateCheck;
    QLabel* m_languageLabel;

    // 会议设置
    QGroupBox* m_conferenceGroup;
    QCheckBox* m_autoJoinAudioCheck;
    QCheckBox* m_autoJoinVideoCheck;

    // 高级设置
    QGroupBox* m_advancedGroup;
    QSpinBox* m_maxRecentItemsSpin;
    QPushButton* m_clearRecentButton;
    QLabel* m_maxRecentItemsLabel;

    // 按钮
    QDialogButtonBox* m_buttonBox;
    QPushButton* m_applyButton;
    QPushButton* m_resetButton;

    // 布局
    QVBoxLayout* m_mainLayout;

    // 原始设置（用于检测更改）
    ApplicationSettings m_originalSettings;
};

#endif // SETTINGSDIALOG_H
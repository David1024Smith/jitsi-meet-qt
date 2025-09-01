#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QMap>
#include <QVariantMap>

class QTabWidget;
class QDialogButtonBox;
class QVBoxLayout;
class QWidget;
class QStackedWidget;
class QListWidget;
class QSplitter;
class QLabel;

/**
 * @brief 设置对话框
 * 
 * 该对话框提供应用程序的设置界面，包括多个设置分类和选项。
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    enum SettingsPage {
        GeneralPage,
        AudioPage,
        VideoPage,
        NetworkPage,
        SecurityPage,
        AppearancePage,
        AdvancedPage,
        AboutPage
    };
    Q_ENUM(SettingsPage)

    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    /**
     * @brief 显示特定设置页面
     * @param page 设置页面
     */
    void showPage(SettingsPage page);

    /**
     * @brief 获取当前设置页面
     * @return 当前页面
     */
    SettingsPage currentPage() const;

    /**
     * @brief 设置设置值
     * @param key 设置键
     * @param value 设置值
     */
    void setSetting(const QString& key, const QVariant& value);

    /**
     * @brief 获取设置值
     * @param key 设置键
     * @param defaultValue 默认值
     * @return 设置值
     */
    QVariant getSetting(const QString& key, const QVariant& defaultValue = QVariant()) const;

    /**
     * @brief 重置所有设置
     */
    void resetAllSettings();

    /**
     * @brief 重置特定页面的设置
     * @param page 设置页面
     */
    void resetPageSettings(SettingsPage page);

    /**
     * @brief 应用设置
     * @return 是否成功应用
     */
    bool applySettings();

    /**
     * @brief 加载设置
     * @return 是否成功加载
     */
    bool loadSettings();

    /**
     * @brief 保存设置
     * @return 是否成功保存
     */
    bool saveSettings();

    /**
     * @brief 验证设置
     * @return 是否有效
     */
    bool validateSettings();

    /**
     * @brief 获取验证错误
     * @return 错误列表
     */
    QStringList validationErrors() const;

public slots:
    /**
     * @brief 接受设置
     */
    void accept() override;

    /**
     * @brief 拒绝设置
     */
    void reject() override;

signals:
    /**
     * @brief 设置已更改信号
     * @param settings 设置映射
     */
    void settingsChanged(const QVariantMap& settings);

    /**
     * @brief 设置已应用信号
     */
    void settingsApplied();

    /**
     * @brief 设置已重置信号
     */
    void settingsReset();

    /**
     * @brief 页面已更改信号
     * @param page 设置页面
     */
    void pageChanged(SettingsPage page);

private slots:
    void onPageChanged(int index);
    void onApplyClicked();
    void onResetClicked();
    void onCategorySelectionChanged();
    void onSettingValueChanged();
    void onRestoreDefaultsClicked();

private:
    void setupUI();
    void createPages();
    void createGeneralPage();
    void createAudioPage();
    void createVideoPage();
    void createNetworkPage();
    void createSecurityPage();
    void createAppearancePage();
    void createAdvancedPage();
    void createAboutPage();
    void setupConnections();
    void updateButtonStates();
    void savePageState(SettingsPage page);
    void restorePageState(SettingsPage page);
    QString pageToString(SettingsPage page) const;
    SettingsPage stringToPage(const QString& pageName) const;
    QWidget* createPageWidget(SettingsPage page);
    void collectSettings();
    void applySettingsToPage(SettingsPage page);
    void validatePage(SettingsPage page);

    QVBoxLayout* m_mainLayout;
    QSplitter* m_splitter;
    QListWidget* m_categoryList;
    QStackedWidget* m_pageStack;
    QDialogButtonBox* m_buttonBox;
    QLabel* m_headerLabel;
    
    QMap<SettingsPage, QWidget*> m_pageWidgets;
    QMap<SettingsPage, QVariantMap> m_pageSettings;
    QMap<SettingsPage, bool> m_pageModified;
    
    QVariantMap m_settings;
    QVariantMap m_originalSettings;
    QStringList m_errors;
    
    SettingsPage m_currentPage;
    bool m_settingsModified;
    bool m_applyingSettings;
};

#endif // SETTINGSDIALOG_H
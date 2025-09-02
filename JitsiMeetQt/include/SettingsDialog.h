#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QGroupBox>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTextEdit>
#include <QFileDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QTimer>
#include <QCloseEvent>
#include <QShowEvent>
#include <QJsonObject>
#include <QJsonDocument>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QTabWidget;
class QWidget;
class QLabel;
class QLineEdit;
class QPushButton;
class QComboBox;
class QCheckBox;
class QSpinBox;
class QSlider;
class QGroupBox;
class QListWidget;
class QListWidgetItem;
class QTextEdit;
class QProgressBar;
class QTimer;
QT_END_NAMESPACE

class ConfigurationManager;

/**
 * @brief 设置对话框类 - 应用程序配置界面
 * 
 * 这个类提供了完整的设置界面，包括：
 * - 常规设置（语言、主题、启动选项）
 * - 服务器设置（默认服务器、超时、代理）
 * - 音视频设置（默认设备、质量、编解码器）
 * - 界面设置（窗口大小、字体、颜色）
 * - 高级设置（日志、缓存、实验性功能）
 * - 关于信息（版本、许可证、更新）
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 设置页面枚举
     */
    enum SettingsPage {
        GeneralPage = 0,    ///< 常规设置页面
        ServerPage,         ///< 服务器设置页面
        AudioVideoPage,     ///< 音视频设置页面
        InterfacePage,      ///< 界面设置页面
        AdvancedPage,       ///< 高级设置页面
        AboutPage           ///< 关于页面
    };
    
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SettingsDialog();
    
    /**
     * @brief 显示指定的设置页面
     * @param page 要显示的页面
     */
    void showPage(SettingsPage page);
    
    /**
     * @brief 应用设置更改
     */
    void applySettings();
    
    /**
     * @brief 重置设置到默认值
     */
    void resetToDefaults();
    
    /**
     * @brief 导入设置
     * @param filePath 设置文件路径
     * @return 是否成功导入
     */
    bool importSettings(const QString& filePath);
    
    /**
     * @brief 导出设置
     * @param filePath 导出文件路径
     * @return 是否成功导出
     */
    bool exportSettings(const QString& filePath);

protected:
    /**
     * @brief 窗口关闭事件处理
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent *event) override;
    
    /**
     * @brief 窗口显示事件处理
     * @param event 显示事件
     */
    void showEvent(QShowEvent *event) override;

public slots:
    /**
     * @brief 确定按钮点击处理
     */
    void onOkClicked();
    
    /**
     * @brief 取消按钮点击处理
     */
    void onCancelClicked();
    
    /**
     * @brief 应用按钮点击处理
     */
    void onApplyClicked();
    
    /**
     * @brief 重置按钮点击处理
     */
    void onResetClicked();
    
    /**
     * @brief 导入按钮点击处理
     */
    void onImportClicked();
    
    /**
     * @brief 导出按钮点击处理
     */
    void onExportClicked();
    
    /**
     * @brief 语言选择改变处理
     * @param index 选择的索引
     */
    void onLanguageChanged(int index);
    
    /**
     * @brief 主题选择改变处理
     * @param index 选择的索引
     */
    void onThemeChanged(int index);
    
    /**
     * @brief 开机自启动选项改变处理
     * @param checked 是否选中
     */
    void onStartupChanged(bool checked);
    
    /**
     * @brief 系统托盘选项改变处理
     * @param checked 是否选中
     */
    void onSystemTrayChanged(bool checked);
    
    /**
     * @brief 最小化到托盘选项改变处理
     * @param checked 是否选中
     */
    void onMinimizeToTrayChanged(bool checked);
    
    /**
     * @brief 默认服务器改变处理
     * @param text 服务器URL
     */
    void onDefaultServerChanged(const QString& text);
    
    /**
     * @brief 连接超时改变处理
     * @param value 超时值
     */
    void onConnectionTimeoutChanged(int value);
    
    /**
     * @brief 代理设置改变处理
     */
    void onProxySettingsChanged();
    
    /**
     * @brief 音频设备选择改变处理
     * @param index 选择的索引
     */
    void onAudioDeviceChanged(int index);
    
    /**
     * @brief 视频设备选择改变处理
     * @param index 选择的索引
     */
    void onVideoDeviceChanged(int index);
    
    /**
     * @brief 音频质量改变处理
     * @param index 选择的索引
     */
    void onAudioQualityChanged(int index);
    
    /**
     * @brief 视频质量改变处理
     * @param index 选择的索引
     */
    void onVideoQualityChanged(int index);
    
    /**
     * @brief 默认静音选项改变处理
     * @param checked 是否选中
     */
    void onDefaultMuteChanged(bool checked);
    
    /**
     * @brief 默认关闭摄像头选项改变处理
     * @param checked 是否选中
     */
    void onDefaultCameraOffChanged(bool checked);
    
    /**
     * @brief 字体选择按钮点击处理
     */
    void onFontSelectClicked();
    
    /**
     * @brief 颜色选择按钮点击处理
     */
    void onColorSelectClicked();
    
    /**
     * @brief 窗口大小选项改变处理
     */
    void onWindowSizeChanged();
    
    /**
     * @brief 日志级别改变处理
     * @param index 选择的索引
     */
    void onLogLevelChanged(int index);
    
    /**
     * @brief 缓存大小改变处理
     * @param value 缓存大小
     */
    void onCacheSizeChanged(int value);
    
    /**
     * @brief 清除缓存按钮点击处理
     */
    void onClearCacheClicked();
    
    /**
     * @brief 实验性功能选项改变处理
     */
    void onExperimentalFeaturesChanged();
    
    /**
     * @brief 检查更新按钮点击处理
     */
    void onCheckUpdatesClicked();
    
    /**
     * @brief 访问网站按钮点击处理
     */
    void onVisitWebsiteClicked();
    
    /**
     * @brief 查看许可证按钮点击处理
     */
    void onViewLicenseClicked();

signals:
    /**
     * @brief 设置已应用信号
     */
    void settingsApplied();
    
    /**
     * @brief 语言改变信号
     * @param language 新语言
     */
    void languageChanged(const QString& language);
    
    /**
     * @brief 主题改变信号
     * @param theme 新主题
     */
    void themeChanged(const QString& theme);
    
    /**
     * @brief 需要重启应用程序信号
     * @param reason 重启原因
     */
    void restartRequired(const QString& reason);

private slots:
    /**
     * @brief 设置验证定时器超时处理
     */
    void onValidationTimeout();
    
    /**
     * @brief 缓存清理完成处理
     */
    void onCacheClearFinished();

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();
    
    /**
     * @brief 创建常规设置页面
     * @return 常规设置页面部件
     */
    QWidget* createGeneralPage();
    
    /**
     * @brief 创建服务器设置页面
     * @return 服务器设置页面部件
     */
    QWidget* createServerPage();
    
    /**
     * @brief 创建音视频设置页面
     * @return 音视频设置页面部件
     */
    QWidget* createAudioVideoPage();
    
    /**
     * @brief 创建界面设置页面
     * @return 界面设置页面部件
     */
    QWidget* createInterfacePage();
    
    /**
     * @brief 创建高级设置页面
     * @return 高级设置页面部件
     */
    QWidget* createAdvancedPage();
    
    /**
     * @brief 创建关于页面
     * @return 关于页面部件
     */
    QWidget* createAboutPage();
    
    /**
     * @brief 初始化连接
     */
    void initializeConnections();
    
    /**
     * @brief 加载设置
     */
    void loadSettings();
    
    /**
     * @brief 保存设置
     */
    void saveSettings();
    
    /**
     * @brief 验证设置
     * @return 设置是否有效
     */
    bool validateSettings();
    
    /**
     * @brief 检查是否有更改
     * @return 是否有更改
     */
    bool hasChanges() const;
    
    /**
     * @brief 更新UI状态
     */
    void updateUIState();
    
    /**
     * @brief 更新应用按钮状态
     */
    void updateApplyButtonState();
    
    /**
     * @brief 加载可用语言
     */
    void loadAvailableLanguages();
    
    /**
     * @brief 加载可用主题
     */
    void loadAvailableThemes();
    
    /**
     * @brief 加载音视频设备
     */
    void loadAudioVideoDevices();
    
    /**
     * @brief 获取缓存大小
     * @return 缓存大小（MB）
     */
    int getCacheSize() const;
    
    /**
     * @brief 清除应用程序缓存
     */
    void clearApplicationCache();
    
    /**
     * @brief 检查应用程序更新
     */
    void checkForUpdates();

private:
    // 主要组件
    QTabWidget* m_tabWidget;            ///< 标签页部件
    QVBoxLayout* m_mainLayout;          ///< 主布局
    QHBoxLayout* m_buttonLayout;        ///< 按钮布局
    
    // 按钮
    QPushButton* m_okButton;            ///< 确定按钮
    QPushButton* m_cancelButton;        ///< 取消按钮
    QPushButton* m_applyButton;         ///< 应用按钮
    QPushButton* m_resetButton;         ///< 重置按钮
    QPushButton* m_importButton;        ///< 导入按钮
    QPushButton* m_exportButton;        ///< 导出按钮
    
    // 常规设置页面
    QWidget* m_generalPage;             ///< 常规设置页面
    QComboBox* m_languageCombo;         ///< 语言选择框
    QComboBox* m_themeCombo;            ///< 主题选择框
    QCheckBox* m_startupCheck;          ///< 开机自启动复选框
    QCheckBox* m_systemTrayCheck;       ///< 系统托盘复选框
    QCheckBox* m_minimizeToTrayCheck;   ///< 最小化到托盘复选框
    QLineEdit* m_defaultNameEdit;       ///< 默认显示名称输入框
    
    // 服务器设置页面
    QWidget* m_serverPage;              ///< 服务器设置页面
    QLineEdit* m_defaultServerEdit;     ///< 默认服务器输入框
    QSpinBox* m_connectionTimeoutSpin;  ///< 连接超时输入框
    QCheckBox* m_proxyEnabledCheck;     ///< 代理启用复选框
    QLineEdit* m_proxyHostEdit;         ///< 代理主机输入框
    QSpinBox* m_proxyPortSpin;          ///< 代理端口输入框
    QLineEdit* m_proxyUserEdit;         ///< 代理用户名输入框
    QLineEdit* m_proxyPasswordEdit;     ///< 代理密码输入框
    
    // 音视频设置页面
    QWidget* m_audioVideoPage;          ///< 音视频设置页面
    QComboBox* m_audioDeviceCombo;      ///< 音频设备选择框
    QComboBox* m_videoDeviceCombo;      ///< 视频设备选择框
    QComboBox* m_audioQualityCombo;     ///< 音频质量选择框
    QComboBox* m_videoQualityCombo;     ///< 视频质量选择框
    QCheckBox* m_defaultMuteCheck;      ///< 默认静音复选框
    QCheckBox* m_defaultCameraOffCheck; ///< 默认关闭摄像头复选框
    QSlider* m_micVolumeSlider;         ///< 麦克风音量滑块
    QSlider* m_speakerVolumeSlider;     ///< 扬声器音量滑块
    
    // 界面设置页面
    QWidget* m_interfacePage;           ///< 界面设置页面
    QPushButton* m_fontSelectButton;    ///< 字体选择按钮
    QPushButton* m_colorSelectButton;   ///< 颜色选择按钮
    QSpinBox* m_windowWidthSpin;        ///< 窗口宽度输入框
    QSpinBox* m_windowHeightSpin;       ///< 窗口高度输入框
    QCheckBox* m_rememberSizeCheck;     ///< 记住窗口大小复选框
    QCheckBox* m_rememberPositionCheck; ///< 记住窗口位置复选框
    
    // 高级设置页面
    QWidget* m_advancedPage;            ///< 高级设置页面
    QComboBox* m_logLevelCombo;         ///< 日志级别选择框
    QSpinBox* m_cacheSizeSpin;          ///< 缓存大小输入框
    QPushButton* m_clearCacheButton;    ///< 清除缓存按钮
    QCheckBox* m_hardwareAccelCheck;    ///< 硬件加速复选框
    QCheckBox* m_experimentalFeaturesCheck; ///< 实验性功能复选框
    QCheckBox* m_debugModeCheck;        ///< 调试模式复选框
    
    // 关于页面
    QWidget* m_aboutPage;               ///< 关于页面
    QLabel* m_versionLabel;             ///< 版本标签
    QLabel* m_buildLabel;               ///< 构建信息标签
    QLabel* m_qtVersionLabel;           ///< Qt版本标签
    QPushButton* m_checkUpdatesButton;  ///< 检查更新按钮
    QPushButton* m_visitWebsiteButton;  ///< 访问网站按钮
    QPushButton* m_viewLicenseButton;   ///< 查看许可证按钮
    QTextEdit* m_creditsText;           ///< 致谢文本
    
    // 状态组件
    QProgressBar* m_progressBar;        ///< 进度条
    QLabel* m_statusLabel;              ///< 状态标签
    
    // 定时器
    QTimer* m_validationTimer;          ///< 验证定时器
    
    // 管理器
    ConfigurationManager* m_configManager; ///< 配置管理器
    
    // 状态变量
    bool m_hasChanges;                  ///< 是否有更改
    bool m_isLoading;                   ///< 是否正在加载
    QJsonObject m_originalSettings;     ///< 原始设置
    QJsonObject m_currentSettings;      ///< 当前设置
    
    // 常量
    static const int VALIDATION_DELAY = 500;    ///< 验证延迟（毫秒）
    static const int MIN_CACHE_SIZE = 10;       ///< 最小缓存大小（MB）
    static const int MAX_CACHE_SIZE = 1000;     ///< 最大缓存大小（MB）
    static const int DEFAULT_CACHE_SIZE = 100;  ///< 默认缓存大小（MB）
};

#endif // SETTINGSDIALOG_H
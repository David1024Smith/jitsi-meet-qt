#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QSpinBox>
#include <QSlider>
#include <QFrame>
#include <QScrollArea>
#include <QWidget>

QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QLabel;
class QLineEdit;
class QPushButton;
class QCheckBox;
class QSpinBox;
class QSlider;
class QFrame;
class QScrollArea;
class QWidget;
QT_END_NAMESPACE

class ConfigurationManager;

/**
 * @brief 设置对话框类 - 应用程序配置界面
 * 
 * 根据用户提供的图片重新设计的设置界面
 */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit SettingsDialog(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~SettingsDialog();

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
     * @brief 服务器URL改变处理
     * @param text 服务器URL
     */
    void onServerUrlChanged(const QString& text);
    
    /**
     * @brief 服务器超时改变处理
     * @param value 超时值
     */
    void onServerTimeoutChanged(int value);
    
    /**
     * @brief 始终置顶选项改变处理
     * @param checked 是否选中
     */
    void onAlwaysOnTopChanged(bool checked);
    
    /**
     * @brief 禁用自动增益控制选项改变处理
     * @param checked 是否选中
     */
    void onDisableAGCChanged(bool checked);

signals:
    /**
     * @brief 设置已应用信号
     */
    void settingsApplied();

private:
    /**
     * @brief 初始化用户界面
     */
    void initializeUI();
    
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
     * @brief 创建标题栏
     * @return 标题栏部件
     */
    QWidget* createTitleBar();
    
    /**
     * @brief 创建内容区域
     * @return 内容区域部件
     */
    QWidget* createContentArea();
    
    /**
     * @brief 创建按钮区域
     * @return 按钮区域部件
     */
    QWidget* createButtonArea();

private:
    // 主布局
    QVBoxLayout* m_mainLayout;          ///< 主布局
    
    // 标题栏
    QWidget* m_titleBar;                ///< 标题栏
    QLabel* m_titleLabel;               ///< 标题标签
    QPushButton* m_closeButton;         ///< 关闭按钮
    
    // 内容区域
    QScrollArea* m_scrollArea;          ///< 滚动区域
    QWidget* m_contentWidget;           ///< 内容部件
    QVBoxLayout* m_contentLayout;       ///< 内容布局
    
    // 服务器设置
    QFrame* m_serverFrame;              ///< 服务器设置框架
    QLabel* m_serverUrlLabel;           ///< 服务器网址标签
    QLineEdit* m_serverUrlEdit;         ///< 服务器网址输入框
    QLabel* m_serverTimeoutLabel;       ///< 服务器超时标签
    QSpinBox* m_serverTimeoutSpin;      ///< 服务器超时输入框
    
    // 开关设置
    QFrame* m_switchFrame;              ///< 开关设置框架
    QCheckBox* m_alwaysOnTopCheck;      ///< 始终置顶复选框
    QCheckBox* m_disableAGCCheck;       ///< 禁用自动增益控制复选框
    
    // 按钮区域
    QWidget* m_buttonArea;              ///< 按钮区域
    QHBoxLayout* m_buttonLayout;        ///< 按钮布局
    QPushButton* m_okButton;            ///< 确定按钮
    QPushButton* m_cancelButton;        ///< 取消按钮
    
    // 配置管理器
    ConfigurationManager* m_configManager; ///< 配置管理器
};

#endif // SETTINGSDIALOG_H
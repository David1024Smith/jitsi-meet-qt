#ifndef CONNECTIONWIDGET_H
#define CONNECTIONWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QGroupBox>
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>

// 前向声明
class INetworkManager;
class NetworkConfig;

/**
 * @brief 连接控制组件
 * 
 * ConnectionWidget提供网络连接的控制界面，包括服务器配置、
 * 连接参数设置、连接控制按钮等功能。
 */
class ConnectionWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(bool autoConnect READ autoConnect WRITE setAutoConnect NOTIFY autoConnectChanged)

public:
    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit ConnectionWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~ConnectionWidget();

    /**
     * @brief 设置网络管理器
     * @param manager 网络管理器指针
     */
    void setNetworkManager(INetworkManager* manager);

    /**
     * @brief 获取网络管理器
     * @return 网络管理器指针
     */
    INetworkManager* networkManager() const;

    /**
     * @brief 设置网络配置
     * @param config 网络配置对象
     */
    void setNetworkConfig(NetworkConfig* config);

    /**
     * @brief 获取网络配置
     * @return 网络配置对象
     */
    NetworkConfig* networkConfig() const;

    /**
     * @brief 设置是否只读模式
     * @param readOnly 是否只读
     */
    void setReadOnly(bool readOnly);

    /**
     * @brief 获取是否只读模式
     * @return 是否只读
     */
    bool isReadOnly() const;

    /**
     * @brief 设置是否自动连接
     * @param enabled 是否启用自动连接
     */
    void setAutoConnect(bool enabled);

    /**
     * @brief 获取是否自动连接
     * @return 是否启用自动连接
     */
    bool autoConnect() const;

    /**
     * @brief 获取当前服务器URL
     * @return 服务器URL
     */
    QString serverUrl() const;

    /**
     * @brief 设置服务器URL
     * @param url 服务器URL
     */
    void setServerUrl(const QString& url);

    /**
     * @brief 获取当前服务器端口
     * @return 服务器端口
     */
    int serverPort() const;

    /**
     * @brief 设置服务器端口
     * @param port 服务器端口
     */
    void setServerPort(int port);

public slots:
    /**
     * @brief 连接到服务器
     */
    void connectToServer();

    /**
     * @brief 断开连接
     */
    void disconnectFromServer();

    /**
     * @brief 重新连接
     */
    void reconnect();

    /**
     * @brief 测试连接
     */
    void testConnection();

    /**
     * @brief 应用配置更改
     */
    void applyConfiguration();

    /**
     * @brief 重置配置
     */
    void resetConfiguration();

    /**
     * @brief 加载预设配置
     * @param presetName 预设名称
     */
    void loadPreset(const QString& presetName);

    /**
     * @brief 保存当前配置为预设
     * @param presetName 预设名称
     */
    void savePreset(const QString& presetName);

signals:
    /**
     * @brief 连接请求信号
     * @param serverUrl 服务器URL
     */
    void connectionRequested(const QString& serverUrl);

    /**
     * @brief 断开连接请求信号
     */
    void disconnectionRequested();

    /**
     * @brief 配置改变信号
     */
    void configurationChanged();

    /**
     * @brief 只读模式改变信号
     * @param readOnly 是否只读
     */
    void readOnlyChanged(bool readOnly);

    /**
     * @brief 自动连接设置改变信号
     * @param enabled 是否启用
     */
    void autoConnectChanged(bool enabled);

    /**
     * @brief 连接测试完成信号
     * @param success 测试是否成功
     * @param latency 延迟时间（毫秒）
     */
    void connectionTestCompleted(bool success, int latency);

protected:
    /**
     * @brief 显示事件
     * @param event 显示事件
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief 隐藏事件
     * @param event 隐藏事件
     */
    void hideEvent(QHideEvent* event) override;

private slots:
    /**
     * @brief 处理连接按钮点击
     */
    void handleConnectButtonClicked();

    /**
     * @brief 处理断开按钮点击
     */
    void handleDisconnectButtonClicked();

    /**
     * @brief 处理测试按钮点击
     */
    void handleTestButtonClicked();

    /**
     * @brief 处理服务器URL改变
     */
    void handleServerUrlChanged();

    /**
     * @brief 处理服务器端口改变
     */
    void handleServerPortChanged();

    /**
     * @brief 处理连接状态改变
     * @param state 新的连接状态
     */
    void handleConnectionStateChanged(int state);

    /**
     * @brief 处理配置改变
     */
    void handleConfigurationChanged();

    /**
     * @brief 处理预设选择改变
     * @param presetName 预设名称
     */
    void handlePresetChanged(const QString& presetName);

    /**
     * @brief 处理连接测试结果
     * @param success 测试是否成功
     * @param latency 延迟时间
     */
    void handleConnectionTestResult(bool success, int latency);

private:
    /**
     * @brief 初始化UI
     */
    void initializeUI();

    /**
     * @brief 创建服务器配置组
     * @return 服务器配置组件
     */
    QGroupBox* createServerConfigGroup();

    /**
     * @brief 创建连接选项组
     * @return 连接选项组件
     */
    QGroupBox* createConnectionOptionsGroup();

    /**
     * @brief 创建控制按钮组
     * @return 控制按钮组件
     */
    QWidget* createControlButtonsGroup();

    /**
     * @brief 创建状态显示组
     * @return 状态显示组件
     */
    QWidget* createStatusGroup();

    /**
     * @brief 更新UI状态
     */
    void updateUIState();

    /**
     * @brief 更新连接按钮状态
     * @param state 连接状态
     */
    void updateConnectionButtons(int state);

    /**
     * @brief 验证输入
     * @return 输入是否有效
     */
    bool validateInput();

    /**
     * @brief 从UI更新配置
     */
    void updateConfigFromUI();

    /**
     * @brief 从配置更新UI
     */
    void updateUIFromConfig();

    /**
     * @brief 加载预设列表
     */
    void loadPresetList();

    /**
     * @brief 保存预设列表
     */
    void savePresetList();

    /**
     * @brief 应用样式
     */
    void applyStyles();

    /**
     * @brief 设置工具提示
     */
    void setupTooltips();

    class Private;
    Private* d;
};

#endif // CONNECTIONWIDGET_H
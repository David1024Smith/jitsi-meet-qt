#ifndef JOINDIALOG_H
#define JOINDIALOG_H

#include <QDialog>
#include <QString>
#include <QVariantMap>
#include <memory>

class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QProgressBar;
class QTextEdit;

/**
 * @brief 加入会议对话框类
 * 
 * 提供用户加入会议的界面，包括会议链接输入、用户信息设置和音视频选项
 */
class JoinDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit JoinDialog(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~JoinDialog();

    /**
     * @brief 设置会议URL
     * @param url 会议链接
     */
    void setMeetingUrl(const QString& url);

    /**
     * @brief 获取会议URL
     * @return 会议链接
     */
    QString getMeetingUrl() const;

    /**
     * @brief 设置显示名称
     * @param name 显示名称
     */
    void setDisplayName(const QString& name);

    /**
     * @brief 获取显示名称
     * @return 显示名称
     */
    QString getDisplayName() const;

    /**
     * @brief 设置邮箱地址
     * @param email 邮箱地址
     */
    void setEmail(const QString& email);

    /**
     * @brief 获取邮箱地址
     * @return 邮箱地址
     */
    QString getEmail() const;

    /**
     * @brief 设置音频启用状态
     * @param enabled 是否启用
     */
    void setAudioEnabled(bool enabled);

    /**
     * @brief 获取音频启用状态
     * @return 是否启用
     */
    bool isAudioEnabled() const;

    /**
     * @brief 设置视频启用状态
     * @param enabled 是否启用
     */
    void setVideoEnabled(bool enabled);

    /**
     * @brief 获取视频启用状态
     * @return 是否启用
     */
    bool isVideoEnabled() const;

    /**
     * @brief 设置会议密码
     * @param password 会议密码
     */
    void setMeetingPassword(const QString& password);

    /**
     * @brief 获取会议密码
     * @return 会议密码
     */
    QString getMeetingPassword() const;

    /**
     * @brief 设置是否记住设置
     * @param remember 是否记住
     */
    void setRememberSettings(bool remember);

    /**
     * @brief 获取是否记住设置
     * @return 是否记住
     */
    bool rememberSettings() const;

    /**
     * @brief 获取加入设置
     * @return 设置映射
     */
    QVariantMap getJoinSettings() const;

    /**
     * @brief 设置加入设置
     * @param settings 设置映射
     */
    void setJoinSettings(const QVariantMap& settings);

    /**
     * @brief 设置是否显示高级选项
     * @param show 是否显示
     */
    void setShowAdvancedOptions(bool show);

    /**
     * @brief 获取是否显示高级选项
     * @return 是否显示
     */
    bool showAdvancedOptions() const;

    /**
     * @brief 验证输入
     * @return 验证是否通过
     */
    bool validateInput();

    /**
     * @brief 获取验证错误
     * @return 错误列表
     */
    QStringList getValidationErrors() const;

    /**
     * @brief 加载保存的设置
     */
    void loadSavedSettings();

    /**
     * @brief 保存当前设置
     */
    void saveCurrentSettings();

    /**
     * @brief 重置为默认设置
     */
    void resetToDefaults();

public slots:
    /**
     * @brief 显示加载状态
     * @param message 加载消息
     */
    void showLoading(const QString& message = QString());

    /**
     * @brief 隐藏加载状态
     */
    void hideLoading();

    /**
     * @brief 显示错误信息
     * @param error 错误信息
     */
    void showError(const QString& error);

    /**
     * @brief 清除错误信息
     */
    void clearError();

    /**
     * @brief 设置会议信息
     * @param meetingInfo 会议信息映射
     */
    void setMeetingInfo(const QVariantMap& meetingInfo);

    /**
     * @brief 预填充用户信息
     * @param userInfo 用户信息映射
     */
    void prefillUserInfo(const QVariantMap& userInfo);

protected:
    /**
     * @brief 接受对话框
     */
    void accept() override;

    /**
     * @brief 拒绝对话框
     */
    void reject() override;

    /**
     * @brief 显示事件
     * @param event 显示事件
     */
    void showEvent(QShowEvent* event) override;

    /**
     * @brief 关闭事件
     * @param event 关闭事件
     */
    void closeEvent(QCloseEvent* event) override;

private slots:
    /**
     * @brief 处理URL改变
     * @param url 新URL
     */
    void handleUrlChanged(const QString& url);

    /**
     * @brief 处理显示名称改变
     * @param name 新显示名称
     */
    void handleDisplayNameChanged(const QString& name);

    /**
     * @brief 处理音频状态改变
     * @param enabled 新状态
     */
    void handleAudioToggled(bool enabled);

    /**
     * @brief 处理视频状态改变
     * @param enabled 新状态
     */
    void handleVideoToggled(bool enabled);

    /**
     * @brief 处理高级选项切换
     * @param show 是否显示
     */
    void handleAdvancedOptionsToggled(bool show);

    /**
     * @brief 处理测试音频按钮点击
     */
    void handleTestAudioClicked();

    /**
     * @brief 处理测试视频按钮点击
     */
    void handleTestVideoClicked();

    /**
     * @brief 处理从剪贴板粘贴
     */
    void handlePasteFromClipboard();

    /**
     * @brief 处理URL验证完成
     * @param url 验证的URL
     * @param valid 是否有效
     */
    void handleUrlValidated(const QString& url, bool valid);

    /**
     * @brief 处理会议信息获取完成
     * @param meetingInfo 会议信息
     */
    void handleMeetingInfoReceived(const QVariantMap& meetingInfo);

signals:
    /**
     * @brief 加入会议信号
     * @param url 会议链接
     * @param displayName 显示名称
     * @param audioEnabled 音频状态
     * @param videoEnabled 视频状态
     * @param settings 其他设置
     */
    void joinMeeting(const QString& url, 
                    const QString& displayName,
                    bool audioEnabled, 
                    bool videoEnabled,
                    const QVariantMap& settings);

    /**
     * @brief URL验证请求信号
     * @param url 需要验证的URL
     */
    void validateUrlRequested(const QString& url);

    /**
     * @brief 获取会议信息请求信号
     * @param url 会议链接
     */
    void getMeetingInfoRequested(const QString& url);

    /**
     * @brief 测试音频请求信号
     */
    void testAudioRequested();

    /**
     * @brief 测试视频请求信号
     */
    void testVideoRequested();

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建基本信息区域
     */
    void createBasicInfoArea();

    /**
     * @brief 创建音视频选项区域
     */
    void createMediaOptionsArea();

    /**
     * @brief 创建高级选项区域
     */
    void createAdvancedOptionsArea();

    /**
     * @brief 创建按钮区域
     */
    void createButtonArea();

    /**
     * @brief 设置布局
     */
    void setupLayout();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 应用样式
     */
    void applyStyles();

    /**
     * @brief 更新界面状态
     */
    void updateUIState();

    /**
     * @brief 验证URL格式
     * @param url URL字符串
     * @return 是否有效
     */
    bool validateUrlFormat(const QString& url) const;

    /**
     * @brief 验证显示名称
     * @param name 显示名称
     * @return 是否有效
     */
    bool validateDisplayName(const QString& name) const;

    /**
     * @brief 验证邮箱格式
     * @param email 邮箱地址
     * @return 是否有效
     */
    bool validateEmailFormat(const QString& email) const;

    /**
     * @brief 获取设置键
     * @param key 键名
     * @return 完整设置键
     */
    QString getSettingsKey(const QString& key) const;

    /**
     * @brief 设置输入焦点
     */
    void setInputFocus();

    /**
     * @brief 更新按钮状态
     */
    void updateButtonStates();

    /**
     * @brief 显示会议预览
     * @param meetingInfo 会议信息
     */
    void showMeetingPreview(const QVariantMap& meetingInfo);

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // JOINDIALOG_H
#ifndef CREATEDIALOG_H
#define CREATEDIALOG_H

#include <QDialog>
#include <QString>
#include <QVariantMap>
#include <QDateTime>
#include <memory>

class QVBoxLayout;
class QHBoxLayout;
class QFormLayout;
class QGridLayout;
class QLabel;
class QPushButton;
class QLineEdit;
class QTextEdit;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QDateTimeEdit;
class QGroupBox;
class QTabWidget;
class QSlider;
class QProgressBar;

/**
 * @brief 创建会议对话框类
 * 
 * 提供创建新会议的界面，包括会议基本信息、安全设置、音视频选项等
 */
class CreateDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 会议类型枚举
     */
    enum MeetingType {
        InstantMeeting,   ///< 即时会议
        ScheduledMeeting  ///< 预定会议
    };
    Q_ENUM(MeetingType)

    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit CreateDialog(QWidget* parent = nullptr);

    /**
     * @brief 析构函数
     */
    ~CreateDialog();

    /**
     * @brief 设置会议名称
     * @param name 会议名称
     */
    void setMeetingName(const QString& name);

    /**
     * @brief 获取会议名称
     * @return 会议名称
     */
    QString getMeetingName() const;

    /**
     * @brief 设置会议描述
     * @param description 会议描述
     */
    void setMeetingDescription(const QString& description);

    /**
     * @brief 获取会议描述
     * @return 会议描述
     */
    QString getMeetingDescription() const;

    /**
     * @brief 设置会议类型
     * @param type 会议类型
     */
    void setMeetingType(MeetingType type);

    /**
     * @brief 获取会议类型
     * @return 会议类型
     */
    MeetingType getMeetingType() const;

    /**
     * @brief 设置服务器地址
     * @param server 服务器地址
     */
    void setServer(const QString& server);

    /**
     * @brief 获取服务器地址
     * @return 服务器地址
     */
    QString getServer() const;

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
     * @brief 设置是否需要密码
     * @param required 是否需要密码
     */
    void setPasswordRequired(bool required);

    /**
     * @brief 获取是否需要密码
     * @return 是否需要密码
     */
    bool isPasswordRequired() const;

    /**
     * @brief 设置最大参与者数量
     * @param maxParticipants 最大参与者数量
     */
    void setMaxParticipants(int maxParticipants);

    /**
     * @brief 获取最大参与者数量
     * @return 最大参与者数量
     */
    int getMaxParticipants() const;

    /**
     * @brief 设置预定开始时间
     * @param startTime 开始时间
     */
    void setScheduledStartTime(const QDateTime& startTime);

    /**
     * @brief 获取预定开始时间
     * @return 开始时间
     */
    QDateTime getScheduledStartTime() const;

    /**
     * @brief 设置预定持续时间
     * @param duration 持续时间（分钟）
     */
    void setScheduledDuration(int duration);

    /**
     * @brief 获取预定持续时间
     * @return 持续时间（分钟）
     */
    int getScheduledDuration() const;

    /**
     * @brief 设置是否允许访客
     * @param allow 是否允许访客
     */
    void setAllowGuests(bool allow);

    /**
     * @brief 获取是否允许访客
     * @return 是否允许访客
     */
    bool getAllowGuests() const;

    /**
     * @brief 设置是否启用等候室
     * @param enabled 是否启用
     */
    void setWaitingRoomEnabled(bool enabled);

    /**
     * @brief 获取是否启用等候室
     * @return 是否启用
     */
    bool isWaitingRoomEnabled() const;

    /**
     * @brief 设置是否启用录制
     * @param enabled 是否启用
     */
    void setRecordingEnabled(bool enabled);

    /**
     * @brief 获取是否启用录制
     * @return 是否启用
     */
    bool isRecordingEnabled() const;

    /**
     * @brief 获取会议设置
     * @return 设置映射
     */
    QVariantMap getMeetingSettings() const;

    /**
     * @brief 设置会议设置
     * @param settings 设置映射
     */
    void setMeetingSettings(const QVariantMap& settings);

    /**
     * @brief 获取邀请列表
     * @return 邀请邮箱列表
     */
    QStringList getInviteList() const;

    /**
     * @brief 设置邀请列表
     * @param inviteList 邀请邮箱列表
     */
    void setInviteList(const QStringList& inviteList);

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
     * @brief 生成会议URL预览
     * @return 会议URL
     */
    QString generateMeetingUrlPreview() const;

    /**
     * @brief 加载模板设置
     * @param templateName 模板名称
     */
    void loadTemplate(const QString& templateName);

    /**
     * @brief 保存为模板
     * @param templateName 模板名称
     */
    void saveAsTemplate(const QString& templateName);

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
     * @brief 显示成功信息
     * @param message 成功信息
     */
    void showSuccess(const QString& message);

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

private slots:
    /**
     * @brief 处理会议名称改变
     * @param name 新会议名称
     */
    void handleMeetingNameChanged(const QString& name);

    /**
     * @brief 处理会议类型改变
     * @param type 新会议类型
     */
    void handleMeetingTypeChanged(int type);

    /**
     * @brief 处理服务器改变
     * @param server 新服务器
     */
    void handleServerChanged(const QString& server);

    /**
     * @brief 处理密码要求改变
     * @param required 是否需要密码
     */
    void handlePasswordRequiredChanged(bool required);

    /**
     * @brief 处理密码改变
     * @param password 新密码
     */
    void handlePasswordChanged(const QString& password);

    /**
     * @brief 处理生成随机密码
     */
    void handleGenerateRandomPassword();

    /**
     * @brief 处理预定时间改变
     * @param dateTime 新时间
     */
    void handleScheduledTimeChanged(const QDateTime& dateTime);

    /**
     * @brief 处理添加邀请
     */
    void handleAddInvite();

    /**
     * @brief 处理移除邀请
     */
    void handleRemoveInvite();

    /**
     * @brief 处理从通讯录导入
     */
    void handleImportFromContacts();

    /**
     * @brief 处理预览会议URL
     */
    void handlePreviewUrl();

    /**
     * @brief 处理复制会议URL
     */
    void handleCopyUrl();

    /**
     * @brief 处理高级设置切换
     * @param show 是否显示
     */
    void handleAdvancedSettingsToggled(bool show);

    /**
     * @brief 处理模板选择改变
     * @param templateName 模板名称
     */
    void handleTemplateChanged(const QString& templateName);

    /**
     * @brief 处理保存模板
     */
    void handleSaveTemplate();

signals:
    /**
     * @brief 创建会议信号
     * @param meetingName 会议名称
     * @param settings 会议设置
     */
    void createMeeting(const QString& meetingName, const QVariantMap& settings);

    /**
     * @brief 预定会议信号
     * @param meetingName 会议名称
     * @param startTime 开始时间
     * @param duration 持续时间
     * @param settings 会议设置
     */
    void scheduleMeeting(const QString& meetingName, 
                        const QDateTime& startTime,
                        int duration,
                        const QVariantMap& settings);

    /**
     * @brief 发送邀请信号
     * @param inviteList 邀请列表
     * @param meetingUrl 会议链接
     * @param message 邀请消息
     */
    void sendInvitations(const QStringList& inviteList, 
                        const QString& meetingUrl,
                        const QString& message);

    /**
     * @brief 验证服务器信号
     * @param server 服务器地址
     */
    void validateServer(const QString& server);

    /**
     * @brief 获取通讯录信号
     */
    void getContactsRequested();

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建基本信息标签页
     */
    void createBasicInfoTab();

    /**
     * @brief 创建安全设置标签页
     */
    void createSecurityTab();

    /**
     * @brief 创建音视频设置标签页
     */
    void createMediaTab();

    /**
     * @brief 创建邀请标签页
     */
    void createInviteTab();

    /**
     * @brief 创建高级设置标签页
     */
    void createAdvancedTab();

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
     * @brief 验证会议名称
     * @param name 会议名称
     * @return 是否有效
     */
    bool validateMeetingName(const QString& name) const;

    /**
     * @brief 验证服务器地址
     * @param server 服务器地址
     * @return 是否有效
     */
    bool validateServerAddress(const QString& server) const;

    /**
     * @brief 验证密码强度
     * @param password 密码
     * @return 强度等级 (0-3)
     */
    int validatePasswordStrength(const QString& password) const;

    /**
     * @brief 验证邮箱地址
     * @param email 邮箱地址
     * @return 是否有效
     */
    bool validateEmailAddress(const QString& email) const;

    /**
     * @brief 生成随机密码
     * @param length 密码长度
     * @return 随机密码
     */
    QString generateRandomPassword(int length = 8) const;

    /**
     * @brief 生成房间名称
     * @param meetingName 会议名称
     * @return 房间名称
     */
    QString generateRoomName(const QString& meetingName) const;

    /**
     * @brief 更新URL预览
     */
    void updateUrlPreview();

    /**
     * @brief 更新按钮状态
     */
    void updateButtonStates();

    /**
     * @brief 加载可用模板
     */
    void loadAvailableTemplates();

    /**
     * @brief 获取默认设置
     * @return 默认设置映射
     */
    QVariantMap getDefaultSettings() const;

private:
    class Private;
    std::unique_ptr<Private> d;
};

#endif // CREATEDIALOG_H
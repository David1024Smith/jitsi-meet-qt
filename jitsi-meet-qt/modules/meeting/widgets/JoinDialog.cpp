#include "JoinDialog.h"
#include "../include/MeetingManager.h"
#include "../config/MeetingConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QMessageBox>
#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QDebug>
#include <QShowEvent>
#include <QCloseEvent>
#include <QClipboard>
#include <QApplication>

class JoinDialog::Private
{
public:
    // 核心组件
    MeetingManager* meetingManager = nullptr;
    MeetingConfig* config = nullptr;
    
    // UI元素
    QLineEdit* urlEdit = nullptr;
    QLineEdit* displayNameEdit = nullptr;
    QLineEdit* emailEdit = nullptr;
    QLineEdit* passwordEdit = nullptr;
    QCheckBox* audioCheckBox = nullptr;
    QCheckBox* videoCheckBox = nullptr;
    QCheckBox* rememberCheckBox = nullptr;
    QCheckBox* advancedCheckBox = nullptr;
    QGroupBox* advancedGroup = nullptr;
    QProgressBar* progressBar = nullptr;
    QLabel* statusLabel = nullptr;
    QPushButton* joinButton = nullptr;
    QPushButton* testAudioButton = nullptr;
    QPushButton* testVideoButton = nullptr;
    QPushButton* pasteButton = nullptr;
    
    // 状态
    bool isJoining = false;
    QStringList validationErrors;
    
    // 验证器
    QRegularExpressionValidator* urlValidator = nullptr;
    QRegularExpressionValidator* nameValidator = nullptr;
    QRegularExpressionValidator* emailValidator = nullptr;
};

JoinDialog::JoinDialog(QWidget* parent)
    : QDialog(parent)
    , d(std::make_unique<Private>())
{
    setWindowTitle(tr("加入会议"));
    setModal(true);
    resize(450, 600);
    
    initializeUI();
    connectSignals();
    applyStyles();
    loadSavedSettings();
    updateUIState();
}

JoinDialog::~JoinDialog()
{
    // 析构函数实现
    if (d->isJoining) {
        // 如果正在加入会议，取消操作
        d->isJoining = false;
    }
    
    // 保存当前设置
    if (rememberSettings()) {
        saveCurrentSettings();
    }
    
    // 清理资源
    d.reset();
}

void JoinDialog::setMeetingUrl(const QString& url)
{
    if (d->urlEdit) {
        d->urlEdit->setText(url);
    }
}

QString JoinDialog::getMeetingUrl() const
{
    return d->urlEdit ? d->urlEdit->text().trimmed() : QString();
}

void JoinDialog::setDisplayName(const QString& name)
{
    if (d->displayNameEdit) {
        d->displayNameEdit->setText(name);
    }
}

QString JoinDialog::getDisplayName() const
{
    return d->displayNameEdit ? d->displayNameEdit->text().trimmed() : QString();
}

void JoinDialog::setEmail(const QString& email)
{
    if (d->emailEdit) {
        d->emailEdit->setText(email);
    }
}

QString JoinDialog::getEmail() const
{
    return d->emailEdit ? d->emailEdit->text().trimmed() : QString();
}

void JoinDialog::setAudioEnabled(bool enabled)
{
    if (d->audioCheckBox) {
        d->audioCheckBox->setChecked(enabled);
    }
}

bool JoinDialog::isAudioEnabled() const
{
    return d->audioCheckBox ? d->audioCheckBox->isChecked() : true;
}

void JoinDialog::setVideoEnabled(bool enabled)
{
    if (d->videoCheckBox) {
        d->videoCheckBox->setChecked(enabled);
    }
}

bool JoinDialog::isVideoEnabled() const
{
    return d->videoCheckBox ? d->videoCheckBox->isChecked() : true;
}

void JoinDialog::setMeetingPassword(const QString& password)
{
    if (d->passwordEdit) {
        d->passwordEdit->setText(password);
    }
}

QString JoinDialog::getMeetingPassword() const
{
    return d->passwordEdit ? d->passwordEdit->text() : QString();
}

void JoinDialog::setRememberSettings(bool remember)
{
    if (d->rememberCheckBox) {
        d->rememberCheckBox->setChecked(remember);
    }
}

bool JoinDialog::rememberSettings() const
{
    return d->rememberCheckBox ? d->rememberCheckBox->isChecked() : false;
}

QVariantMap JoinDialog::getJoinSettings() const
{
    QVariantMap settings;
    settings["url"] = getMeetingUrl();
    settings["displayName"] = getDisplayName();
    settings["email"] = getEmail();
    settings["password"] = getMeetingPassword();
    settings["audioEnabled"] = isAudioEnabled();
    settings["videoEnabled"] = isVideoEnabled();
    settings["rememberSettings"] = rememberSettings();
    settings["showAdvancedOptions"] = showAdvancedOptions();
    return settings;
}

void JoinDialog::setJoinSettings(const QVariantMap& settings)
{
    setMeetingUrl(settings.value("url").toString());
    setDisplayName(settings.value("displayName").toString());
    setEmail(settings.value("email").toString());
    setMeetingPassword(settings.value("password").toString());
    setAudioEnabled(settings.value("audioEnabled", true).toBool());
    setVideoEnabled(settings.value("videoEnabled", true).toBool());
    setRememberSettings(settings.value("rememberSettings", false).toBool());
    setShowAdvancedOptions(settings.value("showAdvancedOptions", false).toBool());
}

void JoinDialog::setShowAdvancedOptions(bool show)
{
    if (d->advancedCheckBox) {
        d->advancedCheckBox->setChecked(show);
    }
    if (d->advancedGroup) {
        d->advancedGroup->setVisible(show);
    }
}

bool JoinDialog::showAdvancedOptions() const
{
    return d->advancedCheckBox ? d->advancedCheckBox->isChecked() : false;
}

bool JoinDialog::validateInput()
{
    d->validationErrors.clear();
    
    // 验证URL
    QString url = getMeetingUrl();
    if (url.isEmpty()) {
        d->validationErrors << tr("请输入会议链接");
    } else if (!validateUrlFormat(url)) {
        d->validationErrors << tr("会议链接格式不正确");
    }
    
    // 验证显示名称
    QString name = getDisplayName();
    if (name.isEmpty()) {
        d->validationErrors << tr("请输入显示名称");
    } else if (!validateDisplayName(name)) {
        d->validationErrors << tr("显示名称格式不正确");
    }
    
    // 验证邮箱（如果提供）
    QString email = getEmail();
    if (!email.isEmpty() && !validateEmailFormat(email)) {
        d->validationErrors << tr("邮箱格式不正确");
    }
    
    updateButtonStates();
    return d->validationErrors.isEmpty();
}

QStringList JoinDialog::getValidationErrors() const
{
    return d->validationErrors;
}

void JoinDialog::loadSavedSettings()
{
    // 加载保存的设置
    qDebug() << "Loading saved settings";
}

void JoinDialog::saveCurrentSettings()
{
    // 保存当前设置
    qDebug() << "Saving current settings";
}

void JoinDialog::resetToDefaults()
{
    setMeetingUrl("");
    setDisplayName("");
    setEmail("");
    setMeetingPassword("");
    setAudioEnabled(true);
    setVideoEnabled(true);
    setRememberSettings(false);
    setShowAdvancedOptions(false);
}

void JoinDialog::showLoading(const QString& message)
{
    Q_UNUSED(message)
    if (d->progressBar) {
        d->progressBar->setVisible(true);
    }
    if (d->statusLabel) {
        d->statusLabel->setText(message.isEmpty() ? tr("正在加入会议...") : message);
        d->statusLabel->setVisible(true);
    }
    d->isJoining = true;
    updateButtonStates();
}

void JoinDialog::hideLoading()
{
    if (d->progressBar) {
        d->progressBar->setVisible(false);
    }
    if (d->statusLabel) {
        d->statusLabel->setVisible(false);
    }
    d->isJoining = false;
    updateButtonStates();
}

void JoinDialog::showError(const QString& error)
{
    if (d->statusLabel) {
        d->statusLabel->setText(error);
        d->statusLabel->setStyleSheet("color: red;");
        d->statusLabel->setVisible(true);
    }
    QMessageBox::warning(this, tr("错误"), error);
}

void JoinDialog::clearError()
{
    if (d->statusLabel) {
        d->statusLabel->setVisible(false);
        d->statusLabel->setStyleSheet("");
    }
}

void JoinDialog::setMeetingInfo(const QVariantMap& meetingInfo)
{
    Q_UNUSED(meetingInfo)
    // 设置会议信息
    qDebug() << "Setting meeting info:" << meetingInfo;
}

void JoinDialog::prefillUserInfo(const QVariantMap& userInfo)
{
    if (userInfo.contains("displayName")) {
        setDisplayName(userInfo.value("displayName").toString());
    }
    if (userInfo.contains("email")) {
        setEmail(userInfo.value("email").toString());
    }
}

void JoinDialog::accept()
{
    if (validateInput()) {
        emit joinMeeting(getMeetingUrl(), getDisplayName(), 
                        isAudioEnabled(), isVideoEnabled(), 
                        getJoinSettings());
        QDialog::accept();
    } else {
        showError(d->validationErrors.join("\n"));
    }
}

void JoinDialog::reject()
{
    if (d->isJoining) {
        // 如果正在加入，询问是否确认取消
        int ret = QMessageBox::question(this, tr("确认"), 
                                       tr("正在加入会议，确定要取消吗？"),
                                       QMessageBox::Yes | QMessageBox::No);
        if (ret != QMessageBox::Yes) {
            return;
        }
    }
    QDialog::reject();
}

void JoinDialog::showEvent(QShowEvent* event)
{
    QDialog::showEvent(event);
    setInputFocus();
}

void JoinDialog::closeEvent(QCloseEvent* event)
{
    if (d->isJoining) {
        event->ignore();
        reject();
    } else {
        QDialog::closeEvent(event);
    }
}

void JoinDialog::handleUrlChanged(const QString& url)
{
    Q_UNUSED(url)
    validateInput();
    emit validateUrlRequested(url);
}

void JoinDialog::handleDisplayNameChanged(const QString& name)
{
    Q_UNUSED(name)
    validateInput();
}

void JoinDialog::handleAudioToggled(bool enabled)
{
    Q_UNUSED(enabled)
    qDebug() << "Audio toggled:" << enabled;
}

void JoinDialog::handleVideoToggled(bool enabled)
{
    Q_UNUSED(enabled)
    qDebug() << "Video toggled:" << enabled;
}

void JoinDialog::handleAdvancedOptionsToggled(bool show)
{
    setShowAdvancedOptions(show);
}

void JoinDialog::handleTestAudioClicked()
{
    emit testAudioRequested();
}

void JoinDialog::handleTestVideoClicked()
{
    emit testVideoRequested();
}

void JoinDialog::handlePasteFromClipboard()
{
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();
    if (!text.isEmpty() && validateUrlFormat(text)) {
        setMeetingUrl(text);
    }
}

void JoinDialog::handleUrlValidated(const QString& url, bool valid)
{
    Q_UNUSED(url)
    Q_UNUSED(valid)
    qDebug() << "URL validation result:" << url << valid;
}

void JoinDialog::handleMeetingInfoReceived(const QVariantMap& meetingInfo)
{
    setMeetingInfo(meetingInfo);
    showMeetingPreview(meetingInfo);
}

void JoinDialog::initializeUI()
{
    createBasicInfoArea();
    createMediaOptionsArea();
    createAdvancedOptionsArea();
    createButtonArea();
    setupLayout();
}

void JoinDialog::createBasicInfoArea()
{
    // 创建基本信息区域的UI元素
    d->urlEdit = new QLineEdit(this);
    d->displayNameEdit = new QLineEdit(this);
    d->emailEdit = new QLineEdit(this);
    d->passwordEdit = new QLineEdit(this);
    d->passwordEdit->setEchoMode(QLineEdit::Password);
    
    d->pasteButton = new QPushButton(tr("粘贴"), this);
}

void JoinDialog::createMediaOptionsArea()
{
    // 创建音视频选项区域的UI元素
    d->audioCheckBox = new QCheckBox(tr("启用音频"), this);
    d->videoCheckBox = new QCheckBox(tr("启用视频"), this);
    d->audioCheckBox->setChecked(true);
    d->videoCheckBox->setChecked(true);
    
    d->testAudioButton = new QPushButton(tr("测试音频"), this);
    d->testVideoButton = new QPushButton(tr("测试视频"), this);
}

void JoinDialog::createAdvancedOptionsArea()
{
    // 创建高级选项区域
    d->advancedCheckBox = new QCheckBox(tr("显示高级选项"), this);
    d->advancedGroup = new QGroupBox(tr("高级选项"), this);
    d->advancedGroup->setVisible(false);
    
    d->rememberCheckBox = new QCheckBox(tr("记住设置"), this);
}

void JoinDialog::createButtonArea()
{
    // 创建按钮区域
    d->joinButton = new QPushButton(tr("加入会议"), this);
    QPushButton* cancelButton = new QPushButton(tr("取消"), this);
    
    d->progressBar = new QProgressBar(this);
    d->progressBar->setVisible(false);
    d->progressBar->setRange(0, 0); // 无限进度条
    
    d->statusLabel = new QLabel(this);
    d->statusLabel->setVisible(false);
    
    connect(d->joinButton, &QPushButton::clicked, this, &JoinDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &JoinDialog::reject);
}

void JoinDialog::setupLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 基本信息表单
    QFormLayout* formLayout = new QFormLayout();
    formLayout->addRow(tr("会议链接:"), d->urlEdit);
    formLayout->addRow("", d->pasteButton);
    formLayout->addRow(tr("显示名称:"), d->displayNameEdit);
    formLayout->addRow(tr("邮箱:"), d->emailEdit);
    formLayout->addRow(tr("会议密码:"), d->passwordEdit);
    
    mainLayout->addLayout(formLayout);
    
    // 音视频选项
    QGroupBox* mediaGroup = new QGroupBox(tr("音视频选项"), this);
    QVBoxLayout* mediaLayout = new QVBoxLayout(mediaGroup);
    mediaLayout->addWidget(d->audioCheckBox);
    mediaLayout->addWidget(d->videoCheckBox);
    
    QHBoxLayout* testLayout = new QHBoxLayout();
    testLayout->addWidget(d->testAudioButton);
    testLayout->addWidget(d->testVideoButton);
    mediaLayout->addLayout(testLayout);
    
    mainLayout->addWidget(mediaGroup);
    
    // 高级选项
    mainLayout->addWidget(d->advancedCheckBox);
    
    QVBoxLayout* advancedLayout = new QVBoxLayout(d->advancedGroup);
    advancedLayout->addWidget(d->rememberCheckBox);
    
    mainLayout->addWidget(d->advancedGroup);
    
    // 状态和按钮
    mainLayout->addWidget(d->progressBar);
    mainLayout->addWidget(d->statusLabel);
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    buttonLayout->addWidget(d->joinButton);
    QPushButton* cancelButton = new QPushButton(tr("取消"), this);
    buttonLayout->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, &JoinDialog::reject);
    
    mainLayout->addLayout(buttonLayout);
}

void JoinDialog::connectSignals()
{
    connect(d->urlEdit, &QLineEdit::textChanged, this, &JoinDialog::handleUrlChanged);
    connect(d->displayNameEdit, &QLineEdit::textChanged, this, &JoinDialog::handleDisplayNameChanged);
    connect(d->audioCheckBox, &QCheckBox::toggled, this, &JoinDialog::handleAudioToggled);
    connect(d->videoCheckBox, &QCheckBox::toggled, this, &JoinDialog::handleVideoToggled);
    connect(d->advancedCheckBox, &QCheckBox::toggled, this, &JoinDialog::handleAdvancedOptionsToggled);
    connect(d->testAudioButton, &QPushButton::clicked, this, &JoinDialog::handleTestAudioClicked);
    connect(d->testVideoButton, &QPushButton::clicked, this, &JoinDialog::handleTestVideoClicked);
    connect(d->pasteButton, &QPushButton::clicked, this, &JoinDialog::handlePasteFromClipboard);
}

void JoinDialog::applyStyles()
{
    // 应用样式
    setStyleSheet(
        "QGroupBox { font-weight: bold; }"
        "QLineEdit { padding: 5px; }"
        "QPushButton { padding: 8px 16px; }"
    );
}

void JoinDialog::updateUIState()
{
    updateButtonStates();
}

bool JoinDialog::validateUrlFormat(const QString& url) const
{
    // 简单的URL格式验证
    QRegularExpression urlRegex(R"(^https?://[\w\.-]+(/.*)?$)");
    return urlRegex.match(url).hasMatch();
}

bool JoinDialog::validateDisplayName(const QString& name) const
{
    // 显示名称验证：不能为空，长度限制
    return !name.trimmed().isEmpty() && name.length() <= 50;
}

bool JoinDialog::validateEmailFormat(const QString& email) const
{
    // 邮箱格式验证
    QRegularExpression emailRegex(R"(^[\w\.-]+@[\w\.-]+\.[a-zA-Z]{2,}$)");
    return emailRegex.match(email).hasMatch();
}

QString JoinDialog::getSettingsKey(const QString& key) const
{
    return QString("JoinDialog/%1").arg(key);
}

void JoinDialog::setInputFocus()
{
    if (d->urlEdit && d->urlEdit->text().isEmpty()) {
        d->urlEdit->setFocus();
    } else if (d->displayNameEdit && d->displayNameEdit->text().isEmpty()) {
        d->displayNameEdit->setFocus();
    }
}

void JoinDialog::updateButtonStates()
{
    bool canJoin = !d->isJoining && validateInput();
    if (d->joinButton) {
        d->joinButton->setEnabled(canJoin);
    }
}

void JoinDialog::showMeetingPreview(const QVariantMap& meetingInfo)
{
    Q_UNUSED(meetingInfo)
    // 显示会议预览信息
    qDebug() << "Showing meeting preview:" << meetingInfo;
}
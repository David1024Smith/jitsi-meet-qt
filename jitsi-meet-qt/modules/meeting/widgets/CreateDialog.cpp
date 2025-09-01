#include "CreateDialog.h"
#include "../include/MeetingManager.h"
#include "../config/MeetingConfig.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QGroupBox>
#include <QMessageBox>
#include <QValidator>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QTimer>
#include <QDebug>

class CreateDialog::Private
{
public:
    // 核心组件
    MeetingManager* meetingManager = nullptr;
    MeetingConfig* config = nullptr;
    
    // UI元素
    QLineEdit* meetingNameEdit = nullptr;
    QLineEdit* meetingSubjectEdit = nullptr;
    QComboBox* serverComboBox = nullptr;
    QCheckBox* passwordCheckBox = nullptr;
    QLineEdit* passwordEdit = nullptr;
    QCheckBox* startAudioMutedCheckBox = nullptr;
    QCheckBox* startVideoMutedCheckBox = nullptr;
    QCheckBox* enableRecordingCheckBox = nullptr;
    QCheckBox* enableLiveStreamingCheckBox = nullptr;
    QTextEdit* descriptionEdit = nullptr;
    
    // 状态
    bool isCreating = false;
    
    // 验证器
    QRegularExpressionValidator* nameValidator = nullptr;
};

CreateDialog::CreateDialog(QWidget* parent)
    : QDialog(parent)
    , d(std::make_unique<Private>())
{
    setWindowTitle(tr("Create New Meeting"));
    setMinimumWidth(400);
    
    initializeUI();
    setupLayout();
    connectSignals();
    loadSettings();
}

CreateDialog::~CreateDialog() = default;

void CreateDialog::setMeetingManager(MeetingManager* manager)
{
    d->meetingManager = manager;
}

void CreateDialog::setMeetingConfig(MeetingConfig* config)
{
    d->config = config;
    loadSettings();
}

void CreateDialog::initializeUI()
{
    // 创建验证器
    d->nameValidator = new QRegularExpressionValidator(QRegularExpression("[a-zA-Z0-9_-]+"), this);
    
    // 创建输入控件
    d->meetingNameEdit = new QLineEdit(this);
    d->meetingNameEdit->setPlaceholderText(tr("Enter meeting name"));
    d->meetingNameEdit->setValidator(d->nameValidator);
    
    d->meetingSubjectEdit = new QLineEdit(this);
    d->meetingSubjectEdit->setPlaceholderText(tr("Enter meeting subject"));
    
    d->serverComboBox = new QComboBox(this);
    d->serverComboBox->setEditable(true);
    d->serverComboBox->addItem("meet.jit.si");
    d->serverComboBox->addItem("8x8.vc");
    d->serverComboBox->addItem("jitsi.example.com");
    
    d->passwordCheckBox = new QCheckBox(tr("Require password"), this);
    d->passwordEdit = new QLineEdit(this);
    d->passwordEdit->setPlaceholderText(tr("Enter password"));
    d->passwordEdit->setEnabled(false);
    
    d->startAudioMutedCheckBox = new QCheckBox(tr("Start with audio muted"), this);
    d->startVideoMutedCheckBox = new QCheckBox(tr("Start with video muted"), this);
    d->enableRecordingCheckBox = new QCheckBox(tr("Enable recording"), this);
    d->enableLiveStreamingCheckBox = new QCheckBox(tr("Enable live streaming"), this);
    
    d->descriptionEdit = new QTextEdit(this);
    d->descriptionEdit->setPlaceholderText(tr("Enter meeting description (optional)"));
    d->descriptionEdit->setMaximumHeight(100);
}

void CreateDialog::setupLayout()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // 基本信息组
    QGroupBox* basicGroup = new QGroupBox(tr("Basic Information"), this);
    QFormLayout* basicLayout = new QFormLayout(basicGroup);
    basicLayout->addRow(tr("Meeting Name:"), d->meetingNameEdit);
    basicLayout->addRow(tr("Meeting Subject:"), d->meetingSubjectEdit);
    basicLayout->addRow(tr("Server:"), d->serverComboBox);
    mainLayout->addWidget(basicGroup);
    
    // 安全组
    QGroupBox* securityGroup = new QGroupBox(tr("Security"), this);
    QVBoxLayout* securityLayout = new QVBoxLayout(securityGroup);
    securityLayout->addWidget(d->passwordCheckBox);
    securityLayout->addWidget(d->passwordEdit);
    mainLayout->addWidget(securityGroup);
    
    // 设置组
    QGroupBox* settingsGroup = new QGroupBox(tr("Settings"), this);
    QVBoxLayout* settingsLayout = new QVBoxLayout(settingsGroup);
    settingsLayout->addWidget(d->startAudioMutedCheckBox);
    settingsLayout->addWidget(d->startVideoMutedCheckBox);
    settingsLayout->addWidget(d->enableRecordingCheckBox);
    settingsLayout->addWidget(d->enableLiveStreamingCheckBox);
    mainLayout->addWidget(settingsGroup);
    
    // 描述组
    QGroupBox* descGroup = new QGroupBox(tr("Description"), this);
    QVBoxLayout* descLayout = new QVBoxLayout(descGroup);
    descLayout->addWidget(d->descriptionEdit);
    mainLayout->addWidget(descGroup);
    
    // 按钮
    QDialogButtonBox* buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    mainLayout->addWidget(buttonBox);
    
    // 连接按钮信号
    connect(buttonBox, &QDialogButtonBox::accepted, this, &CreateDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &CreateDialog::reject);
}

void CreateDialog::connectSignals()
{
    connect(d->passwordCheckBox, &QCheckBox::toggled,
            d->passwordEdit, &QLineEdit::setEnabled);
    
    connect(d->meetingNameEdit, &QLineEdit::textChanged,
            this, &CreateDialog::validateInput);
}

void CreateDialog::loadSettings()
{
    if (!d->config) {
        return;
    }
    
    // 加载默认设置
    d->startAudioMutedCheckBox->setChecked(d->config->getCustomSetting("startWithAudioMuted", false).toBool());
    d->startVideoMutedCheckBox->setChecked(d->config->getCustomSetting("startWithVideoMuted", false).toBool());
    QStringList servers = d->config->serverList();
    if (!servers.isEmpty()) {
        d->serverComboBox->clear();
        d->serverComboBox->addItems(servers);
    }
    
    // 设置默认服务器
    QString defaultServer = d->config->defaultServer();
    if (!defaultServer.isEmpty()) {
        int index = d->serverComboBox->findText(defaultServer);
        if (index >= 0) {
            d->serverComboBox->setCurrentIndex(index);
        } else {
            d->serverComboBox->setCurrentText(defaultServer);
        }
    }
}

void CreateDialog::saveSettings()
{
    if (!d->config) {
        return;
    }
    
    // 保存默认设置
    if (d->config) {
        d->config->setOption("startWithAudioMuted", d->startAudioMutedCheckBox->isChecked());
        d->config->setOption("startWithVideoMuted", d->startVideoMutedCheckBox->isChecked());
    }
    
    // 保存当前服务器
    QString currentServer = d->serverComboBox->currentText();
    if (d->config) {
        d->config->setOption("defaultServer", currentServer);
    }
    
    // 更新服务器列表
    QStringList servers;
    for (int i = 0; i < d->serverComboBox->count(); ++i) {
        servers << d->serverComboBox->itemText(i);
    }
    
    // 如果当前服务器不在列表中，添加它
    if (!servers.contains(currentServer)) {
        servers.prepend(currentServer);
    }
    
    // 限制列表大小为10个
    while (servers.size() > 10) {
        servers.removeLast();
    }
    
    d->config->setValue("serverList", servers);
}

void CreateDialog::validateInput()
{
    bool isValid = !d->meetingNameEdit->text().isEmpty();
    
    if (d->passwordCheckBox->isChecked()) {
        isValid = isValid && !d->passwordEdit->text().isEmpty();
    }
    
    QPushButton* okButton = dynamic_cast<QDialogButtonBox*>(
        findChild<QDialogButtonBox*>())->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setEnabled(isValid);
    }
}

void CreateDialog::accept()
{
    if (!d->meetingManager) {
        QMessageBox::critical(this, tr("Error"), tr("Meeting manager not available"));
        return;
    }
    
    if (d->meetingNameEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Meeting name is required"));
        return;
    }
    
    if (d->passwordCheckBox->isChecked() && d->passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, tr("Validation Error"), tr("Password is required"));
        return;
    }
    
    // 收集会议设置
    QVariantMap settings;
    settings["name"] = d->meetingNameEdit->text();
    settings["subject"] = d->meetingSubjectEdit->text();
    settings["server"] = d->serverComboBox->currentText();
    settings["description"] = d->descriptionEdit->toPlainText();
    
    if (d->passwordCheckBox->isChecked()) {
        settings["password"] = d->passwordEdit->text();
    }
    
    settings["startWithAudioMuted"] = d->startAudioMutedCheckBox->isChecked();
    settings["startWithVideoMuted"] = d->startVideoMutedCheckBox->isChecked();
    settings["enableRecording"] = d->enableRecordingCheckBox->isChecked();
    settings["enableLiveStreaming"] = d->enableLiveStreamingCheckBox->isChecked();
    
    // 保存设置
    saveSettings();
    
    // 创建会议
    d->isCreating = true;
    
    // 异步创建会议
    QTimer::singleShot(100, this, [this, settings]() {
        bool success = d->meetingManager->createMeeting(settings["name"].toString(), settings);
        d->isCreating = false;
        
        if (success) {
            QDialog::accept();
        } else {
            QMessageBox::critical(this, tr("Error"), tr("Failed to create meeting"));
        }
    });
}

void CreateDialog::reject()
{
    if (d->isCreating) {
        QMessageBox::warning(this, tr("Warning"), tr("Meeting creation in progress, please wait..."));
        return;
    }
    
    QDialog::reject();
}

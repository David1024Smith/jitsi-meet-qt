#include "AudioControlWidget.h"
#include "VolumeSliderWidget.h"
#include "../include/AudioManager.h"
#include <QApplication>
#include <QStyle>

AudioControlWidget::AudioControlWidget(QWidget *parent)
    : QWidget(parent)
    , m_audioManager(nullptr)
    , m_mainLayout(nullptr)
    , m_deviceGroup(nullptr)
    , m_inputDeviceCombo(nullptr)
    , m_outputDeviceCombo(nullptr)
    , m_inputDeviceLabel(nullptr)
    , m_outputDeviceLabel(nullptr)
    , m_volumeGroup(nullptr)
    , m_masterVolumeSlider(nullptr)
    , m_microphoneGainSlider(nullptr)
    , m_muteButton(nullptr)
    , m_volumeLabel(nullptr)
    , m_gainLabel(nullptr)
    , m_qualityGroup(nullptr)
    , m_qualityCombo(nullptr)
    , m_qualityLabel(nullptr)
    , m_advancedGroup(nullptr)
    , m_noiseSuppressionCheck(nullptr)
    , m_echoCancellationCheck(nullptr)
    , m_autoGainControlCheck(nullptr)
    , m_testButton(nullptr)
    , m_showAdvancedCheck(nullptr)
    , m_statusGroup(nullptr)
    , m_statusLabel(nullptr)
    , m_audioLevelBar(nullptr)
    , m_latencyLabel(nullptr)
    , m_showAdvanced(false)
    , m_isTestingAudio(false)
{
    initializeUI();
    connectSignals();
    updateUIState();
}

AudioControlWidget::~AudioControlWidget()
{
    // 析构函数
}

void AudioControlWidget::setAudioManager(AudioManager *audioManager)
{
    if (m_audioManager == audioManager) {
        return;
    }

    // 断开旧连接
    if (m_audioManager) {
        disconnect(m_audioManager, nullptr, this, nullptr);
    }

    m_audioManager = audioManager;

    // 建立新连接
    if (m_audioManager) {
        connect(m_audioManager, &AudioManager::statusChanged, 
                this, &AudioControlWidget::onAudioManagerStatusChanged);
        connect(m_audioManager, &AudioManager::devicesUpdated, 
                this, &AudioControlWidget::onDevicesUpdated);
        connect(m_audioManager, &AudioManager::audioStarted, 
                this, [this]() { updateAudioStatus(true); });
        connect(m_audioManager, &AudioManager::audioStopped, 
                this, [this]() { updateAudioStatus(false); });
    }

    updateDeviceList();
    updateUIState();
}

AudioManager* AudioControlWidget::audioManager() const
{
    return m_audioManager;
}

void AudioControlWidget::setShowAdvancedOptions(bool show)
{
    if (m_showAdvanced == show) {
        return;
    }

    m_showAdvanced = show;
    if (m_advancedGroup) {
        m_advancedGroup->setVisible(show);
    }
    if (m_showAdvancedCheck) {
        m_showAdvancedCheck->setChecked(show);
    }
}

bool AudioControlWidget::showAdvancedOptions() const
{
    return m_showAdvanced;
}

void AudioControlWidget::refreshUI()
{
    updateDeviceList();
    updateUIState();
}

void AudioControlWidget::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    updateUIState();
}

void AudioControlWidget::updateDeviceList()
{
    if (!m_audioManager) {
        return;
    }

    // 更新输入设备列表
    QStringList inputDevices = m_audioManager->availableInputDevices();
    QString currentInputDevice; // 从AudioManager获取当前设备
    populateDeviceComboBox(m_inputDeviceCombo, inputDevices, currentInputDevice);

    // 更新输出设备列表
    QStringList outputDevices = m_audioManager->availableOutputDevices();
    QString currentOutputDevice; // 从AudioManager获取当前设备
    populateDeviceComboBox(m_outputDeviceCombo, outputDevices, currentOutputDevice);
}

void AudioControlWidget::updateVolumeDisplay(qreal volume)
{
    if (m_masterVolumeSlider) {
        m_masterVolumeSlider->setVolume(volume);
    }
}

void AudioControlWidget::updateMuteDisplay(bool muted)
{
    if (m_masterVolumeSlider) {
        m_masterVolumeSlider->setMuted(muted);
    }
    if (m_muteButton) {
        m_muteButton->setChecked(muted);
        m_muteButton->setText(muted ? "🔇" : "🔊");
    }
}

void AudioControlWidget::updateAudioStatus(bool active)
{
    if (m_statusLabel) {
        m_statusLabel->setText(active ? QStringLiteral("音频活动") : QStringLiteral("音频停止"));
    }
    
    // 更新测试按钮状态
    if (m_testButton) {
        m_testButton->setEnabled(!active || m_isTestingAudio);
        if (m_isTestingAudio && !active) {
            m_isTestingAudio = false;
            m_testButton->setText(QStringLiteral("测试音频"));
        }
    }
}

void AudioControlWidget::onInputDeviceSelectionChanged(int index)
{
    if (!m_audioManager || !m_inputDeviceCombo || index < 0) {
        return;
    }

    QString deviceId = m_inputDeviceCombo->itemData(index).toString();
    if (!deviceId.isEmpty()) {
        emit inputDeviceChanged(deviceId);
    }
}

void AudioControlWidget::onOutputDeviceSelectionChanged(int index)
{
    if (!m_audioManager || !m_outputDeviceCombo || index < 0) {
        return;
    }

    QString deviceId = m_outputDeviceCombo->itemData(index).toString();
    if (!deviceId.isEmpty()) {
        emit outputDeviceChanged(deviceId);
    }
}

void AudioControlWidget::onMasterVolumeChanged(qreal volume)
{
    emit masterVolumeChanged(volume);
}

void AudioControlWidget::onMicrophoneGainChanged(qreal gain)
{
    emit microphoneGainChanged(gain);
}

void AudioControlWidget::onMuteButtonClicked(bool checked)
{
    emit muteStateChanged(checked);
}

void AudioControlWidget::onAudioQualityChanged(int index)
{
    if (index >= 0) {
        emit audioQualityChanged(index);
    }
}

void AudioControlWidget::onTestButtonClicked()
{
    if (m_isTestingAudio) {
        emit stopAudioTest();
        m_isTestingAudio = false;
        m_testButton->setText(QStringLiteral("测试音频"));
    } else {
        emit startAudioTest();
        m_isTestingAudio = true;
        m_testButton->setText(QStringLiteral("停止测试"));
    }
}

void AudioControlWidget::onAdvancedOptionsToggled(bool checked)
{
    setShowAdvancedOptions(checked);
}

void AudioControlWidget::onAudioManagerStatusChanged()
{
    updateUIState();
}

void AudioControlWidget::onDevicesUpdated()
{
    updateDeviceList();
}

void AudioControlWidget::initializeUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(6, 6, 6, 6);
    m_mainLayout->setSpacing(8);

    // 创建各个组件组
    m_deviceGroup = createDeviceSelectionGroup();
    m_volumeGroup = createVolumeControlGroup();
    m_qualityGroup = createAudioQualityGroup();
    m_advancedGroup = createAdvancedOptionsGroup();
    m_statusGroup = createStatusGroup();

    // 添加到主布局
    m_mainLayout->addWidget(m_deviceGroup);
    m_mainLayout->addWidget(m_volumeGroup);
    m_mainLayout->addWidget(m_qualityGroup);
    m_mainLayout->addWidget(m_advancedGroup);
    m_mainLayout->addWidget(m_statusGroup);
    m_mainLayout->addStretch();

    // 设置初始状态
    m_advancedGroup->setVisible(m_showAdvanced);
}

QGroupBox* AudioControlWidget::createDeviceSelectionGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("设备选择"), this);
    QVBoxLayout *layout = new QVBoxLayout(group);

    // 输入设备
    QHBoxLayout *inputLayout = new QHBoxLayout();
    m_inputDeviceLabel = new QLabel(QStringLiteral("麦克风:"), group);
    m_inputDeviceCombo = new QComboBox(group);
    m_inputDeviceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    inputLayout->addWidget(m_inputDeviceLabel);
    inputLayout->addWidget(m_inputDeviceCombo);

    // 输出设备
    QHBoxLayout *outputLayout = new QHBoxLayout();
    m_outputDeviceLabel = new QLabel(QStringLiteral("扬声器:"), group);
    m_outputDeviceCombo = new QComboBox(group);
    m_outputDeviceCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    outputLayout->addWidget(m_outputDeviceLabel);
    outputLayout->addWidget(m_outputDeviceCombo);

    layout->addLayout(inputLayout);
    layout->addLayout(outputLayout);

    return group;
}

QGroupBox* AudioControlWidget::createVolumeControlGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("音量控制"), this);
    QVBoxLayout *layout = new QVBoxLayout(group);

    // 主音量控制
    QHBoxLayout *masterLayout = new QHBoxLayout();
    m_volumeLabel = new QLabel(QStringLiteral("主音量:"), group);
    m_masterVolumeSlider = new VolumeSliderWidget(VolumeSliderWidget::Horizontal, group);
    m_masterVolumeSlider->setLabelText(QStringLiteral("主音量"));
    m_masterVolumeSlider->setShowMuteButton(true);
    masterLayout->addWidget(m_volumeLabel);
    masterLayout->addWidget(m_masterVolumeSlider, 1);

    // 麦克风增益控制
    QHBoxLayout *gainLayout = new QHBoxLayout();
    m_gainLabel = new QLabel(QStringLiteral("麦克风增益:"), group);
    m_microphoneGainSlider = new VolumeSliderWidget(VolumeSliderWidget::Horizontal, group);
    m_microphoneGainSlider->setLabelText(QStringLiteral("增益"));
    m_microphoneGainSlider->setShowMuteButton(false);
    gainLayout->addWidget(m_gainLabel);
    gainLayout->addWidget(m_microphoneGainSlider, 1);

    // 静音按钮
    m_muteButton = new QPushButton(QStringLiteral("🔊"), group);
    m_muteButton->setCheckable(true);
    m_muteButton->setFixedSize(40, 30);
    m_muteButton->setToolTip(QStringLiteral("静音/取消静音"));

    QHBoxLayout *muteLayout = new QHBoxLayout();
    muteLayout->addStretch();
    muteLayout->addWidget(m_muteButton);
    muteLayout->addStretch();

    layout->addLayout(masterLayout);
    layout->addLayout(gainLayout);
    layout->addLayout(muteLayout);

    return group;
}

QGroupBox* AudioControlWidget::createAudioQualityGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("音频质量"), this);
    QHBoxLayout *layout = new QHBoxLayout(group);

    m_qualityLabel = new QLabel(QStringLiteral("质量预设:"), group);
    m_qualityCombo = new QComboBox(group);
    m_qualityCombo->addItem(QStringLiteral("低质量 (16kHz, 单声道)"), 0);
    m_qualityCombo->addItem(QStringLiteral("标准质量 (44.1kHz, 立体声)"), 1);
    m_qualityCombo->addItem(QStringLiteral("高质量 (48kHz, 立体声, 24位)"), 2);
    m_qualityCombo->setCurrentIndex(1); // 默认标准质量

    layout->addWidget(m_qualityLabel);
    layout->addWidget(m_qualityCombo, 1);

    return group;
}

QGroupBox* AudioControlWidget::createAdvancedOptionsGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("高级选项"), this);
    QVBoxLayout *layout = new QVBoxLayout(group);

    // 音频处理选项
    m_noiseSuppressionCheck = new QCheckBox(QStringLiteral("噪声抑制"), group);
    m_echoCancellationCheck = new QCheckBox(QStringLiteral("回声消除"), group);
    m_autoGainControlCheck = new QCheckBox(QStringLiteral("自动增益控制"), group);

    // 测试按钮
    m_testButton = new QPushButton(QStringLiteral("测试音频"), group);
    m_testButton->setToolTip(QStringLiteral("播放测试音频以检查音频设置"));

    // 显示高级选项复选框
    m_showAdvancedCheck = new QCheckBox(QStringLiteral("显示高级选项"), group);
    m_showAdvancedCheck->setChecked(m_showAdvanced);

    layout->addWidget(m_noiseSuppressionCheck);
    layout->addWidget(m_echoCancellationCheck);
    layout->addWidget(m_autoGainControlCheck);
    layout->addWidget(m_testButton);
    layout->addStretch();
    layout->addWidget(m_showAdvancedCheck);

    return group;
}

QGroupBox* AudioControlWidget::createStatusGroup()
{
    QGroupBox *group = new QGroupBox(QStringLiteral("状态信息"), this);
    QVBoxLayout *layout = new QVBoxLayout(group);

    // 状态标签
    m_statusLabel = new QLabel(QStringLiteral("音频停止"), group);
    m_statusLabel->setAlignment(Qt::AlignCenter);

    // 音频级别指示器
    m_audioLevelBar = new QProgressBar(group);
    m_audioLevelBar->setRange(0, 100);
    m_audioLevelBar->setValue(0);
    m_audioLevelBar->setTextVisible(false);
    m_audioLevelBar->setMaximumHeight(10);

    // 延迟标签
    m_latencyLabel = new QLabel(QStringLiteral("延迟: -- ms"), group);
    m_latencyLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(m_statusLabel);
    layout->addWidget(new QLabel(QStringLiteral("音频级别:"), group));
    layout->addWidget(m_audioLevelBar);
    layout->addWidget(m_latencyLabel);

    return group;
}

void AudioControlWidget::connectSignals()
{
    // 设备选择信号
    connect(m_inputDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AudioControlWidget::onInputDeviceSelectionChanged);
    connect(m_outputDeviceCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AudioControlWidget::onOutputDeviceSelectionChanged);

    // 音量控制信号
    connect(m_masterVolumeSlider, &VolumeSliderWidget::volumeChanged,
            this, &AudioControlWidget::onMasterVolumeChanged);
    connect(m_microphoneGainSlider, &VolumeSliderWidget::volumeChanged,
            this, &AudioControlWidget::onMicrophoneGainChanged);
    connect(m_muteButton, &QPushButton::clicked,
            this, &AudioControlWidget::onMuteButtonClicked);

    // 质量设置信号
    connect(m_qualityCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &AudioControlWidget::onAudioQualityChanged);

    // 高级选项信号
    connect(m_testButton, &QPushButton::clicked,
            this, &AudioControlWidget::onTestButtonClicked);
    connect(m_showAdvancedCheck, &QCheckBox::toggled,
            this, &AudioControlWidget::onAdvancedOptionsToggled);
}

void AudioControlWidget::updateUIState()
{
    bool hasAudioManager = (m_audioManager != nullptr);
    bool isEnabled = this->isEnabled() && hasAudioManager;

    // 更新设备选择控件状态
    if (m_inputDeviceCombo) {
        m_inputDeviceCombo->setEnabled(isEnabled);
    }
    if (m_outputDeviceCombo) {
        m_outputDeviceCombo->setEnabled(isEnabled);
    }

    // 更新音量控制状态
    if (m_masterVolumeSlider) {
        m_masterVolumeSlider->setEnabled(isEnabled);
    }
    if (m_microphoneGainSlider) {
        m_microphoneGainSlider->setEnabled(isEnabled);
    }
    if (m_muteButton) {
        m_muteButton->setEnabled(isEnabled);
    }

    // 更新质量设置状态
    if (m_qualityCombo) {
        m_qualityCombo->setEnabled(isEnabled);
    }

    // 更新高级选项状态
    if (m_noiseSuppressionCheck) {
        m_noiseSuppressionCheck->setEnabled(isEnabled);
    }
    if (m_echoCancellationCheck) {
        m_echoCancellationCheck->setEnabled(isEnabled);
    }
    if (m_autoGainControlCheck) {
        m_autoGainControlCheck->setEnabled(isEnabled);
    }
    if (m_testButton) {
        m_testButton->setEnabled(isEnabled);
    }

    // 更新状态显示
    if (!hasAudioManager && m_statusLabel) {
        m_statusLabel->setText(QStringLiteral("未连接音频管理器"));
    }
}

void AudioControlWidget::populateDeviceComboBox(QComboBox *comboBox,
                                               const QStringList &devices,
                                               const QString &currentDevice)
{
    if (!comboBox) {
        return;
    }

    comboBox->blockSignals(true);
    comboBox->clear();

    if (devices.isEmpty()) {
        comboBox->addItem(QStringLiteral("无可用设备"), QString());
        comboBox->setEnabled(false);
    } else {
        comboBox->setEnabled(true);
        int currentIndex = -1;

        for (int i = 0; i < devices.size(); ++i) {
            const QString &deviceId = devices.at(i);
            // 这里应该从AudioManager获取设备的友好名称
            QString displayName = deviceId; // 简化处理
            
            comboBox->addItem(displayName, deviceId);
            
            if (deviceId == currentDevice) {
                currentIndex = i;
            }
        }

        if (currentIndex >= 0) {
            comboBox->setCurrentIndex(currentIndex);
        }
    }

    comboBox->blockSignals(false);
}
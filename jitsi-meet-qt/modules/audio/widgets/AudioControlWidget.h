#ifndef AUDIOCONTROLWIDGET_H
#define AUDIOCONTROLWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QProgressBar>

// 前向声明
class AudioManager;
class VolumeSliderWidget;

/**
 * @brief 音频控制组件
 * 
 * AudioControlWidget提供完整的音频控制界面，包括设备选择、
 * 音量控制、静音开关、音频质量设置等功能。
 */
class AudioControlWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父组件
     */
    explicit AudioControlWidget(QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~AudioControlWidget();

    /**
     * @brief 设置音频管理器
     * @param audioManager 音频管理器实例
     */
    void setAudioManager(AudioManager *audioManager);

    /**
     * @brief 获取音频管理器
     * @return 音频管理器实例
     */
    AudioManager* audioManager() const;

    /**
     * @brief 设置是否显示高级选项
     * @param show 是否显示
     */
    void setShowAdvancedOptions(bool show);

    /**
     * @brief 检查是否显示高级选项
     * @return 显示返回true
     */
    bool showAdvancedOptions() const;

    /**
     * @brief 刷新界面状态
     */
    void refreshUI();

    /**
     * @brief 设置启用状态
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

public slots:
    /**
     * @brief 更新设备列表
     */
    void updateDeviceList();

    /**
     * @brief 更新音量显示
     * @param volume 音量值
     */
    void updateVolumeDisplay(qreal volume);

    /**
     * @brief 更新静音状态显示
     * @param muted 静音状态
     */
    void updateMuteDisplay(bool muted);

    /**
     * @brief 更新音频状态显示
     * @param active 音频是否活动
     */
    void updateAudioStatus(bool active);

signals:
    /**
     * @brief 输入设备改变信号
     * @param deviceId 新设备ID
     */
    void inputDeviceChanged(const QString &deviceId);

    /**
     * @brief 输出设备改变信号
     * @param deviceId 新设备ID
     */
    void outputDeviceChanged(const QString &deviceId);

    /**
     * @brief 主音量改变信号
     * @param volume 新音量值
     */
    void masterVolumeChanged(qreal volume);

    /**
     * @brief 麦克风增益改变信号
     * @param gain 新增益值
     */
    void microphoneGainChanged(qreal gain);

    /**
     * @brief 静音状态改变信号
     * @param muted 新静音状态
     */
    void muteStateChanged(bool muted);

    /**
     * @brief 音频质量改变信号
     * @param quality 新质量设置
     */
    void audioQualityChanged(int quality);

    /**
     * @brief 开始音频测试信号
     */
    void startAudioTest();

    /**
     * @brief 停止音频测试信号
     */
    void stopAudioTest();

private slots:
    /**
     * @brief 处理输入设备选择改变
     * @param index 选择索引
     */
    void onInputDeviceSelectionChanged(int index);

    /**
     * @brief 处理输出设备选择改变
     * @param index 选择索引
     */
    void onOutputDeviceSelectionChanged(int index);

    /**
     * @brief 处理主音量改变
     * @param volume 新音量值
     */
    void onMasterVolumeChanged(qreal volume);

    /**
     * @brief 处理麦克风增益改变
     * @param gain 新增益值
     */
    void onMicrophoneGainChanged(qreal gain);

    /**
     * @brief 处理静音按钮点击
     * @param checked 是否选中
     */
    void onMuteButtonClicked(bool checked);

    /**
     * @brief 处理音频质量改变
     * @param index 质量索引
     */
    void onAudioQualityChanged(int index);

    /**
     * @brief 处理测试按钮点击
     */
    void onTestButtonClicked();

    /**
     * @brief 处理高级选项切换
     * @param checked 是否显示高级选项
     */
    void onAdvancedOptionsToggled(bool checked);

    /**
     * @brief 处理音频管理器状态改变
     */
    void onAudioManagerStatusChanged();

    /**
     * @brief 处理设备列表更新
     */
    void onDevicesUpdated();

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建设备选择组
     * @return 设备选择组件
     */
    QGroupBox* createDeviceSelectionGroup();

    /**
     * @brief 创建音量控制组
     * @return 音量控制组件
     */
    QGroupBox* createVolumeControlGroup();

    /**
     * @brief 创建音频质量组
     * @return 音频质量组件
     */
    QGroupBox* createAudioQualityGroup();

    /**
     * @brief 创建高级选项组
     * @return 高级选项组件
     */
    QGroupBox* createAdvancedOptionsGroup();

    /**
     * @brief 创建状态显示组
     * @return 状态显示组件
     */
    QGroupBox* createStatusGroup();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 更新界面状态
     */
    void updateUIState();

    /**
     * @brief 填充设备列表
     * @param comboBox 下拉框
     * @param devices 设备列表
     * @param currentDevice 当前设备
     */
    void populateDeviceComboBox(QComboBox *comboBox,
                               const QStringList &devices,
                               const QString &currentDevice);

private:
    // 音频管理器
    AudioManager *m_audioManager;

    // 主布局
    QVBoxLayout *m_mainLayout;

    // 设备选择组件
    QGroupBox *m_deviceGroup;
    QComboBox *m_inputDeviceCombo;
    QComboBox *m_outputDeviceCombo;
    QLabel *m_inputDeviceLabel;
    QLabel *m_outputDeviceLabel;

    // 音量控制组件
    QGroupBox *m_volumeGroup;
    VolumeSliderWidget *m_masterVolumeSlider;
    VolumeSliderWidget *m_microphoneGainSlider;
    QPushButton *m_muteButton;
    QLabel *m_volumeLabel;
    QLabel *m_gainLabel;

    // 音频质量组件
    QGroupBox *m_qualityGroup;
    QComboBox *m_qualityCombo;
    QLabel *m_qualityLabel;

    // 高级选项组件
    QGroupBox *m_advancedGroup;
    QCheckBox *m_noiseSuppressionCheck;
    QCheckBox *m_echoCancellationCheck;
    QCheckBox *m_autoGainControlCheck;
    QPushButton *m_testButton;
    QCheckBox *m_showAdvancedCheck;

    // 状态显示组件
    QGroupBox *m_statusGroup;
    QLabel *m_statusLabel;
    QProgressBar *m_audioLevelBar;
    QLabel *m_latencyLabel;

    // 状态变量
    bool m_showAdvanced;
    bool m_isTestingAudio;
};

#endif // AUDIOCONTROLWIDGET_H
#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QGroupBox>
#include <QDebug>

#include "../utils/AudioUtils.h"
#include "../widgets/AudioControlWidget.h"
#include "../widgets/VolumeSliderWidget.h"
#include "../include/AudioManager.h"

/**
 * @brief 音频工具类和UI组件示例应用
 * 
 * 这个示例展示了如何使用AudioUtils工具类和音频UI组件，
 * 包括音频格式转换、音量控制、设备选择等功能。
 */
class AudioUtilsExampleWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AudioUtilsExampleWindow(QWidget *parent = nullptr)
        : QMainWindow(parent)
        , m_audioManager(nullptr)
        , m_audioControlWidget(nullptr)
        , m_volumeSlider(nullptr)
        , m_logTextEdit(nullptr)
    {
        setupUI();
        setupAudioManager();
        runAudioUtilsTests();
    }

private slots:
    void onGenerateTestTone()
    {
        logMessage("生成测试音频...");
        
        AudioUtils::AudioFormat format;
        format.sampleRate = 44100;
        format.channels = 2;
        format.sampleSize = 16;
        format.isSigned = true;
        format.isFloat = false;
        
        // 生成1秒的440Hz正弦波
        QByteArray testAudio = AudioUtils::generateTestTone(440, 1000, format, 0.5);
        
        logMessage(QString("生成了 %1 字节的测试音频数据").arg(testAudio.size()));
        
        // 计算音频属性
        qreal rmsVolume = AudioUtils::calculateRMSVolume(testAudio, format);
        qreal peakVolume = AudioUtils::calculatePeakVolume(testAudio, format);
        int duration = AudioUtils::calculateAudioDuration(testAudio.size(), format);
        
        logMessage(QString("RMS音量: %1, 峰值音量: %2, 持续时间: %3ms")
                  .arg(rmsVolume, 0, 'f', 3)
                  .arg(peakVolume, 0, 'f', 3)
                  .arg(duration));
    }
    
    void onTestFormatConversion()
    {
        logMessage("测试音频格式转换...");
        
        // 创建输入格式 (44.1kHz, 立体声, 16位)
        AudioUtils::AudioFormat inputFormat;
        inputFormat.sampleRate = 44100;
        inputFormat.channels = 2;
        inputFormat.sampleSize = 16;
        inputFormat.isSigned = true;
        inputFormat.isFloat = false;
        
        // 创建输出格式 (48kHz, 单声道, 16位)
        AudioUtils::AudioFormat outputFormat;
        outputFormat.sampleRate = 48000;
        outputFormat.channels = 1;
        outputFormat.sampleSize = 16;
        outputFormat.isSigned = true;
        outputFormat.isFloat = false;
        
        // 生成测试数据
        QByteArray inputData = AudioUtils::generateTestTone(1000, 500, inputFormat, 0.3);
        logMessage(QString("输入数据: %1 字节, %2")
                  .arg(inputData.size())
                  .arg(AudioUtils::formatToDebugString(inputFormat)));
        
        // 执行格式转换
        QByteArray outputData = AudioUtils::convertAudioFormat(inputData, inputFormat, outputFormat);
        logMessage(QString("输出数据: %1 字节, %2")
                  .arg(outputData.size())
                  .arg(AudioUtils::formatToDebugString(outputFormat)));
        
        // 验证转换结果
        bool isValid = AudioUtils::validateAudioData(outputData, outputFormat);
        logMessage(QString("转换结果验证: %1").arg(isValid ? "成功" : "失败"));
    }
    
    void onTestVolumeProcessing()
    {
        logMessage("测试音量处理...");
        
        AudioUtils::AudioFormat format = AudioUtils::getFormatForQualityPreset(AudioUtils::StandardQuality);
        QByteArray audioData = AudioUtils::generateTestTone(800, 1000, format, 0.8);
        
        logMessage(QString("原始音频: RMS=%1, Peak=%2")
                  .arg(AudioUtils::calculateRMSVolume(audioData, format), 0, 'f', 3)
                  .arg(AudioUtils::calculatePeakVolume(audioData, format), 0, 'f', 3));
        
        // 应用50%音量
        QByteArray reducedVolume = AudioUtils::applyVolumeGain(audioData, 0.5, format);
        logMessage(QString("50%音量: RMS=%1, Peak=%2")
                  .arg(AudioUtils::calculateRMSVolume(reducedVolume, format), 0, 'f', 3)
                  .arg(AudioUtils::calculatePeakVolume(reducedVolume, format), 0, 'f', 3));
        
        // 应用200%音量 (放大)
        QByteArray amplified = AudioUtils::applyVolumeGain(audioData, 2.0, format);
        logMessage(QString("200%音量: RMS=%1, Peak=%2")
                  .arg(AudioUtils::calculateRMSVolume(amplified, format), 0, 'f', 3)
                  .arg(AudioUtils::calculatePeakVolume(amplified, format), 0, 'f', 3));
    }
    
    void onTestAudioMixing()
    {
        logMessage("测试音频混合...");
        
        AudioUtils::AudioFormat format = AudioUtils::getFormatForQualityPreset(AudioUtils::StandardQuality);
        
        // 生成两个不同频率的音频
        QByteArray audio1 = AudioUtils::generateTestTone(440, 1000, format, 0.5); // A4音符
        QByteArray audio2 = AudioUtils::generateTestTone(880, 1000, format, 0.5); // A5音符
        
        logMessage(QString("音频1 (440Hz): %1 字节").arg(audio1.size()));
        logMessage(QString("音频2 (880Hz): %1 字节").arg(audio2.size()));
        
        // 等比例混合
        QByteArray mixed = AudioUtils::mixAudioStreams(audio1, audio2, format, 0.5);
        logMessage(QString("混合音频: %1 字节").arg(mixed.size()));
        
        // 计算混合后的音量
        qreal mixedRMS = AudioUtils::calculateRMSVolume(mixed, format);
        qreal mixedPeak = AudioUtils::calculatePeakVolume(mixed, format);
        logMessage(QString("混合音频音量: RMS=%1, Peak=%2")
                  .arg(mixedRMS, 0, 'f', 3)
                  .arg(mixedPeak, 0, 'f', 3));
    }
    
    void onVolumeSliderChanged(qreal volume)
    {
        logMessage(QString("音量滑块改变: %1%").arg(static_cast<int>(volume * 100)));
    }
    
    void onMuteStateChanged(bool muted)
    {
        logMessage(QString("静音状态改变: %1").arg(muted ? "静音" : "取消静音"));
    }
    
    void onInputDeviceChanged(const QString &deviceId)
    {
        logMessage(QString("输入设备改变: %1").arg(deviceId));
    }
    
    void onOutputDeviceChanged(const QString &deviceId)
    {
        logMessage(QString("输出设备改变: %1").arg(deviceId));
    }
    
    void onAudioQualityChanged(int quality)
    {
        AudioUtils::QualityPreset preset = static_cast<AudioUtils::QualityPreset>(quality);
        QString description = AudioUtils::getQualityPresetDescription(preset);
        int bitrate = AudioUtils::getBitrateForQualityPreset(preset);
        
        logMessage(QString("音频质量改变: %1 (比特率: %2 kbps)")
                  .arg(description)
                  .arg(bitrate));
    }

private:
    void setupUI()
    {
        setWindowTitle("音频工具类和UI组件示例");
        setMinimumSize(800, 600);
        
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);
        
        QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);
        
        // 左侧：控制面板
        QWidget *controlPanel = createControlPanel();
        mainLayout->addWidget(controlPanel, 1);
        
        // 右侧：日志显示
        QWidget *logPanel = createLogPanel();
        mainLayout->addWidget(logPanel, 1);
    }
    
    QWidget* createControlPanel()
    {
        QWidget *panel = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(panel);
        
        // 音频工具测试按钮
        QGroupBox *utilsGroup = new QGroupBox("音频工具测试");
        QVBoxLayout *utilsLayout = new QVBoxLayout(utilsGroup);
        
        QPushButton *generateToneBtn = new QPushButton("生成测试音频");
        QPushButton *formatConversionBtn = new QPushButton("测试格式转换");
        QPushButton *volumeProcessingBtn = new QPushButton("测试音量处理");
        QPushButton *audioMixingBtn = new QPushButton("测试音频混合");
        
        connect(generateToneBtn, &QPushButton::clicked, this, &AudioUtilsExampleWindow::onGenerateTestTone);
        connect(formatConversionBtn, &QPushButton::clicked, this, &AudioUtilsExampleWindow::onTestFormatConversion);
        connect(volumeProcessingBtn, &QPushButton::clicked, this, &AudioUtilsExampleWindow::onTestVolumeProcessing);
        connect(audioMixingBtn, &QPushButton::clicked, this, &AudioUtilsExampleWindow::onTestAudioMixing);
        
        utilsLayout->addWidget(generateToneBtn);
        utilsLayout->addWidget(formatConversionBtn);
        utilsLayout->addWidget(volumeProcessingBtn);
        utilsLayout->addWidget(audioMixingBtn);
        
        // 音量滑块示例
        QGroupBox *sliderGroup = new QGroupBox("音量滑块示例");
        QVBoxLayout *sliderLayout = new QVBoxLayout(sliderGroup);
        
        m_volumeSlider = new VolumeSliderWidget(VolumeSliderWidget::Horizontal);
        m_volumeSlider->setLabelText("示例音量");
        m_volumeSlider->setShowMuteButton(true);
        m_volumeSlider->setShowValueLabel(true);
        m_volumeSlider->setShowLevelIndicator(true);
        
        connect(m_volumeSlider, &VolumeSliderWidget::volumeChanged,
                this, &AudioUtilsExampleWindow::onVolumeSliderChanged);
        connect(m_volumeSlider, &VolumeSliderWidget::muteChanged,
                this, &AudioUtilsExampleWindow::onMuteStateChanged);
        
        sliderLayout->addWidget(m_volumeSlider);
        
        // 音频控制组件
        QGroupBox *controlGroup = new QGroupBox("音频控制组件");
        QVBoxLayout *controlLayout = new QVBoxLayout(controlGroup);
        
        m_audioControlWidget = new AudioControlWidget();
        m_audioControlWidget->setShowAdvancedOptions(true);
        
        connect(m_audioControlWidget, &AudioControlWidget::inputDeviceChanged,
                this, &AudioUtilsExampleWindow::onInputDeviceChanged);
        connect(m_audioControlWidget, &AudioControlWidget::outputDeviceChanged,
                this, &AudioUtilsExampleWindow::onOutputDeviceChanged);
        connect(m_audioControlWidget, &AudioControlWidget::audioQualityChanged,
                this, &AudioUtilsExampleWindow::onAudioQualityChanged);
        
        controlLayout->addWidget(m_audioControlWidget);
        
        layout->addWidget(utilsGroup);
        layout->addWidget(sliderGroup);
        layout->addWidget(controlGroup);
        layout->addStretch();
        
        return panel;
    }
    
    QWidget* createLogPanel()
    {
        QWidget *panel = new QWidget();
        QVBoxLayout *layout = new QVBoxLayout(panel);
        
        QLabel *logLabel = new QLabel("操作日志:");
        m_logTextEdit = new QTextEdit();
        m_logTextEdit->setReadOnly(true);
        m_logTextEdit->setFont(QFont("Consolas", 9));
        
        QPushButton *clearLogBtn = new QPushButton("清空日志");
        connect(clearLogBtn, &QPushButton::clicked, m_logTextEdit, &QTextEdit::clear);
        
        layout->addWidget(logLabel);
        layout->addWidget(m_logTextEdit, 1);
        layout->addWidget(clearLogBtn);
        
        return panel;
    }
    
    void setupAudioManager()
    {
        // 这里应该创建真实的AudioManager实例
        // 为了示例，我们暂时不创建
        logMessage("音频管理器设置完成 (示例模式)");
        
        // 如果有AudioManager实例，可以这样设置：
        // m_audioManager = new AudioManager(this);
        // m_audioControlWidget->setAudioManager(m_audioManager);
    }
    
    void runAudioUtilsTests()
    {
        logMessage("=== 音频工具类和UI组件示例 ===");
        logMessage("点击左侧按钮测试各种功能");
        
        // 显示支持的音频格式信息
        QList<int> sampleRates = AudioUtils::supportedSampleRates();
        QList<int> channels = AudioUtils::supportedChannelCounts();
        QList<int> sampleSizes = AudioUtils::supportedSampleSizes();
        
        logMessage(QString("支持的采样率: %1").arg(listToString(sampleRates)));
        logMessage(QString("支持的声道数: %1").arg(listToString(channels)));
        logMessage(QString("支持的样本大小: %1").arg(listToString(sampleSizes)));
        
        // 显示质量预设信息
        for (int i = 0; i < 3; ++i) {
            AudioUtils::QualityPreset preset = static_cast<AudioUtils::QualityPreset>(i);
            QString description = AudioUtils::getQualityPresetDescription(preset);
            int bitrate = AudioUtils::getBitrateForQualityPreset(preset);
            AudioUtils::AudioFormat format = AudioUtils::getFormatForQualityPreset(preset);
            
            logMessage(QString("质量预设 %1: %2 (比特率: %3 kbps, 格式: %4)")
                      .arg(i)
                      .arg(description)
                      .arg(bitrate)
                      .arg(AudioUtils::formatToDebugString(format)));
        }
    }
    
    void logMessage(const QString &message)
    {
        if (m_logTextEdit) {
            QString timestamp = QTime::currentTime().toString("hh:mm:ss.zzz");
            m_logTextEdit->append(QString("[%1] %2").arg(timestamp, message));
        }
    }
    
    template<typename T>
    QString listToString(const QList<T> &list)
    {
        QStringList strings;
        for (const T &item : list) {
            strings << QString::number(item);
        }
        return strings.join(", ");
    }

private:
    AudioManager *m_audioManager;
    AudioControlWidget *m_audioControlWidget;
    VolumeSliderWidget *m_volumeSlider;
    QTextEdit *m_logTextEdit;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    AudioUtilsExampleWindow window;
    window.show();
    
    return app.exec();
}

#include "audio_utils_example.moc"
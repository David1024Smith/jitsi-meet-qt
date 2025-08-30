#include <QCoreApplication>
#include <QDebug>
#include "../config/AudioConfig.h"

/**
 * @brief AudioConfig使用示例
 * 
 * 演示如何使用AudioConfig类进行音频配置管理
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qDebug() << "=== AudioConfig 使用示例 ===";
    
    // 创建AudioConfig实例
    AudioConfig audioConfig;
    
    // 1. 基础配置设置
    qDebug() << "\n1. 设置基础音频配置:";
    audioConfig.setPreferredInputDevice("default_microphone");
    audioConfig.setPreferredOutputDevice("default_speakers");
    audioConfig.setSampleRate(48000);
    audioConfig.setChannels(2);
    
    qDebug() << "输入设备:" << audioConfig.preferredInputDevice();
    qDebug() << "输出设备:" << audioConfig.preferredOutputDevice();
    qDebug() << "采样率:" << audioConfig.sampleRate() << "Hz";
    qDebug() << "声道数:" << audioConfig.channels();
    
    // 2. 音量控制
    qDebug() << "\n2. 音量控制:";
    audioConfig.setMasterVolume(0.8);
    audioConfig.setMicrophoneGain(0.6);
    audioConfig.setMuted(false);
    
    qDebug() << "主音量:" << audioConfig.masterVolume();
    qDebug() << "麦克风增益:" << audioConfig.microphoneGain();
    qDebug() << "静音状态:" << (audioConfig.isMuted() ? "是" : "否");
    
    // 3. 质量预设
    qDebug() << "\n3. 质量预设测试:";
    qDebug() << "应用高质量预设...";
    audioConfig.setQualityPreset(AudioConfig::HighQuality);
    
    qDebug() << "采样率:" << audioConfig.sampleRate() << "Hz";
    qDebug() << "声道数:" << audioConfig.channels();
    qDebug() << "比特率:" << audioConfig.bitrate() << "kbps";
    qDebug() << "缓冲区大小:" << audioConfig.bufferSize() << "samples";
    
    return 0;
}    // 
4. 音频处理功能
    qDebug() << "\n4. 音频处理功能:";
    audioConfig.setNoiseSuppressionEnabled(true);
    audioConfig.setEchoCancellationEnabled(true);
    audioConfig.setAutoGainControlEnabled(false);
    
    qDebug() << "噪声抑制:" << (audioConfig.isNoiseSuppressionEnabled() ? "启用" : "禁用");
    qDebug() << "回声消除:" << (audioConfig.isEchoCancellationEnabled() ? "启用" : "禁用");
    qDebug() << "自动增益控制:" << (audioConfig.isAutoGainControlEnabled() ? "启用" : "禁用");
    
    // 5. 自定义参数
    qDebug() << "\n5. 自定义参数:";
    audioConfig.setCustomParameter("custom_filter", "low_pass");
    audioConfig.setCustomParameter("custom_threshold", 0.5);
    
    qDebug() << "自定义滤波器:" << audioConfig.customParameter("custom_filter").toString();
    qDebug() << "自定义阈值:" << audioConfig.customParameter("custom_threshold").toReal();
    
    // 6. 配置验证
    qDebug() << "\n6. 配置验证:";
    bool isValid = audioConfig.validate();
    qDebug() << "配置有效性:" << (isValid ? "有效" : "无效");
    
    // 7. 配置保存和加载
    qDebug() << "\n7. 配置持久化:";
    QString configPath = "./audio_config_example.ini";
    audioConfig.setConfigFilePath(configPath);
    
    bool saved = audioConfig.save();
    qDebug() << "配置保存:" << (saved ? "成功" : "失败");
    
    // 创建新实例并加载配置
    AudioConfig loadedConfig;
    loadedConfig.setConfigFilePath(configPath);
    bool loaded = loadedConfig.load();
    qDebug() << "配置加载:" << (loaded ? "成功" : "失败");
    
    if (loaded) {
        qDebug() << "加载的采样率:" << loadedConfig.sampleRate() << "Hz";
        qDebug() << "加载的主音量:" << loadedConfig.masterVolume();
    }
    
    // 8. 序列化测试
    qDebug() << "\n8. 序列化测试:";
    QVariantMap configMap = audioConfig.toVariantMap();
    qDebug() << "配置序列化完成，包含" << configMap.size() << "个参数";
    
    AudioConfig deserializedConfig;
    deserializedConfig.fromVariantMap(configMap);
    qDebug() << "反序列化完成";
    qDebug() << "反序列化后的采样率:" << deserializedConfig.sampleRate() << "Hz";
    
    qDebug() << "\n=== 示例完成 ===";
    
    return 0;
}
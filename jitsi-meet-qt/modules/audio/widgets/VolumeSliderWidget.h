#ifndef VOLUMESLIDERWIDGET_H
#define VOLUMESLIDERWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QProgressBar>
#include <QTimer>

/**
 * @brief 音量滑块组件
 * 
 * VolumeSliderWidget提供专业的音量控制界面，包括音量滑块、
 * 数值显示、静音按钮和音量级别指示器等功能。
 */
class VolumeSliderWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 滑块方向枚举
     */
    enum Orientation {
        Horizontal,     ///< 水平方向
        Vertical        ///< 垂直方向
    };
    Q_ENUM(Orientation)

    /**
     * @brief 显示模式枚举
     */
    enum DisplayMode {
        Percentage,     ///< 百分比显示 (0-100%)
        Decibel,        ///< 分贝显示 (-∞ to 0dB)
        Linear          ///< 线性显示 (0.0-1.0)
    };
    Q_ENUM(DisplayMode)

    /**
     * @brief 构造函数
     * @param orientation 滑块方向
     * @param parent 父组件
     */
    explicit VolumeSliderWidget(Orientation orientation = Horizontal, 
                               QWidget *parent = nullptr);

    /**
     * @brief 析构函数
     */
    virtual ~VolumeSliderWidget();

    /**
     * @brief 设置音量值
     * @param volume 音量值 (0.0-1.0)
     */
    void setVolume(qreal volume);

    /**
     * @brief 获取音量值
     * @return 音量值 (0.0-1.0)
     */
    qreal volume() const;

    /**
     * @brief 设置静音状态
     * @param muted 是否静音
     */
    void setMuted(bool muted);

    /**
     * @brief 获取静音状态
     * @return 是否静音
     */
    bool isMuted() const;

    /**
     * @brief 设置显示模式
     * @param mode 显示模式
     */
    void setDisplayMode(DisplayMode mode);

    /**
     * @brief 获取显示模式
     * @return 显示模式
     */
    DisplayMode displayMode() const;

    /**
     * @brief 设置滑块方向
     * @param orientation 方向
     */
    void setOrientation(Orientation orientation);

    /**
     * @brief 获取滑块方向
     * @return 方向
     */
    Orientation orientation() const;

    /**
     * @brief 设置是否显示静音按钮
     * @param show 是否显示
     */
    void setShowMuteButton(bool show);

    /**
     * @brief 检查是否显示静音按钮
     * @return 显示返回true
     */
    bool showMuteButton() const;

    /**
     * @brief 设置是否显示数值标签
     * @param show 是否显示
     */
    void setShowValueLabel(bool show);

    /**
     * @brief 检查是否显示数值标签
     * @return 显示返回true
     */
    bool showValueLabel() const;

    /**
     * @brief 设置是否显示音量级别指示器
     * @param show 是否显示
     */
    void setShowLevelIndicator(bool show);

    /**
     * @brief 检查是否显示音量级别指示器
     * @return 显示返回true
     */
    bool showLevelIndicator() const;

    /**
     * @brief 设置音量范围
     * @param minimum 最小值 (0.0-1.0)
     * @param maximum 最大值 (0.0-1.0)
     */
    void setVolumeRange(qreal minimum, qreal maximum);

    /**
     * @brief 获取最小音量值
     * @return 最小音量值
     */
    qreal minimumVolume() const;

    /**
     * @brief 获取最大音量值
     * @return 最大音量值
     */
    qreal maximumVolume() const;

    /**
     * @brief 设置音量步长
     * @param step 步长值
     */
    void setVolumeStep(qreal step);

    /**
     * @brief 获取音量步长
     * @return 步长值
     */
    qreal volumeStep() const;

    /**
     * @brief 设置当前音频级别 (用于级别指示器)
     * @param level 音频级别 (0.0-1.0)
     */
    void setAudioLevel(qreal level);

    /**
     * @brief 获取当前音频级别
     * @return 音频级别 (0.0-1.0)
     */
    qreal audioLevel() const;

    /**
     * @brief 设置标签文本
     * @param text 标签文本
     */
    void setLabelText(const QString &text);

    /**
     * @brief 获取标签文本
     * @return 标签文本
     */
    QString labelText() const;

    /**
     * @brief 设置启用状态
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

public slots:
    /**
     * @brief 增加音量
     */
    void increaseVolume();

    /**
     * @brief 减少音量
     */
    void decreaseVolume();

    /**
     * @brief 切换静音状态
     */
    void toggleMute();

    /**
     * @brief 重置为默认音量
     */
    void resetToDefault();

signals:
    /**
     * @brief 音量改变信号
     * @param volume 新音量值 (0.0-1.0)
     */
    void volumeChanged(qreal volume);

    /**
     * @brief 静音状态改变信号
     * @param muted 新静音状态
     */
    void muteChanged(bool muted);

    /**
     * @brief 音量调整开始信号
     */
    void volumeAdjustmentStarted();

    /**
     * @brief 音量调整结束信号
     */
    void volumeAdjustmentFinished();

protected:
    /**
     * @brief 重写鼠标滚轮事件
     * @param event 滚轮事件
     */
    void wheelEvent(QWheelEvent *event) override;

    /**
     * @brief 重写键盘事件
     * @param event 键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;

    /**
     * @brief 重写大小提示
     * @return 建议大小
     */
    QSize sizeHint() const override;

    /**
     * @brief 重写最小大小提示
     * @return 最小大小
     */
    QSize minimumSizeHint() const override;

private slots:
    /**
     * @brief 处理滑块值改变
     * @param value 滑块值
     */
    void onSliderValueChanged(int value);

    /**
     * @brief 处理滑块按下
     */
    void onSliderPressed();

    /**
     * @brief 处理滑块释放
     */
    void onSliderReleased();

    /**
     * @brief 处理静音按钮点击
     * @param checked 是否选中
     */
    void onMuteButtonClicked(bool checked);

    /**
     * @brief 更新级别指示器
     */
    void updateLevelIndicator();

private:
    /**
     * @brief 初始化界面
     */
    void initializeUI();

    /**
     * @brief 创建水平布局
     */
    void createHorizontalLayout();

    /**
     * @brief 创建垂直布局
     */
    void createVerticalLayout();

    /**
     * @brief 连接信号槽
     */
    void connectSignals();

    /**
     * @brief 更新数值标签
     */
    void updateValueLabel();

    /**
     * @brief 更新静音按钮状态
     */
    void updateMuteButton();

    /**
     * @brief 音量值转换为滑块值
     * @param volume 音量值 (0.0-1.0)
     * @return 滑块值
     */
    int volumeToSliderValue(qreal volume) const;

    /**
     * @brief 滑块值转换为音量值
     * @param sliderValue 滑块值
     * @return 音量值 (0.0-1.0)
     */
    qreal sliderValueToVolume(int sliderValue) const;

    /**
     * @brief 格式化音量显示文本
     * @param volume 音量值
     * @return 格式化的文本
     */
    QString formatVolumeText(qreal volume) const;

    /**
     * @brief 音量值转换为分贝
     * @param volume 音量值 (0.0-1.0)
     * @return 分贝值
     */
    qreal volumeToDecibel(qreal volume) const;

    /**
     * @brief 分贝转换为音量值
     * @param decibel 分贝值
     * @return 音量值 (0.0-1.0)
     */
    qreal decibelToVolume(qreal decibel) const;

private:
    // 布局组件
    QHBoxLayout *m_horizontalLayout;
    QVBoxLayout *m_verticalLayout;

    // 控制组件
    QSlider *m_volumeSlider;
    QPushButton *m_muteButton;
    QLabel *m_valueLabel;
    QLabel *m_titleLabel;
    QProgressBar *m_levelIndicator;

    // 配置参数
    Orientation m_orientation;
    DisplayMode m_displayMode;
    qreal m_volume;
    qreal m_minimumVolume;
    qreal m_maximumVolume;
    qreal m_volumeStep;
    qreal m_audioLevel;
    bool m_muted;
    bool m_showMuteButton;
    bool m_showValueLabel;
    bool m_showLevelIndicator;
    QString m_labelText;

    // 状态变量
    qreal m_volumeBeforeMute;
    bool m_isAdjusting;

    // 定时器
    QTimer *m_levelUpdateTimer;

    // 常量
    static const int SLIDER_RANGE = 1000;
    static const qreal DEFAULT_VOLUME;
    static const qreal DEFAULT_STEP;
};

#endif // VOLUMESLIDERWIDGET_H
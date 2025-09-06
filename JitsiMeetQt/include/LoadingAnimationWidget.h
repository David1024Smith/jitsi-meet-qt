#ifndef LOADINGANIMATIONWIDGET_H
#define LOADINGANIMATIONWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMovie>
#include <QPainter>
#include <QPixmap>
#include <QFont>
#include <QFontMetrics>
#include <QRect>
#include <QColor>
#include <QBrush>
#include <QPen>
#include <QLinearGradient>
#include <QRadialGradient>
#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QSequentialAnimationGroup>
#include <QGraphicsEffect>

/**
 * @brief 全屏启动动画界面组件
 * 
 * 这个类提供了一个全屏的启动动画界面，用于在会议加载过程中显示给用户。
 * 包含以下功能：
 * - 全屏覆盖显示
 * - 动态加载动画效果
 * - 加载进度指示
 * - 状态文本显示
 * - 平滑的淡入淡出过渡
 * - 自定义品牌元素
 */
class LoadingAnimationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal opacity READ opacity WRITE setOpacity)
    Q_PROPERTY(qreal rotationAngle READ rotationAngle WRITE setRotationAngle)
    Q_PROPERTY(qreal pulseScale READ pulseScale WRITE setPulseScale)

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口
     */
    explicit LoadingAnimationWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~LoadingAnimationWidget();
    
    /**
     * @brief 显示启动动画
     * @param message 初始加载消息
     */
    void showAnimation(const QString& message = QString("正在加载会议..."));
    
    /**
     * @brief 隐藏启动动画
     * @param fadeOut 是否使用淡出效果
     */
    void hideAnimation(bool fadeOut = true);
    
    /**
     * @brief 更新加载进度
     * @param progress 进度值 (0-100)
     */
    void updateProgress(int progress);
    
    /**
     * @brief 更新状态消息
     * @param message 状态消息
     */
    void updateMessage(const QString& message);
    
    /**
     * @brief 设置品牌Logo
     * @param logoPath Logo图片路径
     */
    void setLogo(const QString& logoPath);
    
    /**
     * @brief 设置主题颜色
     * @param primaryColor 主色调
     * @param secondaryColor 辅助色调
     */
    void setThemeColors(const QColor& primaryColor, const QColor& secondaryColor);
    
    /**
     * @brief 获取透明度属性
     * @return 当前透明度值
     */
    qreal opacity() const { return m_opacity; }
    
    /**
     * @brief 设置透明度属性
     * @param opacity 透明度值 (0.0-1.0)
     */
    void setOpacity(qreal opacity);
    
    /**
     * @brief 获取旋转角度属性
     * @return 当前旋转角度
     */
    qreal rotationAngle() const { return m_rotationAngle; }
    
    /**
     * @brief 设置旋转角度属性
     * @param angle 旋转角度
     */
    void setRotationAngle(qreal angle);
    
    /**
     * @brief 获取脉冲缩放属性
     * @return 当前缩放值
     */
    qreal pulseScale() const { return m_pulseScale; }
    
    /**
     * @brief 设置脉冲缩放属性
     * @param scale 缩放值
     */
    void setPulseScale(qreal scale);

signals:
    /**
     * @brief 动画显示完成信号
     */
    void animationShown();
    
    /**
     * @brief 动画隐藏完成信号
     */
    void animationHidden();
    
    /**
     * @brief 用户取消加载信号
     */
    void loadingCancelled();

protected:
    /**
     * @brief 绘制事件
     * @param event 绘制事件
     */
    void paintEvent(QPaintEvent *event) override;
    
    /**
     * @brief 鼠标按下事件
     * @param event 鼠标事件
     */
    void mousePressEvent(QMouseEvent *event) override;
    
    /**
     * @brief 键盘按下事件
     * @param event 键盘事件
     */
    void keyPressEvent(QKeyEvent *event) override;
    
    /**
     * @brief 窗口大小改变事件
     * @param event 大小改变事件
     */
    void resizeEvent(QResizeEvent *event) override;

private slots:
    /**
     * @brief 动画更新槽函数
     */
    void onAnimationUpdate();
    
    /**
     * @brief 淡入动画完成槽函数
     */
    void onFadeInFinished();
    
    /**
     * @brief 淡出动画完成槽函数
     */
    void onFadeOutFinished();
    
    /**
     * @brief 旋转动画更新槽函数
     */
    void onRotationUpdate();
    
    /**
     * @brief 脉冲动画更新槽函数
     */
    void onPulseUpdate();

private:
    /**
     * @brief 初始化UI组件
     */
    void initializeUI();
    
    /**
     * @brief 初始化动画
     */
    void initializeAnimations();
    
    /**
     * @brief 启动旋转动画
     */
    void startRotationAnimation();
    
    /**
     * @brief 停止旋转动画
     */
    void stopRotationAnimation();
    
    /**
     * @brief 启动脉冲动画
     */
    void startPulseAnimation();
    
    /**
     * @brief 停止脉冲动画
     */
    void stopPulseAnimation();
    
    /**
     * @brief 绘制背景
     * @param painter 绘制器
     */
    void drawBackground(QPainter* painter);
    
    /**
     * @brief 绘制Logo
     * @param painter 绘制器
     */
    void drawLogo(QPainter* painter);
    
    /**
     * @brief 绘制加载指示器
     * @param painter 绘制器
     */
    void drawLoadingIndicator(QPainter* painter);
    
    /**
     * @brief 绘制进度条
     * @param painter 绘制器
     */
    void drawProgressBar(QPainter* painter);
    
    /**
     * @brief 绘制状态文本
     * @param painter 绘制器
     */
    void drawStatusText(QPainter* painter);
    
    /**
     * @brief 计算布局位置
     */
    void calculateLayout();

private:
    // UI组件
    QVBoxLayout* m_mainLayout;              ///< 主布局
    QLabel* m_logoLabel;                    ///< Logo标签
    QLabel* m_statusLabel;                  ///< 状态标签
    QProgressBar* m_progressBar;            ///< 进度条
    
    // 动画组件
    QPropertyAnimation* m_fadeInAnimation;   ///< 淡入动画
    QPropertyAnimation* m_fadeOutAnimation;  ///< 淡出动画
    QPropertyAnimation* m_rotationAnimation; ///< 旋转动画
    QPropertyAnimation* m_pulseAnimation;    ///< 脉冲动画
    QParallelAnimationGroup* m_animationGroup; ///< 动画组
    QTimer* m_animationTimer;               ///< 动画定时器
    
    // 图形效果
    QGraphicsOpacityEffect* m_opacityEffect; ///< 透明度效果
    
    // 状态变量
    QString m_currentMessage;               ///< 当前消息
    QString m_logoPath;                     ///< Logo路径
    QPixmap m_logoPixmap;                   ///< Logo图片
    int m_currentProgress;                  ///< 当前进度
    qreal m_opacity;                        ///< 透明度
    qreal m_rotationAngle;                  ///< 旋转角度
    qreal m_pulseScale;                     ///< 脉冲缩放
    bool m_isVisible;                       ///< 是否可见
    bool m_animationRunning;                ///< 动画是否运行中
    
    // 主题配置
    QColor m_primaryColor;                  ///< 主色调
    QColor m_secondaryColor;                ///< 辅助色调
    QColor m_backgroundColor;               ///< 背景色
    QColor m_textColor;                     ///< 文本色
    
    // 布局配置
    QRect m_logoRect;                       ///< Logo区域
    QRect m_indicatorRect;                  ///< 指示器区域
    QRect m_progressRect;                   ///< 进度条区域
    QRect m_textRect;                       ///< 文本区域
    
    // 常量配置
    static const int ANIMATION_DURATION = 300;     ///< 动画持续时间(ms)
    static const int ROTATION_DURATION = 2000;     ///< 旋转动画持续时间(ms)
    static const int PULSE_DURATION = 1500;        ///< 脉冲动画持续时间(ms)
    static const int INDICATOR_SIZE = 60;           ///< 指示器大小
    static const int PROGRESS_HEIGHT = 4;          ///< 进度条高度
    static const int LOGO_MAX_SIZE = 120;          ///< Logo最大尺寸
};

#endif // LOADINGANIMATIONWIDGET_H
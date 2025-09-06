#include "LoadingAnimationWidget.h"
#include <QApplication>
#include <QScreen>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QDebug>
#include <QStyleOption>
#include <QGraphicsDropShadowEffect>
#include <QtMath>

/**
 * @brief LoadingAnimationWidget构造函数
 * @param parent 父窗口
 */
LoadingAnimationWidget::LoadingAnimationWidget(QWidget *parent)
    : QWidget(parent)
    , m_mainLayout(nullptr)
    , m_logoLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_progressBar(nullptr)
    , m_fadeInAnimation(nullptr)
    , m_fadeOutAnimation(nullptr)
    , m_rotationAnimation(nullptr)
    , m_pulseAnimation(nullptr)
    , m_animationGroup(nullptr)
    , m_animationTimer(nullptr)
    , m_opacityEffect(nullptr)
    , m_currentMessage("正在加载会议...")
    , m_logoPath()
    , m_logoPixmap()
    , m_currentProgress(0)
    , m_opacity(0.0)
    , m_rotationAngle(0.0)
    , m_pulseScale(1.0)
    , m_isVisible(false)
    , m_animationRunning(false)
    , m_primaryColor(QColor(0, 122, 255))      // 蓝色主题
    , m_secondaryColor(QColor(52, 199, 89))    // 绿色辅助
    , m_backgroundColor(QColor(28, 28, 30))    // 深色背景
    , m_textColor(QColor(255, 255, 255))       // 白色文本
{
    // 设置组件属性（嵌入式模式）
    setFocusPolicy(Qt::StrongFocus);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    
    // 设置最小尺寸
    setMinimumSize(300, 200);
    
    // 初始化UI和动画
    initializeUI();
    initializeAnimations();
    
    // 设置默认Logo
    setLogo(":/icons/app.svg");
    
    qDebug() << "LoadingAnimationWidget: 启动动画组件初始化完成";
}

/**
 * @brief LoadingAnimationWidget析构函数
 */
LoadingAnimationWidget::~LoadingAnimationWidget()
{
    // 停止所有动画
    stopRotationAnimation();
    stopPulseAnimation();
    
    if (m_animationGroup && m_animationGroup->state() == QAbstractAnimation::Running) {
        m_animationGroup->stop();
    }
    
    qDebug() << "LoadingAnimationWidget: 启动动画组件已销毁";
}

/**
 * @brief 初始化UI组件
 */
void LoadingAnimationWidget::initializeUI()
{
    // 创建主布局
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(50, 50, 50, 50);
    m_mainLayout->setSpacing(30);
    m_mainLayout->setAlignment(Qt::AlignCenter);
    
    // 创建Logo标签
    m_logoLabel = new QLabel(this);
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_logoLabel->setFixedSize(LOGO_MAX_SIZE, LOGO_MAX_SIZE);
    m_logoLabel->setScaledContents(true);
    
    // 创建状态标签
    m_statusLabel = new QLabel(m_currentMessage, this);
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setWordWrap(true);
    
    // 设置状态标签样式
    QFont statusFont = m_statusLabel->font();
    statusFont.setPointSize(16);
    statusFont.setWeight(QFont::Medium);
    m_statusLabel->setFont(statusFont);
    m_statusLabel->setStyleSheet(QString("QLabel { color: %1; }").arg(m_textColor.name()));
    
    // 创建进度条
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setFixedHeight(PROGRESS_HEIGHT);
    m_progressBar->setTextVisible(false);
    
    // 设置进度条样式
    QString progressStyle = QString(
        "QProgressBar {"
        "    border: none;"
        "    border-radius: %1px;"
        "    background-color: rgba(255, 255, 255, 0.2);"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: %1px;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 %2, stop:1 %3);"
        "}"
    ).arg(PROGRESS_HEIGHT / 2).arg(m_primaryColor.name()).arg(m_secondaryColor.name());
    
    m_progressBar->setStyleSheet(progressStyle);
    
    // 添加到布局
    m_mainLayout->addStretch(2);
    m_mainLayout->addWidget(m_logoLabel);
    m_mainLayout->addSpacing(40);
    m_mainLayout->addWidget(m_statusLabel);
    m_mainLayout->addSpacing(20);
    m_mainLayout->addWidget(m_progressBar);
    m_mainLayout->addStretch(3);
    
    // 创建透明度效果
    m_opacityEffect = new QGraphicsOpacityEffect(this);
    m_opacityEffect->setOpacity(0.0);
    setGraphicsEffect(m_opacityEffect);
    
    // 计算布局
    calculateLayout();
}

/**
 * @brief 初始化动画
 */
void LoadingAnimationWidget::initializeAnimations()
{
    // 创建淡入动画
    m_fadeInAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeInAnimation->setDuration(ANIMATION_DURATION);
    m_fadeInAnimation->setStartValue(0.0);
    m_fadeInAnimation->setEndValue(1.0);
    m_fadeInAnimation->setEasingCurve(QEasingCurve::OutCubic);
    
    // 创建淡出动画
    m_fadeOutAnimation = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeOutAnimation->setDuration(ANIMATION_DURATION);
    m_fadeOutAnimation->setStartValue(1.0);
    m_fadeOutAnimation->setEndValue(0.0);
    m_fadeOutAnimation->setEasingCurve(QEasingCurve::InCubic);
    
    // 创建旋转动画
    m_rotationAnimation = new QPropertyAnimation(this, "rotationAngle", this);
    m_rotationAnimation->setDuration(ROTATION_DURATION);
    m_rotationAnimation->setStartValue(0.0);
    m_rotationAnimation->setEndValue(360.0);
    m_rotationAnimation->setLoopCount(-1); // 无限循环
    m_rotationAnimation->setEasingCurve(QEasingCurve::Linear);
    
    // 创建脉冲动画
    m_pulseAnimation = new QPropertyAnimation(this, "pulseScale", this);
    m_pulseAnimation->setDuration(PULSE_DURATION);
    m_pulseAnimation->setStartValue(0.8);
    m_pulseAnimation->setEndValue(1.2);
    m_pulseAnimation->setLoopCount(-1); // 无限循环
    m_pulseAnimation->setEasingCurve(QEasingCurve::InOutSine);
    
    // 创建动画组
    m_animationGroup = new QParallelAnimationGroup(this);
    
    // 创建动画定时器
    m_animationTimer = new QTimer(this);
    m_animationTimer->setInterval(16); // 约60FPS
    
    // 连接信号
    connect(m_fadeInAnimation, &QPropertyAnimation::finished, this, &LoadingAnimationWidget::onFadeInFinished);
    connect(m_fadeOutAnimation, &QPropertyAnimation::finished, this, &LoadingAnimationWidget::onFadeOutFinished);
    connect(m_animationTimer, &QTimer::timeout, this, &LoadingAnimationWidget::onAnimationUpdate);
    connect(m_rotationAnimation, &QPropertyAnimation::valueChanged, this, &LoadingAnimationWidget::onRotationUpdate);
    connect(m_pulseAnimation, &QPropertyAnimation::valueChanged, this, &LoadingAnimationWidget::onPulseUpdate);
}

/**
 * @brief 显示启动动画
 * @param message 初始加载消息
 */
void LoadingAnimationWidget::showAnimation(const QString& message)
{
    if (m_isVisible) {
        return;
    }
    
    qDebug() << "LoadingAnimationWidget: 显示启动动画:" << message;
    
    // 更新消息
    if (!message.isEmpty()) {
        updateMessage(message);
    }
    
    // 重置状态
    m_currentProgress = 0;
    m_progressBar->setValue(0);
    m_rotationAngle = 0.0;
    m_pulseScale = 1.0;
    
    // 显示组件（嵌入式模式）
    show();
    setVisible(true);
    
    // 启动淡入动画
    m_isVisible = true;
    m_animationRunning = true;
    m_fadeInAnimation->start();
    
    // 启动循环动画
    startRotationAnimation();
    startPulseAnimation();
    m_animationTimer->start();
}

/**
 * @brief 隐藏启动动画
 * @param fadeOut 是否使用淡出效果
 */
void LoadingAnimationWidget::hideAnimation(bool fadeOut)
{
    if (!m_isVisible) {
        return;
    }
    
    qDebug() << "LoadingAnimationWidget: 隐藏启动动画, 淡出:" << fadeOut;
    
    // 停止循环动画
    stopRotationAnimation();
    stopPulseAnimation();
    m_animationTimer->stop();
    
    m_animationRunning = false;
    
    if (fadeOut) {
        // 启动淡出动画
        m_fadeOutAnimation->start();
    } else {
        // 直接隐藏（嵌入式模式）
        m_isVisible = false;
        setVisible(false);
        emit animationHidden();
    }
}

/**
 * @brief 更新加载进度
 * @param progress 进度值 (0-100)
 */
void LoadingAnimationWidget::updateProgress(int progress)
{
    progress = qBound(0, progress, 100);
    
    if (m_currentProgress != progress) {
        m_currentProgress = progress;
        m_progressBar->setValue(progress);
        
        qDebug() << "LoadingAnimationWidget: 更新进度:" << progress << "%";
        
        // 根据进度更新消息
        if (progress >= 90) {
            updateMessage("即将完成...");
        } else if (progress >= 70) {
            updateMessage("正在建立连接...");
        } else if (progress >= 50) {
            updateMessage("正在加载资源...");
        } else if (progress >= 30) {
            updateMessage("正在初始化...");
        }
        
        update(); // 触发重绘
    }
}

/**
 * @brief 更新状态消息
 * @param message 状态消息
 */
void LoadingAnimationWidget::updateMessage(const QString& message)
{
    if (m_currentMessage != message) {
        m_currentMessage = message;
        m_statusLabel->setText(message);
        
        qDebug() << "LoadingAnimationWidget: 更新消息:" << message;
        
        update(); // 触发重绘
    }
}

/**
 * @brief 设置品牌Logo
 * @param logoPath Logo图片路径
 */
void LoadingAnimationWidget::setLogo(const QString& logoPath)
{
    m_logoPath = logoPath;
    
    if (!logoPath.isEmpty()) {
        m_logoPixmap = QPixmap(logoPath);
        if (!m_logoPixmap.isNull()) {
            // 缩放Logo到合适大小
            m_logoPixmap = m_logoPixmap.scaled(LOGO_MAX_SIZE, LOGO_MAX_SIZE, 
                                             Qt::KeepAspectRatio, Qt::SmoothTransformation);
            m_logoLabel->setPixmap(m_logoPixmap);
            
            qDebug() << "LoadingAnimationWidget: 设置Logo:" << logoPath;
        } else {
            qWarning() << "LoadingAnimationWidget: 无法加载Logo:" << logoPath;
        }
    }
}

/**
 * @brief 设置主题颜色
 * @param primaryColor 主色调
 * @param secondaryColor 辅助色调
 */
void LoadingAnimationWidget::setThemeColors(const QColor& primaryColor, const QColor& secondaryColor)
{
    m_primaryColor = primaryColor;
    m_secondaryColor = secondaryColor;
    
    // 更新进度条样式
    QString progressStyle = QString(
        "QProgressBar {"
        "    border: none;"
        "    border-radius: %1px;"
        "    background-color: rgba(255, 255, 255, 0.2);"
        "}"
        "QProgressBar::chunk {"
        "    border-radius: %1px;"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:0, "
        "        stop:0 %2, stop:1 %3);"
        "}"
    ).arg(PROGRESS_HEIGHT / 2).arg(primaryColor.name()).arg(secondaryColor.name());
    
    m_progressBar->setStyleSheet(progressStyle);
    
    qDebug() << "LoadingAnimationWidget: 设置主题颜色:" << primaryColor.name() << secondaryColor.name();
    
    update(); // 触发重绘
}

/**
 * @brief 设置透明度属性
 * @param opacity 透明度值 (0.0-1.0)
 */
void LoadingAnimationWidget::setOpacity(qreal opacity)
{
    m_opacity = qBound(0.0, opacity, 1.0);
    update();
}

/**
 * @brief 设置旋转角度属性
 * @param angle 旋转角度
 */
void LoadingAnimationWidget::setRotationAngle(qreal angle)
{
    m_rotationAngle = angle;
    update();
}

/**
 * @brief 设置脉冲缩放属性
 * @param scale 缩放值
 */
void LoadingAnimationWidget::setPulseScale(qreal scale)
{
    m_pulseScale = qBound(0.5, scale, 2.0);
    update();
}

/**
 * @brief 启动旋转动画
 */
void LoadingAnimationWidget::startRotationAnimation()
{
    if (m_rotationAnimation && m_rotationAnimation->state() != QAbstractAnimation::Running) {
        m_rotationAnimation->start();
    }
}

/**
 * @brief 停止旋转动画
 */
void LoadingAnimationWidget::stopRotationAnimation()
{
    if (m_rotationAnimation && m_rotationAnimation->state() == QAbstractAnimation::Running) {
        m_rotationAnimation->stop();
    }
}

/**
 * @brief 启动脉冲动画
 */
void LoadingAnimationWidget::startPulseAnimation()
{
    if (m_pulseAnimation && m_pulseAnimation->state() != QAbstractAnimation::Running) {
        m_pulseAnimation->start();
    }
}

/**
 * @brief 停止脉冲动画
 */
void LoadingAnimationWidget::stopPulseAnimation()
{
    if (m_pulseAnimation && m_pulseAnimation->state() == QAbstractAnimation::Running) {
        m_pulseAnimation->stop();
    }
}

/**
 * @brief 计算布局位置
 */
void LoadingAnimationWidget::calculateLayout()
{
    QRect clientRect = rect();
    int centerX = clientRect.width() / 2;
    int centerY = clientRect.height() / 2;
    
    // 根据容器大小动态调整Logo尺寸
    int logoSize = qMin(qMin(clientRect.width(), clientRect.height()) / 4, LOGO_MAX_SIZE);
    logoSize = qMax(logoSize, 40); // 最小40像素
    
    // 根据容器大小动态调整指示器尺寸
    int indicatorSize = qMin(qMin(clientRect.width(), clientRect.height()) / 8, INDICATOR_SIZE);
    indicatorSize = qMax(indicatorSize, 30); // 最小30像素
    
    // 计算垂直间距
    int verticalSpacing = qMax(clientRect.height() / 10, 20);
    
    // Logo区域 - 位于上方
    int logoY = centerY - verticalSpacing - logoSize / 2;
    m_logoRect = QRect(centerX - logoSize / 2, logoY, logoSize, logoSize);
    
    // 指示器区域 - 位于中心
    m_indicatorRect = QRect(centerX - indicatorSize / 2, centerY - indicatorSize / 2, indicatorSize, indicatorSize);
    
    // 进度条区域 - 位于指示器下方
    int progressWidth = qMin(qMax(clientRect.width() * 0.6, 200.0), 400.0);
    int progressY = centerY + indicatorSize / 2 + verticalSpacing / 2;
    m_progressRect = QRect(centerX - progressWidth / 2, progressY, progressWidth, PROGRESS_HEIGHT);
    
    // 文本区域 - 位于进度条下方
    int textY = progressY + PROGRESS_HEIGHT + 20;
    int textHeight = qMax(clientRect.height() / 8, 40);
    m_textRect = QRect(20, textY, clientRect.width() - 40, textHeight);
}

/**
 * @brief 绘制事件
 * @param event 绘制事件
 */
void LoadingAnimationWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)
    
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 绘制背景
    drawBackground(&painter);
    
    // 绘制加载指示器
    drawLoadingIndicator(&painter);
}

/**
 * @brief 绘制背景
 * @param painter 绘制器
 */
void LoadingAnimationWidget::drawBackground(QPainter* painter)
{
    // 绘制渐变背景
    QLinearGradient gradient(0, 0, 0, height());
    gradient.setColorAt(0, m_backgroundColor);
    gradient.setColorAt(1, m_backgroundColor.darker(120));
    
    painter->fillRect(rect(), gradient);
    
    // 绘制装饰性圆圈
    painter->save();
    painter->setPen(Qt::NoPen);
    
    // 大圆圈
    QRadialGradient radialGradient(width() * 0.3, height() * 0.7, width() * 0.4);
    radialGradient.setColorAt(0, QColor(m_primaryColor.red(), m_primaryColor.green(), m_primaryColor.blue(), 30));
    radialGradient.setColorAt(1, Qt::transparent);
    painter->setBrush(radialGradient);
    painter->drawEllipse(QPointF(width() * 0.3, height() * 0.7), width() * 0.4, width() * 0.4);
    
    // 小圆圈
    QRadialGradient smallGradient(width() * 0.8, height() * 0.2, width() * 0.2);
    smallGradient.setColorAt(0, QColor(m_secondaryColor.red(), m_secondaryColor.green(), m_secondaryColor.blue(), 20));
    smallGradient.setColorAt(1, Qt::transparent);
    painter->setBrush(smallGradient);
    painter->drawEllipse(QPointF(width() * 0.8, height() * 0.2), width() * 0.2, width() * 0.2);
    
    painter->restore();
}

/**
 * @brief 绘制加载指示器
 * @param painter 绘制器
 */
void LoadingAnimationWidget::drawLoadingIndicator(QPainter* painter)
{
    if (!m_animationRunning) {
        return;
    }
    
    painter->save();
    
    // 计算中心位置
    QPointF center(width() / 2.0, height() / 2.0 + 20);
    
    // 应用脉冲缩放
    painter->translate(center);
    painter->scale(m_pulseScale, m_pulseScale);
    painter->rotate(m_rotationAngle);
    
    // 绘制旋转指示器
    int dotCount = 8;
    qreal dotRadius = 4;
    qreal circleRadius = INDICATOR_SIZE / 2.0 - dotRadius;
    
    for (int i = 0; i < dotCount; ++i) {
        qreal angle = (360.0 / dotCount) * i;
        qreal radian = qDegreesToRadians(angle);
        
        QPointF dotPos(circleRadius * qCos(radian), circleRadius * qSin(radian));
        
        // 计算透明度（创建拖尾效果）
        qreal alpha = 1.0 - (i / static_cast<qreal>(dotCount));
        QColor dotColor = m_primaryColor;
        dotColor.setAlphaF(alpha * 0.8);
        
        painter->setBrush(dotColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(dotPos, dotRadius, dotRadius);
    }
    
    painter->restore();
}

/**
 * @brief 鼠标按下事件
 * @param event 鼠标事件
 */
void LoadingAnimationWidget::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event)
    // 阻止鼠标事件传递，但不做任何处理
}

/**
 * @brief 键盘按下事件
 * @param event 键盘事件
 */
void LoadingAnimationWidget::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        // ESC键取消加载
        qDebug() << "LoadingAnimationWidget: 用户按ESC键取消加载";
        emit loadingCancelled();
    }
    
    QWidget::keyPressEvent(event);
}

/**
 * @brief 窗口大小改变事件
 * @param event 大小改变事件
 */
void LoadingAnimationWidget::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    calculateLayout();
}

/**
 * @brief 动画更新槽函数
 */
void LoadingAnimationWidget::onAnimationUpdate()
{
    update(); // 触发重绘
}

/**
 * @brief 淡入动画完成槽函数
 */
void LoadingAnimationWidget::onFadeInFinished()
{
    qDebug() << "LoadingAnimationWidget: 淡入动画完成";
    emit animationShown();
}

/**
 * @brief 淡出动画完成槽函数
 */
void LoadingAnimationWidget::onFadeOutFinished()
{
    m_isVisible = false;
    m_animationRunning = false;
    setVisible(false);
    emit animationHidden();
    
    qDebug() << "LoadingAnimationWidget: 淡出动画完成";
}

/**
 * @brief 旋转动画更新槽函数
 */
void LoadingAnimationWidget::onRotationUpdate()
{
    update(); // 触发重绘
}

/**
 * @brief 脉冲动画更新槽函数
 */
void LoadingAnimationWidget::onPulseUpdate()
{
    update(); // 触发重绘
}
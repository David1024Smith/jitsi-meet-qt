#include "VolumeSliderWidget.h"
#include <QWheelEvent>
#include <QKeyEvent>
#include <QtMath>
#include <QApplication>
#include <QStyle>

// 静态常量定义
const qreal VolumeSliderWidget::DEFAULT_VOLUME = 0.7;
const qreal VolumeSliderWidget::DEFAULT_STEP = 0.05;

VolumeSliderWidget::VolumeSliderWidget(Orientation orientation, QWidget *parent)
    : QWidget(parent)
    , m_horizontalLayout(nullptr)
    , m_verticalLayout(nullptr)
    , m_volumeSlider(nullptr)
    , m_muteButton(nullptr)
    , m_valueLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_levelIndicator(nullptr)
    , m_orientation(orientation)
    , m_displayMode(Percentage)
    , m_volume(DEFAULT_VOLUME)
    , m_minimumVolume(0.0)
    , m_maximumVolume(1.0)
    , m_volumeStep(DEFAULT_STEP)
    , m_audioLevel(0.0)
    , m_muted(false)
    , m_showMuteButton(true)
    , m_showValueLabel(true)
    , m_showLevelIndicator(false)
    , m_volumeBeforeMute(DEFAULT_VOLUME)
    , m_isAdjusting(false)
    , m_levelUpdateTimer(new QTimer(this))
{
    initializeUI();
    connectSignals();
    
    // 设置定时器
    m_levelUpdateTimer->setInterval(50); // 20 FPS更新
    connect(m_levelUpdateTimer, &QTimer::timeout, this, &VolumeSliderWidget::updateLevelIndicator);
}

VolumeSliderWidget::~VolumeSliderWidget()
{
    if (m_levelUpdateTimer) {
        m_levelUpdateTimer->stop();
    }
}

void VolumeSliderWidget::setVolume(qreal volume)
{
    qreal clampedVolume = qBound(m_minimumVolume, volume, m_maximumVolume);
    if (qFuzzyCompare(m_volume, clampedVolume)) {
        return;
    }

    m_volume = clampedVolume;
    
    // 更新滑块值
    if (m_volumeSlider) {
        m_volumeSlider->blockSignals(true);
        m_volumeSlider->setValue(volumeToSliderValue(m_volume));
        m_volumeSlider->blockSignals(false);
    }
    
    updateValueLabel();
    
    if (!m_isAdjusting) {
        emit volumeChanged(m_volume);
    }
}

qreal VolumeSliderWidget::volume() const
{
    return m_volume;
}

void VolumeSliderWidget::setMuted(bool muted)
{
    if (m_muted == muted) {
        return;
    }

    if (muted && !m_muted) {
        // 保存当前音量
        m_volumeBeforeMute = m_volume;
    } else if (!muted && m_muted) {
        // 恢复之前的音量
        setVolume(m_volumeBeforeMute);
    }

    m_muted = muted;
    updateMuteButton();
    updateValueLabel();
    
    emit muteChanged(m_muted);
}

bool VolumeSliderWidget::isMuted() const
{
    return m_muted;
}

void VolumeSliderWidget::setDisplayMode(DisplayMode mode)
{
    if (m_displayMode == mode) {
        return;
    }

    m_displayMode = mode;
    updateValueLabel();
}

VolumeSliderWidget::DisplayMode VolumeSliderWidget::displayMode() const
{
    return m_displayMode;
}

void VolumeSliderWidget::setOrientation(Orientation orientation)
{
    if (m_orientation == orientation) {
        return;
    }

    m_orientation = orientation;
    
    // 重新创建布局
    if (m_orientation == Horizontal) {
        createHorizontalLayout();
    } else {
        createVerticalLayout();
    }
}

VolumeSliderWidget::Orientation VolumeSliderWidget::orientation() const
{
    return m_orientation;
}

void VolumeSliderWidget::setShowMuteButton(bool show)
{
    if (m_showMuteButton == show) {
        return;
    }

    m_showMuteButton = show;
    if (m_muteButton) {
        m_muteButton->setVisible(show);
    }
}

bool VolumeSliderWidget::showMuteButton() const
{
    return m_showMuteButton;
}

void VolumeSliderWidget::setShowValueLabel(bool show)
{
    if (m_showValueLabel == show) {
        return;
    }

    m_showValueLabel = show;
    if (m_valueLabel) {
        m_valueLabel->setVisible(show);
    }
}

bool VolumeSliderWidget::showValueLabel() const
{
    return m_showValueLabel;
}

void VolumeSliderWidget::setShowLevelIndicator(bool show)
{
    if (m_showLevelIndicator == show) {
        return;
    }

    m_showLevelIndicator = show;
    if (m_levelIndicator) {
        m_levelIndicator->setVisible(show);
        if (show) {
            m_levelUpdateTimer->start();
        } else {
            m_levelUpdateTimer->stop();
        }
    }
}

bool VolumeSliderWidget::showLevelIndicator() const
{
    return m_showLevelIndicator;
}

void VolumeSliderWidget::setVolumeRange(qreal minimum, qreal maximum)
{
    if (minimum >= maximum || minimum < 0.0 || maximum > 1.0) {
        return;
    }

    m_minimumVolume = minimum;
    m_maximumVolume = maximum;
    
    // 确保当前音量在新范围内
    setVolume(m_volume);
}

qreal VolumeSliderWidget::minimumVolume() const
{
    return m_minimumVolume;
}

qreal VolumeSliderWidget::maximumVolume() const
{
    return m_maximumVolume;
}

void VolumeSliderWidget::setVolumeStep(qreal step)
{
    if (step > 0.0 && step <= 1.0) {
        m_volumeStep = step;
    }
}

qreal VolumeSliderWidget::volumeStep() const
{
    return m_volumeStep;
}

void VolumeSliderWidget::setAudioLevel(qreal level)
{
    m_audioLevel = qBound(0.0, level, 1.0);
}

qreal VolumeSliderWidget::audioLevel() const
{
    return m_audioLevel;
}

void VolumeSliderWidget::setLabelText(const QString &text)
{
    m_labelText = text;
    if (m_titleLabel) {
        m_titleLabel->setText(text);
        m_titleLabel->setVisible(!text.isEmpty());
    }
}

QString VolumeSliderWidget::labelText() const
{
    return m_labelText;
}

void VolumeSliderWidget::setEnabled(bool enabled)
{
    QWidget::setEnabled(enabled);
    
    if (m_volumeSlider) {
        m_volumeSlider->setEnabled(enabled);
    }
    if (m_muteButton) {
        m_muteButton->setEnabled(enabled);
    }
}

void VolumeSliderWidget::increaseVolume()
{
    setVolume(m_volume + m_volumeStep);
}

void VolumeSliderWidget::decreaseVolume()
{
    setVolume(m_volume - m_volumeStep);
}

void VolumeSliderWidget::toggleMute()
{
    setMuted(!m_muted);
}

void VolumeSliderWidget::resetToDefault()
{
    setVolume(DEFAULT_VOLUME);
    setMuted(false);
}

void VolumeSliderWidget::wheelEvent(QWheelEvent *event)
{
    if (!isEnabled()) {
        QWidget::wheelEvent(event);
        return;
    }

    int delta = event->angleDelta().y();
    if (delta > 0) {
        increaseVolume();
    } else if (delta < 0) {
        decreaseVolume();
    }

    event->accept();
}

void VolumeSliderWidget::keyPressEvent(QKeyEvent *event)
{
    if (!isEnabled()) {
        QWidget::keyPressEvent(event);
        return;
    }

    switch (event->key()) {
    case Qt::Key_Up:
    case Qt::Key_Right:
        increaseVolume();
        event->accept();
        break;
    case Qt::Key_Down:
    case Qt::Key_Left:
        decreaseVolume();
        event->accept();
        break;
    case Qt::Key_Space:
    case Qt::Key_M:
        toggleMute();
        event->accept();
        break;
    case Qt::Key_Home:
        setVolume(m_maximumVolume);
        event->accept();
        break;
    case Qt::Key_End:
        setVolume(m_minimumVolume);
        event->accept();
        break;
    default:
        QWidget::keyPressEvent(event);
        break;
    }
}

QSize VolumeSliderWidget::sizeHint() const
{
    if (m_orientation == Horizontal) {
        return QSize(200, 60);
    } else {
        return QSize(60, 200);
    }
}

QSize VolumeSliderWidget::minimumSizeHint() const
{
    if (m_orientation == Horizontal) {
        return QSize(100, 30);
    } else {
        return QSize(30, 100);
    }
}

void VolumeSliderWidget::onSliderValueChanged(int value)
{
    qreal newVolume = sliderValueToVolume(value);
    if (!qFuzzyCompare(m_volume, newVolume)) {
        m_volume = newVolume;
        updateValueLabel();
        
        if (!m_isAdjusting) {
            emit volumeChanged(m_volume);
        }
    }
}

void VolumeSliderWidget::onSliderPressed()
{
    m_isAdjusting = true;
    emit volumeAdjustmentStarted();
}

void VolumeSliderWidget::onSliderReleased()
{
    m_isAdjusting = false;
    emit volumeAdjustmentFinished();
    emit volumeChanged(m_volume);
}

void VolumeSliderWidget::onMuteButtonClicked(bool checked)
{
    Q_UNUSED(checked)
    toggleMute();
}

void VolumeSliderWidget::updateLevelIndicator()
{
    if (m_levelIndicator && m_showLevelIndicator) {
        int levelValue = static_cast<int>(m_audioLevel * 100);
        m_levelIndicator->setValue(levelValue);
    }
}

void VolumeSliderWidget::initializeUI()
{
    setFocusPolicy(Qt::StrongFocus);
    
    // 创建控件
    m_volumeSlider = new QSlider(this);
    m_volumeSlider->setRange(0, SLIDER_RANGE);
    m_volumeSlider->setValue(volumeToSliderValue(m_volume));
    m_volumeSlider->setFocusPolicy(Qt::StrongFocus);
    
    m_muteButton = new QPushButton(this);
    m_muteButton->setCheckable(true);
    m_muteButton->setFixedSize(24, 24);
    m_muteButton->setFocusPolicy(Qt::NoFocus);
    
    m_valueLabel = new QLabel(this);
    m_valueLabel->setAlignment(Qt::AlignCenter);
    m_valueLabel->setMinimumWidth(50);
    
    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setVisible(!m_labelText.isEmpty());
    
    m_levelIndicator = new QProgressBar(this);
    m_levelIndicator->setRange(0, 100);
    m_levelIndicator->setValue(0);
    m_levelIndicator->setTextVisible(false);
    m_levelIndicator->setVisible(m_showLevelIndicator);
    
    // 设置样式
    updateMuteButton();
    updateValueLabel();
    
    // 创建布局
    if (m_orientation == Horizontal) {
        createHorizontalLayout();
    } else {
        createVerticalLayout();
    }
}

void VolumeSliderWidget::createHorizontalLayout()
{
    // 清理旧布局
    if (layout()) {
        delete layout();
    }

    m_horizontalLayout = new QHBoxLayout(this);
    m_horizontalLayout->setContentsMargins(2, 2, 2, 2);
    m_horizontalLayout->setSpacing(4);
    
    // 设置滑块方向
    m_volumeSlider->setOrientation(Qt::Horizontal);
    m_levelIndicator->setOrientation(Qt::Horizontal);
    
    // 添加控件
    if (!m_labelText.isEmpty()) {
        m_horizontalLayout->addWidget(m_titleLabel);
    }
    
    if (m_showMuteButton) {
        m_horizontalLayout->addWidget(m_muteButton);
    }
    
    m_horizontalLayout->addWidget(m_volumeSlider, 1);
    
    if (m_showValueLabel) {
        m_horizontalLayout->addWidget(m_valueLabel);
    }
    
    if (m_showLevelIndicator) {
        m_horizontalLayout->addWidget(m_levelIndicator);
    }
}

void VolumeSliderWidget::createVerticalLayout()
{
    // 清理旧布局
    if (layout()) {
        delete layout();
    }

    m_verticalLayout = new QVBoxLayout(this);
    m_verticalLayout->setContentsMargins(2, 2, 2, 2);
    m_verticalLayout->setSpacing(4);
    
    // 设置滑块方向
    m_volumeSlider->setOrientation(Qt::Vertical);
    m_levelIndicator->setOrientation(Qt::Vertical);
    
    // 添加控件
    if (!m_labelText.isEmpty()) {
        m_verticalLayout->addWidget(m_titleLabel);
    }
    
    if (m_showValueLabel) {
        m_verticalLayout->addWidget(m_valueLabel);
    }
    
    m_verticalLayout->addWidget(m_volumeSlider, 1);
    
    if (m_showLevelIndicator) {
        m_verticalLayout->addWidget(m_levelIndicator);
    }
    
    if (m_showMuteButton) {
        m_verticalLayout->addWidget(m_muteButton);
    }
}

void VolumeSliderWidget::connectSignals()
{
    connect(m_volumeSlider, &QSlider::valueChanged, this, &VolumeSliderWidget::onSliderValueChanged);
    connect(m_volumeSlider, &QSlider::sliderPressed, this, &VolumeSliderWidget::onSliderPressed);
    connect(m_volumeSlider, &QSlider::sliderReleased, this, &VolumeSliderWidget::onSliderReleased);
    connect(m_muteButton, &QPushButton::clicked, this, &VolumeSliderWidget::onMuteButtonClicked);
}

void VolumeSliderWidget::updateValueLabel()
{
    if (!m_valueLabel) {
        return;
    }

    QString text;
    if (m_muted) {
        text = QStringLiteral("静音");
    } else {
        text = formatVolumeText(m_volume);
    }
    
    m_valueLabel->setText(text);
}

void VolumeSliderWidget::updateMuteButton()
{
    if (!m_muteButton) {
        return;
    }

    m_muteButton->setChecked(m_muted);
    
    // 设置图标和提示文本
    if (m_muted) {
        m_muteButton->setText("🔇");
        m_muteButton->setToolTip(QStringLiteral("取消静音"));
    } else {
        if (m_volume > 0.7) {
            m_muteButton->setText("🔊");
        } else if (m_volume > 0.3) {
            m_muteButton->setText("🔉");
        } else {
            m_muteButton->setText("🔈");
        }
        m_muteButton->setToolTip(QStringLiteral("静音"));
    }
}

int VolumeSliderWidget::volumeToSliderValue(qreal volume) const
{
    qreal normalizedVolume = (volume - m_minimumVolume) / (m_maximumVolume - m_minimumVolume);
    return static_cast<int>(normalizedVolume * SLIDER_RANGE);
}

qreal VolumeSliderWidget::sliderValueToVolume(int sliderValue) const
{
    qreal normalizedValue = static_cast<qreal>(sliderValue) / SLIDER_RANGE;
    return m_minimumVolume + normalizedValue * (m_maximumVolume - m_minimumVolume);
}

QString VolumeSliderWidget::formatVolumeText(qreal volume) const
{
    switch (m_displayMode) {
    case Percentage:
        return QStringLiteral("%1%").arg(static_cast<int>(volume * 100));
    case Decibel:
        return QStringLiteral("%1dB").arg(volumeToDecibel(volume), 0, 'f', 1);
    case Linear:
        return QStringLiteral("%1").arg(volume, 0, 'f', 2);
    }
    return QString();
}

qreal VolumeSliderWidget::volumeToDecibel(qreal volume) const
{
    if (volume <= 0.0) {
        return -60.0; // -∞的近似值
    }
    return 20.0 * qLn(volume) / qLn(10.0);
}

qreal VolumeSliderWidget::decibelToVolume(qreal decibel) const
{
    if (decibel <= -60.0) {
        return 0.0;
    }
    return qPow(10.0, decibel / 20.0);
}
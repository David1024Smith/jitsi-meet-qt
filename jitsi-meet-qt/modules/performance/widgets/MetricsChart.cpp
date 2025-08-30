#include "MetricsChart.h"
#include "PerformanceManager.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QDateTime>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QtCharts/QSplineSeries>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QBarCategoryAxis>
#include <cmath>

MetricsChart::MetricsChart(QWidget *parent)
    : QWidget(parent)
    , m_chartType(LineChart)
    , m_metricType(CPUUsage)
    , m_timeRange(Last5Minutes)
    , m_customTimeRange(QDateTime::currentDateTime().addSecs(-300), QDateTime::currentDateTime())
    , m_mainLayout(nullptr)
    , m_controlLayout(nullptr)
    , m_chart(nullptr)
    , m_chartView(nullptr)
    , m_timeRangeCombo(nullptr)
    , m_chartTypeCombo(nullptr)
    , m_maxPointsSpinBox(nullptr)
    , m_zoomFitButton(nullptr)
    , m_exportButton(nullptr)
    , m_lineSeries(nullptr)
    , m_areaSeries(nullptr)
    , m_barSeries(nullptr)
    , m_thresholdSeries(nullptr)
    , m_xAxis(nullptr)
    , m_yAxis(nullptr)
    , m_maxDataPoints(300)
    , m_thresholdValue(0.0)
    , m_hasThreshold(false)
    , m_updateTimer(new QTimer(this))
    , m_realTimeUpdateActive(false)
    , m_updateInterval(1000)
{
    initializeUI();
    createChart();
    
    // 连接定时器
    connect(m_updateTimer, &QTimer::timeout, this, &MetricsChart::handleRealTimeUpdate);
}

MetricsChart::~MetricsChart()
{
    stopRealTimeUpdate();
}

void MetricsChart::setChartType(ChartType type)
{
    if (m_chartType == type) {
        return;
    }
    
    m_chartType = type;
    applyChartType();
    
    if (m_chartTypeCombo) {
        m_chartTypeCombo->setCurrentIndex(static_cast<int>(type));
    }
    
    emit chartTypeChanged(type);
}

MetricsChart::ChartType MetricsChart::chartType() const
{
    return m_chartType;
}v
oid MetricsChart::setMetricType(MetricType type)
{
    if (m_metricType == type) {
        return;
    }
    
    m_metricType = type;
    
    // 更新图表标题和Y轴标签
    QString title, yLabel;
    switch (type) {
    case CPUUsage:
        title = "CPU Usage";
        yLabel = "Usage (%)";
        break;
    case MemoryUsage:
        title = "Memory Usage";
        yLabel = "Usage (MB)";
        break;
    case NetworkLatency:
        title = "Network Latency";
        yLabel = "Latency (ms)";
        break;
    case NetworkBandwidth:
        title = "Network Bandwidth";
        yLabel = "Bandwidth (Mbps)";
        break;
    case AudioLatency:
        title = "Audio Latency";
        yLabel = "Latency (ms)";
        break;
    case VideoFrameRate:
        title = "Video Frame Rate";
        yLabel = "Frame Rate (fps)";
        break;
    case SystemLoad:
        title = "System Load";
        yLabel = "Load";
        break;
    case CustomMetric:
        title = "Custom Metric";
        yLabel = "Value";
        break;
    }
    
    setChartTitle(title);
    setYAxisLabel(yLabel);
    
    // 清除现有数据
    clearData();
}

MetricsChart::MetricType MetricsChart::metricType() const
{
    return m_metricType;
}

void MetricsChart::setTimeRange(TimeRange range)
{
    if (m_timeRange == range) {
        return;
    }
    
    m_timeRange = range;
    applyTimeRange();
    
    if (m_timeRangeCombo) {
        m_timeRangeCombo->setCurrentIndex(static_cast<int>(range));
    }
    
    emit timeRangeChanged(range);
}

MetricsChart::TimeRange MetricsChart::timeRange() const
{
    return m_timeRange;
}

void MetricsChart::setCustomTimeRange(const QDateTime& from, const QDateTime& to)
{
    m_customTimeRange = qMakePair(from, to);
    
    if (m_timeRange == CustomRange) {
        applyTimeRange();
    }
}

QPair<QDateTime, QDateTime> MetricsChart::customTimeRange() const
{
    return m_customTimeRange;
}

void MetricsChart::setChartTitle(const QString& title)
{
    if (m_chart) {
        m_chart->setTitle(title);
    }
}

QString MetricsChart::chartTitle() const
{
    return m_chart ? m_chart->title() : QString();
}

void MetricsChart::setYAxisLabel(const QString& label)
{
    if (m_yAxis) {
        m_yAxis->setTitleText(label);
    }
}

QString MetricsChart::yAxisLabel() const
{
    return m_yAxis ? m_yAxis->titleText() : QString();
}void Metr
icsChart::setMaxDataPoints(int maxPoints)
{
    if (maxPoints < 10 || maxPoints > 10000) {
        qWarning() << "MetricsChart: Invalid max data points:" << maxPoints;
        return;
    }
    
    m_maxDataPoints = maxPoints;
    
    // 如果当前数据点超过限制，删除旧数据
    while (m_dataPoints.size() > m_maxDataPoints) {
        m_dataPoints.removeFirst();
    }
    
    updateChartData();
    
    if (m_maxPointsSpinBox) {
        m_maxPointsSpinBox->setValue(maxPoints);
    }
}

int MetricsChart::maxDataPoints() const
{
    return m_maxDataPoints;
}

void MetricsChart::setUpdateInterval(int interval)
{
    if (interval < 100 || interval > 60000) {
        qWarning() << "MetricsChart: Invalid update interval:" << interval;
        return;
    }
    
    m_updateInterval = interval;
    
    if (m_updateTimer->isActive()) {
        m_updateTimer->setInterval(m_updateInterval);
    }
}

int MetricsChart::updateInterval() const
{
    return m_updateInterval;
}

void MetricsChart::startRealTimeUpdate()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_realTimeUpdateActive) {
        return;
    }
    
    m_realTimeUpdateActive = true;
    m_updateTimer->start(m_updateInterval);
    
    qDebug() << "MetricsChart: Real-time update started for" << chartTitle();
}

void MetricsChart::stopRealTimeUpdate()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_realTimeUpdateActive) {
        return;
    }
    
    m_realTimeUpdateActive = false;
    m_updateTimer->stop();
    
    qDebug() << "MetricsChart: Real-time update stopped for" << chartTitle();
}

bool MetricsChart::isRealTimeUpdateActive() const
{
    QMutexLocker locker(&m_mutex);
    return m_realTimeUpdateActive;
}

void MetricsChart::addDataPoint(const QDateTime& timestamp, double value)
{
    QMutexLocker locker(&m_mutex);
    
    m_dataPoints.append(qMakePair(timestamp, value));
    
    // 限制数据点数量
    while (m_dataPoints.size() > m_maxDataPoints) {
        m_dataPoints.removeFirst();
    }
    
    // 如果正在实时更新，立即更新图表
    if (m_realTimeUpdateActive) {
        updateChartData();
    }
}

void MetricsChart::addDataPoints(const QList<QPair<QDateTime, double>>& dataPoints)
{
    QMutexLocker locker(&m_mutex);
    
    for (const auto& point : dataPoints) {
        m_dataPoints.append(point);
    }
    
    // 限制数据点数量
    while (m_dataPoints.size() > m_maxDataPoints) {
        m_dataPoints.removeFirst();
    }
    
    updateChartData();
}void M
etricsChart::setDataSeries(const QList<QPair<QDateTime, double>>& dataPoints)
{
    QMutexLocker locker(&m_mutex);
    
    m_dataPoints = dataPoints;
    
    // 限制数据点数量
    while (m_dataPoints.size() > m_maxDataPoints) {
        m_dataPoints.removeFirst();
    }
    
    updateChartData();
}

void MetricsChart::clearData()
{
    QMutexLocker locker(&m_mutex);
    
    m_dataPoints.clear();
    updateChartData();
}

void MetricsChart::refreshChart()
{
    updateChartData();
    configureAxes();
}

bool MetricsChart::exportChart(const QString& filePath, const QString& format)
{
    if (!m_chartView) {
        qWarning() << "MetricsChart: No chart view available for export";
        return false;
    }
    
    try {
        if (format.toLower() == "png" || format.toLower() == "jpg" || format.toLower() == "jpeg") {
            // 导出为图片
            QPixmap pixmap = m_chartView->grab();
            return pixmap.save(filePath, format.toUpper().toLocal8Bit().data());
        } else if (format.toLower() == "json") {
            // 导出为JSON数据
            QJsonObject exportData;
            exportData["chartTitle"] = chartTitle();
            exportData["yAxisLabel"] = yAxisLabel();
            exportData["metricType"] = static_cast<int>(m_metricType);
            exportData["chartType"] = static_cast<int>(m_chartType);
            exportData["timeRange"] = static_cast<int>(m_timeRange);
            exportData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
            
            // 导出数据点
            QJsonArray dataArray;
            for (const auto& point : m_dataPoints) {
                QJsonObject pointObj;
                pointObj["timestamp"] = point.first.toString(Qt::ISODate);
                pointObj["value"] = point.second;
                dataArray.append(pointObj);
            }
            exportData["dataPoints"] = dataArray;
            
            // 导出统计信息
            exportData["statistics"] = QJsonObject::fromVariantMap(getStatistics());
            
            QJsonDocument doc(exportData);
            
            QFile file(filePath);
            if (!file.open(QIODevice::WriteOnly)) {
                return false;
            }
            
            file.write(doc.toJson());
            file.close();
            return true;
        }
        
        qWarning() << "MetricsChart: Unsupported export format:" << format;
        return false;
        
    } catch (const std::exception& e) {
        qCritical() << "MetricsChart: Exception during export:" << e.what();
        return false;
    }
}

QVariantMap MetricsChart::getStatistics() const
{
    QMutexLocker locker(&m_mutex);
    return calculateStatistics();
}

void MetricsChart::setThresholdLine(double threshold, const QColor& color)
{
    m_thresholdValue = threshold;
    m_hasThreshold = true;
    
    // 移除现有阈值线
    if (m_thresholdSeries && m_chart) {
        m_chart->removeSeries(m_thresholdSeries);
        delete m_thresholdSeries;
    }
    
    // 创建新的阈值线
    m_thresholdSeries = new QLineSeries();
    m_thresholdSeries->setName("Threshold");
    
    QPen pen(color);
    pen.setStyle(Qt::DashLine);
    pen.setWidth(2);
    m_thresholdSeries->setPen(pen);
    
    // 添加阈值线数据点
    if (!m_dataPoints.isEmpty()) {
        QDateTime startTime = m_dataPoints.first().first;
        QDateTime endTime = m_dataPoints.last().first;
        
        m_thresholdSeries->append(startTime.toMSecsSinceEpoch(), threshold);
        m_thresholdSeries->append(endTime.toMSecsSinceEpoch(), threshold);
    }
    
    if (m_chart) {
        m_chart->addSeries(m_thresholdSeries);
        m_thresholdSeries->attachAxis(m_xAxis);
        m_thresholdSeries->attachAxis(m_yAxis);
    }
}

void MetricsChart::removeThresholdLine()
{
    m_hasThreshold = false;
    
    if (m_thresholdSeries && m_chart) {
        m_chart->removeSeries(m_thresholdSeries);
        delete m_thresholdSeries;
        m_thresholdSeries = nullptr;
    }
}vo
id MetricsChart::setChartTheme(QChart::ChartTheme theme)
{
    if (m_chart) {
        m_chart->setTheme(theme);
    }
}

QChart::ChartTheme MetricsChart::chartTheme() const
{
    return m_chart ? m_chart->theme() : QChart::ChartThemeLight;
}

void MetricsChart::updateMetrics(const PerformanceMetrics& metrics)
{
    double value = getMetricValue(metrics);
    QDateTime currentTime = QDateTime::currentDateTime();
    
    addDataPoint(currentTime, value);
}

void MetricsChart::zoomToFit()
{
    if (m_chartView) {
        m_chartView->chart()->zoomReset();
    }
}

void MetricsChart::resetZoom()
{
    zoomToFit();
}

void MetricsChart::zoomIn()
{
    if (m_chartView) {
        m_chartView->chart()->zoom(1.2);
    }
}

void MetricsChart::zoomOut()
{
    if (m_chartView) {
        m_chartView->chart()->zoom(0.8);
    }
}

void MetricsChart::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    
    // 调整图表大小
    if (m_chartView) {
        m_chartView->resize(event->size());
    }
}

void MetricsChart::mousePressEvent(QMouseEvent* event)
{
    QWidget::mousePressEvent(event);
    
    if (event->button() == Qt::LeftButton && m_chartView) {
        // 获取点击位置对应的数据点
        QPointF chartPoint = m_chartView->chart()->mapToValue(event->pos());
        QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(chartPoint.x()));
        double value = chartPoint.y();
        
        emit dataPointClicked(timestamp, value);
    }
}

void MetricsChart::mouseMoveEvent(QMouseEvent* event)
{
    QWidget::mouseMoveEvent(event);
    
    // 可以在这里实现鼠标悬停显示数据点信息的功能
}

void MetricsChart::wheelEvent(QWheelEvent* event)
{
    if (m_chartView) {
        // 实现鼠标滚轮缩放
        const qreal factor = 1.2;
        if (event->angleDelta().y() > 0) {
            m_chartView->chart()->zoom(factor);
        } else {
            m_chartView->chart()->zoom(1.0 / factor);
        }
    }
    
    QWidget::wheelEvent(event);
}

void MetricsChart::handleRealTimeUpdate()
{
    // 实时更新时刷新图表
    refreshChart();
}

void MetricsChart::handleTimeRangeChanged(int index)
{
    if (index >= 0 && index < static_cast<int>(CustomRange) + 1) {
        setTimeRange(static_cast<TimeRange>(index));
    }
}

void MetricsChart::handleChartTypeChanged(int index)
{
    if (index >= 0 && index < static_cast<int>(ScatterChart) + 1) {
        setChartType(static_cast<ChartType>(index));
    }
}

void MetricsChart::handleMaxDataPointsChanged(int maxPoints)
{
    setMaxDataPoints(maxPoints);
}void Met
ricsChart::initializeUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);
    
    // 创建控制面板
    QWidget* controlPanel = createControlPanel();
    m_mainLayout->addWidget(controlPanel);
    
    setLayout(m_mainLayout);
}

void MetricsChart::createChart()
{
    // 创建图表
    m_chart = new QChart();
    m_chart->setTitle("Performance Metrics");
    m_chart->setAnimationOptions(QChart::SeriesAnimations);
    
    // 创建图表视图
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(300);
    
    // 创建坐标轴
    m_xAxis = new QDateTimeAxis();
    m_xAxis->setFormat("hh:mm:ss");
    m_xAxis->setTitleText("Time");
    
    m_yAxis = new QValueAxis();
    m_yAxis->setTitleText("Value");
    
    m_chart->addAxis(m_xAxis, Qt::AlignBottom);
    m_chart->addAxis(m_yAxis, Qt::AlignLeft);
    
    // 应用图表类型
    applyChartType();
    
    // 配置坐标轴
    configureAxes();
    
    m_mainLayout->addWidget(m_chartView);
}

QWidget* MetricsChart::createControlPanel()
{
    QWidget* controlWidget = new QWidget();
    m_controlLayout = new QHBoxLayout(controlWidget);
    m_controlLayout->setContentsMargins(0, 0, 0, 0);
    
    // 时间范围选择
    m_controlLayout->addWidget(new QLabel("Time Range:"));
    m_timeRangeCombo = new QComboBox();
    m_timeRangeCombo->addItems({"1 Minute", "5 Minutes", "15 Minutes", "30 Minutes", 
                               "1 Hour", "6 Hours", "24 Hours", "Custom"});
    m_timeRangeCombo->setCurrentIndex(static_cast<int>(m_timeRange));
    connect(m_timeRangeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MetricsChart::handleTimeRangeChanged);
    m_controlLayout->addWidget(m_timeRangeCombo);
    
    m_controlLayout->addSpacing(20);
    
    // 图表类型选择
    m_controlLayout->addWidget(new QLabel("Chart Type:"));
    m_chartTypeCombo = new QComboBox();
    m_chartTypeCombo->addItems({"Line", "Area", "Bar", "Spline", "Scatter"});
    m_chartTypeCombo->setCurrentIndex(static_cast<int>(m_chartType));
    connect(m_chartTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MetricsChart::handleChartTypeChanged);
    m_controlLayout->addWidget(m_chartTypeCombo);
    
    m_controlLayout->addSpacing(20);
    
    // 最大数据点数选择
    m_controlLayout->addWidget(new QLabel("Max Points:"));
    m_maxPointsSpinBox = new QSpinBox();
    m_maxPointsSpinBox->setRange(10, 10000);
    m_maxPointsSpinBox->setValue(m_maxDataPoints);
    connect(m_maxPointsSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MetricsChart::handleMaxDataPointsChanged);
    m_controlLayout->addWidget(m_maxPointsSpinBox);
    
    m_controlLayout->addStretch();
    
    // 缩放适合按钮
    m_zoomFitButton = new QPushButton("Zoom to Fit");
    connect(m_zoomFitButton, &QPushButton::clicked, this, &MetricsChart::zoomToFit);
    m_controlLayout->addWidget(m_zoomFitButton);
    
    // 导出按钮
    m_exportButton = new QPushButton("Export");
    connect(m_exportButton, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Export Chart", 
                                                       "chart.png", "PNG Files (*.png);;JSON Files (*.json)");
        if (!fileName.isEmpty()) {
            QString format = fileName.endsWith(".json") ? "json" : "png";
            if (exportChart(fileName, format)) {
                QMessageBox::information(this, "Export", "Chart exported successfully!");
            } else {
                QMessageBox::warning(this, "Export", "Failed to export chart.");
            }
        }
    });
    m_controlLayout->addWidget(m_exportButton);
    
    return controlWidget;
}void Me
tricsChart::updateChartData()
{
    if (!m_chart) {
        return;
    }
    
    // 获取当前时间范围内的数据
    QList<QPair<QDateTime, double>> filteredData;
    if (m_timeRange == CustomRange) {
        filteredData = filterDataPoints(m_customTimeRange.first, m_customTimeRange.second);
    } else {
        QDateTime endTime = QDateTime::currentDateTime();
        QDateTime startTime = endTime.addMSecs(-getTimeRangeMilliseconds(m_timeRange));
        filteredData = filterDataPoints(startTime, endTime);
    }
    
    // 更新当前系列的数据
    if (m_lineSeries) {
        m_lineSeries->clear();
        for (const auto& point : filteredData) {
            m_lineSeries->append(point.first.toMSecsSinceEpoch(), point.second);
        }
    }
    
    if (m_areaSeries && m_areaSeries->upperSeries()) {
        m_areaSeries->upperSeries()->clear();
        for (const auto& point : filteredData) {
            m_areaSeries->upperSeries()->append(point.first.toMSecsSinceEpoch(), point.second);
        }
    }
    
    if (m_barSeries) {
        m_barSeries->clear();
        // 柱状图需要特殊处理
        if (!filteredData.isEmpty()) {
            QBarSet* barSet = new QBarSet("Values");
            QStringList categories;
            
            for (const auto& point : filteredData) {
                *barSet << point.second;
                categories << point.first.toString("hh:mm");
            }
            
            m_barSeries->append(barSet);
        }
    }
    
    // 更新阈值线
    if (m_hasThreshold && m_thresholdSeries && !filteredData.isEmpty()) {
        m_thresholdSeries->clear();
        QDateTime startTime = filteredData.first().first;
        QDateTime endTime = filteredData.last().first;
        
        m_thresholdSeries->append(startTime.toMSecsSinceEpoch(), m_thresholdValue);
        m_thresholdSeries->append(endTime.toMSecsSinceEpoch(), m_thresholdValue);
    }
    
    // 重新配置坐标轴
    configureAxes();
}

void MetricsChart::configureAxes()
{
    if (!m_chart || !m_xAxis || !m_yAxis) {
        return;
    }
    
    // 配置X轴（时间轴）
    QDateTime endTime = QDateTime::currentDateTime();
    QDateTime startTime;
    
    if (m_timeRange == CustomRange) {
        startTime = m_customTimeRange.first;
        endTime = m_customTimeRange.second;
    } else {
        startTime = endTime.addMSecs(-getTimeRangeMilliseconds(m_timeRange));
    }
    
    m_xAxis->setRange(startTime, endTime);
    
    // 根据时间范围调整时间格式
    qint64 rangeMSecs = startTime.msecsTo(endTime);
    if (rangeMSecs <= 60000) { // 1分钟以内
        m_xAxis->setFormat("hh:mm:ss");
    } else if (rangeMSecs <= 3600000) { // 1小时以内
        m_xAxis->setFormat("hh:mm");
    } else { // 超过1小时
        m_xAxis->setFormat("MM-dd hh:mm");
    }
    
    // 配置Y轴
    if (!m_dataPoints.isEmpty()) {
        double minValue = std::numeric_limits<double>::max();
        double maxValue = std::numeric_limits<double>::lowest();
        
        for (const auto& point : m_dataPoints) {
            minValue = std::min(minValue, point.second);
            maxValue = std::max(maxValue, point.second);
        }
        
        // 添加一些边距
        double range = maxValue - minValue;
        double margin = range * 0.1;
        
        m_yAxis->setRange(minValue - margin, maxValue + margin);
    } else {
        // 默认范围
        switch (m_metricType) {
        case CPUUsage:
            m_yAxis->setRange(0, 100);
            break;
        case MemoryUsage:
            m_yAxis->setRange(0, 8192); // 8GB
            break;
        case NetworkLatency:
            m_yAxis->setRange(0, 1000);
            break;
        case NetworkBandwidth:
            m_yAxis->setRange(0, 100);
            break;
        case AudioLatency:
            m_yAxis->setRange(0, 500);
            break;
        case VideoFrameRate:
            m_yAxis->setRange(0, 60);
            break;
        default:
            m_yAxis->setRange(0, 100);
            break;
        }
    }
}vo
id MetricsChart::applyChartType()
{
    if (!m_chart) {
        return;
    }
    
    // 移除现有系列
    m_chart->removeAllSeries();
    
    // 删除现有系列对象
    if (m_lineSeries) {
        delete m_lineSeries;
        m_lineSeries = nullptr;
    }
    if (m_areaSeries) {
        delete m_areaSeries;
        m_areaSeries = nullptr;
    }
    if (m_barSeries) {
        delete m_barSeries;
        m_barSeries = nullptr;
    }
    
    // 根据图表类型创建新系列
    switch (m_chartType) {
    case LineChart:
        m_lineSeries = new QLineSeries();
        m_lineSeries->setName("Values");
        m_chart->addSeries(m_lineSeries);
        m_lineSeries->attachAxis(m_xAxis);
        m_lineSeries->attachAxis(m_yAxis);
        break;
        
    case AreaChart: {
        QLineSeries* upperSeries = new QLineSeries();
        QLineSeries* lowerSeries = new QLineSeries();
        m_areaSeries = new QAreaSeries(upperSeries, lowerSeries);
        m_areaSeries->setName("Values");
        m_chart->addSeries(m_areaSeries);
        m_areaSeries->attachAxis(m_xAxis);
        m_areaSeries->attachAxis(m_yAxis);
        break;
    }
    
    case BarChart:
        m_barSeries = new QBarSeries();
        m_barSeries->setName("Values");
        m_chart->addSeries(m_barSeries);
        m_barSeries->attachAxis(m_xAxis);
        m_barSeries->attachAxis(m_yAxis);
        break;
        
    case SplineChart:
        m_lineSeries = new QSplineSeries();
        m_lineSeries->setName("Values");
        m_chart->addSeries(m_lineSeries);
        m_lineSeries->attachAxis(m_xAxis);
        m_lineSeries->attachAxis(m_yAxis);
        break;
        
    case ScatterChart: {
        QScatterSeries* scatterSeries = new QScatterSeries();
        scatterSeries->setName("Values");
        scatterSeries->setMarkerSize(8);
        m_chart->addSeries(scatterSeries);
        scatterSeries->attachAxis(m_xAxis);
        scatterSeries->attachAxis(m_yAxis);
        // 将散点图系列存储在线系列指针中以便统一处理
        m_lineSeries = scatterSeries;
        break;
    }
    }
    
    // 重新添加阈值线
    if (m_hasThreshold) {
        setThresholdLine(m_thresholdValue, Qt::red);
    }
    
    // 更新数据
    updateChartData();
}

void MetricsChart::applyTimeRange()
{
    updateChartData();
}

double MetricsChart::getMetricValue(const PerformanceMetrics& metrics) const
{
    switch (m_metricType) {
    case CPUUsage:
        return metrics.system.cpuUsage;
    case MemoryUsage:
        return metrics.system.memoryUsage;
    case NetworkLatency:
        return metrics.network.latency;
    case NetworkBandwidth:
        return metrics.network.bandwidth;
    case AudioLatency:
        return metrics.audio.latency;
    case VideoFrameRate:
        return metrics.video.frameRate;
    case SystemLoad:
        return metrics.system.cpuUsage; // 使用CPU使用率作为系统负载的近似值
    case CustomMetric:
    default:
        return 0.0;
    }
}

QString MetricsChart::getMetricUnit() const
{
    switch (m_metricType) {
    case CPUUsage:
        return "%";
    case MemoryUsage:
        return "MB";
    case NetworkLatency:
    case AudioLatency:
        return "ms";
    case NetworkBandwidth:
        return "Mbps";
    case VideoFrameRate:
        return "fps";
    case SystemLoad:
        return "";
    case CustomMetric:
    default:
        return "";
    }
}qi
nt64 MetricsChart::getTimeRangeMilliseconds(TimeRange range) const
{
    switch (range) {
    case Last1Minute:
        return 60 * 1000;
    case Last5Minutes:
        return 5 * 60 * 1000;
    case Last15Minutes:
        return 15 * 60 * 1000;
    case Last30Minutes:
        return 30 * 60 * 1000;
    case Last1Hour:
        return 60 * 60 * 1000;
    case Last6Hours:
        return 6 * 60 * 60 * 1000;
    case Last24Hours:
        return 24 * 60 * 60 * 1000;
    case CustomRange:
    default:
        return 5 * 60 * 1000; // 默认5分钟
    }
}

QList<QPair<QDateTime, double>> MetricsChart::filterDataPoints(const QDateTime& from, const QDateTime& to) const
{
    QList<QPair<QDateTime, double>> filtered;
    
    for (const auto& point : m_dataPoints) {
        if (point.first >= from && point.first <= to) {
            filtered.append(point);
        }
    }
    
    return filtered;
}

QVariantMap MetricsChart::calculateStatistics() const
{
    QVariantMap stats;
    
    if (m_dataPoints.isEmpty()) {
        stats["count"] = 0;
        stats["min"] = 0.0;
        stats["max"] = 0.0;
        stats["average"] = 0.0;
        stats["standardDeviation"] = 0.0;
        return stats;
    }
    
    // 基本统计
    double sum = 0.0;
    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    
    for (const auto& point : m_dataPoints) {
        double value = point.second;
        sum += value;
        min = std::min(min, value);
        max = std::max(max, value);
    }
    
    int count = m_dataPoints.size();
    double average = sum / count;
    
    // 计算标准差
    double sumSquaredDiff = 0.0;
    for (const auto& point : m_dataPoints) {
        double diff = point.second - average;
        sumSquaredDiff += diff * diff;
    }
    double standardDeviation = std::sqrt(sumSquaredDiff / count);
    
    // 计算趋势（简单线性回归斜率）
    double trend = 0.0;
    if (count > 1) {
        double sumX = 0.0, sumY = 0.0, sumXY = 0.0, sumX2 = 0.0;
        
        for (int i = 0; i < count; ++i) {
            double x = i; // 使用索引作为X值
            double y = m_dataPoints[i].second;
            
            sumX += x;
            sumY += y;
            sumXY += x * y;
            sumX2 += x * x;
        }
        
        double denominator = count * sumX2 - sumX * sumX;
        if (std::abs(denominator) > 1e-10) {
            trend = (count * sumXY - sumX * sumY) / denominator;
        }
    }
    
    stats["count"] = count;
    stats["min"] = min;
    stats["max"] = max;
    stats["average"] = average;
    stats["standardDeviation"] = standardDeviation;
    stats["trend"] = trend;
    stats["unit"] = getMetricUnit();
    
    // 时间范围信息
    if (!m_dataPoints.isEmpty()) {
        stats["startTime"] = m_dataPoints.first().first.toString(Qt::ISODate);
        stats["endTime"] = m_dataPoints.last().first.toString(Qt::ISODate);
        stats["duration"] = m_dataPoints.first().first.msecsTo(m_dataPoints.last().first);
    }
    
    return stats;
}
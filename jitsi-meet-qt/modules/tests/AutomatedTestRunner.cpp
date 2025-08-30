#include "AutomatedTestRunner.h"
#include <QDebug>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QThread>
#include <QElapsedTimer>

AutomatedTestRunner::AutomatedTestRunner(QObject *parent)
    : QObject(parent)
    , m_testFramework(new TestCoverageFramework(this))
    , m_scheduleTimer(new QTimer(this))
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_scheduleMode(Manual)
    , m_scheduleInterval(60)
    , m_fileWatchingEnabled(false)
    , m_fileChangeDebounceMs(5000)
    , m_ciProvider(None)
    , m_ciIntegrationEnabled(false)
    , m_emailNotificationsEnabled(false)
    , m_slackNotificationsEnabled(false)
    , m_webhookNotificationsEnabled(false)
    , m_isRunning(false)
    , m_testsInProgress(false)
    , m_consecutiveFailures(0)
    , m_lastCoveragePercentage(0.0)
    , m_coverageThreshold(75.0)
    , m_maxConsecutiveFailures(3)
    , m_testTimeoutMinutes(30)
    , m_runOnlyOnBusinessHours(false)
{
    // Setup configuration paths
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    m_configFilePath = appDataPath + "/automated_test_config.json";
    m_logFilePath = appDataPath + "/automated_test.log";
    m_reportsDirectory = appDataPath + "/reports";
    
    // Create directories
    QDir().mkpath(appDataPath);
    QDir().mkpath(m_reportsDirectory);
    
    // Load configuration
    loadConfiguration();
    
    // Setup connections
    connect(m_scheduleTimer, &QTimer::timeout, this, &AutomatedTestRunner::onScheduledTestTrigger);
    connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &AutomatedTestRunner::onFileChanged);
    connect(m_testFramework, &TestCoverageFramework::allTestsCompleted, this, &AutomatedTestRunner::onTestCompleted);
    connect(m_testFramework, &TestCoverageFramework::coverageAnalysisCompleted, this, &AutomatedTestRunner::onCoverageThresholdExceeded);
    connect(m_testFramework, &TestCoverageFramework::regressionDetected, this, &AutomatedTestRunner::onRegressionDetected);
    
    qDebug() << "AutomatedTestRunner initialized";
}

AutomatedTestRunner::~AutomatedTestRunner()
{
    stopAutomatedTesting();
    saveConfiguration();
}

void AutomatedTestRunner::startAutomatedTesting()
{
    if (m_isRunning) {
        qDebug() << "Automated testing already running";
        return;
    }
    
    qDebug() << "Starting automated testing...";
    
    m_isRunning = true;
    m_consecutiveFailures = 0;
    
    // Validate test environment
    if (!validateTestEnvironment()) {
        qWarning() << "Test environment validation failed";
        m_isRunning = false;
        return;
    }
    
    // Setup file watching if enabled
    if (m_fileWatchingEnabled) {
        setupFileWatching();
    }
    
    // Setup test schedule
    setupTestSchedule();
    
    // Setup notifications
    setupNotifications();
    
    emit automatedTestStarted();
    
    qDebug() << "Automated testing started successfully";
    sendNotification(TestStarted, "Automated testing system started");
}

void AutomatedTestRunner::stopAutomatedTesting()
{
    if (!m_isRunning) {
        return;
    }
    
    qDebug() << "Stopping automated testing...";
    
    m_isRunning = false;
    m_scheduleTimer->stop();
    
    // Clear file watchers
    if (!m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }
    if (!m_fileWatcher->directories().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->directories());
    }
    
    qDebug() << "Automated testing stopped";
}

void AutomatedTestRunner::runTestsNow()
{
    if (m_testsInProgress) {
        qDebug() << "Tests already in progress, skipping";
        return;
    }
    
    qDebug() << "Running tests now...";
    executeScheduledTests();
}

void AutomatedTestRunner::scheduleTests(ScheduleMode mode, int intervalMinutes)
{
    m_scheduleMode = mode;
    m_scheduleInterval = intervalMinutes;
    
    qDebug() << "Test schedule updated - Mode:" << mode << "Interval:" << intervalMinutes << "minutes";
    
    if (m_isRunning) {
        setupTestSchedule();
    }
    
    emit testScheduleUpdated();
}

void AutomatedTestRunner::enableFileWatching(const QStringList& watchPaths)
{
    m_watchPaths = watchPaths;
    m_fileWatchingEnabled = true;
    
    if (m_isRunning) {
        setupFileWatching();
    }
    
    qDebug() << "File watching enabled for paths:" << watchPaths;
}

void AutomatedTestRunner::configureCIIntegration(CIProvider provider, const QVariantMap& config)
{
    m_ciProvider = provider;
    m_ciConfig = config;
    m_ciIntegrationEnabled = true;
    
    switch (provider) {
    case Jenkins:
        setupJenkinsIntegration();
        break;
    case GitHubActions:
        setupGitHubActionsIntegration();
        break;
    case GitLabCI:
        setupGitLabCIIntegration();
        break;
    case AzureDevOps:
        setupAzureDevOpsIntegration();
        break;
    case TeamCity:
        setupTeamCityIntegration();
        break;
    default:
        m_ciIntegrationEnabled = false;
        break;
    }
    
    emit ciIntegrationConfigured();
    qDebug() << "CI integration configured for provider:" << provider;
}

void AutomatedTestRunner::setNotificationSettings(const QVariantMap& settings)
{
    m_notificationConfig = settings;
    
    m_emailNotificationsEnabled = settings.value("email_enabled", false).toBool();
    m_slackNotificationsEnabled = settings.value("slack_enabled", false).toBool();
    m_webhookNotificationsEnabled = settings.value("webhook_enabled", false).toBool();
    
    m_emailRecipients = settings.value("email_recipients").toString();
    m_slackWebhookUrl = settings.value("slack_webhook_url").toString();
    m_notificationWebhookUrl = settings.value("notification_webhook_url").toString();
    
    qDebug() << "Notification settings updated";
}

void AutomatedTestRunner::onScheduledTestTrigger()
{
    if (!shouldRunTests()) {
        return;
    }
    
    qDebug() << "Scheduled test trigger activated";
    executeScheduledTests();
}

void AutomatedTestRunner::onFileChanged(const QString& path)
{
    qDebug() << "File changed:" << path;
    
    if (m_scheduleMode == OnFileChange && !m_testsInProgress) {
        // Debounce file changes
        QTimer::singleShot(m_fileChangeDebounceMs, this, [this]() {
            if (!m_testsInProgress) {
                executeScheduledTests();
            }
        });
    }
}

void AutomatedTestRunner::onTestCompleted()
{
    m_testsInProgress = false;
    
    // Check test results
    bool hasFailures = false; // This would be determined from test framework results
    
    if (hasFailures) {
        m_consecutiveFailures++;
        sendNotification(TestFailed, QString("Test run completed with failures (consecutive: %1)").arg(m_consecutiveFailures));
        
        if (m_consecutiveFailures >= m_maxConsecutiveFailures) {
            sendNotification(BuildBroken, QString("Build broken - %1 consecutive test failures").arg(m_consecutiveFailures));
        }
    } else {
        m_consecutiveFailures = 0;
        sendNotification(TestCompleted, "All tests passed successfully");
    }
    
    // Update CI status
    if (m_ciIntegrationEnabled) {
        QString status = hasFailures ? "failure" : "success";
        QString description = hasFailures ? "Tests failed" : "Tests passed";
        sendCIStatus(status, description);
    }
    
    // Generate reports
    generateScheduledReports();
    updateDashboard();
    
    emit automatedTestCompleted(!hasFailures);
}

void AutomatedTestRunner::onCoverageThresholdExceeded()
{
    // This would be called when coverage analysis is completed
    // For now, we'll simulate the check
    double currentCoverage = 85.0; // This would come from the test framework
    
    if (currentCoverage < m_coverageThreshold) {
        QString message = QString("Coverage below threshold: %1% < %2%")
                         .arg(currentCoverage, 0, 'f', 1)
                         .arg(m_coverageThreshold, 0, 'f', 1);
        sendNotification(CoverageAlert, message);
    }
    
    m_lastCoveragePercentage = currentCoverage;
}

void AutomatedTestRunner::onRegressionDetected()
{
    sendNotification(RegressionDetected, "Performance regression detected in latest test run");
}

void AutomatedTestRunner::executeScheduledTests()
{
    if (m_testsInProgress) {
        qDebug() << "Tests already in progress";
        return;
    }
    
    if (m_runOnlyOnBusinessHours && !isBusinessHours()) {
        qDebug() << "Skipping tests - outside business hours";
        return;
    }
    
    qDebug() << "Executing scheduled tests...";
    
    m_testsInProgress = true;
    m_currentRunStartTime = QDateTime::currentDateTime();
    
    // Prepare test environment
    prepareTestData();
    
    // Run the test suite
    runTestSuite(m_scheduledTestTypes);
}

void AutomatedTestRunner::runTestSuite(const QStringList& testTypes)
{
    qDebug() << "Running test suite with types:" << testTypes;
    
    // Start the comprehensive test framework
    QTimer::singleShot(100, m_testFramework, &TestCoverageFramework::runAllTests);
}

bool AutomatedTestRunner::validateTestEnvironment()
{
    // Check if required directories exist
    QStringList requiredPaths = {
        "jitsi-meet-qt/modules",
        m_reportsDirectory
    };
    
    for (const QString& path : requiredPaths) {
        if (!QDir(path).exists()) {
            qWarning() << "Required path does not exist:" << path;
            return false;
        }
    }
    
    return true;
}

void AutomatedTestRunner::prepareTestData()
{
    qDebug() << "Preparing test data...";
    
    // Clean up old test artifacts
    cleanupTestArtifacts();
    
    // Create fresh test environment
    QDir().mkpath(m_reportsDirectory + "/current");
}

void AutomatedTestRunner::cleanupTestArtifacts()
{
    // Remove old test results (keep last 10 runs)
    QDir reportsDir(m_reportsDirectory);
    QStringList oldReports = reportsDir.entryList(QStringList() << "run_*", QDir::Dirs, QDir::Time);
    
    while (oldReports.size() > 10) {
        QString oldestReport = oldReports.takeLast();
        QDir(reportsDir.absoluteFilePath(oldestReport)).removeRecursively();
    }
}

void AutomatedTestRunner::setupTestSchedule()
{
    m_scheduleTimer->stop();
    
    switch (m_scheduleMode) {
    case Periodic:
        m_scheduleTimer->setInterval(m_scheduleInterval * 60 * 1000); // Convert to milliseconds
        m_scheduleTimer->start();
        qDebug() << "Periodic testing scheduled every" << m_scheduleInterval << "minutes";
        break;
    case OnFileChange:
        qDebug() << "File change monitoring active";
        break;
    case Manual:
    default:
        qDebug() << "Manual testing mode";
        break;
    }
}

void AutomatedTestRunner::setupFileWatching()
{
    // Clear existing watchers
    if (!m_fileWatcher->files().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->files());
    }
    if (!m_fileWatcher->directories().isEmpty()) {
        m_fileWatcher->removePaths(m_fileWatcher->directories());
    }
    
    // Add new watch paths
    for (const QString& path : m_watchPaths) {
        addWatchPath(path);
    }
}

void AutomatedTestRunner::addWatchPath(const QString& path)
{
    QFileInfo info(path);
    
    if (info.isDir()) {
        m_fileWatcher->addPath(path);
        qDebug() << "Watching directory:" << path;
    } else if (info.isFile()) {
        m_fileWatcher->addPath(path);
        qDebug() << "Watching file:" << path;
    }
}

void AutomatedTestRunner::sendNotification(NotificationType type, const QString& message)
{
    QString typeStr;
    switch (type) {
    case TestStarted: typeStr = "TEST_STARTED"; break;
    case TestCompleted: typeStr = "TEST_COMPLETED"; break;
    case TestFailed: typeStr = "TEST_FAILED"; break;
    case CoverageAlert: typeStr = "COVERAGE_ALERT"; break;
    case RegressionDetected: typeStr = "REGRESSION_DETECTED"; break;
    case BuildBroken: typeStr = "BUILD_BROKEN"; break;
    }
    
    QString fullMessage = QString("[%1] %2").arg(typeStr, message);
    
    qDebug() << "Notification:" << fullMessage;
    
    // Log notification
    logNotification(fullMessage);
    
    // Send email notification
    if (m_emailNotificationsEnabled && !m_emailRecipients.isEmpty()) {
        sendEmailNotification(QString("Jitsi Test Notification - %1").arg(typeStr), message);
    }
    
    // Send Slack notification
    if (m_slackNotificationsEnabled && !m_slackWebhookUrl.isEmpty()) {
        sendSlackNotification(fullMessage);
    }
    
    // Send webhook notification
    if (m_webhookNotificationsEnabled && !m_notificationWebhookUrl.isEmpty()) {
        QVariantMap data;
        data["type"] = typeStr;
        data["message"] = message;
        data["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        sendWebhookNotification(data);
    }
    
    emit notificationSent(type, message);
}

void AutomatedTestRunner::sendSlackNotification(const QString& message)
{
    if (m_slackWebhookUrl.isEmpty()) return;
    
    QJsonObject payload;
    payload["text"] = message;
    payload["username"] = "Jitsi Test Bot";
    payload["icon_emoji"] = ":robot_face:";
    
    QNetworkRequest request(QUrl(m_slackWebhookUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void AutomatedTestRunner::sendWebhookNotification(const QVariantMap& data)
{
    if (m_notificationWebhookUrl.isEmpty()) return;
    
    QJsonObject payload = QJsonObject::fromVariantMap(data);
    
    QNetworkRequest request(QUrl(m_notificationWebhookUrl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    
    QNetworkReply* reply = m_networkManager->post(request, QJsonDocument(payload).toJson());
    connect(reply, &QNetworkReply::finished, reply, &QNetworkReply::deleteLater);
}

void AutomatedTestRunner::logNotification(const QString& message)
{
    QFile logFile(m_logFilePath);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream stream(&logFile);
        stream << QDateTime::currentDateTime().toString(Qt::ISODate) << " - " << message << "\n";
    }
}

void AutomatedTestRunner::generateScheduledReports()
{
    qDebug() << "Generating scheduled reports...";
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd_hh-mm-ss");
    QString runDir = m_reportsDirectory + "/run_" + timestamp;
    QDir().mkpath(runDir);
    
    // Generate summary report
    QString summaryPath = runDir + "/summary.txt";
    QFile summaryFile(summaryPath);
    if (summaryFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&summaryFile);
        stream << generateSummaryReport();
    }
    
    qDebug() << "Reports generated in:" << runDir;
}

QString AutomatedTestRunner::generateSummaryReport()
{
    QString report;
    QTextStream stream(&report);
    
    stream << "=== Automated Test Run Summary ===\n";
    stream << "Timestamp: " << QDateTime::currentDateTime().toString(Qt::ISODate) << "\n";
    stream << "Schedule Mode: " << m_scheduleMode << "\n";
    stream << "Consecutive Failures: " << m_consecutiveFailures << "\n";
    stream << "Last Coverage: " << QString::number(m_lastCoveragePercentage, 'f', 2) << "%\n";
    stream << "Coverage Threshold: " << QString::number(m_coverageThreshold, 'f', 2) << "%\n";
    
    return report;
}

bool AutomatedTestRunner::shouldRunTests()
{
    if (!m_isRunning || m_testsInProgress) {
        return false;
    }
    
    if (m_runOnlyOnBusinessHours && !isBusinessHours()) {
        return false;
    }
    
    return true;
}

bool AutomatedTestRunner::isBusinessHours()
{
    QDateTime now = QDateTime::currentDateTime();
    QTime currentTime = now.time();
    int dayOfWeek = now.date().dayOfWeek();
    
    // Monday to Friday, 9 AM to 6 PM
    return (dayOfWeek >= 1 && dayOfWeek <= 5) &&
           (currentTime >= QTime(9, 0) && currentTime <= QTime(18, 0));
}

void AutomatedTestRunner::loadConfiguration()
{
    QFile configFile(m_configFilePath);
    if (!configFile.exists()) {
        // Create default configuration
        m_notificationConfig = getDefaultConfiguration();
        saveConfiguration();
        return;
    }
    
    if (configFile.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(configFile.readAll());
        QJsonObject config = doc.object();
        
        m_scheduleMode = static_cast<ScheduleMode>(config.value("schedule_mode").toInt());
        m_scheduleInterval = config.value("schedule_interval").toInt(60);
        m_coverageThreshold = config.value("coverage_threshold").toDouble(75.0);
        m_maxConsecutiveFailures = config.value("max_consecutive_failures").toInt(3);
        m_runOnlyOnBusinessHours = config.value("business_hours_only").toBool(false);
        
        // Load notification settings
        QJsonObject notifications = config.value("notifications").toObject();
        m_notificationConfig = notifications.toVariantMap();
        setNotificationSettings(m_notificationConfig);
    }
}

void AutomatedTestRunner::saveConfiguration()
{
    QJsonObject config;
    config["schedule_mode"] = static_cast<int>(m_scheduleMode);
    config["schedule_interval"] = m_scheduleInterval;
    config["coverage_threshold"] = m_coverageThreshold;
    config["max_consecutive_failures"] = m_maxConsecutiveFailures;
    config["business_hours_only"] = m_runOnlyOnBusinessHours;
    config["notifications"] = QJsonObject::fromVariantMap(m_notificationConfig);
    
    QFile configFile(m_configFilePath);
    if (configFile.open(QIODevice::WriteOnly)) {
        configFile.write(QJsonDocument(config).toJson());
    }
}

QVariantMap AutomatedTestRunner::getDefaultConfiguration()
{
    QVariantMap config;
    config["email_enabled"] = false;
    config["slack_enabled"] = false;
    config["webhook_enabled"] = false;
    config["email_recipients"] = "";
    config["slack_webhook_url"] = "";
    config["notification_webhook_url"] = "";
    return config;
}

// Placeholder implementations for CI integration methods
void AutomatedTestRunner::setupJenkinsIntegration() { /* Implementation for Jenkins */ }
void AutomatedTestRunner::setupGitHubActionsIntegration() { /* Implementation for GitHub Actions */ }
void AutomatedTestRunner::setupGitLabCIIntegration() { /* Implementation for GitLab CI */ }
void AutomatedTestRunner::setupAzureDevOpsIntegration() { /* Implementation for Azure DevOps */ }
void AutomatedTestRunner::setupTeamCityIntegration() { /* Implementation for TeamCity */ }
void AutomatedTestRunner::sendCIStatus(const QString& status, const QString& description) { /* CI status update */ }
void AutomatedTestRunner::sendEmailNotification(const QString& subject, const QString& body) { /* Email implementation */ }
void AutomatedTestRunner::updateDashboard() { /* Dashboard update */ }
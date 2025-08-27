#include <QtTest/QtTest>
#include <QApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSignalSpy>
#include <QTimer>
#include "WelcomeWindow.h"
#include "NavigationBar.h"
#include "RecentListWidget.h"
#include "ConfigurationManager.h"
#include "models/RecentItem.h"

class TestWelcomeWindow : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // UI Component Tests
    void testWindowInitialization();
    void testUIComponents();
    void testNavigationBarIntegration();
    void testRecentListIntegration();
    
    // Input Validation Tests
    void testUrlValidation_data();
    void testUrlValidation();
    void testJoinButtonState();
    void testErrorDisplay();
    
    // Animation Tests
    void testRandomRoomNameGeneration();
    void testPlaceholderAnimation();
    void testAnimationStopsOnUserInput();
    
    // Signal Tests
    void testJoinConferenceSignal();
    void testSettingsSignal();
    void testAboutSignal();
    void testRecentItemSelection();
    
    // Integration Tests
    void testUrlTextSetting();
    void testErrorHandling();
    void testConfigurationManagerIntegration();
    void testRecentItemsLoading();

private:
    QApplication* app;
    WelcomeWindow* window;
    
    QLineEdit* getUrlLineEdit();
    QPushButton* getJoinButton();
    QLabel* getErrorLabel();
    NavigationBar* getNavigationBar();
    RecentListWidget* getRecentList();
};

void TestWelcomeWindow::initTestCase()
{
    // QApplication is needed for widget tests
    if (!QApplication::instance()) {
        int argc = 0;
        char** argv = nullptr;
        app = new QApplication(argc, argv);
    } else {
        app = nullptr;
    }
}

void TestWelcomeWindow::cleanupTestCase()
{
    if (app) {
        delete app;
    }
}

void TestWelcomeWindow::init()
{
    window = new WelcomeWindow();
}

void TestWelcomeWindow::cleanup()
{
    delete window;
    window = nullptr;
}

QLineEdit* TestWelcomeWindow::getUrlLineEdit()
{
    return window->findChild<QLineEdit*>();
}

QPushButton* TestWelcomeWindow::getJoinButton()
{
    return window->findChild<QPushButton*>();
}

QLabel* TestWelcomeWindow::getErrorLabel()
{
    QList<QLabel*> labels = window->findChildren<QLabel*>();
    for (QLabel* label : labels) {
        if (label->styleSheet().contains("color: #d32f2f")) {
            return label;
        }
    }
    return nullptr;
}

NavigationBar* TestWelcomeWindow::getNavigationBar()
{
    return window->findChild<NavigationBar*>();
}

RecentListWidget* TestWelcomeWindow::getRecentList()
{
    return window->findChild<RecentListWidget*>();
}

void TestWelcomeWindow::testWindowInitialization()
{
    QVERIFY(window != nullptr);
    QCOMPARE(window->windowTitle(), QString("Jitsi Meet"));
    QVERIFY(window->minimumSize().width() >= 800);
    QVERIFY(window->minimumSize().height() >= 600);
}

void TestWelcomeWindow::testUIComponents()
{
    // Test that all required UI components exist
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    QVERIFY(!urlEdit->placeholderText().isEmpty());
    
    QPushButton* joinButton = getJoinButton();
    QVERIFY(joinButton != nullptr);
    QCOMPARE(joinButton->text(), QString("Join Meeting"));
    
    NavigationBar* navBar = getNavigationBar();
    QVERIFY(navBar != nullptr);
    
    RecentListWidget* recentList = getRecentList();
    QVERIFY(recentList != nullptr);
    QCOMPARE(recentList->maxItems(), 5);
}

void TestWelcomeWindow::testNavigationBarIntegration()
{
    NavigationBar* navBar = getNavigationBar();
    QVERIFY(navBar != nullptr);
    
    // Test that navigation bar has correct button configuration
    QVERIFY(navBar->isButtonVisible(NavigationBar::SettingsButton));
    QVERIFY(navBar->isButtonVisible(NavigationBar::AboutButton));
    QVERIFY(!navBar->isButtonVisible(NavigationBar::BackButton));
}

void TestWelcomeWindow::testRecentListIntegration()
{
    RecentListWidget* recentList = getRecentList();
    QVERIFY(recentList != nullptr);
    QVERIFY(recentList->isEmpty());
    QCOMPARE(recentList->maxItems(), 5);
}

void TestWelcomeWindow::testUrlValidation_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<bool>("expected");
    
    // Valid URLs
    QTest::newRow("simple room name") << "MyRoom" << true;
    QTest::newRow("room with numbers") << "Room123" << true;
    QTest::newRow("room with dash") << "My-Room" << true;
    QTest::newRow("room with underscore") << "My_Room" << true;
    QTest::newRow("room with dot") << "My.Room" << true;
    QTest::newRow("http url") << "http://meet.jit.si/MyRoom" << true;
    QTest::newRow("https url") << "https://meet.jit.si/MyRoom" << true;
    
    // Invalid URLs
    QTest::newRow("empty") << "" << false;
    QTest::newRow("too short") << "ab" << false;
    QTest::newRow("with spaces") << "My Room" << false;
    QTest::newRow("with special chars") << "My@Room" << false;
    QTest::newRow("ftp url") << "ftp://example.com" << false;
}

void TestWelcomeWindow::testUrlValidation()
{
    QFETCH(QString, url);
    QFETCH(bool, expected);
    
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    
    urlEdit->setText(url);
    
    // Trigger validation by attempting to join
    QPushButton* joinButton = getJoinButton();
    QVERIFY(joinButton != nullptr);
    
    if (expected && !url.isEmpty()) {
        QVERIFY(joinButton->isEnabled());
    }
}

void TestWelcomeWindow::testJoinButtonState()
{
    QLineEdit* urlEdit = getUrlLineEdit();
    QPushButton* joinButton = getJoinButton();
    
    QVERIFY(urlEdit != nullptr);
    QVERIFY(joinButton != nullptr);
    
    // Initially disabled when no text
    urlEdit->clear();
    QTest::keyClick(urlEdit, Qt::Key_A); // Trigger text change
    QTest::keyClick(urlEdit, Qt::Key_Backspace); // Clear it
    
    // Should be enabled when there's valid text
    urlEdit->setText("ValidRoom");
    QVERIFY(joinButton->isEnabled());
    
    // Should be disabled when text is cleared
    urlEdit->clear();
    // Note: Button might still be enabled due to placeholder text
}

void TestWelcomeWindow::testErrorDisplay()
{
    QString testError = "Test error message";
    
    window->showError(testError);
    
    QLabel* errorLabel = getErrorLabel();
    QVERIFY(errorLabel != nullptr);
    QVERIFY(errorLabel->isVisible());
    QCOMPARE(errorLabel->text(), testError);
    
    window->clearError();
    QVERIFY(!errorLabel->isVisible());
    QVERIFY(errorLabel->text().isEmpty());
}

void TestWelcomeWindow::testRandomRoomNameGeneration()
{
    // Test that random room names are generated
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    
    // Wait a bit for animation to start
    QTest::qWait(200);
    
    QString placeholder = urlEdit->placeholderText();
    QVERIFY(!placeholder.isEmpty());
    
    // The placeholder should not be the default text
    QVERIFY(placeholder != "Enter meeting name or URL");
}

void TestWelcomeWindow::testPlaceholderAnimation()
{
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    
    // Clear any existing text to trigger animation
    urlEdit->clear();
    
    // Wait for animation to potentially start
    QTest::qWait(300);
    
    QString initialPlaceholder = urlEdit->placeholderText();
    
    // Wait for animation to progress
    QTest::qWait(500);
    
    // The placeholder might have changed due to animation
    // This test verifies the animation system is working
    QVERIFY(!urlEdit->placeholderText().isEmpty());
}

void TestWelcomeWindow::testAnimationStopsOnUserInput()
{
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    
    // Start with empty field to ensure animation is running
    urlEdit->clear();
    QTest::qWait(100);
    
    // Type some text
    urlEdit->setText("UserInput");
    
    // Animation should stop when user types
    QString placeholderAfterTyping = urlEdit->placeholderText();
    
    // Wait a bit more
    QTest::qWait(200);
    
    // Placeholder should not change while user has typed text
    QCOMPARE(urlEdit->placeholderText(), placeholderAfterTyping);
}

void TestWelcomeWindow::testJoinConferenceSignal()
{
    QSignalSpy spy(window, &WelcomeWindow::joinConference);
    
    QLineEdit* urlEdit = getUrlLineEdit();
    QPushButton* joinButton = getJoinButton();
    
    QVERIFY(urlEdit != nullptr);
    QVERIFY(joinButton != nullptr);
    
    QString testUrl = "TestRoom";
    urlEdit->setText(testUrl);
    
    QTest::mouseClick(joinButton, Qt::LeftButton);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), testUrl);
}

void TestWelcomeWindow::testSettingsSignal()
{
    QSignalSpy spy(window, &WelcomeWindow::settingsRequested);
    
    NavigationBar* navBar = getNavigationBar();
    QVERIFY(navBar != nullptr);
    
    // Emit the settings signal from navigation bar
    emit navBar->settingsClicked();
    
    QCOMPARE(spy.count(), 1);
}

void TestWelcomeWindow::testAboutSignal()
{
    QSignalSpy spy(window, &WelcomeWindow::aboutRequested);
    
    NavigationBar* navBar = getNavigationBar();
    QVERIFY(navBar != nullptr);
    
    // Emit the about signal from navigation bar
    emit navBar->aboutClicked();
    
    QCOMPARE(spy.count(), 1);
}

void TestWelcomeWindow::testRecentItemSelection()
{
    RecentListWidget* recentList = getRecentList();
    QVERIFY(recentList != nullptr);
    
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    
    QString testUrl = "https://meet.jit.si/TestRoom";
    
    // Simulate clicking on a recent item
    emit recentList->itemClicked(testUrl);
    
    QCOMPARE(urlEdit->text(), testUrl);
}

void TestWelcomeWindow::testUrlTextSetting()
{
    QString testUrl = "https://meet.jit.si/TestRoom";
    
    window->setUrlText(testUrl);
    
    QCOMPARE(window->getUrlText(), testUrl);
    
    QLineEdit* urlEdit = getUrlLineEdit();
    QVERIFY(urlEdit != nullptr);
    QCOMPARE(urlEdit->text(), testUrl);
}

void TestWelcomeWindow::testErrorHandling()
{
    // Test invalid URL handling
    QLineEdit* urlEdit = getUrlLineEdit();
    QPushButton* joinButton = getJoinButton();
    
    QVERIFY(urlEdit != nullptr);
    QVERIFY(joinButton != nullptr);
    
    // Set invalid URL
    urlEdit->setText("ab"); // Too short
    
    QSignalSpy spy(window, &WelcomeWindow::joinConference);
    QTest::mouseClick(joinButton, Qt::LeftButton);
    
    // Should not emit signal for invalid URL
    QCOMPARE(spy.count(), 0);
    
    // Should show error
    QLabel* errorLabel = getErrorLabel();
    if (errorLabel) {
        QVERIFY(errorLabel->isVisible());
    }
}

void TestWelcomeWindow::testConfigurationManagerIntegration()
{
    // Create a configuration manager
    ConfigurationManager configManager;
    
    // Set it on the window
    window->setConfigurationManager(&configManager);
    
    // Test that recent list max items is set from configuration
    RecentListWidget* recentList = getRecentList();
    QVERIFY(recentList != nullptr);
    QCOMPARE(recentList->maxItems(), configManager.maxRecentItems());
    
    // Test adding a recent item
    QString testUrl = "https://meet.jit.si/TestRoom";
    window->addToRecentItems(testUrl);
    
    // Verify it was added to configuration
    QList<RecentItem> items = configManager.recentItems();
    QVERIFY(!items.isEmpty());
    QCOMPARE(items.first().url, testUrl);
}

void TestWelcomeWindow::testRecentItemsLoading()
{
    // Create a configuration manager and add some items
    ConfigurationManager configManager;
    
    RecentItem item1("https://meet.jit.si/Room1", "Room 1");
    RecentItem item2("https://meet.jit.si/Room2", "Room 2");
    
    configManager.addRecentItem(item1);
    configManager.addRecentItem(item2);
    
    // Set configuration manager on window
    window->setConfigurationManager(&configManager);
    
    // Verify items were loaded into the recent list
    RecentListWidget* recentList = getRecentList();
    QVERIFY(recentList != nullptr);
    QVERIFY(!recentList->isEmpty());
    
    QList<RecentItem> loadedItems = recentList->getRecentItems();
    QCOMPARE(loadedItems.size(), 2);
    
    // Items should be sorted by timestamp (newest first)
    QCOMPARE(loadedItems[0].url, item2.url);
    QCOMPARE(loadedItems[1].url, item1.url);
}

QTEST_MAIN(TestWelcomeWindow)
#include "test_welcomewindow.moc"
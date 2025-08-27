#include <QtTest/QtTest>
#include <QJsonObject>
#include <QDateTime>
#include "models/ApplicationSettings.h"
#include "models/RecentItem.h"
#include "JitsiConstants.h"

class TestModels : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // ApplicationSettings tests
    void testApplicationSettingsDefaults();
    void testApplicationSettingsCopyConstructor();
    void testApplicationSettingsAssignment();
    void testApplicationSettingsEquality();
    void testApplicationSettingsValidation_data();
    void testApplicationSettingsValidation();
    void testApplicationSettingsResetToDefaults();
    void testApplicationSettingsVariantMapSerialization();
    void testApplicationSettingsToString();
    
    // RecentItem tests
    void testRecentItemDefaults();
    void testRecentItemConstructorWithParameters();
    void testRecentItemValidation();
    void testRecentItemDisplayText();
    void testRecentItemComparison();
    void testRecentItemEquality();
    void testRecentItemJsonSerialization();
    void testRecentItemUpdateAccess();
    void testRecentItemRoomNameExtraction_data();
    void testRecentItemRoomNameExtraction();

private:
    ApplicationSettings* m_settings;
    RecentItem* m_recentItem;
};

void TestModels::initTestCase()
{
    // Global test setup
}

void TestModels::cleanupTestCase()
{
    // Global test cleanup
}

void TestModels::init()
{
    // Per-test setup
    m_settings = new ApplicationSettings();
    m_recentItem = new RecentItem();
}

void TestModels::cleanup()
{
    // Per-test cleanup
    delete m_settings;
    delete m_recentItem;
    m_settings = nullptr;
    m_recentItem = nullptr;
}

// ApplicationSettings Tests

void TestModels::testApplicationSettingsDefaults()
{
    ApplicationSettings settings;
    
    QCOMPARE(settings.defaultServerUrl, JitsiConstants::DEFAULT_SERVER_URL);
    QCOMPARE(settings.serverTimeout, JitsiConstants::DEFAULT_SERVER_TIMEOUT);
    QCOMPARE(settings.language, JitsiConstants::DEFAULT_LANGUAGE);
    QCOMPARE(settings.darkMode, false);
    QCOMPARE(settings.maximized, false);
    QCOMPARE(settings.rememberWindowState, true);
    QCOMPARE(settings.autoJoinAudio, true);
    QCOMPARE(settings.autoJoinVideo, false);
    QCOMPARE(settings.maxRecentItems, JitsiConstants::MAX_RECENT_ITEMS);
    QVERIFY(settings.recentUrls.isEmpty());
    QVERIFY(settings.isValid());
}

void TestModels::testApplicationSettingsCopyConstructor()
{
    ApplicationSettings original;
    original.defaultServerUrl = "https://test.example.com";
    original.language = "zh-CN";
    original.darkMode = true;
    original.recentUrls << "https://meet.example.com/room1";
    
    ApplicationSettings copy(original);
    
    QCOMPARE(copy.defaultServerUrl, original.defaultServerUrl);
    QCOMPARE(copy.language, original.language);
    QCOMPARE(copy.darkMode, original.darkMode);
    QCOMPARE(copy.recentUrls, original.recentUrls);
    QVERIFY(copy == original);
}

void TestModels::testApplicationSettingsAssignment()
{
    ApplicationSettings original;
    original.defaultServerUrl = "https://test.example.com";
    original.language = "zh-CN";
    original.darkMode = true;
    
    ApplicationSettings assigned;
    assigned = original;
    
    QCOMPARE(assigned.defaultServerUrl, original.defaultServerUrl);
    QCOMPARE(assigned.language, original.language);
    QCOMPARE(assigned.darkMode, original.darkMode);
    QVERIFY(assigned == original);
}

void TestModels::testApplicationSettingsEquality()
{
    ApplicationSettings settings1;
    ApplicationSettings settings2;
    
    QVERIFY(settings1 == settings2);
    QVERIFY(!(settings1 != settings2));
    
    settings2.darkMode = true;
    QVERIFY(settings1 != settings2);
    QVERIFY(!(settings1 == settings2));
}

void TestModels::testApplicationSettingsValidation_data()
{
    QTest::addColumn<QString>("serverUrl");
    QTest::addColumn<int>("timeout");
    QTest::addColumn<QRect>("geometry");
    QTest::addColumn<int>("maxRecent");
    QTest::addColumn<QString>("language");
    QTest::addColumn<bool>("expected");
    
    // Valid cases
    QTest::newRow("valid defaults") 
        << "https://meet.jit.si" << 30 << QRect(100, 100, 800, 600) << 10 << "auto" << true;
    QTest::newRow("valid custom") 
        << "http://localhost:8080" << 60 << QRect(0, 0, 1200, 800) << 20 << "en-US" << true;
    
    // Invalid server URLs
    QTest::newRow("empty server url") 
        << "" << 30 << QRect(100, 100, 800, 600) << 10 << "auto" << false;
    QTest::newRow("invalid protocol") 
        << "ftp://example.com" << 30 << QRect(100, 100, 800, 600) << 10 << "auto" << false;
    QTest::newRow("malformed url") 
        << "not-a-url" << 30 << QRect(100, 100, 800, 600) << 10 << "auto" << false;
    
    // Invalid timeouts
    QTest::newRow("negative timeout") 
        << "https://meet.jit.si" << -1 << QRect(100, 100, 800, 600) << 10 << "auto" << false;
    QTest::newRow("zero timeout") 
        << "https://meet.jit.si" << 0 << QRect(100, 100, 800, 600) << 10 << "auto" << false;
    QTest::newRow("too large timeout") 
        << "https://meet.jit.si" << 400 << QRect(100, 100, 800, 600) << 10 << "auto" << false;
    
    // Invalid geometry
    QTest::newRow("too small width") 
        << "https://meet.jit.si" << 30 << QRect(100, 100, 400, 600) << 10 << "auto" << false;
    QTest::newRow("too small height") 
        << "https://meet.jit.si" << 30 << QRect(100, 100, 800, 300) << 10 << "auto" << false;
    
    // Invalid max recent items
    QTest::newRow("negative max recent") 
        << "https://meet.jit.si" << 30 << QRect(100, 100, 800, 600) << -1 << "auto" << false;
    QTest::newRow("too large max recent") 
        << "https://meet.jit.si" << 30 << QRect(100, 100, 800, 600) << 150 << "auto" << false;
    
    // Invalid language
    QTest::newRow("empty language") 
        << "https://meet.jit.si" << 30 << QRect(100, 100, 800, 600) << 10 << "" << false;
}

void TestModels::testApplicationSettingsValidation()
{
    QFETCH(QString, serverUrl);
    QFETCH(int, timeout);
    QFETCH(QRect, geometry);
    QFETCH(int, maxRecent);
    QFETCH(QString, language);
    QFETCH(bool, expected);
    
    ApplicationSettings settings;
    settings.defaultServerUrl = serverUrl;
    settings.serverTimeout = timeout;
    settings.windowGeometry = geometry;
    settings.maxRecentItems = maxRecent;
    settings.language = language;
    
    QCOMPARE(settings.isValid(), expected);
}

void TestModels::testApplicationSettingsResetToDefaults()
{
    ApplicationSettings settings;
    
    // Modify settings
    settings.defaultServerUrl = "https://custom.server.com";
    settings.language = "zh-CN";
    settings.darkMode = true;
    settings.recentUrls << "test-url";
    
    // Reset to defaults
    settings.resetToDefaults();
    
    // Verify defaults are restored
    QCOMPARE(settings.defaultServerUrl, JitsiConstants::DEFAULT_SERVER_URL);
    QCOMPARE(settings.language, JitsiConstants::DEFAULT_LANGUAGE);
    QCOMPARE(settings.darkMode, false);
    QVERIFY(settings.recentUrls.isEmpty());
    QVERIFY(settings.isValid());
}

void TestModels::testApplicationSettingsVariantMapSerialization()
{
    ApplicationSettings original;
    original.defaultServerUrl = "https://test.example.com";
    original.language = "zh-CN";
    original.darkMode = true;
    original.recentUrls << "url1" << "url2";
    
    // Serialize to variant map
    QVariantMap map = original.toVariantMap();
    
    // Deserialize from variant map
    ApplicationSettings deserialized;
    deserialized.fromVariantMap(map);
    
    // Verify equality
    QCOMPARE(deserialized.defaultServerUrl, original.defaultServerUrl);
    QCOMPARE(deserialized.language, original.language);
    QCOMPARE(deserialized.darkMode, original.darkMode);
    QCOMPARE(deserialized.recentUrls, original.recentUrls);
    QVERIFY(deserialized == original);
}

void TestModels::testApplicationSettingsToString()
{
    ApplicationSettings settings;
    QString str = settings.toString();
    
    QVERIFY(!str.isEmpty());
    QVERIFY(str.contains("ApplicationSettings"));
    QVERIFY(str.contains(settings.defaultServerUrl));
    QVERIFY(str.contains(settings.language));
}

// RecentItem Tests

void TestModels::testRecentItemDefaults()
{
    RecentItem item;
    
    QVERIFY(item.url.isEmpty());
    QVERIFY(item.displayName.isEmpty());
    QCOMPARE(item.accessCount, 0);
    QVERIFY(!item.isValid()); // Invalid because URL is empty
}

void TestModels::testRecentItemConstructorWithParameters()
{
    QString testUrl = "https://meet.example.com/test-room";
    QString testDisplayName = "Test Room";
    
    RecentItem item(testUrl, testDisplayName);
    
    QCOMPARE(item.url, testUrl);
    QCOMPARE(item.displayName, testDisplayName);
    QCOMPARE(item.accessCount, 1);
    QVERIFY(item.timestamp.isValid());
    QVERIFY(item.isValid());
}

void TestModels::testRecentItemValidation()
{
    RecentItem item;
    
    // Invalid: empty URL
    QVERIFY(!item.isValid());
    
    // Valid: URL set
    item.url = "https://meet.example.com/room";
    item.timestamp = QDateTime::currentDateTime();
    QVERIFY(item.isValid());
    
    // Invalid: invalid timestamp
    item.timestamp = QDateTime();
    QVERIFY(!item.isValid());
}

void TestModels::testRecentItemDisplayText()
{
    RecentItem item;
    
    // Empty display name should return URL
    item.url = "https://meet.example.com/room";
    QCOMPARE(item.getDisplayText(), item.url);
    
    // Non-empty display name should return display name
    item.displayName = "My Room";
    QCOMPARE(item.getDisplayText(), "My Room");
}

void TestModels::testRecentItemComparison()
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime earlier = now.addSecs(-3600); // 1 hour ago
    
    RecentItem item1("url1");
    item1.timestamp = now;
    
    RecentItem item2("url2");
    item2.timestamp = earlier;
    
    // item1 should be "less than" item2 (newer items are "smaller")
    QVERIFY(item1 < item2);
    QVERIFY(!(item2 < item1));
}

void TestModels::testRecentItemEquality()
{
    RecentItem item1("https://meet.example.com/room");
    RecentItem item2("https://meet.example.com/room");
    RecentItem item3("https://meet.example.com/other-room");
    
    QVERIFY(item1 == item2);
    QVERIFY(!(item1 == item3));
}

void TestModels::testRecentItemJsonSerialization()
{
    QString testUrl = "https://meet.example.com/test-room";
    QString testDisplayName = "Test Room";
    QDateTime testTime = QDateTime::currentDateTime();
    
    RecentItem original(testUrl, testDisplayName);
    original.timestamp = testTime;
    original.accessCount = 5;
    
    // Serialize to JSON
    QJsonObject json = original.toJson();
    
    // Deserialize from JSON
    RecentItem deserialized = RecentItem::fromJson(json);
    
    // Verify equality
    QCOMPARE(deserialized.url, original.url);
    QCOMPARE(deserialized.displayName, original.displayName);
    QCOMPARE(deserialized.accessCount, original.accessCount);
    // Note: timestamp comparison might have slight precision differences
    QVERIFY(qAbs(deserialized.timestamp.msecsTo(original.timestamp)) < 1000);
}

void TestModels::testRecentItemUpdateAccess()
{
    RecentItem item("https://meet.example.com/room");
    QDateTime originalTime = item.timestamp;
    int originalCount = item.accessCount;
    
    // Wait a bit to ensure timestamp difference
    QTest::qWait(10);
    
    item.updateAccess();
    
    QVERIFY(item.timestamp > originalTime);
    QCOMPARE(item.accessCount, originalCount + 1);
}

void TestModels::testRecentItemRoomNameExtraction_data()
{
    QTest::addColumn<QString>("url");
    QTest::addColumn<QString>("expected");
    
    QTest::newRow("simple room") 
        << "https://meet.jit.si/test-room" << "test-room";
    QTest::newRow("nested path") 
        << "https://meet.example.com/path/to/room" << "room";
    QTest::newRow("with query params") 
        << "https://meet.jit.si/room?param=value" << "room";
    QTest::newRow("with fragment") 
        << "https://meet.jit.si/room#fragment" << "room";
    QTest::newRow("root path") 
        << "https://meet.jit.si/" << "meet.jit.si";
    QTest::newRow("no path") 
        << "https://meet.jit.si" << "meet.jit.si";
    QTest::newRow("empty url") 
        << "" << "";
}

void TestModels::testRecentItemRoomNameExtraction()
{
    QFETCH(QString, url);
    QFETCH(QString, expected);
    
    RecentItem item;
    QString result = item.extractRoomNameFromUrl(url);
    
    QCOMPARE(result, expected);
}

QTEST_MAIN(TestModels)
#include "test_models.moc"
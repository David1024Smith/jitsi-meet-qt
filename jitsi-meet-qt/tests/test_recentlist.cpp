#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include "RecentListWidget.h"
#include "models/RecentItem.h"

class TestRecentList : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // RecentItem tests
    void testRecentItemCreation();
    void testRecentItemValidation();
    void testRecentItemSerialization();
    void testRecentItemComparison();
    
    // RecentListWidget tests
    void testRecentListWidgetCreation();
    void testAddRecentItem();
    void testRemoveRecentItem();
    void testClearRecentItems();
    void testMaxItemsLimit();
    void testEmptyState();
    void testItemClickSignals();
    void testItemSorting();

private:
    RecentListWidget* m_widget;
    QApplication* m_app;
};

void TestRecentList::initTestCase()
{
    // Qt Test framework handles QApplication creation
}

void TestRecentList::cleanupTestCase()
{
    // Cleanup handled by Qt Test framework
}

void TestRecentList::init()
{
    m_widget = new RecentListWidget();
}

void TestRecentList::cleanup()
{
    delete m_widget;
    m_widget = nullptr;
}

void TestRecentList::testRecentItemCreation()
{
    // Test default constructor
    RecentItem item1;
    QVERIFY(!item1.isValid());
    QCOMPARE(item1.accessCount, 0);
    
    // Test parameterized constructor
    QString testUrl = "https://meet.jit.si/test-room";
    RecentItem item2(testUrl, "Test Room");
    QVERIFY(item2.isValid());
    QCOMPARE(item2.url, testUrl);
    QCOMPARE(item2.displayName, QString("Test Room"));
    QCOMPARE(item2.accessCount, 1);
    QVERIFY(item2.timestamp.isValid());
    
    // Test constructor without display name
    RecentItem item3(testUrl);
    QVERIFY(item3.isValid());
    QCOMPARE(item3.url, testUrl);
    QVERIFY(!item3.displayName.isEmpty()); // Should extract from URL
}

void TestRecentList::testRecentItemValidation()
{
    RecentItem validItem("https://meet.jit.si/room", "Room");
    QVERIFY(validItem.isValid());
    
    RecentItem invalidItem;
    QVERIFY(!invalidItem.isValid());
    
    RecentItem emptyUrlItem("", "Room");
    QVERIFY(!emptyUrlItem.isValid());
}

void TestRecentList::testRecentItemSerialization()
{
    QString testUrl = "https://meet.jit.si/test-room";
    RecentItem original(testUrl, "Test Room");
    
    // Test serialization
    QJsonObject json = original.toJson();
    QVERIFY(!json.isEmpty());
    QCOMPARE(json["url"].toString(), testUrl);
    QCOMPARE(json["displayName"].toString(), QString("Test Room"));
    QCOMPARE(json["accessCount"].toInt(), 1);
    
    // Test deserialization
    RecentItem restored = RecentItem::fromJson(json);
    QCOMPARE(restored.url, original.url);
    QCOMPARE(restored.displayName, original.displayName);
    QCOMPARE(restored.accessCount, original.accessCount);
}

void TestRecentList::testRecentItemComparison()
{
    RecentItem item1("https://meet.jit.si/room1", "Room 1");
    QTest::qWait(10); // Ensure different timestamps
    RecentItem item2("https://meet.jit.si/room2", "Room 2");
    
    // item2 should be "less than" item1 (newer timestamp)
    QVERIFY(item2 < item1);
    
    // Test equality
    RecentItem item3("https://meet.jit.si/room1", "Different Name");
    QVERIFY(item1 == item3); // Same URL
}

void TestRecentList::testRecentListWidgetCreation()
{
    QVERIFY(m_widget != nullptr);
    QVERIFY(m_widget->isEmpty());
    QCOMPARE(m_widget->maxItems(), 10); // Default max items
}

void TestRecentList::testAddRecentItem()
{
    QSignalSpy spy(m_widget, &RecentListWidget::listChanged);
    
    RecentItem item("https://meet.jit.si/test", "Test");
    m_widget->addRecentItem(item);
    
    QVERIFY(!m_widget->isEmpty());
    QCOMPARE(m_widget->getRecentItems().size(), 1);
    QCOMPARE(spy.count(), 1);
    
    // Add same item again (should update, not duplicate)
    m_widget->addRecentItem(item);
    QCOMPARE(m_widget->getRecentItems().size(), 1);
    QCOMPARE(spy.count(), 2);
}

void TestRecentList::testRemoveRecentItem()
{
    QString testUrl = "https://meet.jit.si/test";
    RecentItem item(testUrl, "Test");
    m_widget->addRecentItem(item);
    
    QSignalSpy spy(m_widget, &RecentListWidget::listChanged);
    
    m_widget->removeRecentItem(testUrl);
    QVERIFY(m_widget->isEmpty());
    QCOMPARE(spy.count(), 1);
}

void TestRecentList::testClearRecentItems()
{
    // Add multiple items
    m_widget->addRecentItem(RecentItem("https://meet.jit.si/room1", "Room 1"));
    m_widget->addRecentItem(RecentItem("https://meet.jit.si/room2", "Room 2"));
    
    QSignalSpy spy(m_widget, &RecentListWidget::listChanged);
    
    m_widget->clearRecentItems();
    QVERIFY(m_widget->isEmpty());
    QCOMPARE(spy.count(), 1);
}

void TestRecentList::testMaxItemsLimit()
{
    m_widget->setMaxItems(3);
    QCOMPARE(m_widget->maxItems(), 3);
    
    // Add more items than the limit
    for (int i = 0; i < 5; ++i) {
        QString url = QString("https://meet.jit.si/room%1").arg(i);
        m_widget->addRecentItem(RecentItem(url, QString("Room %1").arg(i)));
    }
    
    // Should only keep the maximum number of items
    QCOMPARE(m_widget->getRecentItems().size(), 3);
}

void TestRecentList::testEmptyState()
{
    // Widget should show empty state when no items
    QVERIFY(m_widget->isEmpty());
    
    // Add an item
    m_widget->addRecentItem(RecentItem("https://meet.jit.si/test", "Test"));
    QVERIFY(!m_widget->isEmpty());
    
    // Clear items
    m_widget->clearRecentItems();
    QVERIFY(m_widget->isEmpty());
}

void TestRecentList::testItemClickSignals()
{
    QString testUrl = "https://meet.jit.si/test";
    RecentItem item(testUrl, "Test");
    m_widget->addRecentItem(item);
    
    QSignalSpy clickSpy(m_widget, &RecentListWidget::itemClicked);
    QSignalSpy doubleClickSpy(m_widget, &RecentListWidget::itemDoubleClicked);
    
    // Note: Testing actual clicks would require more complex setup
    // This test verifies the signals exist and can be connected
    QVERIFY(clickSpy.isValid());
    QVERIFY(doubleClickSpy.isValid());
}

void TestRecentList::testItemSorting()
{
    // Add items with different timestamps
    RecentItem item1("https://meet.jit.si/room1", "Room 1");
    QTest::qWait(10);
    RecentItem item2("https://meet.jit.si/room2", "Room 2");
    QTest::qWait(10);
    RecentItem item3("https://meet.jit.si/room3", "Room 3");
    
    // Add in reverse order
    m_widget->addRecentItem(item1);
    m_widget->addRecentItem(item2);
    m_widget->addRecentItem(item3);
    
    QList<RecentItem> items = m_widget->getRecentItems();
    QCOMPARE(items.size(), 3);
    
    // Should be sorted by timestamp (newest first)
    QCOMPARE(items[0].url, item3.url);
    QCOMPARE(items[1].url, item2.url);
    QCOMPARE(items[2].url, item1.url);
}

QTEST_MAIN(TestRecentList)
#include "test_recentlist.moc"
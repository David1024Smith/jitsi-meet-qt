#include <QtTest/QtTest>
#include <QApplication>
#include <QSignalSpy>
#include <QPushButton>
#include "NavigationBar.h"

class TestNavigationBar : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // 测试基本功能
    void testConstructor();
    void testButtonConfiguration();
    void testTitleSetting();
    void testButtonVisibility();
    
    // 测试信号发射
    void testSettingsButtonSignal();
    void testAboutButtonSignal();
    void testBackButtonSignal();
    
    // 测试样式和布局
    void testFixedHeight();
    void testButtonStyles();

private:
    NavigationBar* m_navigationBar;
};

void TestNavigationBar::initTestCase()
{
    // 测试套件初始化
}

void TestNavigationBar::cleanupTestCase()
{
    // 测试套件清理
}

void TestNavigationBar::init()
{
    m_navigationBar = new NavigationBar();
}

void TestNavigationBar::cleanup()
{
    delete m_navigationBar;
    m_navigationBar = nullptr;
}

void TestNavigationBar::testConstructor()
{
    QVERIFY(m_navigationBar != nullptr);
    QCOMPARE(m_navigationBar->height(), 50);
    
    // 默认配置应该显示设置和关于按钮
    QVERIFY(m_navigationBar->isButtonVisible(NavigationBar::SettingsButton));
    QVERIFY(m_navigationBar->isButtonVisible(NavigationBar::AboutButton));
    QVERIFY(!m_navigationBar->isButtonVisible(NavigationBar::BackButton));
}

void TestNavigationBar::testButtonConfiguration()
{
    // 测试只显示返回按钮
    m_navigationBar->setButtonConfiguration(NavigationBar::BackButton);
    QVERIFY(m_navigationBar->isButtonVisible(NavigationBar::BackButton));
    QVERIFY(!m_navigationBar->isButtonVisible(NavigationBar::SettingsButton));
    QVERIFY(!m_navigationBar->isButtonVisible(NavigationBar::AboutButton));
    
    // 测试显示所有按钮
    NavigationBar::ButtonTypes allButtons = NavigationBar::BackButton | 
                                           NavigationBar::SettingsButton | 
                                           NavigationBar::AboutButton;
    m_navigationBar->setButtonConfiguration(allButtons);
    QVERIFY(m_navigationBar->isButtonVisible(NavigationBar::BackButton));
    QVERIFY(m_navigationBar->isButtonVisible(NavigationBar::SettingsButton));
    QVERIFY(m_navigationBar->isButtonVisible(NavigationBar::AboutButton));
    
    // 测试不显示任何按钮
    m_navigationBar->setButtonConfiguration(NavigationBar::ButtonTypes());
    QVERIFY(!m_navigationBar->isButtonVisible(NavigationBar::BackButton));
    QVERIFY(!m_navigationBar->isButtonVisible(NavigationBar::SettingsButton));
    QVERIFY(!m_navigationBar->isButtonVisible(NavigationBar::AboutButton));
}

void TestNavigationBar::testTitleSetting()
{
    const QString testTitle = "Test Title";
    m_navigationBar->setTitle(testTitle);
    
    // 查找标题标签并验证文本
    QLabel* titleLabel = m_navigationBar->findChild<QLabel*>("titleLabel");
    QVERIFY(titleLabel != nullptr);
    QCOMPARE(titleLabel->text(), testTitle);
}

void TestNavigationBar::testButtonVisibility()
{
    // 设置显示返回按钮
    m_navigationBar->setButtonConfiguration(NavigationBar::BackButton);
    
    QPushButton* backButton = m_navigationBar->findChild<QPushButton*>("backButton");
    QPushButton* settingsButton = m_navigationBar->findChild<QPushButton*>("settingsButton");
    QPushButton* aboutButton = m_navigationBar->findChild<QPushButton*>("aboutButton");
    
    QVERIFY(backButton != nullptr);
    QVERIFY(settingsButton != nullptr);
    QVERIFY(aboutButton != nullptr);
    
    QVERIFY(backButton->isVisible());
    QVERIFY(!settingsButton->isVisible());
    QVERIFY(!aboutButton->isVisible());
}

void TestNavigationBar::testSettingsButtonSignal()
{
    QSignalSpy spy(m_navigationBar, &NavigationBar::settingsClicked);
    
    QPushButton* settingsButton = m_navigationBar->findChild<QPushButton*>("settingsButton");
    QVERIFY(settingsButton != nullptr);
    
    // 模拟点击设置按钮
    settingsButton->click();
    
    QCOMPARE(spy.count(), 1);
}

void TestNavigationBar::testAboutButtonSignal()
{
    QSignalSpy spy(m_navigationBar, &NavigationBar::aboutClicked);
    
    QPushButton* aboutButton = m_navigationBar->findChild<QPushButton*>("aboutButton");
    QVERIFY(aboutButton != nullptr);
    
    // 模拟点击关于按钮
    aboutButton->click();
    
    QCOMPARE(spy.count(), 1);
}

void TestNavigationBar::testBackButtonSignal()
{
    // 首先设置显示返回按钮
    m_navigationBar->setButtonConfiguration(NavigationBar::BackButton);
    
    QSignalSpy spy(m_navigationBar, &NavigationBar::backClicked);
    
    QPushButton* backButton = m_navigationBar->findChild<QPushButton*>("backButton");
    QVERIFY(backButton != nullptr);
    
    // 模拟点击返回按钮
    backButton->click();
    
    QCOMPARE(spy.count(), 1);
}

void TestNavigationBar::testFixedHeight()
{
    QCOMPARE(m_navigationBar->height(), 50);
    QCOMPARE(m_navigationBar->minimumHeight(), 50);
    QCOMPARE(m_navigationBar->maximumHeight(), 50);
}

void TestNavigationBar::testButtonStyles()
{
    // 验证按钮有正确的对象名称用于样式
    QPushButton* backButton = m_navigationBar->findChild<QPushButton*>("backButton");
    QPushButton* settingsButton = m_navigationBar->findChild<QPushButton*>("settingsButton");
    QPushButton* aboutButton = m_navigationBar->findChild<QPushButton*>("aboutButton");
    
    QVERIFY(backButton != nullptr);
    QVERIFY(settingsButton != nullptr);
    QVERIFY(aboutButton != nullptr);
    
    QCOMPARE(backButton->objectName(), QString("backButton"));
    QCOMPARE(settingsButton->objectName(), QString("settingsButton"));
    QCOMPARE(aboutButton->objectName(), QString("aboutButton"));
    
    // 验证按钮文本
    QCOMPARE(backButton->text(), QString("← 返回"));
    QCOMPARE(settingsButton->text(), QString("设置"));
    QCOMPARE(aboutButton->text(), QString("关于"));
}

QTEST_MAIN(TestNavigationBar)
#include "test_navigationbar.moc"
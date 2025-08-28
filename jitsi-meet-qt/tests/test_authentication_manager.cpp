#include <QtTest/QtTest>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include "AuthenticationManager.h"

class TestAuthenticationManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();

    // JWT Token Tests
    void testParseValidJWTToken();
    void testParseInvalidJWTToken();
    void testVerifyJWTToken();
    void testJWTTokenExpiration();

    // Authentication Tests
    void testGuestAuthentication();
    void testJWTAuthentication();
    void testPasswordAuthentication();
    void testAuthenticationFailure();

    // State Management Tests
    void testAuthStateChanges();
    void testLogout();
    void testTokenRefresh();

    // Room Permissions Tests
    void testRoomPermissionCheck();
    void testRoomPermissionUpdate();

private:
    AuthenticationManager* m_authManager;
    QString createTestJWTToken(const QJsonObject& payload = QJsonObject());
    QString base64UrlEncode(const QByteArray& input);
};

void TestAuthenticationManager::initTestCase()
{
    qDebug() << "Starting AuthenticationManager tests";
}

void TestAuthenticationManager::cleanupTestCase()
{
    qDebug() << "AuthenticationManager tests completed";
}

void TestAuthenticationManager::init()
{
    m_authManager = new AuthenticationManager(this);
}

void TestAuthenticationManager::cleanup()
{
    delete m_authManager;
    m_authManager = nullptr;
}

void TestAuthenticationManager::testParseValidJWTToken()
{
    // 创建测试JWT payload
    QJsonObject payload;
    payload["sub"] = "test-user-123";
    payload["name"] = "Test User";
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();

    QString token = createTestJWTToken(payload);
    
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(token);
    
    QVERIFY(tokenInfo.isValid);
    QCOMPARE(tokenInfo.claims["sub"].toString(), QString("test-user-123"));
    QCOMPARE(tokenInfo.claims["name"].toString(), QString("Test User"));
    QVERIFY(tokenInfo.issuedAt.isValid());
    QVERIFY(tokenInfo.expiresAt.isValid());
}

void TestAuthenticationManager::testParseInvalidJWTToken()
{
    // 测试无效格式的token
    QString invalidToken = "invalid.token";
    
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(invalidToken);
    
    QVERIFY(!tokenInfo.isValid);
}

void TestAuthenticationManager::testVerifyJWTToken()
{
    // 创建有效的JWT token
    QJsonObject payload;
    payload["sub"] = "test-user-123";
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();

    QString token = createTestJWTToken(payload);
    AuthenticationManager::JWTTokenInfo tokenInfo = m_authManager->parseJWTToken(token);
    
    QVERIFY(m_authManager->verifyJWTToken(tokenInfo));
    
    // 测试过期的token
    QJsonObject expiredPayload;
    expiredPayload["sub"] = "test-user-123";
    expiredPayload["iat"] = QDateTime::currentDateTime().addSecs(-7200).toSecsSinceEpoch();
    expiredPayload["exp"] = QDateTime::currentDateTime().addSecs(-3600).toSecsSinceEpoch();

    QString expiredToken = createTestJWTToken(expiredPayload);
    AuthenticationManager::JWTTokenInfo expiredTokenInfo = m_authManager->parseJWTToken(expiredToken);
    
    QVERIFY(!m_authManager->verifyJWTToken(expiredTokenInfo));
}

void TestAuthenticationManager::testJWTTokenExpiration()
{
    QSignalSpy tokenExpiringSpy(m_authManager, &AuthenticationManager::tokenExpiring);
    QSignalSpy tokenExpiredSpy(m_authManager, &AuthenticationManager::tokenExpired);
    
    // 创建即将过期的token（1分钟后过期）
    QJsonObject payload;
    payload["sub"] = "test-user-123";
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(60).toSecsSinceEpoch();

    QString token = createTestJWTToken(payload);
    m_authManager->authenticateWithJWT(token);
    
    // 模拟时间流逝，检查过期信号
    // 注意：这里需要手动调用checkTokenExpiration来模拟定时器
    // 在实际测试中可能需要使用QTest::qWait()或模拟时间
}

void TestAuthenticationManager::testGuestAuthentication()
{
    QSignalSpy authSucceededSpy(m_authManager, &AuthenticationManager::authenticationSucceeded);
    QSignalSpy authStateChangedSpy(m_authManager, &AuthenticationManager::authStateChanged);
    
    // 模拟访客认证
    m_authManager->authenticate("https://meet.jit.si", "test-room", "Test User");
    
    // 等待认证完成
    QTest::qWait(1500);
    
    QVERIFY(authSucceededSpy.count() > 0);
    QCOMPARE(m_authManager->authState(), AuthenticationManager::Authenticated);
    QCOMPARE(m_authManager->authType(), AuthenticationManager::Guest);
}

void TestAuthenticationManager::testJWTAuthentication()
{
    QSignalSpy authSucceededSpy(m_authManager, &AuthenticationManager::authenticationSucceeded);
    QSignalSpy authStateChangedSpy(m_authManager, &AuthenticationManager::authStateChanged);
    
    // 创建有效的JWT token
    QJsonObject payload;
    payload["sub"] = "test-user-123";
    payload["name"] = "Test User";
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();

    QString token = createTestJWTToken(payload);
    
    m_authManager->authenticateWithJWT(token);
    
    QCOMPARE(authSucceededSpy.count(), 1);
    QCOMPARE(m_authManager->authState(), AuthenticationManager::Authenticated);
    QCOMPARE(m_authManager->authType(), AuthenticationManager::JWT);
    QCOMPARE(m_authManager->userId(), QString("test-user-123"));
}

void TestAuthenticationManager::testPasswordAuthentication()
{
    QSignalSpy authFailedSpy(m_authManager, &AuthenticationManager::authenticationFailed);
    
    // 测试空密码
    m_authManager->authenticateWithPassword("");
    
    QCOMPARE(authFailedSpy.count(), 1);
    
    // 测试有效密码（这里会发送网络请求，实际测试中可能需要mock）
    m_authManager->authenticateWithPassword("test-password");
    
    // 由于这是网络请求，需要等待或使用mock
}

void TestAuthenticationManager::testAuthenticationFailure()
{
    QSignalSpy authFailedSpy(m_authManager, &AuthenticationManager::authenticationFailed);
    
    // 测试无效JWT token
    m_authManager->authenticateWithJWT("invalid-token");
    
    QCOMPARE(authFailedSpy.count(), 1);
    QCOMPARE(m_authManager->authState(), AuthenticationManager::NotAuthenticated);
}

void TestAuthenticationManager::testAuthStateChanges()
{
    QSignalSpy authStateChangedSpy(m_authManager, &AuthenticationManager::authStateChanged);
    
    QCOMPARE(m_authManager->authState(), AuthenticationManager::NotAuthenticated);
    
    // 开始认证
    m_authManager->authenticate("https://meet.jit.si", "test-room", "Test User");
    
    // 应该有状态变化信号
    QTest::qWait(100);
    QVERIFY(authStateChangedSpy.count() > 0);
}

void TestAuthenticationManager::testLogout()
{
    // 先进行认证
    QJsonObject payload;
    payload["sub"] = "test-user-123";
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();

    QString token = createTestJWTToken(payload);
    m_authManager->authenticateWithJWT(token);
    
    QCOMPARE(m_authManager->authState(), AuthenticationManager::Authenticated);
    
    // 注销
    QSignalSpy authStateChangedSpy(m_authManager, &AuthenticationManager::authStateChanged);
    m_authManager->logout();
    
    QCOMPARE(m_authManager->authState(), AuthenticationManager::NotAuthenticated);
    QCOMPARE(m_authManager->authType(), AuthenticationManager::None);
    QVERIFY(m_authManager->authToken().isEmpty());
    QVERIFY(m_authManager->userId().isEmpty());
}

void TestAuthenticationManager::testTokenRefresh()
{
    // 这个测试需要mock网络请求，因为涉及到实际的HTTP请求
    // 在实际项目中，可以使用QNetworkAccessManager的mock或测试服务器
    
    QJsonObject payload;
    payload["sub"] = "test-user-123";
    payload["iat"] = QDateTime::currentDateTime().toSecsSinceEpoch();
    payload["exp"] = QDateTime::currentDateTime().addSecs(3600).toSecsSinceEpoch();

    QString token = createTestJWTToken(payload);
    m_authManager->authenticateWithJWT(token);
    
    // 调用刷新token
    m_authManager->refreshAuthToken();
    
    // 在实际测试中，这里需要验证网络请求是否正确发送
}

void TestAuthenticationManager::testRoomPermissionCheck()
{
    // 测试房间权限检查
    m_authManager->checkRoomPermissions("test-room");
    
    // 这里需要mock网络响应来测试权限解析
}

void TestAuthenticationManager::testRoomPermissionUpdate()
{
    QSignalSpy permissionsUpdatedSpy(m_authManager, &AuthenticationManager::roomPermissionsUpdated);
    
    // 这个测试需要模拟网络响应来触发权限更新信号
    // 在实际实现中，可以通过mock QNetworkReply来实现
}

QString TestAuthenticationManager::createTestJWTToken(const QJsonObject& payload)
{
    // 创建简单的JWT header
    QJsonObject header;
    header["alg"] = "HS256";
    header["typ"] = "JWT";
    
    QJsonDocument headerDoc(header);
    QJsonDocument payloadDoc(payload);
    
    QString headerB64 = base64UrlEncode(headerDoc.toJson(QJsonDocument::Compact));
    QString payloadB64 = base64UrlEncode(payloadDoc.toJson(QJsonDocument::Compact));
    QString signature = base64UrlEncode("fake-signature");
    
    return headerB64 + "." + payloadB64 + "." + signature;
}

QString TestAuthenticationManager::base64UrlEncode(const QByteArray& input)
{
    QString encoded = input.toBase64();
    encoded.replace('+', '-');
    encoded.replace('/', '_');
    encoded.remove('=');
    return encoded;
}

QTEST_MAIN(TestAuthenticationManager)
#include "test_authentication_manager.moc"
#include "MockTheme.h"

MockTheme::MockTheme(const QString& name, QObject* parent)
    : BaseTheme(name, parent)
    , m_mockValid(true)
    , m_mockLoadResult(true)
    , m_loadFromResourceCallCount(0)
    , m_loadFromFileCallCount(0)
{
    // Set default mock values
    setPrimaryColor(QColor("#007ACC"));
    setBackgroundColor(QColor("#1E1E1E"));
    setTextColor(QColor("#FFFFFF"));
    setAccentColor(QColor("#0E639C"));
    
    m_mockStyleSheet = "/* Mock Theme Stylesheet */\n"
                      "QWidget { background-color: #1E1E1E; color: #FFFFFF; }";
    
    m_mockProperties["primaryColor"] = "#007ACC";
    m_mockProperties["backgroundColor"] = "#1E1E1E";
    m_mockProperties["textColor"] = "#FFFFFF";
    m_mockProperties["accentColor"] = "#0E639C";
}

MockTheme::~MockTheme()
{
}

bool MockTheme::loadFromResource(const QString& resourcePath)
{
    m_loadFromResourceCallCount++;
    m_lastResourcePath = resourcePath;
    
    emit mockLoadFromResourceCalled(resourcePath);
    
    if (m_mockLoadResult) {
        // Simulate successful loading
        setStyleSheet(m_mockStyleSheet);
    }
    
    return m_mockLoadResult;
}

bool MockTheme::loadFromFile(const QString& filePath)
{
    m_loadFromFileCallCount++;
    m_lastFilePath = filePath;
    
    emit mockLoadFromFileCalled(filePath);
    
    if (m_mockLoadResult) {
        // Simulate successful loading
        setStyleSheet(m_mockStyleSheet);
    }
    
    return m_mockLoadResult;
}

QString MockTheme::styleSheet() const
{
    return m_mockStyleSheet;
}

QVariantMap MockTheme::properties() const
{
    return m_mockProperties;
}

bool MockTheme::isValid() const
{
    return m_mockValid;
}

void MockTheme::setMockStyleSheet(const QString& styleSheet)
{
    m_mockStyleSheet = styleSheet;
    setStyleSheet(styleSheet);
}

void MockTheme::setMockProperties(const QVariantMap& properties)
{
    m_mockProperties = properties;
    
    // Update theme colors from properties
    if (properties.contains("primaryColor")) {
        setPrimaryColor(QColor(properties["primaryColor"].toString()));
    }
    if (properties.contains("backgroundColor")) {
        setBackgroundColor(QColor(properties["backgroundColor"].toString()));
    }
    if (properties.contains("textColor")) {
        setTextColor(QColor(properties["textColor"].toString()));
    }
    if (properties.contains("accentColor")) {
        setAccentColor(QColor(properties["accentColor"].toString()));
    }
}

void MockTheme::setMockValid(bool valid)
{
    m_mockValid = valid;
}

void MockTheme::setMockLoadResult(bool result)
{
    m_mockLoadResult = result;
}

void MockTheme::resetCallCounts()
{
    m_loadFromResourceCallCount = 0;
    m_loadFromFileCallCount = 0;
    m_lastResourcePath.clear();
    m_lastFilePath.clear();
}

#include "MockTheme.moc"
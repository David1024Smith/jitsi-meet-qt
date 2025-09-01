#include "SettingsWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QSplitter>
#include <QLabel>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QShowEvent>

class SettingsWidget::Private
{
public:
    QLineEdit* searchEdit;
    QTreeWidget* settingsTree;
    QPushButton* expandAllButton;
    QPushButton* collapseAllButton;
    QPushButton* helpButton;
    QSplitter* splitter;
    QString currentSearchText;
};

SettingsWidget::SettingsWidget(QWidget* parent)
    : QWidget(parent)
    , d(new Private)
{
    setupUI();
    connectSignals();
}

SettingsWidget::~SettingsWidget() = default;

void SettingsWidget::setSearchText(const QString& text)
{
    if (d->currentSearchText != text) {
        d->currentSearchText = text;
        d->searchEdit->setText(text);
        filterSettings(text);
    }
}

void SettingsWidget::expandAll()
{
    d->settingsTree->expandAll();
}

void SettingsWidget::collapseAll()
{
    d->settingsTree->collapseAll();
}

void SettingsWidget::showHelp()
{
    // Show help dialog or documentation
}

void SettingsWidget::onSearchTextChanged(const QString& text)
{
    d->currentSearchText = text;
    filterSettings(text);
}

void SettingsWidget::setupUI()
{
    auto* layout = new QVBoxLayout(this);
    
    // Search bar
    auto* searchLayout = new QHBoxLayout;
    d->searchEdit = new QLineEdit;
    d->searchEdit->setPlaceholderText("Search settings...");
    searchLayout->addWidget(new QLabel("Search:"));
    searchLayout->addWidget(d->searchEdit);
    
    // Buttons
    auto* buttonLayout = new QHBoxLayout;
    d->expandAllButton = new QPushButton("Expand All");
    d->collapseAllButton = new QPushButton("Collapse All");
    d->helpButton = new QPushButton("Help");
    
    buttonLayout->addWidget(d->expandAllButton);
    buttonLayout->addWidget(d->collapseAllButton);
    buttonLayout->addStretch();
    buttonLayout->addWidget(d->helpButton);
    
    // Settings tree
    d->settingsTree = new QTreeWidget;
    d->settingsTree->setHeaderLabel("Settings");
    
    // Splitter
    d->splitter = new QSplitter(Qt::Horizontal);
    d->splitter->addWidget(d->settingsTree);
    
    layout->addLayout(searchLayout);
    layout->addLayout(buttonLayout);
    layout->addWidget(d->splitter);
}

void SettingsWidget::connectSignals()
{
    connect(d->searchEdit, &QLineEdit::textChanged,
            this, &SettingsWidget::onSearchTextChanged);
    connect(d->expandAllButton, &QPushButton::clicked,
            this, &SettingsWidget::expandAll);
    connect(d->collapseAllButton, &QPushButton::clicked,
            this, &SettingsWidget::collapseAll);
    connect(d->helpButton, &QPushButton::clicked,
            this, &SettingsWidget::showHelp);
}

void SettingsWidget::filterSettings(const QString& text)
{
    // Filter settings based on search text
    Q_UNUSED(text)
    // Implementation would filter the tree widget items
}

void SettingsWidget::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void SettingsWidget::closeEvent(QCloseEvent* event)
{
    // Handle close event
    event->accept();
}

void SettingsWidget::showEvent(QShowEvent* event)
{
    // Handle show event
    QWidget::showEvent(event);
}

void SettingsWidget::changeEvent(QEvent* event)
{
    // Handle change events (like language changes)
    QWidget::changeEvent(event);
}
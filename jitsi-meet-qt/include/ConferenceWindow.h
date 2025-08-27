#ifndef CONFERENCEWINDOW_H
#define CONFERENCEWINDOW_H

#include <QMainWindow>
#include <QWebEngineView>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QTimer>
#include <QUrl>
#include "NavigationBar.h"

class ConferenceWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ConferenceWindow(QWidget *parent = nullptr);
    ~ConferenceWindow();

    // 加载会议
    void loadConference(const QString& url);
    
    // 设置服务器URL
    void setServerUrl(const QString& serverUrl);
    
    // 获取当前会议URL
    QString currentUrl() const;

public slots:
    // 更新翻译文本
    void retranslateUi();

signals:
    void backToWelcome();
    void conferenceJoined(const QString& url);
    void loadingError(const QString& error);

private slots:
    void onLoadFinished(bool success);
    void onLoadProgress(int progress);
    void onBackButtonClicked();
    void onLoadStarted();
    void hideProgressBar();

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void setupUI();
    void setupWebEngine();
    void setupConnections();
    void applyStyles();
    void showError(const QString& message);
    void showLoading();
    void hideLoading();
    QString buildJitsiUrl(const QString& input);
    bool isValidUrl(const QString& url);

    // UI组件
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    NavigationBar* m_navigationBar;
    QWebEngineView* m_webView;
    QProgressBar* m_progressBar;
    QLabel* m_statusLabel;
    QLabel* m_errorLabel;
    
    // 定时器
    QTimer* m_progressTimer;
    
    // 状态变量
    QString m_currentUrl;
    QString m_serverUrl;
    bool m_isLoading;
};

#endif // CONFERENCEWINDOW_H
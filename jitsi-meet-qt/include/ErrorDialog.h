#ifndef ERRORDIALOG_H
#define ERRORDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QProgressBar>
#include "JitsiError.h"

/**
 * @brief 自定义错误对话框
 * 
 * 提供更好的用户体验的错误显示对话框，支持错误详情展开、
 * 自动关闭、重试机制等功能
 */
class ErrorDialog : public QDialog
{
    Q_OBJECT

public:
    /**
     * @brief 对话框结果枚举
     */
    enum Result {
        Retry = 1,
        Ignore = 2,
        Reset = 3,
        Cancel = 0
    };

public:
    explicit ErrorDialog(const JitsiError& error, QWidget* parent = nullptr);
    ~ErrorDialog();
    
    /**
     * @brief 设置是否显示详细信息
     */
    void setShowDetails(bool show);
    
    /**
     * @brief 设置自动关闭时间（秒）
     */
    void setAutoCloseTimeout(int seconds);
    
    /**
     * @brief 启用/禁用重试按钮
     */
    void setRetryEnabled(bool enabled);
    
    /**
     * @brief 启用/禁用重置按钮
     */
    void setResetEnabled(bool enabled);
    
    /**
     * @brief 设置错误图标
     */
    void setErrorIcon(const QPixmap& icon);
    
    /**
     * @brief 获取用户选择的结果
     */
    Result result() const { return m_result; }

protected:
    void showEvent(QShowEvent* event) override;
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onRetryClicked();
    void onIgnoreClicked();
    void onResetClicked();
    void onCancelClicked();
    void onDetailsToggled();
    void onAutoCloseTimeout();
    void updateCountdown();

private:
    void setupUI();
    void setupConnections();
    void updateButtonStates();
    void startCountdown();
    void stopCountdown();
    
private:
    JitsiError m_error;
    Result m_result;
    
    // UI组件
    QLabel* m_iconLabel;
    QLabel* m_messageLabel;
    QLabel* m_countdownLabel;
    QTextEdit* m_detailsEdit;
    QPushButton* m_retryButton;
    QPushButton* m_ignoreButton;
    QPushButton* m_resetButton;
    QPushButton* m_cancelButton;
    QPushButton* m_detailsButton;
    QProgressBar* m_countdownProgress;
    
    // 布局
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonLayout;
    QHBoxLayout* m_topLayout;
    
    // 定时器
    QTimer* m_autoCloseTimer;
    QTimer* m_countdownTimer;
    
    // 配置
    int m_autoCloseTimeout;
    int m_remainingSeconds;
    bool m_showDetails;
    bool m_retryEnabled;
    bool m_resetEnabled;
};

#endif // ERRORDIALOG_H
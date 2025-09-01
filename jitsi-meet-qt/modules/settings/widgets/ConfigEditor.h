#ifndef CONFIGEDITOR_H
#define CONFIGEDITOR_H

#include <QWidget>
#include <QVariant>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>
#include <memory>

// Forward declarations
class QVBoxLayout;
class QHBoxLayout;
class QSplitter;
class QTreeWidget;
class QTreeWidgetItem;
class QTextEdit;
class QPlainTextEdit;
class QLineEdit;
class QPushButton;
class QComboBox;
class QLabel;
class QTabWidget;
class QToolBar;
class QAction;
class QMenu;
class QCompleter;
class QSyntaxHighlighter;
class ConfigValidator;

/**
 * @brief 配置编辑器组件
 * 
 * 提供高级的配置文件编辑功能，支持JSON/INI/XML格式，语法高亮，
 * 实时验证，自动完成等功能。适合高级用户和开发者使用。
 */
class ConfigEditor : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(EditorMode editorMode READ editorMode WRITE setEditorMode NOTIFY editorModeChanged)
    Q_PROPERTY(QString configFormat READ configFormat WRITE setConfigFormat NOTIFY configFormatChanged)
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged)
    Q_PROPERTY(bool syntaxHighlighting READ isSyntaxHighlightingEnabled WRITE setSyntaxHighlightingEnabled NOTIFY syntaxHighlightingChanged)
    Q_PROPERTY(bool autoValidation READ isAutoValidationEnabled WRITE setAutoValidationEnabled NOTIFY autoValidationChanged)

public:
    /**
     * @brief 编辑器模式枚举
     */
    enum EditorMode {
        TreeMode,       ///< 树形编辑模式
        TextMode,       ///< 文本编辑模式
        SplitMode,      ///< 分割模式（树形+文本）
        FormMode        ///< 表单模式
    };
    Q_ENUM(EditorMode)

    /**
     * @brief 配置格式枚举
     */
    enum ConfigFormat {
        JsonFormat,     ///< JSON 格式
        IniFormat,      ///< INI 格式
        XmlFormat,      ///< XML 格式
        YamlFormat,     ///< YAML 格式
        TomlFormat      ///< TOML 格式
    };
    Q_ENUM(ConfigFormat)

    /**
     * @brief 验证状态枚举
     */
    enum ValidationStatus {
        Valid,          ///< 有效
        Invalid,        ///< 无效
        Warning,        ///< 警告
        Unknown         ///< 未知
    };
    Q_ENUM(ValidationStatus)

    /**
     * @brief 编辑器选项结构
     */
    struct EditorOptions {
        bool showLineNumbers;       ///< 显示行号
        bool wordWrap;              ///< 自动换行
        bool autoIndent;            ///< 自动缩进
        bool autoComplete;          ///< 自动完成
        bool bracketMatching;       ///< 括号匹配
        bool foldingEnabled;        ///< 代码折叠
        int tabSize;                ///< 制表符大小
        QString fontFamily;         ///< 字体族
        int fontSize;               ///< 字体大小
        QString colorScheme;        ///< 颜色方案
        
        EditorOptions() 
            : showLineNumbers(true), wordWrap(false), autoIndent(true)
            , autoComplete(true), bracketMatching(true), foldingEnabled(true)
            , tabSize(4), fontFamily("Consolas"), fontSize(10), colorScheme("default") {}
    };

    explicit ConfigEditor(QWidget* parent = nullptr);
    ~ConfigEditor();

    /**
     * @brief 获取编辑器模式
     * @return 编辑器模式
     */
    EditorMode editorMode() const;

    /**
     * @brief 设置编辑器模式
     * @param mode 编辑器模式
     */
    void setEditorMode(EditorMode mode);

    /**
     * @brief 获取配置格式
     * @return 配置格式字符串
     */
    QString configFormat() const;

    /**
     * @brief 设置配置格式
     * @param format 配置格式字符串
     */
    void setConfigFormat(const QString& format);

    /**
     * @brief 设置配置格式
     * @param format 配置格式枚举
     */
    void setConfigFormat(ConfigFormat format);

    /**
     * @brief 获取配置格式枚举
     * @return 配置格式枚举
     */
    ConfigFormat configFormatEnum() const;

    /**
     * @brief 检查是否为只读模式
     * @return 是否只读
     */
    bool isReadOnly() const;

    /**
     * @brief 设置只读模式
     * @param readOnly 是否只读
     */
    void setReadOnly(bool readOnly);

    /**
     * @brief 检查是否启用语法高亮
     * @return 是否启用语法高亮
     */
    bool isSyntaxHighlightingEnabled() const;

    /**
     * @brief 启用/禁用语法高亮
     * @param enabled 是否启用
     */
    void setSyntaxHighlightingEnabled(bool enabled);

    /**
     * @brief 检查是否启用自动验证
     * @return 是否启用自动验证
     */
    bool isAutoValidationEnabled() const;

    /**
     * @brief 启用/禁用自动验证
     * @param enabled 是否启用
     */
    void setAutoValidationEnabled(bool enabled);

    // 内容管理
    /**
     * @brief 设置配置内容（JSON对象）
     * @param config 配置对象
     */
    void setConfig(const QJsonObject& config);

    /**
     * @brief 获取配置内容（JSON对象）
     * @return 配置对象
     */
    QJsonObject getConfig() const;

    /**
     * @brief 设置配置内容（文本）
     * @param text 配置文本
     */
    void setConfigText(const QString& text);

    /**
     * @brief 获取配置内容（文本）
     * @return 配置文本
     */
    QString getConfigText() const;

    /**
     * @brief 设置配置内容（变体映射）
     * @param config 配置映射
     */
    void setConfigMap(const QVariantMap& config);

    /**
     * @brief 获取配置内容（变体映射）
     * @return 配置映射
     */
    QVariantMap getConfigMap() const;

    /**
     * @brief 清空内容
     */
    void clear();

    /**
     * @brief 检查是否为空
     * @return 是否为空
     */
    bool isEmpty() const;

    /**
     * @brief 检查是否有修改
     * @return 是否有修改
     */
    bool isModified() const;

    /**
     * @brief 设置修改状态
     * @param modified 是否修改
     */
    void setModified(bool modified);

    // 文件操作
    /**
     * @brief 从文件加载配置
     * @param filePath 文件路径
     * @return 加载是否成功
     */
    bool loadFromFile(const QString& filePath);

    /**
     * @brief 保存配置到文件
     * @param filePath 文件路径
     * @return 保存是否成功
     */
    bool saveToFile(const QString& filePath);

    /**
     * @brief 获取当前文件路径
     * @return 文件路径
     */
    QString currentFilePath() const;

    /**
     * @brief 设置当前文件路径
     * @param filePath 文件路径
     */
    void setCurrentFilePath(const QString& filePath);

    // 验证功能
    /**
     * @brief 设置配置验证器
     * @param validator 验证器实例
     */
    void setValidator(ConfigValidator* validator);

    /**
     * @brief 获取配置验证器
     * @return 验证器实例
     */
    ConfigValidator* validator() const;

    /**
     * @brief 验证当前配置
     * @return 验证状态
     */
    ValidationStatus validate();

    /**
     * @brief 获取验证错误
     * @return 错误列表
     */
    QStringList validationErrors() const;

    /**
     * @brief 获取验证警告
     * @return 警告列表
     */
    QStringList validationWarnings() const;

    // 编辑功能
    /**
     * @brief 撤销操作
     */
    void undo();

    /**
     * @brief 重做操作
     */
    void redo();

    /**
     * @brief 检查是否可以撤销
     * @return 是否可以撤销
     */
    bool canUndo() const;

    /**
     * @brief 检查是否可以重做
     * @return 是否可以重做
     */
    bool canRedo() const;

    /**
     * @brief 剪切选中内容
     */
    void cut();

    /**
     * @brief 复制选中内容
     */
    void copy();

    /**
     * @brief 粘贴内容
     */
    void paste();

    /**
     * @brief 全选
     */
    void selectAll();

    /**
     * @brief 查找文本
     * @param text 查找文本
     * @param options 查找选项
     * @return 是否找到
     */
    bool find(const QString& text, int options = 0);

    /**
     * @brief 替换文本
     * @param findText 查找文本
     * @param replaceText 替换文本
     * @param options 查找选项
     * @return 替换次数
     */
    int replace(const QString& findText, const QString& replaceText, int options = 0);

    // 格式化功能
    /**
     * @brief 格式化配置
     */
    void formatConfig();

    /**
     * @brief 压缩配置（移除空白）
     */
    void compactConfig();

    /**
     * @brief 排序配置键
     * @param recursive 是否递归排序
     */
    void sortKeys(bool recursive = true);

    /**
     * @brief 设置缩进
     * @param spaces 空格数
     */
    void setIndentation(int spaces);

    /**
     * @brief 获取缩进
     * @return 空格数
     */
    int indentation() const;

    // 编辑器选项
    /**
     * @brief 设置编辑器选项
     * @param options 编辑器选项
     */
    void setEditorOptions(const EditorOptions& options);

    /**
     * @brief 获取编辑器选项
     * @return 编辑器选项
     */
    EditorOptions editorOptions() const;

    /**
     * @brief 设置颜色方案
     * @param scheme 颜色方案名称
     */
    void setColorScheme(const QString& scheme);

    /**
     * @brief 获取颜色方案
     * @return 颜色方案名称
     */
    QString colorScheme() const;

    /**
     * @brief 设置字体
     * @param font 字体
     */
    void setEditorFont(const QFont& font);

    /**
     * @brief 获取字体
     * @return 字体
     */
    QFont editorFont() const;

    // 工具栏和菜单
    /**
     * @brief 获取工具栏
     * @return 工具栏实例
     */
    QToolBar* toolBar() const;

    /**
     * @brief 获取上下文菜单
     * @return 菜单实例
     */
    QMenu* contextMenu() const;

    /**
     * @brief 添加自定义动作
     * @param action 动作
     */
    void addCustomAction(QAction* action);

    /**
     * @brief 移除自定义动作
     * @param action 动作
     */
    void removeCustomAction(QAction* action);

    // 书签功能
    /**
     * @brief 添加书签
     * @param line 行号
     * @param name 书签名称
     */
    void addBookmark(int line, const QString& name = QString());

    /**
     * @brief 移除书签
     * @param line 行号
     */
    void removeBookmark(int line);

    /**
     * @brief 获取所有书签
     * @return 书签映射（行号 -> 名称）
     */
    QMap<int, QString> bookmarks() const;

    /**
     * @brief 跳转到书签
     * @param line 行号
     */
    void gotoBookmark(int line);

    /**
     * @brief 跳转到下一个书签
     */
    void nextBookmark();

    /**
     * @brief 跳转到上一个书签
     */
    void previousBookmark();

public slots:
    /**
     * @brief 刷新编辑器
     */
    void refresh();

    /**
     * @brief 重新加载文件
     */
    void reload();

    /**
     * @brief 保存文件
     */
    void save();

    /**
     * @brief 另存为
     */
    void saveAs();

    /**
     * @brief 显示查找对话框
     */
    void showFindDialog();

    /**
     * @brief 显示替换对话框
     */
    void showReplaceDialog();

    /**
     * @brief 显示跳转行对话框
     */
    void showGotoLineDialog();

    /**
     * @brief 切换书签
     */
    void toggleBookmark();

    /**
     * @brief 清除所有书签
     */
    void clearBookmarks();

    /**
     * @brief 展开所有节点
     */
    void expandAll();

    /**
     * @brief 折叠所有节点
     */
    void collapseAll();

signals:
    /**
     * @brief 编辑器模式变化信号
     * @param mode 新模式
     */
    void editorModeChanged(EditorMode mode);

    /**
     * @brief 配置格式变化信号
     * @param format 新格式
     */
    void configFormatChanged(const QString& format);

    /**
     * @brief 只读状态变化信号
     * @param readOnly 是否只读
     */
    void readOnlyChanged(bool readOnly);

    /**
     * @brief 语法高亮状态变化信号
     * @param enabled 是否启用
     */
    void syntaxHighlightingChanged(bool enabled);

    /**
     * @brief 自动验证状态变化信号
     * @param enabled 是否启用
     */
    void autoValidationChanged(bool enabled);

    /**
     * @brief 配置内容变化信号
     */
    void configChanged();

    /**
     * @brief 修改状态变化信号
     * @param modified 是否修改
     */
    void modifiedChanged(bool modified);

    /**
     * @brief 文件加载完成信号
     * @param success 是否成功
     * @param filePath 文件路径
     */
    void fileLoaded(bool success, const QString& filePath);

    /**
     * @brief 文件保存完成信号
     * @param success 是否成功
     * @param filePath 文件路径
     */
    void fileSaved(bool success, const QString& filePath);

    /**
     * @brief 验证完成信号
     * @param status 验证状态
     * @param errors 错误列表
     * @param warnings 警告列表
     */
    void validationCompleted(ValidationStatus status, const QStringList& errors, const QStringList& warnings);

    /**
     * @brief 光标位置变化信号
     * @param line 行号
     * @param column 列号
     */
    void cursorPositionChanged(int line, int column);

    /**
     * @brief 选择变化信号
     * @param hasSelection 是否有选择
     */
    void selectionChanged(bool hasSelection);

    /**
     * @brief 错误发生信号
     * @param error 错误信息
     */
    void errorOccurred(const QString& error);

protected:
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void contextMenuEvent(QContextMenuEvent* event) override;

private slots:
    void onTextChanged();
    void onTreeItemChanged(QTreeWidgetItem* item, int column);
    void onCursorPositionChanged();
    void onSelectionChanged();
    void onValidationTimer();

private:
    void setupUI();
    void setupTreeMode();
    void setupTextMode();
    void setupSplitMode();
    void setupFormMode();
    void setupToolBar();
    void setupContextMenu();
    void setupSyntaxHighlighter();
    void setupCompleter();
    void connectSignals();
    void updateTreeFromText();
    void updateTextFromTree();
    void updateValidationStatus();
    void populateTree(const QJsonObject& obj, QTreeWidgetItem* parent = nullptr);
    QJsonObject treeToJson(QTreeWidgetItem* item = nullptr) const;
    QString formatToString(ConfigFormat format) const;
    ConfigFormat stringToFormat(const QString& str) const;
    QString modeToString(EditorMode mode) const;
    EditorMode stringToMode(const QString& str) const;
    void applyEditorOptions();
    void updateLineNumbers();
    void highlightCurrentLine();
    void matchBrackets();
    bool isValidJson(const QString& text) const;
    bool isValidIni(const QString& text) const;
    bool isValidXml(const QString& text) const;
    void showValidationTooltip(const QStringList& errors, const QStringList& warnings);
    void hideValidationTooltip();

    class Private;
    std::unique_ptr<Private> d;
};

#endif // CONFIGEDITOR_H
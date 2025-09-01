#ifndef STYLEUTILS_H
#define STYLEUTILS_H

#include <QColor>
#include <QString>
#include <QFont>
#include <QSize>
#include <QMargins>
#include <QVariantMap>

/**
 * @brief 样式工具类
 * 
 * 提供各种样式相关的静态工具方法，用于处理颜色、字体、图标和样式表。
 */
class StyleUtils
{
public:
    /**
     * @brief 调整颜色亮度
     * @param color 原始颜色
     * @param factor 亮度因子，正值增加亮度，负值减少亮度
     * @return 调整后的颜色
     */
    static QColor adjustBrightness(const QColor& color, qreal factor);

    /**
     * @brief 调整颜色饱和度
     * @param color 原始颜色
     * @param factor 饱和度因子，正值增加饱和度，负值减少饱和度
     * @return 调整后的颜色
     */
    static QColor adjustSaturation(const QColor& color, qreal factor);

    /**
     * @brief 混合两种颜色
     * @param color1 颜色1
     * @param color2 颜色2
     * @param ratio 混合比例，0.0为完全使用颜色1，1.0为完全使用颜色2
     * @return 混合后的颜色
     */
    static QColor blendColors(const QColor& color1, const QColor& color2, qreal ratio);

    /**
     * @brief 计算颜色对比度
     * @param color1 颜色1
     * @param color2 颜色2
     * @return 对比度值，值越大对比度越高
     */
    static qreal calculateContrast(const QColor& color1, const QColor& color2);

    /**
     * @brief 获取适合背景色的文本颜色
     * @param backgroundColor 背景颜色
     * @return 适合的文本颜色，黑色或白色
     */
    static QColor getTextColorForBackground(const QColor& backgroundColor);

    /**
     * @brief 颜色转为十六进制字符串
     * @param color 颜色
     * @param includeAlpha 是否包含透明度
     * @return 十六进制字符串
     */
    static QString colorToHex(const QColor& color, bool includeAlpha = false);

    /**
     * @brief 十六进制字符串转为颜色
     * @param hexString 十六进制字符串
     * @return 颜色
     */
    static QColor hexToColor(const QString& hexString);

    /**
     * @brief 创建渐变样式字符串
     * @param startColor 起始颜色
     * @param endColor 结束颜色
     * @param direction 方向，可以是"to bottom"、"to right"等
     * @return 渐变样式字符串
     */
    static QString createGradientStyle(const QColor& startColor, const QColor& endColor, const QString& direction);

    /**
     * @brief 创建阴影样式字符串
     * @param color 阴影颜色
     * @param offsetX X偏移
     * @param offsetY Y偏移
     * @param blurRadius 模糊半径
     * @param spreadRadius 扩散半径
     * @return 阴影样式字符串
     */
    static QString createShadowStyle(const QColor& color, int offsetX, int offsetY, int blurRadius, int spreadRadius = 0);

    /**
     * @brief 创建边框样式字符串
     * @param width 边框宽度
     * @param style 边框样式
     * @param color 边框颜色
     * @param radius 圆角半径
     * @return 边框样式字符串
     */
    static QString createBorderStyle(int width, const QString& style, const QColor& color, int radius = 0);

    /**
     * @brief 创建字体样式字符串
     * @param font 字体
     * @return 字体样式字符串
     */
    static QString createFontStyle(const QFont& font);

    /**
     * @brief 创建边距样式字符串
     * @param margins 边距
     * @return 边距样式字符串
     */
    static QString createMarginsStyle(const QMargins& margins);

    /**
     * @brief 创建内边距样式字符串
     * @param padding 内边距
     * @return 内边距样式字符串
     */
    static QString createPaddingStyle(const QMargins& padding);

    /**
     * @brief 创建尺寸样式字符串
     * @param size 尺寸
     * @return 尺寸样式字符串
     */
    static QString createSizeStyle(const QSize& size);

    /**
     * @brief 合并样式表
     * @param styles 样式表列表
     * @return 合并后的样式表
     */
    static QString mergeStyleSheets(const QStringList& styles);

    /**
     * @brief 解析样式表
     * @param styleSheet 样式表
     * @return 解析后的样式映射
     */
    static QVariantMap parseStyleSheet(const QString& styleSheet);

    /**
     * @brief 获取颜色亮度
     * @param color 颜色
     * @return 亮度值，0.0到1.0
     */
    static qreal getColorLuminance(const QColor& color);

    /**
     * @brief 检查颜色是否为暗色
     * @param color 颜色
     * @return 是否为暗色
     */
    static bool isDarkColor(const QColor& color);

    /**
     * @brief 检查颜色是否为亮色
     * @param color 颜色
     * @return 是否为亮色
     */
    static bool isLightColor(const QColor& color);

    /**
     * @brief 获取图标颜色
     * @param backgroundColor 背景颜色
     * @param isDarkTheme 是否为暗色主题
     * @return 图标颜色
     */
    static QColor getIconColor(const QColor& backgroundColor, bool isDarkTheme);

    /**
     * @brief 创建禁用状态的颜色
     * @param color 原始颜色
     * @return 禁用状态颜色
     */
    static QColor createDisabledColor(const QColor& color);
};

#endif // STYLEUTILS_H
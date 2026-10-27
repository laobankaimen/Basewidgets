#ifndef StyleManagerH
#define StyleManagerH
/*******************************************************************************
** Qt Advanced Stylesheets
** Copyright (C) 2022 Uwe Kindler
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License as published by the Free Software Foundation; either
** version 2.1 of the License, or (at your option) any later version.
**
** This library is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Lesser General Public License for more details.
**
** You should have received a copy of the GNU Lesser General Public
** License along with this library; If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/


//============================================================================
/// \file   StyleManager.h
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Declaration of CStyleManager class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include <QString>
#include <QVector>
#include <QPair>
#include <QObject>

#include <basewidgets/Styles/acss_globals.h>


QT_FORWARD_DECLARE_CLASS(QIcon)

namespace acss
{
struct QtAdvancedStylesheetPrivate;
using QStringPair = QPair<QString, QString>;
using tColorReplaceList = QVector<QStringPair>;

/**
 * 主题数据及 SVG 管理器。保留历史类名以兼容已有应用，不再处理 QSS。
 */
class ACSS_EXPORT QtAdvancedStylesheet : public QObject
{
	Q_OBJECT
private:
	QtAdvancedStylesheetPrivate* d; ///< private data (pimpl)
	friend struct QtAdvancedStylesheetPrivate;

public:
	enum eError
	{
		NoError,
		CssTemplateError,
		CssExportError,
		ThemeXmlError,
		StyleJsonError,
		ResourceGeneratorError,
	};

	enum eLocation
	{
		ThemesLocation,
		ResourceTemplatesLocation,
		FontsLocation
	};

	/**
	 * Default Constructor
	 */
	QtAdvancedStylesheet(QObject* parent = nullptr);

	/**
	 * Virtual Destructor
	 */
	virtual ~QtAdvancedStylesheet();

	/**
	 * Set the directory path that contains all styles
	 */
	void setStylesDirPath(const QString& DirPath);

	/**
	 * Returns the set styles dir
	 */
	QString stylesDirPath() const;

	/**
	 * Returns the current style
	 */
	QString currentStyle() const;

	/**
	 * Returns the absolute path of the current style.
	 * If your styles stylesDirPath() is C:/styles and your current style is
	 * themes then this function returns C:/styles/themes
	 */
	QString currentStylePath() const;

	/**
	 * Returns the list of available styles in the given styles directory
	 */
	const QStringList& styles() const;

	/**
	 * Returns a list of all themes for the current style.
	 * If no style has been set, this function returns an empty list
	 */
	const QStringList& themes() const;

	/**
	 * Returns a list of all theme variables for colors
	 */
	const QMap<QString, QString>& themeColorVariables() const;

	/**
	 * Returns the absolute dir path for the given location
	 */
	QString path(eLocation Location) const;

	/**
	 * Returns the absolute output dir path where the generated styles will
	 * get stored.
	 * \see setOutputDirPath()
	 */
	QString outputDirPath() const;

	/**
	 * Sets the output directory path where the generated theme will be stored
	 */
	void setOutputDirPath(const QString& Path);

	/**
	 * Returns the output path for the current style.
	 * The output path is the outputDirPath() + the style name.
	 * If your output path is C:/temp/styles and your style is themes
	 * then this functions returns C:/temp/styles/themes
	 */
	QString currentStyleOutputPath() const;

	/**
	 * Returns the value for the given theme variable.
	 * For example themeVariable("primaryColor") may return "#ac2300".
	 * Returns an empty string, if the given theme variable does not exist.
	 */
	QString themeVariableValue(const QString& VariableId) const;

    /** 添加/覆盖主题变量，可用于应用自定义颜色。
     * 批量修改后调用 updateTheme() 统一通知；theme-aware 图标重新替色，
     * 内嵌 bwicon: 文件属于构建时生成的主题变体，不会在运行时重写。
     */
	void setThemeVariableValue(const QString& VariableId, const QString& Value);

	/**
	 * Returns the color for the given VariableId.
	 * If VariableId is not a color variable, then this function returns an invalid
	 * QColor.
	 */
	QColor themeColor(const QString& VariableId) const;

	/**
	 * Returns the current set theme
	 */
	QString currentTheme() const;

    // Legacy query: native rendering has no stylesheet; always returns an empty string.
    QString styleSheet() const;

	/**
	 * Returns the icon for the current style or an empty icon if the style does
	 * not provide an icon
	 */
	const QIcon& styleIcon() const;

	/**
	 * Returns the error state of the object.
	 */
	eError error() const;

	/**
	 * Returns a string describing the last error that occured
	 */
	QString errorString() const;

	/**
	 * This function creates a palette with the theme colors of the currently
	 * selected theme
	 */
	QPalette generateThemePalette() const;

	/**
	 * Read access to the Json object with all stlye parameters
	 */
	const QJsonObject& styleParameters() const;

	/**
	 * The function returns true, if the current theme is a dark theme
	 * and false if the current theme is a light theme.
	 * The application can use this functions to adjust settings depending
	 * on dark or light theme
	 */
	bool isCurrentThemeDark() const;

	/**
	 * Replace SVG color in the given SvgContent file with theme colors.
	 * If an optional ColorReplaceList is provided, then the given list is used
	 * to replace the colors in the given SvgContent file. If no color replace
	 * list is given, the internal color replace list parsed from style json
	 * file is used
	 */
	bool replaceSvgColors(QByteArray& SvgContent,
		const tColorReplaceList& ColorReplaceList = tColorReplaceList());

	/**
	 * Loads SVG data from the given Filename and then passes the loaded
	 * SVG data to replaceSvgColors() function to replace colors with theme
	 * colors.
	 * The function uses the parsed theme colors to replace the colors in
	 * the given SVG icon. If the theme changes, all icons that are loaded
	 * with this function are automatically updated to the new theme colors.
	 */
	QIcon loadThemeAwareSvgIcon(const QString& Filename);

public slots:
    /** 读取指定主题（XML 文件名，不带扩展名），成功返回 true。
     * 此函数只加载数据；随后可覆盖变量，再调用 updateTheme() 统一刷新。
     */
	bool setCurrentTheme(const QString& Theme);

	/**
	 * Sets the default theme that is given in the style Json file
	 */
	void setDefaultTheme();

	/**
	 * Set the current style
	 */
	bool setCurrentStyle(const QString& Style);

    /** 刷新主题并发出 themeChanged；内嵌资源直接复用预生成 SVG，不写磁盘。
     * 外部源主题在 outputDir 下生成 SVG；失败返回 false，可通过 errorString 查询。
     * 所有调用均应在 GUI 线程执行。NativeStyle/ThemeTokens 负责实际绘制。
     */
    bool updateTheme();

    // Compatibility entry point; force is ignored. No CSS is generated or loaded.
    bool updateStylesheet(bool force = false);

    // Legacy palette/resource helper, without stylesheet processing.
    bool processStyleTemplate();

    /** 显式生成当前主题的 SVG 变体到 currentStyleOutputPath，主要供构建阶段使用。 */
	bool generateResources();

    /** 旧 JSON palette 映射入口；NativeStyle 使用 ThemeTokens 的完整 palette，不调用此函数。 */
	void updateApplicationPaletteColors();


signals:
    /** 主题刷新完成；自绘控件可在此读取颜色并重绘。 */
    void themeChanged();
	/**
	 * This signal is emitted if the selected style changed
	 */
	void currentStyleChanged(const QString& Style);

	/**
	 * This signal is emitted, if the selected theme in a style changed
	 */
	void currentThemeChanged(const QString& Theme);

    // Compatibility notification, emitted alongside themeChanged().
	void stylesheetChanged();
}; // class StyleManager
}
 // namespace namespace_name
//-----------------------------------------------------------------------------
#endif // StyleManagerH


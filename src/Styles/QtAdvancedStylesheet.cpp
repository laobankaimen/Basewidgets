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
/// \file   AdvancedStylesheet.cpp
/// \author Uwe Kindler
/// \date   13.12.2021
/// \brief  Implementation of CAdvancedStylesheet class
//============================================================================

//============================================================================
//                                   INCLUDES
//============================================================================
#include <basewidgets/Styles/QtAdvancedStylesheet.h>
#include <basewidgets/Styles/acss_globals.h>
#include <iostream>

#include <QMap>
#include <QXmlStreamReader>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QFontDatabase>
#include <QPointer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QIcon>
#include <QApplication>
#include <QPalette>
#include <QStyle>
#include <QIconEngine>
#include <QSvgRenderer>
#include <QPainter>
#include <QSet>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>


namespace acss
{
class CSVGIconEngine;
Q_GLOBAL_STATIC(QSet<CSVGIconEngine*>, IconEngineInstances)
Q_GLOBAL_STATIC(QMutex, IconEngineInstancesMutex)

/**
 * SvgIcon engine that supports loading from memory buffer
 */
class CSVGIconEngine : public QIconEngine
{
private:
	QByteArray m_SvgTemplate;
	QByteArray m_SvgContent; ///< memory buffer with SVG data load from file
	QPointer<QtAdvancedStylesheet> m_AdvancedStyleheet;

public:
	/**
	 * Creates an icon engine with the given SVG content an assigned
	 * AndvancedStylesheet object
	 */
	explicit CSVGIconEngine(const QByteArray &SvgContent, QtAdvancedStylesheet* Styleeheet)
		: m_SvgTemplate(SvgContent),
		  m_AdvancedStyleheet(Styleeheet)
	{
		update();
		QMutexLocker Locker(IconEngineInstancesMutex());
		IconEngineInstances->insert(this);
	}

	/**
	 * Removes itself from the set of instances
	 */
	virtual ~CSVGIconEngine()
	{
		if (!IconEngineInstances.isDestroyed())
		{
			QMutexLocker Locker(IconEngineInstancesMutex());
			IconEngineInstances->remove(this);
		}
	}

	/**
	 * Update the SVG content with the current theme icon colors
	 */
	void update()
	{
		m_SvgContent = m_SvgTemplate;
		if (m_AdvancedStyleheet) m_AdvancedStyleheet->replaceSvgColors(m_SvgContent);
	}

	/**
	 * Calls update for all Icon engine instances
	 */
	static void updateAllIcons()
	{
		if (IconEngineInstances.isDestroyed())
		{
			return;
		}

		QMutexLocker Locker(IconEngineInstancesMutex());
		for (auto Engine : *IconEngineInstances)
		{
			Engine->update();
		}
	}

	virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode,
	    QIcon::State state) override
	{
		Q_UNUSED(state);
		QSvgRenderer renderer(m_SvgContent);
		if (!renderer.isValid() || rect.isEmpty()) return;
		const QSizeF intrinsic = renderer.viewBoxF().size();
		const QSizeF fitted = intrinsic.isEmpty() ? QSizeF(rect.size()) : intrinsic.scaled(rect.size(), Qt::KeepAspectRatio);
		const QRectF target(QPointF(rect.x()+(rect.width()-fitted.width())/2., rect.y()+(rect.height()-fitted.height())/2.), fitted);
		painter->save();
		if (mode == QIcon::Disabled) painter->setOpacity(painter->opacity()*.45);
		renderer.render(painter, target);
		painter->restore();
	}

	virtual QIconEngine* clone() const override
	{
		return new CSVGIconEngine(m_SvgTemplate, m_AdvancedStyleheet);
	}

	virtual QPixmap pixmap(const QSize &size, QIcon::Mode mode,
	    QIcon::State state) override
	{
		// This function is necessary to create an EMPTY pixmap. It's called always
		// before paint()
		QImage img(size, QImage::Format_ARGB32);
		img.fill(qRgba(0, 0, 0, 0));
		QPixmap pix = QPixmap::fromImage(img, Qt::NoFormatConversion);
		{
			QPainter painter(&pix);
			QRect r(QPoint(0.0, 0.0), size);
			this->paint(&painter, r, mode, state);
		}
		return pix;
	}
};


/**
 * Groups the data the build a parsed palette color entry
 */
struct PaletteColorEntry
{
	QPalette::ColorGroup Group;
	QPalette::ColorRole Role;
	QString ColorVariable;

	PaletteColorEntry(QPalette::ColorGroup group = QPalette::Active,
		QPalette::ColorRole role = QPalette::NoRole,
		const QString& variable = QString())
		: Group(group), Role(role), ColorVariable(variable) {}

	bool isValid() const
	{
		return !ColorVariable.isEmpty() && Role != QPalette::NoRole;
	}
};

/**
 * Converts a color role string into a color role enum
 */
static QPalette::ColorRole colorRoleFromString(const QString& Text)
{
	static QMap<QString, QPalette::ColorRole> ColorRoleMap =
		{{"WindowText", QPalette::WindowText},
		 {"Button", QPalette::Button},
		 {"Light", QPalette::Light},
		 {"Midlight", QPalette::Midlight},
		 {"Dark", QPalette::Dark},
		 {"Mid", QPalette::Mid},
		 {"Text", QPalette::Text},
		 {"BrightText", QPalette::BrightText},
		 {"ButtonTextd", QPalette::ButtonText},
		 {"Base", QPalette::Base},
		 {"Window", QPalette::Window},
		 {"Shadow", QPalette::Shadow},
		 {"Highlight", QPalette::Highlight},
		 {"HighlightedText", QPalette::HighlightedText},
		 {"Link", QPalette::Link},
		 {"LinkVisited", QPalette::LinkVisited},
		 {"AlternateBase", QPalette::AlternateBase},
         {"NoRole", QPalette::NoRole},
         {"ToolTipBase", QPalette::ToolTipBase},
         {"ToolTipText", QPalette::ToolTipText},
#if QT_VERSION >= 0x050C00
         {"PlaceholderText", QPalette::PlaceholderText}
#endif
	};

	return ColorRoleMap.value(Text, QPalette::NoRole);
}


template <class Key, class T>
static void insertIntoMap(QMap<Key, T>& Map, const QMap<Key, T> &map)
{
#if QT_VERSION >= 0x050F00
    Map.insert(map);
#else
    for (auto itc = map.constBegin(); itc != map.constEnd(); ++itc)
    {
        Map.insert(itc.key(), itc.value());
    }
#endif
}


/**
 * Returns the color group string for a given QPalette::ColorGroup
 */
static QString colorGroupString(QPalette::ColorGroup ColorGroup)
{
	switch (ColorGroup)
	{
	case QPalette::Active: return "active";
	case QPalette::Disabled: return "disabled";
	case QPalette::Inactive: return "inactive";
	default:
		return QString();
	}

	return QString();
}


/**
 * Private data class of CAdvancedStylesheet class (pimpl)
 */
struct QtAdvancedStylesheetPrivate
{
	QtAdvancedStylesheet *_this;
	QString StylesDir;
	QString OutputDir;
	QMap<QString, QString> StyleVariables;
	QMap<QString, QString> ThemeColors;
	QMap<QString, QString> ThemeVariables;// theme variables contains StyleVariables and ThemeColors
	QString CurrentStyle;
	QString CurrentTheme;
	QString DefaultTheme;
	QString StyleName;
	QString IconFile;
	QVector<QStringPair> ResourceReplaceList;
	QVector<PaletteColorEntry> PaletteColors;
	QString PaletteBaseColor;
	QJsonObject JsonStyleParam;
	QString ErrorString;
	QtAdvancedStylesheet::eError Error;
	mutable QIcon Icon;
	QStringList Styles;
	QStringList Themes;
	bool IsDarkTheme = false;
	mutable tColorReplaceList IconColorReplaceList;

	/**
	 * Private data constructor
	 */
	QtAdvancedStylesheetPrivate(QtAdvancedStylesheet *_public);

	/**
	 * Parse a list of theme variables
	 */
	bool parseVariablesFromXml(QXmlStreamReader& s, const QString& VariableTagName,
		QMap<QString, QString>& Variable);

	/**
	 * Parse the theme file for
	 */
	bool parseThemeFile(const QString& ThemeFilename);

	/**
	 * Parse the style JSON file
	 */
	bool parseStyleJsonFile();

	/**
	 * Register the style fonts to the font database
	 */
	void addFonts(QDir* Dir = nullptr);

	/**
	 * Generate the resources for the variuous states
	 */
	bool generateResourcesFor(const QString& SubDir,
		const QJsonObject& JsonObject, const QFileInfoList& Entries);

	/**
	 * Replace the in the given content the template color string with the
	 * theme color string
	 */
	void replaceColor(QByteArray& Content, const QString& TemplateColor,
		const QString& ThemeColor) const;

	/**
	 * Set error code and error string
	 */
	void setError(QtAdvancedStylesheet::eError Error, const QString& ErrorString);

	/**
	 * Convenience function to ease clearing the error
	 */
	void clearError()
	{
		setError(QtAdvancedStylesheet::NoError, QString());
	}

	/**
	 * Parse palette from JSON file
	 */
	void parsePaletteFromJson();

	/**
	 * Parse palette color group from the given palette json parameters
	 */
	void parsePaletteColorGroup(QJsonObject& jPalette, QPalette::ColorGroup ColorGroup);

	/**
	 * Use this function to access the icon color replace list, to ensure, that
	 * is is properly initialized
	 */
	const tColorReplaceList& iconColorReplaceList() const;

	/**
	 * Parse a color replace list from the given JsonObject
	 */
	tColorReplaceList parseColorReplaceList(const QJsonObject& JsonObject) const;
};// struct AdvancedStylesheetPrivate


//============================================================================
QtAdvancedStylesheetPrivate::QtAdvancedStylesheetPrivate(
    QtAdvancedStylesheet *_public) :
	_this(_public)
{

}


//============================================================================
void QtAdvancedStylesheetPrivate::setError(QtAdvancedStylesheet::eError Error,
	const QString& ErrorString)
{
	this->Error = Error;
	this->ErrorString = ErrorString;
	if (Error != QtAdvancedStylesheet::NoError)
	{
		qDebug() << "CAdvancedStylesheet Error: " << Error << " " << ErrorString;
	}
}


//============================================================================
void QtAdvancedStylesheetPrivate::addFonts(QDir* Dir)
{
	// I dont't know, if this is the right way to detect, if there are any
	// widgets. The call to QFontDatabase::addApplicationFont() will crash, if
	// there are no widgets
	if (qApp->allWidgets().isEmpty())
	{
		return;
	}

	if (!Dir)
	{
		QDir FontsDir(_this->path(QtAdvancedStylesheet::FontsLocation));
		addFonts(&FontsDir);
	}
	else
	{
		auto Folders = Dir->entryList(QDir::Dirs | QDir::NoDotAndDotDot);
		for (auto Folder : Folders)
		{
			Dir->cd(Folder);
			addFonts(Dir);
			Dir->cdUp();
		}

		auto FontFiles = Dir->entryList({"*.ttf"}, QDir::Files);
		for (auto Font : FontFiles)
		{
            QString FontFilename = Dir->absoluteFilePath(Font);
			QFontDatabase::addApplicationFont(FontFilename);
		}
	}
}


//============================================================================
bool QtAdvancedStylesheetPrivate::parseVariablesFromXml(
	QXmlStreamReader& s, const QString& TagName, QMap<QString, QString>& Variables)
{
	while (s.readNextStartElement())
	{
		if (s.name() != TagName)
		{
			setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme "
                                "file - expected tag <" + TagName + "> instead of " + s.name().toString());
			return false;
		}
		auto Name = s.attributes().value("name").toString();
		if (Name.isEmpty())
		{
			setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme file - "
				"name attribute missing in <" + TagName + "> tag");
			return false;
		}

		auto Value = s.readElementText(QXmlStreamReader::SkipChildElements);
		if (Value.isEmpty())
		{
			setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme file - "
				"text of <" + TagName + "> tag is empty");
			return false;
		}

		Variables.insert(Name, Value);
	}

	return true;
}


//============================================================================
bool QtAdvancedStylesheetPrivate::parseThemeFile(const QString& Theme)
{
	QString ThemeFileName = _this->path(QtAdvancedStylesheet::ThemesLocation) + "/" + Theme;
	QFile ThemeFile(ThemeFileName);
	ThemeFile.open(QIODevice::ReadOnly);
	QXmlStreamReader s(&ThemeFile);
        s.readNextStartElement();
        if (s.name() != QString("resources"))
	{
		setError(QtAdvancedStylesheet::ThemeXmlError, "Malformed theme file - "
                        "expected tag <resources> instead of " + s.name().toString());
		return false;
	}

	IsDarkTheme = (s.attributes().value("dark").toInt() == 1);
    QMap<QString, QString> ColorVariables;
	parseVariablesFromXml(s, "color", ColorVariables);
	this->ThemeVariables = this->StyleVariables;
	insertIntoMap(this->ThemeVariables, ColorVariables);
	this->ThemeColors = ColorVariables;
	return true;
}


//============================================================================
bool QtAdvancedStylesheetPrivate::parseStyleJsonFile()
{
	QDir Dir(_this->currentStylePath());
	auto JsonFiles = Dir.entryInfoList({"*.json"}, QDir::Files);
	if (JsonFiles.count() < 1)
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "Stylesheet folder does "
			"not contain a style json file");
		return false;
	}

	if (JsonFiles.count() > 1)
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "Stylesheet folder "
			"contains multiple theme json files");
		return false;
	}

	QFile StyleJsonFile(JsonFiles[0].absoluteFilePath());
	StyleJsonFile.open(QIODevice::ReadOnly);

	auto JsonData = StyleJsonFile.readAll();
	QJsonParseError ParseError;
	auto JsonDocument = QJsonDocument::fromJson(JsonData, &ParseError);
	if (JsonDocument.isNull())
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "Loading style json file "
			"caused error: " + ParseError.errorString());
		return false;
	}

	auto json = JsonStyleParam = JsonDocument.object();
	StyleName = json.value("name").toString();
	if (StyleName.isEmpty())
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "No key \"name\" found "
			"in style json file");
		return false;
	}

	QMap<QString, QString> Variables;
	auto jvariables = json.value("variables").toObject();
	for (const auto& key : jvariables.keys())
	{
		Variables.insert(key, jvariables.value(key).toString());
	}

	StyleVariables = Variables;
	IconFile = json.value("icon").toString();
	parsePaletteFromJson();

	DefaultTheme = json.value("default_theme").toString();
	if (DefaultTheme.isEmpty())
	{
		setError(QtAdvancedStylesheet::StyleJsonError, "No key \"default_theme\" found "
			"in style json file");
		return false;
	}

	return true;
}


//============================================================================
void QtAdvancedStylesheetPrivate::parsePaletteFromJson()
{
	PaletteBaseColor = QString();
	PaletteColors.clear();
	auto jPalette = JsonStyleParam.value("palette").toObject();
	if (jPalette.isEmpty())
	{
		return;
	}

	PaletteBaseColor = jPalette.value("base_color").toString();
	parsePaletteColorGroup(jPalette, QPalette::Active);
	parsePaletteColorGroup(jPalette, QPalette::Disabled);
	parsePaletteColorGroup(jPalette, QPalette::Inactive);
}


//============================================================================
void QtAdvancedStylesheetPrivate::parsePaletteColorGroup(QJsonObject& jPalette, QPalette::ColorGroup ColorGroup)
{
	auto jColorGroup = jPalette.value(colorGroupString(ColorGroup)).toObject();
	if (jColorGroup.isEmpty())
	{
		return;
	}

	for (auto itc = jColorGroup.constBegin(); itc != jColorGroup.constEnd(); ++itc)
	{
		auto ColorRole = colorRoleFromString(itc.key());
		if (QPalette::NoRole == ColorRole)
		{
			continue;
		}

		this->PaletteColors.append({ColorGroup, ColorRole, itc.value().toString()});
		if (ColorGroup != QPalette::Active)
		{
			continue;
		}
	}
}


//============================================================================
void QtAdvancedStylesheetPrivate::replaceColor(QByteArray& Content,
	const QString& TemplateColor, const QString& ThemeColor) const
{
	Content.replace(TemplateColor.toLatin1(), ThemeColor.toLatin1());
}


//============================================================================
tColorReplaceList QtAdvancedStylesheetPrivate::parseColorReplaceList(const QJsonObject& JsonObject) const
{
	// Fill the color replace list with the values read from style json file
	tColorReplaceList ColorReplaceList;
	for (auto it = JsonObject.constBegin(); it != JsonObject.constEnd(); ++it)
	{
		auto TemplateColor = it.key();
		auto ThemeColor = it.value().toString();
		// If the color starts with an hashtag, then we have a real color value
		// If it does not start with # then it is a theme variable
		if (!ThemeColor.startsWith('#'))
		{
			ThemeColor = _this->themeVariableValue(ThemeColor);
		}
		ColorReplaceList.append({TemplateColor, ThemeColor});
	}

	return ColorReplaceList;
}


//============================================================================
bool QtAdvancedStylesheetPrivate::generateResourcesFor(const QString& SubDir,
	const QJsonObject& JsonObject, const QFileInfoList& Entries)
{
	const QString OutputDir = _this->currentStyleOutputPath() + "/" + SubDir;
	if (!QDir().mkpath(OutputDir))
	{
		setError(QtAdvancedStylesheet::ResourceGeneratorError, "Error "
			"creating resource output folder: " + OutputDir);
		return false;
	}

	auto ColorReplaceList = parseColorReplaceList(JsonObject);

	// Now loop through all resources svg files and replace the colors
	for (const auto& Entry : Entries)
	{
		QFile SvgFile(Entry.absoluteFilePath());
        if (!SvgFile.open(QIODevice::ReadOnly)) {
            setError(QtAdvancedStylesheet::ResourceGeneratorError, SvgFile.errorString());
            return false;
        }
		auto Content = SvgFile.readAll();
		SvgFile.close();
        _this->replaceSvgColors(Content, ColorReplaceList);
		QString OutputFilename = OutputDir + "/" + Entry.fileName();
		QFile OutputFile(OutputFilename);
        if (!OutputFile.open(QIODevice::WriteOnly) || OutputFile.write(Content) != Content.size()) {
            setError(QtAdvancedStylesheet::ResourceGeneratorError, OutputFile.errorString());
            return false;
        }
        OutputFile.close();
	}

	return true;
}


//============================================================================
const tColorReplaceList& QtAdvancedStylesheetPrivate::iconColorReplaceList() const
{
	if (IconColorReplaceList.count())
	{
		return IconColorReplaceList;
	}

	auto jicon_colors = JsonStyleParam.value("icon_colors").toObject();
	if (jicon_colors.isEmpty())
	{
		return IconColorReplaceList;
	}

	IconColorReplaceList = parseColorReplaceList(jicon_colors);
	return IconColorReplaceList;
}


//============================================================================
bool QtAdvancedStylesheet::replaceSvgColors(QByteArray& SvgContent,
	const tColorReplaceList& ColorReplaceList)
{
	const tColorReplaceList& ReplaceList = ColorReplaceList.isEmpty() ?
		d->iconColorReplaceList() : ColorReplaceList;
	bool isFindColor = false;
	for (const auto& Replace : ReplaceList)
	{
		if(SvgContent.contains(Replace.first.toLatin1()))
			isFindColor = true;
		d->replaceColor(SvgContent, Replace.first, Replace.second);
	}
	return isFindColor;
}


//============================================================================
QIcon QtAdvancedStylesheet::loadThemeAwareSvgIcon(const QString& filename)
{
    QString resolved = filename;
    // Source-directory themes must also work without a registered RCC bundle.
    if (filename.startsWith(":/styles/")) {
        const QString source = QDir(stylesDirPath()).filePath(filename.mid(9));
        if (QFile::exists(source)) resolved = source;
    }
    QFile file(resolved);
    if (!file.open(QIODevice::ReadOnly)) return QIcon();
    return QIcon(new CSVGIconEngine(file.readAll(), this));
}


//============================================================================
QtAdvancedStylesheet::QtAdvancedStylesheet(QObject* parent) :
	QObject(parent),
	d(new QtAdvancedStylesheetPrivate(this))
{

}


//============================================================================
QtAdvancedStylesheet::~QtAdvancedStylesheet()
{
	delete d;
}


//============================================================================
void QtAdvancedStylesheet::setStylesDirPath(const QString& DirPath)
{
	d->StylesDir = DirPath;
	QDir Dir(d->StylesDir);
	d->Styles = Dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
}


//============================================================================
QString QtAdvancedStylesheet::stylesDirPath() const
{
	return d->StylesDir;
}


//============================================================================
bool QtAdvancedStylesheet::setCurrentStyle(const QString& Style)
{
	d->clearError();
	d->CurrentStyle = Style;
	QDir Dir(path(ThemesLocation));
	d->Themes = Dir.entryList({"*.xml"}, QDir::Files);
	for (auto& Theme : d->Themes)
	{
		Theme.replace(".xml", "");
	}
	auto Result = d->parseStyleJsonFile();
	QDir::addSearchPath("bwicon", currentStyleOutputPath());
	// Backward compatibility for previously generated CSS/code that still uses `icon:`.
	QDir::addSearchPath("icon", currentStyleOutputPath());
	d->addFonts();
	emit currentStyleChanged(d->CurrentStyle);
	emit stylesheetChanged();
	return Result;
}


//============================================================================
QString QtAdvancedStylesheet::currentStyle() const
{
	return d->CurrentStyle;
}


//============================================================================
QString QtAdvancedStylesheet::currentStylePath() const
{
	return d->StylesDir + "/" + d->CurrentStyle;
}


//============================================================================
QString QtAdvancedStylesheet::outputDirPath() const
{
	return d->OutputDir;
}


//============================================================================
void QtAdvancedStylesheet::setOutputDirPath(const QString& Path)
{
	d->OutputDir = Path;
}


//============================================================================
QString QtAdvancedStylesheet::currentStyleOutputPath() const
{
	QString path = outputDirPath() + "/" + d->CurrentStyle;
	if (!d->CurrentTheme.isEmpty())
	{
		path += "/" + d->CurrentTheme;
	}
	return path;
}


//============================================================================
QString QtAdvancedStylesheet::themeVariableValue(const QString& VariableId) const
{
	return d->ThemeVariables.value(VariableId, QString());
}


//============================================================================
QColor QtAdvancedStylesheet::themeColor(const QString& VariableId) const
{
	auto ColorString = d->ThemeColors.value(VariableId, QString());
	if (ColorString.isEmpty())
	{
		return QColor();
	}

	return QColor(ColorString);
}


//============================================================================
void QtAdvancedStylesheet::setThemeVariableValue(const QString& VariableId, const QString& Value)
{
	d->ThemeVariables.insert(VariableId, Value);
	auto it = d->ThemeColors.find(VariableId);
	if (it != d->ThemeColors.end())
	{
		it.value() = Value;
	}
}


//============================================================================
bool QtAdvancedStylesheet::setCurrentTheme(const QString& Theme)
{
	d->clearError();
	if (d->JsonStyleParam.isEmpty())
	{
		return false;
	}

	if (!d->parseThemeFile(Theme + ".xml"))
	{
		return false;
	}

	d->CurrentTheme = Theme;
	// Keep both schemes alive so stale generated CSS and older code keep working.
	// Built-in themes already contain all generated icon variants. Custom themes can
    // generate them into outputDir; both use the same public bwicon: lookup convention.
    const QString bundled = currentStylePath()+"/"+Theme;
    const QStringList iconSearchPaths = d->StylesDir.startsWith(":/")
        ? QStringList{bundled} : QStringList{currentStyleOutputPath(),bundled};
	QDir::setSearchPaths("bwicon", iconSearchPaths);
	QDir::setSearchPaths("icon", iconSearchPaths);
	
	emit currentThemeChanged(d->CurrentTheme);
	return true;
}


//============================================================================
void QtAdvancedStylesheet::setDefaultTheme()
{
	setCurrentTheme(d->DefaultTheme);
}


//============================================================================
bool QtAdvancedStylesheet::updateTheme()
{
    d->clearError();
    // Embedded RCC variants were generated during the library build. Do not write
    // application directories or regenerate the same SVGs on every theme switch.
    const bool resourcesReady = d->StylesDir.startsWith(":/") || generateResources();
    d->IconColorReplaceList.clear();
    CSVGIconEngine::updateAllIcons();
    emit themeChanged();
    emit stylesheetChanged(); // legacy notification, no stylesheet processing
    return resourcesReady;
}

bool QtAdvancedStylesheet::updateStylesheet(bool force)
{
    Q_UNUSED(force);
    return updateTheme();
}

bool QtAdvancedStylesheet::processStyleTemplate()
{
	updateApplicationPaletteColors();
	return generateResources();
}


//============================================================================
QString QtAdvancedStylesheet::currentTheme() const
{
	return d->CurrentTheme;
}


//============================================================================
QString QtAdvancedStylesheet::styleSheet() const
{
	return QString();
}


//============================================================================
const QIcon& QtAdvancedStylesheet::styleIcon() const
{
	if (d->Icon.isNull() && !d->IconFile.isEmpty())
	{
        const QString path=currentStylePath()+"/"+d->IconFile;
        d->Icon=path.endsWith(".svg",Qt::CaseInsensitive)
            ? const_cast<QtAdvancedStylesheet*>(this)->loadThemeAwareSvgIcon(path) : QIcon(path);
	}

	return d->Icon;
}


//============================================================================
const QStringList& QtAdvancedStylesheet::styles() const
{
	return d->Styles;
}


//============================================================================
const QStringList& QtAdvancedStylesheet::themes() const
{
	return d->Themes;
}


//============================================================================
const QMap<QString, QString>& QtAdvancedStylesheet::themeColorVariables() const
{
	return d->ThemeColors;
}


//============================================================================
QtAdvancedStylesheet::eError QtAdvancedStylesheet::error() const
{
	return d->Error;
}


//============================================================================
QString QtAdvancedStylesheet::errorString() const
{
	return d->ErrorString;
}


//============================================================================
QString QtAdvancedStylesheet::path(eLocation Location) const
{
	switch (Location)
	{
	case ThemesLocation: return currentStylePath() + "/themes";
	case ResourceTemplatesLocation: return currentStylePath() + "/resources";
	case FontsLocation: return currentStylePath() + "/fonts";
	}

	return QString();
}


//============================================================================
bool QtAdvancedStylesheet::generateResources()
{
	QDir ResourceDir(path(QtAdvancedStylesheet::ResourceTemplatesLocation));
	auto Entries = ResourceDir.entryInfoList({"*.svg"}, QDir::Files);

	auto jresources = d->JsonStyleParam.value("resources").toObject();
	if (jresources.isEmpty())
	{
		d->setError(QtAdvancedStylesheet::StyleJsonError, "Key resources "
			"missing in style json file");
		return false;
	}

	// Process all resource generation variants
	bool Result = true;
	for (auto itc = jresources.constBegin(); itc != jresources.constEnd(); ++itc)
	{
		auto Param = itc.value().toObject();
		if (Param.isEmpty())
		{
			d->setError(QtAdvancedStylesheet::StyleJsonError, "Key resources "
				"missing in style json file");
			Result = false;
			continue;
		}
		if (!d->generateResourcesFor(itc.key(), Param, Entries))
		{
			Result = false;
		}
	}

	return Result;
}


//============================================================================
QPalette QtAdvancedStylesheet::generateThemePalette() const
{
	QPalette Palette = qApp->palette();
	if (!d->PaletteBaseColor.isEmpty())
	{
		auto Color = themeColor(d->PaletteBaseColor);
		if (Color.isValid())
		{
			Palette = QPalette(Color);
		}
	}

	for (const auto& Entry : d->PaletteColors)
	{
		auto Color = themeColor(Entry.ColorVariable);
		if (Color.isValid())
		{
			Palette.setColor(Entry.Group, Entry.Role, themeColor(Entry.ColorVariable));
		}
	}

	return Palette;
}


//============================================================================
void QtAdvancedStylesheet::updateApplicationPaletteColors()
{
	qApp->setPalette(generateThemePalette());
}


//============================================================================
const QJsonObject& QtAdvancedStylesheet::styleParameters() const
{
	return d->JsonStyleParam;
}


//============================================================================
bool QtAdvancedStylesheet::isCurrentThemeDark() const
{
	return d->IsDarkTheme;
}


} // namespace acss

//---------------------------------------------------------------------------
// EOF AdvancedStylesheet.cpp

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme engine methods 
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// Qt includes.

#include <Q3Dict>
#include <Q3PtrList>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QTimer>
#include <QPixmap>
#include <QDomDocument>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes.

#include "ddebug.h"
#include "theme.h"
#include "texture.h"
#include "themeengine.h"
#include "themeengine.moc"

namespace Digikam
{

class ThemeEnginePriv
{
public:

    ThemeEnginePriv()
    {
        defaultTheme      = 0;
        currTheme         = 0;
        themeInitiallySet = false;
    }

    Q3PtrList<Theme> themeList;
    Q3Dict<Theme>    themeDict;
    
    Theme*           currTheme;
    Theme*           defaultTheme;
    bool             themeInitiallySet;
};

ThemeEngine* ThemeEngine::m_componentData = 0;

ThemeEngine* ThemeEngine::componentData()
{
    if (!m_componentData)
    { 
        new ThemeEngine();
    }
    return m_componentData;
}

ThemeEngine::ThemeEngine()
{
    m_componentData = this;
    KGlobal::dirs()->addResourceDir("themes", KStandardDirs::installPath("data") + QString("digikam/themes")); 

    d = new ThemeEnginePriv;

    d->themeList.setAutoDelete(false);
    d->themeDict.setAutoDelete(false);
    d->defaultTheme = new Theme(i18n("Default"), QString());
    d->themeList.append(d->defaultTheme);
    d->themeDict.insert(i18n("Default"), d->defaultTheme);
    d->currTheme = d->defaultTheme;

    buildDefaultTheme();
}

ThemeEngine::~ThemeEngine()
{
    d->themeList.setAutoDelete(true);
    d->themeList.clear();
    delete d;
    m_componentData = 0;
}

void ThemeEngine::scanThemes()
{
    d->themeList.remove(d->defaultTheme);
    d->themeList.setAutoDelete(true);
    d->themeList.clear();
    d->themeDict.clear();
    d->currTheme = 0;

    QStringList themes = KGlobal::dirs()->findAllResources("themes", QString(), 
                                          KStandardDirs::Recursive | KStandardDirs::NoDuplicates);

    for (QStringList::iterator it=themes.begin(); it != themes.end();
         ++it)
    {
        QFileInfo fi(*it);
        Theme* theme = new Theme(fi.fileName(), *it);
        d->themeList.append(theme);
        d->themeDict.insert(fi.fileName(), theme);
    }
    
    d->themeList.append(d->defaultTheme);
    d->themeDict.insert(i18n("Default"), d->defaultTheme);
    d->currTheme = d->defaultTheme;
}

QStringList ThemeEngine::themeNames() const
{
    QStringList names;
    for (Theme *t = d->themeList.first(); t; t = d->themeList.next())
    {
        names << t->name;
    }
    names.sort();
    return names;
}

void ThemeEngine::slotChangeTheme(const QString& name)
{
    setCurrentTheme(name);    
}

void ThemeEngine::setCurrentTheme(const QString& name)
{
    Theme* theme = d->themeDict.find(name);
    if (!theme)
    {
        d->currTheme = d->defaultTheme;
        return;
    }

    if (d->currTheme == theme && d->themeInitiallySet)
        return;

    d->currTheme = theme;
    loadTheme();

    // this is only to ensure that even if the chosen theme is the default theme,
    // the signalThemeChanged is emitted when themes are loaded in DigikamApp
    d->themeInitiallySet = true;

    //theme->print();
    QTimer::singleShot(0, this, SIGNAL(signalThemeChanged()));
}

void ThemeEngine::setCurrentTheme(const Theme& theme, const QString& name, bool loadFromDisk)
{
    Theme* t = d->themeDict.find(name);
    if (t)
    {
        d->themeDict.remove(name);
        d->themeList.remove(t);
    }

    t = new Theme(theme);
    t->filePath = theme.filePath;
    d->themeDict.insert(name, t);
    d->themeList.append(t);

    d->currTheme = t;
    if (loadFromDisk)
        loadTheme();
    
    QTimer::singleShot(0, this, SIGNAL(signalThemeChanged()));
}

Theme* ThemeEngine::getCurrentTheme()
{
    return d->currTheme;
}

void ThemeEngine::buildDefaultTheme()
{
    Theme* t = d->defaultTheme;

    QPalette pa = QApplication::palette();
    
    t->baseColor           = pa.color(QPalette::Base);
    t->textRegColor        = pa.color(QPalette::Text);
    t->textSelColor        = pa.color(QPalette::HighlightedText);
    t->textSpecialRegColor = QColor("#0000EF");
    t->textSpecialSelColor = pa.color(QPalette::HighlightedText);

    t->bannerColor         = pa.color(QPalette::Highlight);
    t->bannerColorTo       = pa.color(QPalette::Highlight).dark(120);
    t->bannerBevel         = Theme::FLAT;
    t->bannerGrad          = Theme::SOLID;
    t->bannerBorder        = false;
    t->bannerBorderColor   = Qt::black;
    
    t->thumbRegColor       = pa.color(QPalette::Base);
    t->thumbRegColorTo     = pa.color(QPalette::Base);
    t->thumbRegBevel       = Theme::FLAT;
    t->thumbRegGrad        = Theme::SOLID;
    t->thumbRegBorder      = true;
    t->thumbRegBorderColor = QColor("#E0E0EF");

    t->thumbSelColor       = pa.color(QPalette::Highlight);
    t->thumbSelColorTo     = pa.color(QPalette::Highlight);
    t->thumbSelBevel       = Theme::FLAT;
    t->thumbSelGrad        = Theme::SOLID;
    t->thumbSelBorder      = true;
    t->thumbSelBorderColor = QColor("#E0E0EF");

    t->listRegColor        = pa.color(QPalette::Base);
    t->listRegColorTo      = pa.color(QPalette::Base);
    t->listRegBevel        = Theme::FLAT;
    t->listRegGrad         = Theme::SOLID;
    t->listRegBorder       = false;
    t->listRegBorderColor  = Qt::black;
                        
    t->listSelColor        = pa.color(QPalette::Highlight);
    t->listSelColorTo      = pa.color(QPalette::Highlight);
    t->listSelBevel        = Theme::FLAT;
    t->listSelGrad         = Theme::SOLID;
    t->listSelBorder       = true;
    t->listSelBorderColor  = Qt::black;
}

bool ThemeEngine::loadTheme()
{
    Q_ASSERT( d->currTheme );
    if (!d->currTheme || d->currTheme == d->defaultTheme)
        return false;

    Theme *t = d->currTheme;
    
    // use the default theme as base template to build the themes
    *(t) = *(d->defaultTheme);

    QFile themeFile(t->filePath);

    if (!themeFile.open(IO_ReadOnly))
        return false;

    QDomDocument xmlDoc;
    QString error;
    int row, col;
    if (!xmlDoc.setContent(&themeFile, true, &error, &row, &col))
    {
        DDebug() << "Theme file: " << t->filePath << endl;
        DDebug() << error << " :: row=" << row << " , col=" << col << endl; 
        return false;
    }

    QDomElement rootElem = xmlDoc.documentElement();
    if (rootElem.tagName() != QString::fromLatin1("digikamtheme"))
        return false;

    QString resource;

    // -- base ------------------------------------------------------------------------
    
    resource = resourceValue(rootElem, "BaseColor");
    if (!resource.isEmpty())
        t->baseColor = resource;

    resource = resourceValue(rootElem, "TextRegularColor");
    if (!resource.isEmpty())
        t->textRegColor = resource;
    
    resource = resourceValue(rootElem, "TextSelectedColor");
    if (!resource.isEmpty())
        t->textSelColor = resource;

    resource = resourceValue(rootElem, "TextSpecialRegularColor");
    if (!resource.isEmpty())
        t->textSpecialRegColor = resource;
    
    resource = resourceValue(rootElem, "TextSpecialSelectedColor");
    if (!resource.isEmpty())
        t->textSpecialSelColor = resource;

    // -- banner ------------------------------------------------------------------------
    
    resource = resourceValue(rootElem, "BannerColor");
    if (!resource.isEmpty())
        t->bannerColor = resource;

    resource = resourceValue(rootElem, "BannerColorTo");
    if (!resource.isEmpty())
        t->bannerColorTo = resource;

    resource = resourceValue(rootElem, "BannerBevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
            t->bannerBevel = Theme::FLAT;
        else if (resource.contains("sunken", Qt::CaseInsensitive))
            t->bannerBevel = Theme::SUNKEN;
        else if (resource.contains("raised", Qt::CaseInsensitive))
            t->bannerBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "BannerGradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
            t->bannerGrad = Theme::SOLID;
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
            t->bannerGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", Qt::CaseInsensitive))
            t->bannerGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
            t->bannerGrad = Theme::DIAGONAL;
    }

    resource = resourceValue(rootElem, "BannerBorder");
    if (!resource.isEmpty())
    {
        t->bannerBorder = resource.contains("true", Qt::CaseInsensitive);
    }

    resource = resourceValue(rootElem, "BannerBorderColor");
    if (!resource.isEmpty())
    {
        t->bannerBorderColor = resource;
    }

    // -- thumbnail view ----------------------------------------------------------------
    
    resource = resourceValue(rootElem, "ThumbnailRegular.Color");
    if (!resource.isEmpty())
        t->thumbRegColor = resource;

    resource = resourceValue(rootElem, "ThumbnailRegularColorTo");
    if (!resource.isEmpty())
        t->thumbRegColorTo = resource;

    resource = resourceValue(rootElem, "ThumbnailRegularBevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
            t->thumbRegBevel = Theme::FLAT;
        else if (resource.contains("sunken", Qt::CaseInsensitive))
            t->thumbRegBevel = Theme::SUNKEN;
        else if (resource.contains("raised", Qt::CaseInsensitive))
            t->thumbRegBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ThumbnailRegularGradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
            t->thumbRegGrad = Theme::SOLID;
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
            t->thumbRegGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", Qt::CaseInsensitive))
            t->thumbRegGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
            t->thumbRegGrad = Theme::DIAGONAL;
    }

    resource = resourceValue(rootElem, "ThumbnailRegularBorder");
    if (!resource.isEmpty())
    {
        t->thumbRegBorder = resource.contains("true", Qt::CaseInsensitive);
    }

    resource = resourceValue(rootElem, "ThumbnailRegularBorderColor");
    if (!resource.isEmpty())
    {
        t->thumbRegBorderColor = resource;
    }
    
    resource = resourceValue(rootElem, "ThumbnailSelectedColor");
    if (!resource.isEmpty())
        t->thumbSelColor = resource;

    resource = resourceValue(rootElem, "ThumbnailSelectedColorTo");
    if (!resource.isEmpty())
        t->thumbSelColorTo = resource;

    resource = resourceValue(rootElem, "ThumbnailSelectedBevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
            t->thumbSelBevel = Theme::FLAT;
        else if (resource.contains("sunken", Qt::CaseInsensitive))
            t->thumbSelBevel = Theme::SUNKEN;
        else if (resource.contains("raised", Qt::CaseInsensitive))
            t->thumbSelBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ThumbnailSelectedGradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
            t->thumbSelGrad = Theme::SOLID;
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
            t->thumbSelGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", Qt::CaseInsensitive))
            t->thumbSelGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
            t->thumbSelGrad = Theme::DIAGONAL;
    }
    
    resource = resourceValue(rootElem, "ThumbnailSelectedBorder");
    if (!resource.isEmpty())
    {
        t->thumbSelBorder = resource.contains("true", Qt::CaseInsensitive);
    }

    resource = resourceValue(rootElem, "ThumbnailSelectedBorderColor");
    if (!resource.isEmpty())
    {
        t->thumbSelBorderColor = resource;
    }

    // -- listview view ----------------------------------------------------------------
    
    resource = resourceValue(rootElem, "ListviewRegularColor");
    if (!resource.isEmpty())
        t->listRegColor = resource;

    resource = resourceValue(rootElem, "ListviewRegularColorTo");
    if (!resource.isEmpty())
        t->listRegColorTo = resource;

    resource = resourceValue(rootElem, "ListviewRegularBevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
            t->listRegBevel = Theme::FLAT;
        else if (resource.contains("sunken", Qt::CaseInsensitive))
            t->listRegBevel = Theme::SUNKEN;
        else if (resource.contains("raised", Qt::CaseInsensitive))
            t->listRegBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ListviewRegularGradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
            t->listRegGrad = Theme::SOLID;
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
            t->listRegGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", Qt::CaseInsensitive))
            t->listRegGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
            t->listRegGrad = Theme::DIAGONAL;
    }

    resource = resourceValue(rootElem, "ListviewRegularBorder");
    if (!resource.isEmpty())
    {
        t->listRegBorder = resource.contains("true", Qt::CaseInsensitive);
    }

    resource = resourceValue(rootElem, "ListviewRegularBorderColor");
    if (!resource.isEmpty())
    {
        t->listRegBorderColor = resource;
    }

    resource = resourceValue(rootElem, "ListviewSelectedColor");
    if (!resource.isEmpty())
        t->listSelColor = resource;

    resource = resourceValue(rootElem, "ListviewSelectedColorTo");
    if (!resource.isEmpty())
        t->listSelColorTo = resource;

    resource = resourceValue(rootElem, "ListviewSelectedBevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
            t->listSelBevel = Theme::FLAT;
        else if (resource.contains("sunken", Qt::CaseInsensitive))
            t->listSelBevel = Theme::SUNKEN;
        else if (resource.contains("raised", Qt::CaseInsensitive))
            t->listSelBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ListviewSelectedGradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
            t->listSelGrad = Theme::SOLID;
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
            t->listSelGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", Qt::CaseInsensitive))
            t->listSelGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
            t->listSelGrad = Theme::DIAGONAL;
    }

    resource = resourceValue(rootElem, "ListviewSelectedBorder");
    if (!resource.isEmpty())
    {
        t->listSelBorder = resource.contains("true", Qt::CaseInsensitive);
    }

    resource = resourceValue(rootElem, "ListviewSelectedBorderColor");
    if (!resource.isEmpty())
    {
        t->listSelBorderColor = resource;
    }

    DDebug() << "Theme file loaded: " << t->filePath << endl;
    return true;
}

QString ThemeEngine::resourceValue(const QDomElement &rootElem, const QString& key)
{
    for (QDomNode node = rootElem.firstChild();
         !node.isNull(); node = node.nextSibling()) 
    {
        QDomElement e = node.toElement();
        QString name  = e.tagName(); 
        QString val   = e.attribute(QString::fromLatin1("value")); 

        if (key == name)
        {
            return val;
        } 
    }

    return QString("");
}

QColor ThemeEngine::baseColor() const
{
    return d->currTheme->baseColor;    
}

QColor ThemeEngine::thumbSelColor() const
{
    return d->currTheme->thumbSelColor;    
}

QColor ThemeEngine::thumbRegColor() const
{
    return d->currTheme->thumbRegColor;    
}

QColor ThemeEngine::textRegColor() const
{
    return d->currTheme->textRegColor;    
}

QColor ThemeEngine::textSelColor() const
{
    return d->currTheme->textSelColor;    
}

QColor ThemeEngine::textSpecialRegColor() const
{
    return d->currTheme->textSpecialRegColor;    
}

QColor ThemeEngine::textSpecialSelColor() const
{
    return d->currTheme->textSpecialSelColor;    
}

QPixmap ThemeEngine::bannerPixmap(int w, int h)
{
    Texture tex(w, h, d->currTheme->bannerColor, d->currTheme->bannerColorTo,
                d->currTheme->bannerBevel, d->currTheme->bannerGrad,
                d->currTheme->bannerBorder, d->currTheme->bannerBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::thumbRegPixmap(int w, int h)
{
    Texture tex(w, h, d->currTheme->thumbRegColor, d->currTheme->thumbRegColorTo,
                d->currTheme->thumbRegBevel, d->currTheme->thumbRegGrad,
                d->currTheme->thumbRegBorder, d->currTheme->thumbRegBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::thumbSelPixmap(int w, int h)
{
    Texture tex(w, h, d->currTheme->thumbSelColor, d->currTheme->thumbSelColorTo,
                d->currTheme->thumbSelBevel, d->currTheme->thumbSelGrad,
                d->currTheme->thumbSelBorder, d->currTheme->thumbSelBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::listRegPixmap(int w, int h)
{
    Texture tex(w, h, d->currTheme->listRegColor, d->currTheme->listRegColorTo,
                d->currTheme->listRegBevel, d->currTheme->listRegGrad,
                d->currTheme->listRegBorder, d->currTheme->listRegBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::listSelPixmap(int w, int h)
{
    Texture tex(w, h, d->currTheme->listSelColor, d->currTheme->listSelColorTo,
                d->currTheme->listSelBevel, d->currTheme->listSelGrad,
                d->currTheme->listSelBorder, d->currTheme->listSelBorderColor);
    return tex.renderPixmap();
}

}  // NameSpace Digikam

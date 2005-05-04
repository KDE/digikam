/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-02
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>

#include <qdict.h>
#include <qptrlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qapplication.h>
#include <qpalette.h>
#include <qtimer.h>

extern "C"
{
#include <X11/Xlib.h>
#include <X11/Xresource.h>
}    

#include "theme.h"
#include "texture.h"
#include "themeengine.h"

class ThemeEnginePriv
{
public:

    QPtrList<Theme> themeList;
    QDict<Theme>    themeDict;
    Theme*          currTheme;
    Theme*          defaultTheme;
    XrmDatabase     db;
};


ThemeEngine* ThemeEngine::m_instance = 0;

ThemeEngine* ThemeEngine::instance()
{
    if (!m_instance)
    { 
        new ThemeEngine();
    }
    return m_instance;
}

ThemeEngine::ThemeEngine()
{
    m_instance = this;
    KGlobal::dirs()->addResourceType("themes",
                                     KGlobal::dirs()->kde_default("data")
                                     + "digikam/themes");
    XrmInitialize();
    
    d = new ThemeEnginePriv;
    d->themeList.setAutoDelete(false);
    d->themeDict.setAutoDelete(false);
    d->currTheme = 0;
    d->db        = 0;

    d->defaultTheme = new Theme(i18n("Default"), QString::null);
    d->themeList.append(d->defaultTheme);
    d->themeDict.insert(i18n("Default"), d->defaultTheme);
    d->currTheme = d->defaultTheme;

    buildDefaultTheme();
}

ThemeEngine::~ThemeEngine()
{
    if (d->db)
    {
        XrmDestroyDatabase(d->db);
    }
    
    d->themeList.setAutoDelete(true);
    d->themeList.clear();
    delete d;
    m_instance = 0;
}

void ThemeEngine::scanThemes()
{
    d->themeList.remove(d->defaultTheme);
    d->themeList.setAutoDelete(true);
    d->themeList.clear();
    d->themeDict.clear();
    d->currTheme = 0;

    QStringList themes =
        KGlobal::dirs()->findAllResources( "themes", QString::null,
                                           false, true );

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

    if (d->currTheme == theme)
        return;

    d->currTheme = theme;
    loadTheme();

    //theme->print();
    
    QTimer::singleShot(0, this, SIGNAL(signalThemeChanged()));
}

Theme* ThemeEngine::getCurrentTheme()
{
    return d->currTheme;
}

void ThemeEngine::buildDefaultTheme()
{
    Theme* t = d->defaultTheme;

    QColorGroup cg = QApplication::palette().active();
    
    t->baseColor           = cg.base();
    t->textRegColor        = cg.text();
    t->textSelColor        = cg.highlightedText();
    t->textSpecialRegColor = QColor("#0000EF");
    t->textSpecialSelColor = cg.highlightedText();

    t->bannerColor      = cg.highlight();
    t->bannerColorTo    = cg.highlight().dark(120);
    t->bannerBevel      = Theme::FLAT;
    t->bannerGrad       = Theme::SOLID;
    t->bannerBorder     = false;
    t->bannerBorderColor  = Qt::black;
    
    t->thumbRegColor    = cg.base();
    t->thumbRegColorTo  = cg.base();
    t->thumbRegBevel    = Theme::FLAT;
    t->thumbRegGrad     = Theme::SOLID;
    t->thumbRegBorder   = true;
    t->thumbRegBorderColor = QColor("#E0E0EF");

    t->thumbSelColor    = cg.highlight();
    t->thumbSelColorTo  = cg.highlight();
    t->thumbSelBevel    = Theme::FLAT;
    t->thumbSelGrad     = Theme::SOLID;
    t->thumbSelBorder   = true;
    t->thumbSelBorderColor = QColor("#E0E0EF");

    t->listRegColor     = cg.base();
    t->listRegColorTo   = cg.base();
    t->listRegBevel     = Theme::FLAT;
    t->listRegGrad      = Theme::SOLID;
    t->listRegBorder    = false;
    t->listRegBorderColor = Qt::black;
                        
    t->listSelColor     = cg.highlight();
    t->listSelColorTo   = cg.highlight();
    t->listSelBevel     = Theme::FLAT;
    t->listSelGrad      = Theme::SOLID;
    t->listSelBorder    = true;
    t->listSelBorderColor = Qt::black;
}

void ThemeEngine::loadTheme()
{
    Q_ASSERT( d->currTheme );
    if (!d->currTheme || d->currTheme == d->defaultTheme)
        return;

    Theme *t = d->currTheme;
    
    // use the default theme as base template to build the themes
    *(t) = *(d->defaultTheme);

    if (d->db)
    {
        XrmDestroyDatabase(d->db);
        d->db = 0;
    }
    
    d->db = XrmGetFileDatabase(QFile::encodeName(t->filePath));
    Q_ASSERT( d->db );
    if (!d->db)
        return;

    QString resource;

    // -- base ------------------------------------------------------------------------
    
    resource = resourceValue("base.color", "Base.Color");
    if (!resource.isEmpty())
        t->baseColor = resource;

    resource = resourceValue("text.regular.color", "Text.Regular.Color");
    if (!resource.isEmpty())
        t->textRegColor = resource;
    
    resource = resourceValue("text.selected.color", "Text.Selected.Color");
    if (!resource.isEmpty())
        t->textSelColor = resource;

    resource = resourceValue("text.special.regular.color", "Text.Special.Regular.Color");
    if (!resource.isEmpty())
        t->textSpecialRegColor = resource;
    
    resource = resourceValue("text.special.selected.color", "Text.Special.Selected.Color");
    if (!resource.isEmpty())
        t->textSpecialSelColor = resource;

    // -- banner ------------------------------------------------------------------------
    
    resource = resourceValue("banner.color", "Banner.Color");
    if (!resource.isEmpty())
        t->bannerColor = resource;

    resource = resourceValue("banner.colorTo", "Banner.ColorTo");
    if (!resource.isEmpty())
        t->bannerColorTo = resource;

    resource = resourceValue("banner.bevel", "Banner.Bevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", false))
            t->bannerBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->bannerBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->bannerBevel = Theme::RAISED;
    }

    resource = resourceValue("banner.gradient", "Banner.Gradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", false))
            t->bannerGrad = Theme::SOLID;
        else if (resource.contains("horizontal", false))
            t->bannerGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", false))
            t->bannerGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", false))
            t->bannerGrad = Theme::DIAGONAL;
    }

    resource = resourceValue("banner.border", "Banner.Border");
    if (!resource.isEmpty())
    {
        t->bannerBorder = resource.contains("true", false);
    }

    resource = resourceValue("banner.borderColor", "Banner.BorderColor");
    if (!resource.isEmpty())
    {
        t->bannerBorderColor = resource;
    }

    // -- thumbnail view ----------------------------------------------------------------
    
    resource = resourceValue("thumbnail.regular.color", "Thumbnail.Regular.Color");
    if (!resource.isEmpty())
        t->thumbRegColor = resource;

    resource = resourceValue("thumbnail.regular.colorTo", "Thumbnail.Regular.ColorTo");
    if (!resource.isEmpty())
        t->thumbRegColorTo = resource;

    resource = resourceValue("thumbnail.regular.bevel", "Thumbnail.Regular.Bevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", false))
            t->thumbRegBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->thumbRegBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->thumbRegBevel = Theme::RAISED;
    }

    resource = resourceValue("thumbnail.regular.gradient", "Thumbnail.Regular.Gradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", false))
            t->thumbRegGrad = Theme::SOLID;
        else if (resource.contains("horizontal", false))
            t->thumbRegGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", false))
            t->thumbRegGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", false))
            t->thumbRegGrad = Theme::DIAGONAL;
    }

    resource = resourceValue("thumbnail.regular.border", "Thumbnail.Regular.Border");
    if (!resource.isEmpty())
    {
        t->thumbRegBorder = resource.contains("true", false);
    }

    resource = resourceValue("thumbnail.regular.borderColor", "Thumbnail.Regular.BorderColor");
    if (!resource.isEmpty())
    {
        t->thumbRegBorderColor = resource;
    }
    
    resource = resourceValue("thumbnail.selected.color", "Thumbnail.Selected.Color");
    if (!resource.isEmpty())
        t->thumbSelColor = resource;

    resource = resourceValue("thumbnail.selected.colorTo", "Thumbnail.Selected.ColorTo");
    if (!resource.isEmpty())
        t->thumbSelColorTo = resource;

    resource = resourceValue("thumbnail.selected.bevel", "Thumbnail.Selected.Bevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", false))
            t->thumbSelBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->thumbSelBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->thumbSelBevel = Theme::RAISED;
    }

    resource = resourceValue("thumbnail.selected.gradient", "Thumbnail.Selected.Gradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", false))
            t->thumbSelGrad = Theme::SOLID;
        else if (resource.contains("horizontal", false))
            t->thumbSelGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", false))
            t->thumbSelGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", false))
            t->thumbSelGrad = Theme::DIAGONAL;
    }
    
    resource = resourceValue("thumbnail.selected.border", "Thumbnail.Selected.Border");
    if (!resource.isEmpty())
    {
        t->thumbSelBorder = resource.contains("true", false);
    }

    resource = resourceValue("thumbnail.selected.borderColor", "Thumbnail.Selected.BorderColor");
    if (!resource.isEmpty())
    {
        t->thumbSelBorderColor = resource;
    }

    // -- listview view ----------------------------------------------------------------
    
    resource = resourceValue("listview.regular.color", "Listview.Regular.Color");
    if (!resource.isEmpty())
        t->listRegColor = resource;

    resource = resourceValue("listview.regular.colorTo", "Listview.Regular.ColorTo");
    if (!resource.isEmpty())
        t->listRegColorTo = resource;

    resource = resourceValue("listview.regular.bevel", "Listview.Regular.Bevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", false))
            t->listRegBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->listRegBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->listRegBevel = Theme::RAISED;
    }

    resource = resourceValue("listview.regular.gradient", "Listview.Regular.Gradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", false))
            t->listRegGrad = Theme::SOLID;
        else if (resource.contains("horizontal", false))
            t->listRegGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", false))
            t->listRegGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", false))
            t->listRegGrad = Theme::DIAGONAL;
    }

    resource = resourceValue("listview.regular.border", "Listview.Regular.Border");
    if (!resource.isEmpty())
    {
        t->listRegBorder = resource.contains("true", false);
    }

    resource = resourceValue("listview.regular.borderColor", "Listview.Regular.BorderColor");
    if (!resource.isEmpty())
    {
        t->listRegBorderColor = resource;
    }

    resource = resourceValue("listview.selected.color", "Listview.Selected.Color");
    if (!resource.isEmpty())
        t->listSelColor = resource;

    resource = resourceValue("listview.selected.colorTo", "Listview.Selected.ColorTo");
    if (!resource.isEmpty())
        t->listSelColorTo = resource;

    resource = resourceValue("listview.selected.bevel", "Listview.Selected.Bevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", false))
            t->listSelBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->listSelBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->listSelBevel = Theme::RAISED;
    }

    resource = resourceValue("listview.selected.gradient", "Listview.Selected.Gradient");
    if (!resource.isEmpty())
    {
        if (resource.contains("solid", false))
            t->listSelGrad = Theme::SOLID;
        else if (resource.contains("horizontal", false))
            t->listSelGrad = Theme::HORIZONTAL;
        else if (resource.contains("vertical", false))
            t->listSelGrad = Theme::VERTICAL;
        else if (resource.contains("diagonal", false))
            t->listSelGrad = Theme::DIAGONAL;
    }

    resource = resourceValue("listview.selected.border", "Listview.Selected.Border");
    if (!resource.isEmpty())
    {
        t->listSelBorder = resource.contains("true", false);
    }

    resource = resourceValue("listview.selected.borderColor", "Listview.Selected.BorderColor");
    if (!resource.isEmpty())
    {
        t->listSelBorderColor = resource;
    }
}

QString ThemeEngine::resourceValue(const QString& name, const QString& altName)
{
    if (d->db)
    {        
        XrmValue value;
        char     *value_type;

        XrmGetResource(d->db, name.ascii(), altName.ascii(), &value_type, &value);
        if (value.addr)
            return QString(value.addr);
    }

    return QString("");
}

QColor ThemeEngine::baseColor() const
{
    return d->currTheme->baseColor;    
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

#include "themeengine.moc"

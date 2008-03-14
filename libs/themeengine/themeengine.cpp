/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme engine methods 
 * 
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qdict.h>
#include <qptrlist.h>
#include <qfileinfo.h>
#include <qfile.h>
#include <qapplication.h>
#include <qpalette.h>
#include <qtimer.h>
#include <qtextstream.h>

// KDE includes.

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kuser.h>
#include <kapplication.h>
#include <kglobalsettings.h>

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

    QPalette         defaultPalette;

    QPtrList<Theme>  themeList;
    QDict<Theme>     themeDict;

    Theme           *currTheme;
    Theme           *defaultTheme;
    bool             themeInitiallySet;
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
    m_instance = 0;
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

void ThemeEngine::scanThemes()
{
    d->themeList.remove(d->defaultTheme);
    d->themeList.setAutoDelete(true);
    d->themeList.clear();
    d->themeDict.clear();
    d->currTheme = 0;

    QStringList themes = KGlobal::dirs()->findAllResources( "themes", QString(), false, true );

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

    changePalette();

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

    changePalette();

    QTimer::singleShot(0, this, SIGNAL(signalThemeChanged()));
}

void ThemeEngine::changePalette()
{
    // Make palette for all widgets.
    QPalette plt;

    if (d->currTheme == d->defaultTheme)
        plt = d->defaultPalette;
    else
    {
        plt = kapp->palette();
        int h, s, v;
        const QColor fg(ThemeEngine::instance()->textRegColor());
        const QColor bg(ThemeEngine::instance()->baseColor());
        QColorGroup cg(plt.active());

    /*    bg.hsv(&h, &s, &v);
        v += (v < 128) ? +50 : -50;
        v &= 255; //ensures 0 <= v < 256
        d->currTheme->altBase = QColor(h, s, v, QColor::Hsv);
    */
        fg.hsv(&h, &s, &v);
        v += (v < 128) ? +150 : -150;
        v &= 255; //ensures 0 <= v < 256
        const QColor highlight(h, s, v, QColor::Hsv);

        cg.setColor(QColorGroup::Base,            bg);
        cg.setColor(QColorGroup::Background,      bg.dark(115));
        cg.setColor(QColorGroup::Foreground,      ThemeEngine::instance()->textRegColor());
        cg.setColor(QColorGroup::Highlight,       highlight);
        cg.setColor(QColorGroup::HighlightedText, ThemeEngine::instance()->textSelColor());
        cg.setColor(QColorGroup::Dark,            Qt::darkGray);

        cg.setColor(QColorGroup::Button,          bg);
        cg.setColor(QColorGroup::ButtonText,      ThemeEngine::instance()->textRegColor());

        cg.setColor(QColorGroup::Text,            ThemeEngine::instance()->textRegColor());
        cg.setColor(QColorGroup::Link,            ThemeEngine::instance()->textSpecialRegColor());
        cg.setColor(QColorGroup::LinkVisited,     ThemeEngine::instance()->textSpecialSelColor());

        /*
        cg.setColor(QColorGroup::Light,           ThemeEngine::instance()->textRegColor());
        cg.setColor(QColorGroup::Midlight,        ThemeEngine::instance()->textRegColor());
        cg.setColor(QColorGroup::Mid,             ThemeEngine::instance()->textRegColor());
        cg.setColor(QColorGroup::Shadow,          ThemeEngine::instance()->textRegColor());
        */

        plt.setActive(cg);
        plt.setInactive(cg);
        plt.setDisabled(cg);
    }

    kapp->setPalette(plt, true);
}

Theme* ThemeEngine::getCurrentTheme() const
{
    return d->currTheme;
}

QString ThemeEngine::getCurrentThemeName() const
{
    return d->currTheme->name;
}

void ThemeEngine::buildDefaultTheme()
{
    Theme* t = d->defaultTheme;

    d->defaultPalette      = kapp->palette();
    QColorGroup cg         = d->defaultPalette.active();

    t->baseColor           = cg.base();
    t->textRegColor        = cg.text();
    t->textSelColor        = cg.highlightedText();
    t->textSpecialRegColor = QColor("#0000EF");
    t->textSpecialSelColor = cg.highlightedText();

    t->bannerColor         = cg.highlight();
    t->bannerColorTo       = cg.highlight().dark(120);
    t->bannerBevel         = Theme::FLAT;
    t->bannerGrad          = Theme::SOLID;
    t->bannerBorder        = false;
    t->bannerBorderColor   = Qt::black;

    t->thumbRegColor       = cg.base();
    t->thumbRegColorTo     = cg.base();
    t->thumbRegBevel       = Theme::FLAT;
    t->thumbRegGrad        = Theme::SOLID;
    t->thumbRegBorder      = true;
    t->thumbRegBorderColor = QColor("#E0E0EF");

    t->thumbSelColor       = cg.highlight();
    t->thumbSelColorTo     = cg.highlight();
    t->thumbSelBevel       = Theme::FLAT;
    t->thumbSelGrad        = Theme::SOLID;
    t->thumbSelBorder      = true;
    t->thumbSelBorderColor = QColor("#E0E0EF");

    t->listRegColor        = cg.base();
    t->listRegColorTo      = cg.base();
    t->listRegBevel        = Theme::FLAT;
    t->listRegGrad         = Theme::SOLID;
    t->listRegBorder       = false;
    t->listRegBorderColor  = Qt::black;

    t->listSelColor        = cg.highlight();
    t->listSelColorTo      = cg.highlight();
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
        if (resource.contains("flat", false))
            t->bannerBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->bannerBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->bannerBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "BannerGradient");
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

    resource = resourceValue(rootElem, "BannerBorder");
    if (!resource.isEmpty())
    {
        t->bannerBorder = resource.contains("true", false);
    }

    resource = resourceValue(rootElem, "BannerBorderColor");
    if (!resource.isEmpty())
    {
        t->bannerBorderColor = resource;
    }

    // -- thumbnail view ----------------------------------------------------------------

    resource = resourceValue(rootElem, "ThumbnailRegularColor");
    if (!resource.isEmpty())
        t->thumbRegColor = resource;

    resource = resourceValue(rootElem, "ThumbnailRegularColorTo");
    if (!resource.isEmpty())
        t->thumbRegColorTo = resource;

    resource = resourceValue(rootElem, "ThumbnailRegularBevel");
    if (!resource.isEmpty())
    {
        if (resource.contains("flat", false))
            t->thumbRegBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->thumbRegBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->thumbRegBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ThumbnailRegularGradient");
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

    resource = resourceValue(rootElem, "ThumbnailRegularBorder");
    if (!resource.isEmpty())
    {
        t->thumbRegBorder = resource.contains("true", false);
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
        if (resource.contains("flat", false))
            t->thumbSelBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->thumbSelBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->thumbSelBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ThumbnailSelectedGradient");
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

    resource = resourceValue(rootElem, "ThumbnailSelectedBorder");
    if (!resource.isEmpty())
    {
        t->thumbSelBorder = resource.contains("true", false);
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
        if (resource.contains("flat", false))
            t->listRegBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->listRegBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->listRegBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ListviewRegularGradient");
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

    resource = resourceValue(rootElem, "ListviewRegularBorder");
    if (!resource.isEmpty())
    {
        t->listRegBorder = resource.contains("true", false);
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
        if (resource.contains("flat", false))
            t->listSelBevel = Theme::FLAT;
        else if (resource.contains("sunken", false))
            t->listSelBevel = Theme::SUNKEN;
        else if (resource.contains("raised", false))
            t->listSelBevel = Theme::RAISED;
    }

    resource = resourceValue(rootElem, "ListviewSelectedGradient");
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

    resource = resourceValue(rootElem, "ListviewSelectedBorder");
    if (!resource.isEmpty())
    {
        t->listSelBorder = resource.contains("true", false);
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

bool ThemeEngine::saveTheme()
{
    Q_ASSERT( d->currTheme );
    if (!d->currTheme)
        return false;

    Theme *t = d->currTheme;

    QFileInfo fi(t->filePath);

    QFile themeFile(fi.filePath());

    if (!themeFile.open(IO_WriteOnly))
        return false;

    KUser        user;
    QDomDocument xmlDoc;
    QDomElement  e;
    QString      val;

    // header ------------------------------------------------------------------

    xmlDoc.appendChild(xmlDoc.createProcessingInstruction(QString::fromLatin1("xml"),
                       QString::fromLatin1("version=\"1.0\" encoding=\"UTF-8\"")));

    QString banner = QString("\n/* ============================================================"
                             "\n *"
                             "\n * This file is a part of digiKam project"
                             "\n * http://www.digikam.org"
                             "\n *"
                             "\n * Date        : %1-%2-%3"
                             "\n * Description : %4 colors theme."
                             "\n *"
                             "\n * Copyright (C) %5 by %6"
                             "\n *"
                             "\n * This program is free software; you can redistribute it"
                             "\n * and/or modify it under the terms of the GNU General"
                             "\n * Public License as published by the Free Software Foundation;"
                             "\n * either version 2, or (at your option)"
                             "\n * any later version."
                             "\n * "
                             "\n * This program is distributed in the hope that it will be useful,"
                             "\n * but WITHOUT ANY WARRANTY; without even the implied warranty of"
                             "\n * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"
                             "\n * GNU General Public License for more details."
                             "\n *"
                             "\n * ============================================================ */\n")
                    .arg(QDate::currentDate().year())
                    .arg(QDate::currentDate().month())
                    .arg(QDate::currentDate().day())
                    .arg(fi.fileName())
                    .arg(QDate::currentDate().year())
                    .arg(user.fullName());

    xmlDoc.appendChild(xmlDoc.createComment(banner));

    QDomElement themeElem = xmlDoc.createElement(QString::fromLatin1("digikamtheme")); 
    xmlDoc.appendChild(themeElem);

    // base props --------------------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("name"));
    e.setAttribute(QString::fromLatin1("value"), fi.fileName());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("BaseColor"));
    e.setAttribute(QString::fromLatin1("value"), t->baseColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextRegularColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textRegColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextSelectedColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textSelColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextSpecialRegularColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textSpecialRegColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextSpecialSelectedColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textSpecialSelColor.name().upper());
    themeElem.appendChild(e);

    // banner props ------------------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("BannerColor"));
    e.setAttribute(QString::fromLatin1("value"), t->bannerColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("BannerColorTo"));
    e.setAttribute(QString::fromLatin1("value"), t->bannerColorTo.name().upper());
    themeElem.appendChild(e);

    switch(t->bannerBevel)
    {
        case(Theme::FLAT):
        {
            val = QString("FLAT");
            break;
        }
        case(Theme::RAISED):
        {
            val = QString("RAISED");
            break;
        }
        case(Theme::SUNKEN):
        {
            val = QString("SUNKEN");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("BannerBevel"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    switch(t->bannerGrad)
    {
        case(Theme::SOLID):
        {
            val = QString("SOLID");
            break;
        }
        case(Theme::HORIZONTAL):
        {
            val = QString("HORIZONTAL");
            break;
        }
        case(Theme::VERTICAL):
        {
            val = QString("VERTICAL");
            break;
        }
        case(Theme::DIAGONAL):
        {
            val = QString("DIAGONAL");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("BannerGradient"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("BannerBorder"));
    e.setAttribute(QString::fromLatin1("value"), (t->bannerBorder ? "TRUE" : "FALSE"));
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("BannerBorderColor"));
    e.setAttribute(QString::fromLatin1("value"), t->bannerBorderColor.name().upper());
    themeElem.appendChild(e);

    // thumbnail.regular props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularColor"));
    e.setAttribute(QString::fromLatin1("value"), t->thumbRegColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularColorTo"));
    e.setAttribute(QString::fromLatin1("value"), t->thumbRegColorTo.name().upper());
    themeElem.appendChild(e);

    switch(t->thumbRegBevel)
    {
        case(Theme::FLAT):
        {
            val = QString("FLAT");
            break;
        }
        case(Theme::RAISED):
        {
            val = QString("RAISED");
            break;
        }
        case(Theme::SUNKEN):
        {
            val = QString("SUNKEN");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularBevel"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    switch(t->thumbRegGrad)
    {
        case(Theme::SOLID):
        {
            val = QString("SOLID");
            break;
        }
        case(Theme::HORIZONTAL):
        {
            val = QString("HORIZONTAL");
            break;
        }
        case(Theme::VERTICAL):
        {
            val = QString("VERTICAL");
            break;
        }
        case(Theme::DIAGONAL):
        {
            val = QString("DIAGONAL");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularGradient"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularBorder"));
    e.setAttribute(QString::fromLatin1("value"), (t->thumbRegBorder ? "TRUE" : "FALSE"));
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularBorderColor"));
    e.setAttribute(QString::fromLatin1("value"), t->thumbRegBorderColor.name().upper());
    themeElem.appendChild(e);

    // thumbnail.selected props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedColor"));
    e.setAttribute(QString::fromLatin1("value"), t->thumbSelColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedColorTo"));
    e.setAttribute(QString::fromLatin1("value"), t->thumbSelColorTo.name().upper());
    themeElem.appendChild(e);

    switch(t->thumbSelBevel)
    {
        case(Theme::FLAT):
        {
            val = QString("FLAT");
            break;
        }
        case(Theme::RAISED):
        {
            val = QString("RAISED");
            break;
        }
        case(Theme::SUNKEN):
        {
            val = QString("SUNKEN");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedBevel"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    switch(t->thumbSelGrad)
    {
        case(Theme::SOLID):
        {
            val = QString("SOLID");
            break;
        }
        case(Theme::HORIZONTAL):
        {
            val = QString("HORIZONTAL");
            break;
        }
        case(Theme::VERTICAL):
        {
            val = QString("VERTICAL");
            break;
        }
        case(Theme::DIAGONAL):
        {
            val = QString("DIAGONAL");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedGradient"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedBorder"));
    e.setAttribute(QString::fromLatin1("value"), (t->thumbSelBorder ? "TRUE" : "FALSE"));
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedBorderColor"));
    e.setAttribute(QString::fromLatin1("value"), t->thumbSelBorderColor.name().upper());
    themeElem.appendChild(e);

    // listview.regular props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularColor"));
    e.setAttribute(QString::fromLatin1("value"), t->listRegColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularColorTo"));
    e.setAttribute(QString::fromLatin1("value"), t->listRegColorTo.name().upper());
    themeElem.appendChild(e);

    switch(t->listRegBevel)
    {
        case(Theme::FLAT):
        {
            val = QString("FLAT");
            break;
        }
        case(Theme::RAISED):
        {
            val = QString("RAISED");
            break;
        }
        case(Theme::SUNKEN):
        {
            val = QString("SUNKEN");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularBevel"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    switch(t->listRegGrad)
    {
        case(Theme::SOLID):
        {
            val = QString("SOLID");
            break;
        }
        case(Theme::HORIZONTAL):
        {
            val = QString("HORIZONTAL");
            break;
        }
        case(Theme::VERTICAL):
        {
            val = QString("VERTICAL");
            break;
        }
        case(Theme::DIAGONAL):
        {
            val = QString("DIAGONAL");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularGradient"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularBorder"));
    e.setAttribute(QString::fromLatin1("value"), (t->listRegBorder ? "TRUE" : "FALSE"));
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularBorderColor"));
    e.setAttribute(QString::fromLatin1("value"), t->listRegBorderColor.name().upper());
    themeElem.appendChild(e);

    // listview.selected props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedColor"));
    e.setAttribute(QString::fromLatin1("value"), t->listSelColor.name().upper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedColorTo"));
    e.setAttribute(QString::fromLatin1("value"), t->listSelColorTo.name().upper());
    themeElem.appendChild(e);

    switch(t->listSelBevel)
    {
        case(Theme::FLAT):
        {
            val = QString("FLAT");
            break;
        }
        case(Theme::RAISED):
        {
            val = QString("RAISED");
            break;
        }
        case(Theme::SUNKEN):
        {
            val = QString("SUNKEN");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedBevel"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    switch(t->listSelGrad)
    {
        case(Theme::SOLID):
        {
            val = QString("SOLID");
            break;
        }
        case(Theme::HORIZONTAL):
        {
            val = QString("HORIZONTAL");
            break;
        }
        case(Theme::VERTICAL):
        {
            val = QString("VERTICAL");
            break;
        }
        case(Theme::DIAGONAL):
        {
            val = QString("DIAGONAL");
            break;
        }
    };

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedGradient"));
    e.setAttribute(QString::fromLatin1("value"), val);
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedBorder"));
    e.setAttribute(QString::fromLatin1("value"), (t->listSelBorder ? "TRUE" : "FALSE"));
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedBorderColor"));
    e.setAttribute(QString::fromLatin1("value"), t->listSelBorderColor.name().upper());
    themeElem.appendChild(e);

    // -------------------------------------------------------------------------

    QTextStream stream(&themeFile);
    stream.setEncoding(QTextStream::UnicodeUTF8);
    stream << xmlDoc.toString();
    themeFile.close();

    return true;
}

}  // NameSpace Digikam

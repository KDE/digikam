/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme engine methods
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "themeengine.moc"

// Qt includes

#include <QHash>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QTimer>
#include <QPixmap>
#include <QDomDocument>
#include <QTextStream>
#include <QDate>
#include <QTextCodec>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kuser.h>
#include <kapplication.h>
#include <kglobalsettings.h>
#include <kdebug.h>

// Local includes

#include "theme.h"
#include "texture.h"

namespace Digikam
{

class ThemeEngineCreator
{
public:

    ThemeEngine object;
};

K_GLOBAL_STATIC(ThemeEngineCreator, creator)

// ---------------------------------------------------------------


class ThemeEngine::ThemeEnginePriv
{
public:

    ThemeEnginePriv()
    {
        defaultTheme      = 0;
        currTheme         = 0;
        themeInitiallySet = false;
    }

    bool                   themeInitiallySet;

    QPalette               defaultPalette;

    QHash<QString, Theme*> themeHash;

    Theme*                 currTheme;
    Theme*                 defaultTheme;
};

ThemeEngine::ThemeEngine()
    : d(new ThemeEnginePriv)
{
    KGlobal::dirs()->addResourceDir("themes", KStandardDirs::installPath("data") + QString("digikam/themes"));

    d->defaultTheme = new Theme(i18n("Default"), QString());
    d->themeHash.insert(i18n("Default"), d->defaultTheme);
    d->currTheme    = d->defaultTheme;

    buildDefaultTheme();
}

ThemeEngine::~ThemeEngine()
{
    // Delete all hash items
    while (!d->themeHash.isEmpty())
    {
        Theme* value = *d->themeHash.begin();
        d->themeHash.erase(d->themeHash.begin());
        delete value;
    }

    delete d;
}

ThemeEngine* ThemeEngine::instance()
{
    return &creator->object;
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
    d->themeHash.remove(i18n("Default"));
    d->themeHash.clear();
    d->currTheme = 0;

    QStringList themes = KGlobal::dirs()->findAllResources("themes", QString(),
                         KStandardDirs::Recursive | KStandardDirs::NoDuplicates);

    for (QStringList::const_iterator it = themes.constBegin(); it != themes.constEnd(); ++it)
    {
        QFileInfo fi(*it);
        Theme* theme = new Theme(fi.fileName(), *it);
        d->themeHash.insert(fi.fileName(), theme);
    }

    d->themeHash.insert(i18n("Default"), d->defaultTheme);
    d->currTheme = d->defaultTheme;
}

QStringList ThemeEngine::themeNames() const
{
    QStringList names;
    QHash<QString, Theme*>::iterator it;

    for (it = d->themeHash.begin(); it != d->themeHash.end(); ++it)
    {
        Theme* t = it.value();
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
    QHash<QString, Theme*>::iterator it = d->themeHash.find(name);

    if (it == d->themeHash.end())
    {
        d->currTheme = d->defaultTheme;
        return;
    }

    Theme* theme = it.value();

    if (d->currTheme == theme && d->themeInitiallySet)
    {
        return;
    }

    d->currTheme = theme;
    loadTheme();

    // this is only to ensure that even if the chosen theme is the default theme,
    // the signalThemeChanged is emitted when themes are loaded in DigikamApp
    d->themeInitiallySet = true;

    changePalette();

    //theme->print();
    QTimer::singleShot(0, this, SIGNAL(signalThemeChanged()));
}

void ThemeEngine::setCurrentTheme(const Theme& theme, const QString& name, bool loadFromDisk)
{
    QHash<QString, Theme*>::iterator it = d->themeHash.find(name);

    if (it != d->themeHash.end())
    {
        d->themeHash.remove(name);
    }

    Theme* t = new Theme(theme);
    t->filePath = theme.filePath;
    d->themeHash.insert(name, t);

    d->currTheme = t;

    if (loadFromDisk)
    {
        loadTheme();
    }

    changePalette();

    QTimer::singleShot(0, this, SIGNAL(signalThemeChanged()));
}

void ThemeEngine::changePalette()
{
    // Make palette for all widgets.
    QPalette plt;

    if (d->currTheme == d->defaultTheme)
    {
        plt = d->defaultPalette;
    }
    else
    {
        plt = kapp->palette();
        int h, s, v;
        const QColor fg(ThemeEngine::instance()->textRegColor());
        const QColor bg(ThemeEngine::instance()->baseColor());

        /*
                bg.hsv(&h, &s, &v);
                v += (v < 128) ? +50 : -50;
                v &= 255; //ensures 0 <= v < 256
                d->currTheme->altBase = QColor(h, s, v, QColor::Hsv);
        */
        fg.getHsv(&h, &s, &v);
        v += (v < 128) ? +150 : -150;
        v &= 255; //ensures 0 <= v < 256
        const QColor highlight = QColor::fromHsv(h, s, v);

        plt.setColor(QPalette::Active,   QPalette::Base,            bg);
        plt.setColor(QPalette::Active,   QPalette::Background,      bg.dark(115));
        plt.setColor(QPalette::Active,   QPalette::Foreground,      ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Active,   QPalette::Highlight,       highlight);
        plt.setColor(QPalette::Active,   QPalette::HighlightedText, ThemeEngine::instance()->textSelColor());
        plt.setColor(QPalette::Active,   QPalette::Dark,            Qt::darkGray);
        plt.setColor(QPalette::Active,   QPalette::Button,          bg);
        plt.setColor(QPalette::Active,   QPalette::ButtonText,      ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Active,   QPalette::Text,            ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Active,   QPalette::Link,            ThemeEngine::instance()->textSpecialRegColor());
        plt.setColor(QPalette::Active,   QPalette::LinkVisited,     ThemeEngine::instance()->textSpecialSelColor());

        plt.setColor(QPalette::Inactive, QPalette::Base,            bg);
        plt.setColor(QPalette::Inactive, QPalette::Background,      bg.dark(115));
        plt.setColor(QPalette::Inactive, QPalette::Foreground,      ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Inactive, QPalette::Highlight,       highlight);
        plt.setColor(QPalette::Inactive, QPalette::HighlightedText, ThemeEngine::instance()->textSelColor());
        plt.setColor(QPalette::Inactive, QPalette::Dark,            Qt::darkGray);
        plt.setColor(QPalette::Inactive, QPalette::Button,          bg);
        plt.setColor(QPalette::Inactive, QPalette::ButtonText,      ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Inactive, QPalette::Text,            ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Inactive, QPalette::Link,            ThemeEngine::instance()->textSpecialRegColor());
        plt.setColor(QPalette::Inactive, QPalette::LinkVisited,     ThemeEngine::instance()->textSpecialSelColor());
        plt.setColor(QPalette::Inactive, QPalette::ToolTipText,     ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Inactive, QPalette::ToolTipBase,     bg);

        plt.setColor(QPalette::Disabled, QPalette::Base,            bg);
        plt.setColor(QPalette::Disabled, QPalette::Background,      bg.dark(115));
        plt.setColor(QPalette::Disabled, QPalette::Foreground,      ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Disabled, QPalette::Highlight,       highlight);
        plt.setColor(QPalette::Disabled, QPalette::HighlightedText, ThemeEngine::instance()->textSelColor());
        plt.setColor(QPalette::Disabled, QPalette::Dark,            Qt::darkGray);
        plt.setColor(QPalette::Disabled, QPalette::Button,          bg);
        plt.setColor(QPalette::Disabled, QPalette::ButtonText,      ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Disabled, QPalette::Text,            ThemeEngine::instance()->textRegColor());
        plt.setColor(QPalette::Disabled, QPalette::Link,            ThemeEngine::instance()->textSpecialRegColor());
        plt.setColor(QPalette::Disabled, QPalette::LinkVisited,     ThemeEngine::instance()->textSpecialSelColor());

        /*
                cg.setColor(QColorGroup::Light,           ThemeEngine::instance()->textRegColor());
                cg.setColor(QColorGroup::Midlight,        ThemeEngine::instance()->textRegColor());
                cg.setColor(QColorGroup::Mid,             ThemeEngine::instance()->textRegColor());
                cg.setColor(QColorGroup::Shadow,          ThemeEngine::instance()->textRegColor());
        */
    }

    kapp->setPalette(plt);
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

    t->baseColor           = d->defaultPalette.color(QPalette::Base);
    t->textRegColor        = d->defaultPalette.color(QPalette::Text);
    t->textSelColor        = d->defaultPalette.color(QPalette::HighlightedText);
    t->textSpecialRegColor = QColor("#0000EF");
    t->textSpecialSelColor = d->defaultPalette.color(QPalette::HighlightedText);

    t->bannerColor         = d->defaultPalette.color(QPalette::Highlight);
    t->bannerColorTo       = d->defaultPalette.color(QPalette::Highlight).dark(120);
    t->bannerBevel         = Theme::FLAT;
    t->bannerGrad          = Theme::SOLID;
    t->bannerBorder        = false;
    t->bannerBorderColor   = Qt::black;

    t->thumbRegColor       = d->defaultPalette.color(QPalette::Base);
    t->thumbRegColorTo     = d->defaultPalette.color(QPalette::Base);
    t->thumbRegBevel       = Theme::FLAT;
    t->thumbRegGrad        = Theme::SOLID;
    t->thumbRegBorder      = true;
    t->thumbRegBorderColor = QColor("#E0E0EF");

    t->thumbSelColor       = d->defaultPalette.color(QPalette::Highlight);
    t->thumbSelColorTo     = d->defaultPalette.color(QPalette::Highlight);
    t->thumbSelBevel       = Theme::FLAT;
    t->thumbSelGrad        = Theme::SOLID;
    t->thumbSelBorder      = true;
    t->thumbSelBorderColor = QColor("#E0E0EF");

    t->listRegColor        = d->defaultPalette.color(QPalette::Base);
    t->listRegColorTo      = d->defaultPalette.color(QPalette::Base);
    t->listRegBevel        = Theme::FLAT;
    t->listRegGrad         = Theme::SOLID;
    t->listRegBorder       = false;
    t->listRegBorderColor  = Qt::black;

    t->listSelColor        = d->defaultPalette.color(QPalette::Highlight);
    t->listSelColorTo      = d->defaultPalette.color(QPalette::Highlight);
    t->listSelBevel        = Theme::FLAT;
    t->listSelGrad         = Theme::SOLID;
    t->listSelBorder       = true;
    t->listSelBorderColor  = Qt::black;
}

bool ThemeEngine::loadTheme()
{
    Q_ASSERT( d->currTheme );

    if (!d->currTheme || d->currTheme == d->defaultTheme)
    {
        return false;
    }

    Theme* t = d->currTheme;

    // use the default theme as base template to build the themes
    *(t) = *(d->defaultTheme);

    QFile themeFile(t->filePath);

    if (!themeFile.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QDomDocument xmlDoc;
    QString error;
    int row, col;

    if (!xmlDoc.setContent(&themeFile, true, &error, &row, &col))
    {
        kDebug() << "Theme file: " << t->filePath;
        kDebug() << error << " :: row=" << row << " , col=" << col;
        return false;
    }

    QDomElement rootElem = xmlDoc.documentElement();

    if (rootElem.tagName() != QString::fromLatin1("digikamtheme"))
    {
        return false;
    }

    QString resource;

    // -- base ------------------------------------------------------------------------

    resource = resourceValue(rootElem, "BaseColor");

    if (!resource.isEmpty())
    {
        t->baseColor = resource;
    }

    resource = resourceValue(rootElem, "TextRegularColor");

    if (!resource.isEmpty())
    {
        t->textRegColor = resource;
    }

    resource = resourceValue(rootElem, "TextSelectedColor");

    if (!resource.isEmpty())
    {
        t->textSelColor = resource;
    }

    resource = resourceValue(rootElem, "TextSpecialRegularColor");

    if (!resource.isEmpty())
    {
        t->textSpecialRegColor = resource;
    }

    resource = resourceValue(rootElem, "TextSpecialSelectedColor");

    if (!resource.isEmpty())
    {
        t->textSpecialSelColor = resource;
    }

    // -- banner ------------------------------------------------------------------------

    resource = resourceValue(rootElem, "BannerColor");

    if (!resource.isEmpty())
    {
        t->bannerColor = resource;
    }

    resource = resourceValue(rootElem, "BannerColorTo");

    if (!resource.isEmpty())
    {
        t->bannerColorTo = resource;
    }

    resource = resourceValue(rootElem, "BannerBevel");

    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
        {
            t->bannerBevel = Theme::FLAT;
        }
        else if (resource.contains("sunken", Qt::CaseInsensitive))
        {
            t->bannerBevel = Theme::SUNKEN;
        }
        else if (resource.contains("raised", Qt::CaseInsensitive))
        {
            t->bannerBevel = Theme::RAISED;
        }
    }

    resource = resourceValue(rootElem, "BannerGradient");

    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
        {
            t->bannerGrad = Theme::SOLID;
        }
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
        {
            t->bannerGrad = Theme::HORIZONTAL;
        }
        else if (resource.contains("vertical", Qt::CaseInsensitive))
        {
            t->bannerGrad = Theme::VERTICAL;
        }
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
        {
            t->bannerGrad = Theme::DIAGONAL;
        }
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

    resource = resourceValue(rootElem, "ThumbnailRegularColor");

    if (!resource.isEmpty())
    {
        t->thumbRegColor = resource;
    }

    resource = resourceValue(rootElem, "ThumbnailRegularColorTo");

    if (!resource.isEmpty())
    {
        t->thumbRegColorTo = resource;
    }

    resource = resourceValue(rootElem, "ThumbnailRegularBevel");

    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
        {
            t->thumbRegBevel = Theme::FLAT;
        }
        else if (resource.contains("sunken", Qt::CaseInsensitive))
        {
            t->thumbRegBevel = Theme::SUNKEN;
        }
        else if (resource.contains("raised", Qt::CaseInsensitive))
        {
            t->thumbRegBevel = Theme::RAISED;
        }
    }

    resource = resourceValue(rootElem, "ThumbnailRegularGradient");

    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
        {
            t->thumbRegGrad = Theme::SOLID;
        }
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
        {
            t->thumbRegGrad = Theme::HORIZONTAL;
        }
        else if (resource.contains("vertical", Qt::CaseInsensitive))
        {
            t->thumbRegGrad = Theme::VERTICAL;
        }
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
        {
            t->thumbRegGrad = Theme::DIAGONAL;
        }
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
    {
        t->thumbSelColor = resource;
    }

    resource = resourceValue(rootElem, "ThumbnailSelectedColorTo");

    if (!resource.isEmpty())
    {
        t->thumbSelColorTo = resource;
    }

    resource = resourceValue(rootElem, "ThumbnailSelectedBevel");

    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
        {
            t->thumbSelBevel = Theme::FLAT;
        }
        else if (resource.contains("sunken", Qt::CaseInsensitive))
        {
            t->thumbSelBevel = Theme::SUNKEN;
        }
        else if (resource.contains("raised", Qt::CaseInsensitive))
        {
            t->thumbSelBevel = Theme::RAISED;
        }
    }

    resource = resourceValue(rootElem, "ThumbnailSelectedGradient");

    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
        {
            t->thumbSelGrad = Theme::SOLID;
        }
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
        {
            t->thumbSelGrad = Theme::HORIZONTAL;
        }
        else if (resource.contains("vertical", Qt::CaseInsensitive))
        {
            t->thumbSelGrad = Theme::VERTICAL;
        }
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
        {
            t->thumbSelGrad = Theme::DIAGONAL;
        }
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
    {
        t->listRegColor = resource;
    }

    resource = resourceValue(rootElem, "ListviewRegularColorTo");

    if (!resource.isEmpty())
    {
        t->listRegColorTo = resource;
    }

    resource = resourceValue(rootElem, "ListviewRegularBevel");

    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
        {
            t->listRegBevel = Theme::FLAT;
        }
        else if (resource.contains("sunken", Qt::CaseInsensitive))
        {
            t->listRegBevel = Theme::SUNKEN;
        }
        else if (resource.contains("raised", Qt::CaseInsensitive))
        {
            t->listRegBevel = Theme::RAISED;
        }
    }

    resource = resourceValue(rootElem, "ListviewRegularGradient");

    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
        {
            t->listRegGrad = Theme::SOLID;
        }
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
        {
            t->listRegGrad = Theme::HORIZONTAL;
        }
        else if (resource.contains("vertical", Qt::CaseInsensitive))
        {
            t->listRegGrad = Theme::VERTICAL;
        }
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
        {
            t->listRegGrad = Theme::DIAGONAL;
        }
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
    {
        t->listSelColor = resource;
    }

    resource = resourceValue(rootElem, "ListviewSelectedColorTo");

    if (!resource.isEmpty())
    {
        t->listSelColorTo = resource;
    }

    resource = resourceValue(rootElem, "ListviewSelectedBevel");

    if (!resource.isEmpty())
    {
        if (resource.contains("flat", Qt::CaseInsensitive))
        {
            t->listSelBevel = Theme::FLAT;
        }
        else if (resource.contains("sunken", Qt::CaseInsensitive))
        {
            t->listSelBevel = Theme::SUNKEN;
        }
        else if (resource.contains("raised", Qt::CaseInsensitive))
        {
            t->listSelBevel = Theme::RAISED;
        }
    }

    resource = resourceValue(rootElem, "ListviewSelectedGradient");

    if (!resource.isEmpty())
    {
        if (resource.contains("solid", Qt::CaseInsensitive))
        {
            t->listSelGrad = Theme::SOLID;
        }
        else if (resource.contains("horizontal", Qt::CaseInsensitive))
        {
            t->listSelGrad = Theme::HORIZONTAL;
        }
        else if (resource.contains("vertical", Qt::CaseInsensitive))
        {
            t->listSelGrad = Theme::VERTICAL;
        }
        else if (resource.contains("diagonal", Qt::CaseInsensitive))
        {
            t->listSelGrad = Theme::DIAGONAL;
        }
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

    kDebug() << "Theme file loaded: " << t->filePath;
    return true;
}

QString ThemeEngine::resourceValue(const QDomElement& rootElem, const QString& key)
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

    return QString();
}

bool ThemeEngine::saveTheme()
{
    Q_ASSERT( d->currTheme );

    if (!d->currTheme)
    {
        return false;
    }

    Theme* t = d->currTheme;

    QFileInfo fi(t->filePath);

    QFile themeFile(fi.filePath());

    if (!themeFile.open(QIODevice::WriteOnly))
    {
        return false;
    }

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
                     .arg(user.property(KUser::FullName).toString());

    xmlDoc.appendChild(xmlDoc.createComment(banner));

    QDomElement themeElem = xmlDoc.createElement(QString::fromLatin1("digikamtheme"));
    xmlDoc.appendChild(themeElem);

    // base props --------------------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("name"));
    e.setAttribute(QString::fromLatin1("value"), fi.fileName());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("BaseColor"));
    e.setAttribute(QString::fromLatin1("value"), t->baseColor.name().toUpper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextRegularColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textRegColor.name().toUpper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextSelectedColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textSelColor.name().toUpper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextSpecialRegularColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textSpecialRegColor.name().toUpper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("TextSpecialSelectedColor"));
    e.setAttribute(QString::fromLatin1("value"), t->textSpecialSelColor.name().toUpper());
    themeElem.appendChild(e);

    // banner props ------------------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("BannerColor"));
    e.setAttribute(QString::fromLatin1("value"), t->bannerColor.name().toUpper());
    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("BannerColorTo"));
    e.setAttribute(QString::fromLatin1("value"), t->bannerColorTo.name().toUpper());
    themeElem.appendChild(e);

    switch (t->bannerBevel)
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

    switch (t->bannerGrad)
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

    e.setAttribute(QString::fromLatin1("value"), t->bannerBorderColor.name().toUpper());

    themeElem.appendChild(e);

    // thumbnail.regular props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularColor"));

    e.setAttribute(QString::fromLatin1("value"), t->thumbRegColor.name().toUpper());

    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailRegularColorTo"));

    e.setAttribute(QString::fromLatin1("value"), t->thumbRegColorTo.name().toUpper());

    themeElem.appendChild(e);

    switch (t->thumbRegBevel)
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

    switch (t->thumbRegGrad)
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

    e.setAttribute(QString::fromLatin1("value"), t->thumbRegBorderColor.name().toUpper());

    themeElem.appendChild(e);

    // thumbnail.selected props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedColor"));

    e.setAttribute(QString::fromLatin1("value"), t->thumbSelColor.name().toUpper());

    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ThumbnailSelectedColorTo"));

    e.setAttribute(QString::fromLatin1("value"), t->thumbSelColorTo.name().toUpper());

    themeElem.appendChild(e);

    switch (t->thumbSelBevel)
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

    switch (t->thumbSelGrad)
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

    e.setAttribute(QString::fromLatin1("value"), t->thumbSelBorderColor.name().toUpper());

    themeElem.appendChild(e);

    // listview.regular props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularColor"));

    e.setAttribute(QString::fromLatin1("value"), t->listRegColor.name().toUpper());

    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewRegularColorTo"));

    e.setAttribute(QString::fromLatin1("value"), t->listRegColorTo.name().toUpper());

    themeElem.appendChild(e);

    switch (t->listRegBevel)
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

    switch (t->listRegGrad)
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

    e.setAttribute(QString::fromLatin1("value"), t->listRegBorderColor.name().toUpper());

    themeElem.appendChild(e);

    // listview.selected props -------------------------------------------------

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedColor"));

    e.setAttribute(QString::fromLatin1("value"), t->listSelColor.name().toUpper());

    themeElem.appendChild(e);

    e = xmlDoc.createElement(QString::fromLatin1("ListviewSelectedColorTo"));

    e.setAttribute(QString::fromLatin1("value"), t->listSelColorTo.name().toUpper());

    themeElem.appendChild(e);

    switch (t->listSelBevel)
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

    switch (t->listSelGrad)
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

    e.setAttribute(QString::fromLatin1("value"), t->listSelBorderColor.name().toUpper());

    themeElem.appendChild(e);

    // -------------------------------------------------------------------------

    QTextStream stream(&themeFile);

    stream.setCodec(QTextCodec::codecForName("UTF-8"));

    stream.setAutoDetectUnicode(true);

    stream << xmlDoc.toString();

    themeFile.close();

    return true;
}

}  // namespace Digikam

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

#include <QStringList>
#include <QFileInfo>
#include <QFile>
#include <QApplication>
#include <QPalette>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QDate>
#include <QDesktopWidget>

// KDE includes

#include <kglobal.h>
#include <klocale.h>
#include <kcolorscheme.h>
#include <kactioncollection.h>
#include <kstandarddirs.h>
#include <kselectaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobalsettings.h>
#include <kdebug.h>
#include <kxmlguiwindow.h>

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
        themeMenuAction = 0;
    }

    QMap<QString, QString> themeMap;            // map<theme name, theme config path>
    QPalette               palette;

    KSelectAction*         themeMenuAction;

    Theme                  theme;
};

ThemeEngine::ThemeEngine()
    : d(new ThemeEnginePriv)
{
}

ThemeEngine::~ThemeEngine()
{
    delete d;
}

ThemeEngine* ThemeEngine::instance()
{
    return &creator->object;
}

QColor ThemeEngine::baseColor() const
{
    return d->theme.baseColor;
}

QColor ThemeEngine::thumbSelColor() const
{
    return d->theme.thumbSelColor;
}

QColor ThemeEngine::thumbRegColor() const
{
    return d->theme.thumbRegColor;
}

QColor ThemeEngine::textRegColor() const
{
    return d->theme.textRegColor;
}

QColor ThemeEngine::textSelColor() const
{
    return d->theme.textSelColor;
}

QColor ThemeEngine::textSpecialRegColor() const
{
    return d->theme.textSpecialRegColor;
}

QColor ThemeEngine::textSpecialSelColor() const
{
    return d->theme.textSpecialSelColor;
}

QPixmap ThemeEngine::bannerPixmap(int w, int h)
{
    Texture tex(w, h, d->theme.bannerColor, d->theme.bannerColorTo,
                d->theme.bannerBevel, d->theme.bannerGrad,
                d->theme.bannerBorder, d->theme.bannerBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::thumbRegPixmap(int w, int h)
{
    Texture tex(w, h, d->theme.thumbRegColor, d->theme.thumbRegColorTo,
                d->theme.thumbRegBevel, d->theme.thumbRegGrad,
                d->theme.thumbRegBorder, d->theme.thumbRegBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::thumbSelPixmap(int w, int h)
{
    Texture tex(w, h, d->theme.thumbSelColor, d->theme.thumbSelColorTo,
                d->theme.thumbSelBevel, d->theme.thumbSelGrad,
                d->theme.thumbSelBorder, d->theme.thumbSelBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::listRegPixmap(int w, int h)
{
    Texture tex(w, h, d->theme.listRegColor, d->theme.listRegColorTo,
                d->theme.listRegBevel, d->theme.listRegGrad,
                d->theme.listRegBorder, d->theme.listRegBorderColor);
    return tex.renderPixmap();
}

QPixmap ThemeEngine::listSelPixmap(int w, int h)
{
    Texture tex(w, h, d->theme.listSelColor, d->theme.listSelColorTo,
                d->theme.listSelBevel, d->theme.listSelGrad,
                d->theme.listSelBorder, d->theme.listSelBorderColor);
    return tex.renderPixmap();
}

QString ThemeEngine::currentThemeName() const
{
    if (!d->themeMenuAction) return i18n("Default");
    QString name = d->themeMenuAction->currentText();
    return name.isEmpty() ? i18n("Default") : name;
}

void ThemeEngine::setCurrentTheme(const QString& name)
{
    if (!d->themeMenuAction) return;
    slotChangePalette(name);
    d->themeMenuAction->setCurrentAction(name);
}

void ThemeEngine::buildTheme()
{
    d->theme.baseColor           = d->palette.color(QPalette::Base);
    d->theme.textRegColor        = d->palette.color(QPalette::Text);
    d->theme.textSelColor        = d->palette.color(QPalette::HighlightedText);
    d->theme.textSpecialRegColor = d->palette.color(QPalette::Link);
    d->theme.textSpecialSelColor = d->palette.color(QPalette::HighlightedText);

    d->theme.bannerColor         = d->palette.color(QPalette::Highlight);
    d->theme.bannerColorTo       = d->palette.color(QPalette::Highlight).dark(120);
    d->theme.bannerBevel         = Theme::FLAT;
    d->theme.bannerGrad          = Theme::SOLID;
    d->theme.bannerBorder        = false;
    d->theme.bannerBorderColor   = Qt::black;

    d->theme.thumbRegColor       = d->palette.color(QPalette::Base);
    d->theme.thumbRegColorTo     = d->palette.color(QPalette::Base);
    d->theme.thumbRegBevel       = Theme::FLAT;
    d->theme.thumbRegGrad        = Theme::SOLID;
    d->theme.thumbRegBorder      = true;
    d->theme.thumbRegBorderColor = d->palette.color(QPalette::Midlight);

    d->theme.thumbSelColor       = d->palette.color(QPalette::Highlight);
    d->theme.thumbSelColorTo     = d->palette.color(QPalette::Highlight);
    d->theme.thumbSelBevel       = Theme::FLAT;
    d->theme.thumbSelGrad        = Theme::SOLID;
    d->theme.thumbSelBorder      = true;
    d->theme.thumbSelBorderColor = d->palette.color(QPalette::Midlight);

    d->theme.listRegColor        = d->palette.color(QPalette::Base);
    d->theme.listRegColorTo      = d->palette.color(QPalette::Base);
    d->theme.listRegBevel        = Theme::FLAT;
    d->theme.listRegGrad         = Theme::SOLID;
    d->theme.listRegBorder       = false;
    d->theme.listRegBorderColor  = Qt::black;

    d->theme.listSelColor        = d->palette.color(QPalette::Highlight);
    d->theme.listSelColorTo      = d->palette.color(QPalette::Highlight);
    d->theme.listSelBevel        = Theme::FLAT;
    d->theme.listSelGrad         = Theme::SOLID;
    d->theme.listSelBorder       = true;
    d->theme.listSelBorderColor  = Qt::black;
}

void ThemeEngine::slotChangePalette(const QString& name)
{
    kDebug() << name;
    d->palette = kapp->palette();

    if (name == i18n("Default"))
    {
        d->palette = QApplication::desktop()->palette();
    }
    else
    {
        QString filename = d->themeMap.value(name);
        kDebug() << filename;
        KSharedConfigPtr config        = KSharedConfig::openConfig(filename);
        QPalette::ColorGroup states[3] = { QPalette::Active, QPalette::Inactive, QPalette::Disabled };
        // TT thinks tooltips shouldn't use active, so we use our active colors for all states
        KColorScheme schemeTooltip(QPalette::Active, KColorScheme::Tooltip, config);

        for ( int i = 0; i < 3 ; i++ )
        {
            QPalette::ColorGroup state = states[i];
            KColorScheme schemeView(state,      KColorScheme::View,      config);
            KColorScheme schemeWindow(state,    KColorScheme::Window,    config);
            KColorScheme schemeButton(state,    KColorScheme::Button,    config);
            KColorScheme schemeSelection(state, KColorScheme::Selection, config);

            d->palette.setBrush( state, QPalette::WindowText,      schemeWindow.foreground() );
            d->palette.setBrush( state, QPalette::Window,          schemeWindow.background() );
            d->palette.setBrush( state, QPalette::Base,            schemeView.background() );
            d->palette.setBrush( state, QPalette::Text,            schemeView.foreground() );
            d->palette.setBrush( state, QPalette::Button,          schemeButton.background() );
            d->palette.setBrush( state, QPalette::ButtonText,      schemeButton.foreground() );
            d->palette.setBrush( state, QPalette::Highlight,       schemeSelection.background() );
            d->palette.setBrush( state, QPalette::HighlightedText, schemeSelection.foreground() );
            d->palette.setBrush( state, QPalette::ToolTipBase,     schemeTooltip.background() );
            d->palette.setBrush( state, QPalette::ToolTipText,     schemeTooltip.foreground() );

            d->palette.setColor( state, QPalette::Light,    schemeWindow.shade( KColorScheme::LightShade ) );
            d->palette.setColor( state, QPalette::Midlight, schemeWindow.shade( KColorScheme::MidlightShade ) );
            d->palette.setColor( state, QPalette::Mid,      schemeWindow.shade( KColorScheme::MidShade ) );
            d->palette.setColor( state, QPalette::Dark,     schemeWindow.shade( KColorScheme::DarkShade ) );
            d->palette.setColor( state, QPalette::Shadow,   schemeWindow.shade( KColorScheme::ShadowShade ) );

            d->palette.setBrush( state, QPalette::AlternateBase, schemeView.background( KColorScheme::AlternateBackground) );
            d->palette.setBrush( state, QPalette::Link,          schemeView.foreground( KColorScheme::LinkText ) );
            d->palette.setBrush( state, QPalette::LinkVisited,   schemeView.foreground( KColorScheme::VisitedText ) );
        }
    }

    kapp->setPalette(d->palette);
    buildTheme();
    emit signalThemeChanged();
}

void ThemeEngine::setThemeMenuAction(KSelectAction* const action)
{
    d->themeMenuAction = action;
    populateThemeMenu();
    buildTheme();
}

void ThemeEngine::registerThemeActions(KXmlGuiWindow* const kwin)
{
    if (!d->themeMenuAction) return;
    kwin->actionCollection()->addAction("theme_menu", d->themeMenuAction);
}

void ThemeEngine::populateThemeMenu()
{
    if (!d->themeMenuAction) return;

    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangePalette(const QString&)));

    KAction* action = new KAction(i18n("Default"), d->themeMenuAction);
    action->setCheckable(true);
    d->themeMenuAction->addAction(action);

    const QStringList schemeFiles = KGlobal::dirs()->findAllResources("data", "color-schemes/*.colors", KStandardDirs::NoDuplicates);

    for (int i = 0; i < schemeFiles.size(); ++i)
    {
        // get the file name
        const QString filename = schemeFiles.at(i);
        const QFileInfo info(filename);

        // add the entry
        KSharedConfigPtr config = KSharedConfig::openConfig(filename);
        QIcon icon              = createSchemePreviewIcon(config);
        KConfigGroup group(config, "General");
        const QString name      = group.readEntry("Name", info.baseName());
        action                  = new KAction(name, d->themeMenuAction);
        d->themeMap.insert(name, filename);
        action->setIcon(icon);
        action->setCheckable(true);
        d->themeMenuAction->addAction(action);
    }
}

QPixmap ThemeEngine::createSchemePreviewIcon(const KSharedConfigPtr& config)
{
    // code taken from kdebase/workspace/kcontrol/colors/colorscm.cpp
    const uchar bits1[] = { 0xff, 0xff, 0xff, 0x2c, 0x16, 0x0b };
    const uchar bits2[] = { 0x68, 0x34, 0x1a, 0xff, 0xff, 0xff };
    const QSize bitsSize(24, 2);
    const QBitmap b1    = QBitmap::fromData(bitsSize, bits1);
    const QBitmap b2    = QBitmap::fromData(bitsSize, bits2);

    QPixmap pixmap(23, 16);
    pixmap.fill(Qt::black); // FIXME use some color other than black for borders?

    KConfigGroup group(config, "WM");
    QPainter p(&pixmap);
    KColorScheme windowScheme(QPalette::Active, KColorScheme::Window, config);
    p.fillRect(1,  1, 7, 7, windowScheme.background());
    p.fillRect(2,  2, 5, 2, QBrush(windowScheme.foreground().color(), b1));

    KColorScheme buttonScheme(QPalette::Active, KColorScheme::Button, config);
    p.fillRect(8,  1, 7, 7, buttonScheme.background());
    p.fillRect(9,  2, 5, 2, QBrush(buttonScheme.foreground().color(), b1));

    p.fillRect(15,  1, 7, 7, group.readEntry("activeBackground", QColor(96, 148, 207)));
    p.fillRect(16,  2, 5, 2, QBrush(group.readEntry("activeForeground", QColor(255, 255, 255)), b1));

    KColorScheme viewScheme(QPalette::Active, KColorScheme::View, config);
    p.fillRect(1,  8, 7, 7, viewScheme.background());
    p.fillRect(2, 12, 5, 2, QBrush(viewScheme.foreground().color(), b2));

    KColorScheme selectionScheme(QPalette::Active, KColorScheme::Selection, config);
    p.fillRect(8,  8, 7, 7, selectionScheme.background());
    p.fillRect(9, 12, 5, 2, QBrush(selectionScheme.foreground().color(), b2));

    p.fillRect(15,  8, 7, 7, group.readEntry("inactiveBackground", QColor(224, 223, 222)));
    p.fillRect(16, 12, 5, 2, QBrush(group.readEntry("inactiveForeground", QColor(20, 19, 18)), b2));

    p.end();
    return pixmap;
}

}  // namespace Digikam

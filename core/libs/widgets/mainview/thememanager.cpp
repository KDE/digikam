/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : colors theme manager
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "thememanager.h"

// Qt includes

#include <QStringList>
#include <QFileInfo>
#include <QPalette>
#include <QColor>
#include <QActionGroup>
#include <QBitmap>
#include <QPainter>
#include <QPixmap>
#include <QApplication>
#include <QAction>
#include <QStandardPaths>
#include <QDirIterator>
#include <QMenu>
#include <QStyle>

// Desktop includes

#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kconfiggroup.h>

// Local includes

#include "digikam_debug.h"
#include "dxmlguiwindow.h"
#include "schememanager.h"

namespace Digikam
{

class ThemeManagerCreator
{
public:

    ThemeManager object;
};

Q_GLOBAL_STATIC(ThemeManagerCreator, creator)

// ---------------------------------------------------------------


class ThemeManager::Private
{
public:

    Private()
        : defaultThemeName(i18nc("default theme name", "Default")),
          themeMenuActionGroup(0),
          themeMenuAction(0)
    {
    }

    const QString          defaultThemeName;
    QMap<QString, QString> themeMap;            // map<theme name, theme config path>

    QActionGroup*          themeMenuActionGroup;
    QMenu*                 themeMenuAction;
};

ThemeManager::ThemeManager()
    : d(new Private)
{
}

ThemeManager::~ThemeManager()
{
    delete d;
}

ThemeManager* ThemeManager::instance()
{
    return &creator->object;
}

QString ThemeManager::defaultThemeName() const
{
    return d->defaultThemeName;
}

QString ThemeManager::currentThemeName() const
{
    if (!d->themeMenuAction || !d->themeMenuActionGroup)
    {
        return defaultThemeName();
    }

    QAction* const action = d->themeMenuActionGroup->checkedAction();
    return (!action ? defaultThemeName()
                    : action->text().remove(QLatin1Char('&')));
}

void ThemeManager::setCurrentTheme(const QString& name)
{
    if (!d->themeMenuAction || !d->themeMenuActionGroup)
    {
        return;
    }

    QList<QAction*> list = d->themeMenuActionGroup->actions();

    foreach(QAction* const action, list)
    {
        if (action->text().remove(QLatin1Char('&')) == name)
        {
            action->setChecked(true);
            slotChangePalette();
        }
    }
}

void ThemeManager::slotChangePalette()
{
    updateCurrentDesktopDefaultThemePreview();

    QString theme(currentThemeName());

    if (theme == defaultThemeName() || theme.isEmpty())
    {
        theme = currentDesktopdefaultTheme();
    }

    QString filename        = d->themeMap.value(theme);
    KSharedConfigPtr config = KSharedConfig::openConfig(filename);
    // hint for the style to synchronize the color scheme with the window manager/compositor
    qApp->setProperty("KDE_COLOR_SCHEME_PATH", filename);
    qApp->setPalette(SchemeManager::createApplicationPalette(config));
    qApp->style()->polish(qApp);

    qCDebug(DIGIKAM_WIDGETS_LOG) << theme << " :: " << filename;

    emit signalThemeChanged();
}

void ThemeManager::setThemeMenuAction(QMenu* const action)
{
    d->themeMenuAction = action;
    populateThemeMenu();
}

void ThemeManager::registerThemeActions(DXmlGuiWindow* const win)
{
    if (!win)
    {
        return;
    }

    if (!d->themeMenuAction)
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << "Cannot register theme actions to " << win->windowTitle();
        return;
    }

    win->actionCollection()->addAction(QLatin1String("theme_menu"), d->themeMenuAction->menuAction());
}

void ThemeManager::populateThemeMenu()
{
    if (!d->themeMenuAction)
    {
        return;
    }

    QString theme(currentThemeName());

    d->themeMenuAction->clear();
    delete d->themeMenuActionGroup;

    d->themeMenuActionGroup = new QActionGroup(d->themeMenuAction);

    connect(d->themeMenuActionGroup, SIGNAL(triggered(QAction*)),
            this, SLOT(slotChangePalette()));

    QAction* const action   = new QAction(defaultThemeName(), d->themeMenuActionGroup);
    action->setCheckable(true);
    d->themeMenuAction->addAction(action);

    QStringList schemeFiles;
    QStringList dirs;

    // digiKam colors scheme
    dirs << QStandardPaths::locateAll(QStandardPaths::GenericDataLocation,
                                      QString::fromLatin1("digikam/colorschemes"),
                                      QStandardPaths::LocateDirectory);

    qCDebug(DIGIKAM_WIDGETS_LOG) << "Paths to color scheme : " << dirs;

    Q_FOREACH (const QString& dir, dirs)
    {
        QDirIterator it(dir, QStringList() << QLatin1String("*.colors"));

        while (it.hasNext())
        {
            schemeFiles.append(it.next());
        }
    }

    QMap<QString, QAction*> actionMap;

    for (int i = 0; i < schemeFiles.size(); ++i)
    {
        const QString filename  = schemeFiles.at(i);
        const QFileInfo info(filename);
        KSharedConfigPtr config = KSharedConfig::openConfig(filename);
        QIcon icon              = createSchemePreviewIcon(config);
        KConfigGroup group(config, "General");
        const QString name      = group.readEntry("Name", info.baseName());
        QAction* const ac       = new QAction(name, d->themeMenuActionGroup);
        d->themeMap.insert(name, filename);
        ac->setIcon(icon);
        ac->setCheckable(true);
        actionMap.insert(name, ac);
    }

    // sort the list
    QStringList actionMapKeys = actionMap.keys();
    actionMapKeys.sort();

    foreach(const QString& name, actionMapKeys)
    {
        d->themeMenuAction->addAction(actionMap.value(name));
    }

    updateCurrentDesktopDefaultThemePreview();
    setCurrentTheme(theme);
}

void ThemeManager::updateCurrentDesktopDefaultThemePreview()
{
    QList<QAction*> list = d->themeMenuActionGroup->actions();

    foreach(QAction* const action, list)
    {
        if (action->text().remove(QLatin1Char('&')) == defaultThemeName())
        {
            KSharedConfigPtr config = KSharedConfig::openConfig(d->themeMap.value(currentDesktopdefaultTheme()));
            QIcon icon              = createSchemePreviewIcon(config);
            action->setIcon(icon);
        }
    }
}

QPixmap ThemeManager::createSchemePreviewIcon(const KSharedConfigPtr& config) const
{
    const uchar bits1[] = { 0xff, 0xff, 0xff, 0x2c, 0x16, 0x0b };
    const uchar bits2[] = { 0x68, 0x34, 0x1a, 0xff, 0xff, 0xff };
    const QSize bitsSize(24, 2);
    const QBitmap b1    = QBitmap::fromData(bitsSize, bits1);
    const QBitmap b2    = QBitmap::fromData(bitsSize, bits2);

    QPixmap pixmap(23, 16);
    pixmap.fill(Qt::black); // FIXME use some color other than black for borders?

    KConfigGroup group(config, QLatin1String("WM"));
    QPainter p(&pixmap);
    SchemeManager windowScheme(QPalette::Active, SchemeManager::Window, config);
    p.fillRect(1,  1, 7, 7, windowScheme.background());
    p.fillRect(2,  2, 5, 2, QBrush(windowScheme.foreground().color(), b1));

    SchemeManager buttonScheme(QPalette::Active, SchemeManager::Button, config);
    p.fillRect(8,   1, 7, 7, buttonScheme.background());
    p.fillRect(9,   2, 5, 2, QBrush(buttonScheme.foreground().color(), b1));

    p.fillRect(15,  1, 7, 7, group.readEntry(QLatin1String("activeBackground"),        QColor(96, 148, 207)));
    p.fillRect(16,  2, 5, 2, QBrush(group.readEntry(QLatin1String("activeForeground"), QColor(255, 255, 255)), b1));

    SchemeManager viewScheme(QPalette::Active, SchemeManager::View, config);
    p.fillRect(1,   8, 7, 7, viewScheme.background());
    p.fillRect(2,  12, 5, 2, QBrush(viewScheme.foreground().color(), b2));

    SchemeManager selectionScheme(QPalette::Active, SchemeManager::Selection, config);
    p.fillRect(8,   8, 7, 7, selectionScheme.background());
    p.fillRect(9,  12, 5, 2, QBrush(selectionScheme.foreground().color(), b2));

    p.fillRect(15,  8, 7, 7, group.readEntry(QLatin1String("inactiveBackground"),        QColor(224, 223, 222)));
    p.fillRect(16, 12, 5, 2, QBrush(group.readEntry(QLatin1String("inactiveForeground"), QColor(20,  19,  18)), b2));

    p.end();
    return pixmap;
}

QString ThemeManager::currentDesktopdefaultTheme() const
{
    KSharedConfigPtr config = KSharedConfig::openConfig(QLatin1String("kdeglobals"));
    KConfigGroup group(config, "General");
    return group.readEntry("ColorScheme");
}

void ThemeManager::slotSettingsChanged()
{
    populateThemeMenu();
    slotChangePalette();
}

} // namespace Digikam

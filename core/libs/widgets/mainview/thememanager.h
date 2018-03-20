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

#ifndef THEME_MANAGER_H
#define THEME_MANAGER_H

// Qt includes

#include <QObject>
#include <QPixmap>
#include <QString>

// KDE includes

#include <ksharedconfig.h>

// Local includes

#include "digikam_export.h"

class QMenu;

namespace Digikam
{

class DXmlGuiWindow;
class Theme;

class DIGIKAM_EXPORT ThemeManager : public QObject
{
    Q_OBJECT

public:

    ~ThemeManager();
    static ThemeManager* instance();

    QString currentThemeName() const;
    void    setCurrentTheme(const QString& name);

    QString defaultThemeName() const;

    void    setThemeMenuAction(QMenu* const action);
    void    registerThemeActions(DXmlGuiWindow* const win);

Q_SIGNALS:

    void signalThemeChanged();

private Q_SLOTS:

    void slotChangePalette();
    void slotSettingsChanged();

private:

    ThemeManager();

    void    populateThemeMenu();
    QPixmap createSchemePreviewIcon(const KSharedConfigPtr& config) const;
    QString currentDesktopdefaultTheme() const;
    void    updateCurrentDesktopDefaultThemePreview();

private:

    friend class ThemeManagerCreator;

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // THEME_MANAGER_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-02
 * Description : theme engine methods
 *
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

#ifndef THEMEENGINE_H
#define THEMEENGINE_H

// Qt includes

#include <QObject>
#include <QColor>
#include <QPixmap>

// KDE includes

#include <ksharedconfig.h>

// Local includes

#include "digikam_export.h"

class KXmlGuiWindow;
class KSelectAction;

namespace Digikam
{

class Theme;

class DIGIKAM_EXPORT ThemeEngine : public QObject
{
    Q_OBJECT

public:

    ~ThemeEngine();
    static ThemeEngine* instance();

    QString currentThemeName() const;
    void    setCurrentTheme(const QString& name);

    QString defaultThemeName() const;

    void    setThemeMenuAction(KSelectAction* const action);
    void    registerThemeActions(KXmlGuiWindow* const kwin);

    // TODO: Methods to remove with Theme and Texture Classes
    KDE_DEPRECATED QColor  textSelColor() const;

Q_SIGNALS:

    void signalThemeChanged();

private Q_SLOTS:

    void slotChangePalette();

private:

    ThemeEngine();

    void    populateThemeMenu();
    QPixmap createSchemePreviewIcon(const KSharedConfigPtr& config);
    QString currentKDEdefaultTheme() const;
    void    updateCurrentKDEdefaultThemePreview();

private:

    friend class ThemeEngineCreator;

    class ThemeEnginePriv;
    ThemeEnginePriv* const d;
};

}  // namespace Digikam

#endif /* THEMEENGINE_H */

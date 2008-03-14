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

#ifndef THEMEENGINE_H
#define THEMEENGINE_H

// Qt includes.

#include <qstringlist.h>
#include <qobject.h>
#include <qcolor.h>
#include <qpixmap.h>
#include <qdom.h>

// Digikam includes.

#include "digikam_export.h"

namespace Digikam
{

class Theme;
class ThemeEnginePriv;

class DIGIKAM_EXPORT ThemeEngine : public QObject
{
    Q_OBJECT

public:

    ~ThemeEngine();
    static ThemeEngine* instance();

    void        scanThemes();
    QStringList themeNames() const;
    bool        saveTheme();

    void    setCurrentTheme(const QString& name);
    void    setCurrentTheme(const Theme& theme, const QString& name,
                            bool loadFromDisk=false);

    Theme*  getCurrentTheme() const;
    QString getCurrentThemeName() const;

    QColor  baseColor()     const;
    QColor  thumbSelColor() const;
    QColor  thumbRegColor() const;

    QColor  textRegColor() const;
    QColor  textSelColor() const;
    QColor  textSpecialRegColor() const;
    QColor  textSpecialSelColor() const;

    QPixmap bannerPixmap(int w, int h);
    QPixmap thumbRegPixmap(int w, int h);
    QPixmap thumbSelPixmap(int w, int h);
    QPixmap listRegPixmap(int w, int h);
    QPixmap listSelPixmap(int w, int h);

private:

    ThemeEngine();
    static ThemeEngine* m_instance;

    void    buildDefaultTheme();
    bool    loadTheme();
    void    changePalette();
    QString resourceValue(const QDomElement &rootElem, const QString& key);

signals:

    void signalThemeChanged();

public slots:

    void slotChangeTheme(const QString& name);

private:

    ThemeEnginePriv* d;
};

}  // NameSpace Digikam

#endif /* THEMEENGINE_H */

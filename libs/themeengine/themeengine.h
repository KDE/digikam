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

#ifndef THEMEENGINE_H
#define THEMEENGINE_H

#include <qstringlist.h>
#include <qobject.h>
#include <qcolor.h>
#include <qpixmap.h>

class Theme;
class ThemeEnginePriv;

class ThemeEngine : public QObject
{
    Q_OBJECT
    
public:

    ~ThemeEngine();
    static ThemeEngine* instance();

    void        scanThemes();
    QStringList themeNames() const;
    
    void    setCurrentTheme(const QString& name);
    Theme*  getCurrentTheme();

    QColor  baseColor()    const;
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
    void    loadTheme();
    QString resourceValue(const QString& name, const QString& altName);
    
    ThemeEnginePriv* d;

signals:

    void signalThemeChanged();

public slots:

    void slotChangeTheme(const QString& name);
};

#endif /* THEMEENGINE_H */

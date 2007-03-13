/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Gilles Caulier <caulier dot gilles at gmail dot com> 
 * Date   : 2003-02-10
 * Description : a widget to display spash with progress bar
 *
 * Copyright 2003-2005 by Renchi Raju
 * Copyright 2006-2007 by Gilles Caulier
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

#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

// Qt includes.

#include <qwidget.h>
#include <qstring.h>

// Local Includes.

#include "digikam_export.h"

namespace Digikam
{

class SplashScreenPriv;

class DIGIKAM_EXPORT SplashScreen : public QWidget
{
    Q_OBJECT

public:

    SplashScreen(const QString& splash);
    ~SplashScreen();

    void finish( QWidget *mainWin );
    void repaint();
    void message(const QString &message, int alignment = AlignLeft,
                 const QColor &color = white );

protected:

    void mousePressEvent( QMouseEvent * );
    void drawContents();
    void drawContents(QPainter *painter);
    void animate();
        
private slots:

    void slotClose();

private:

    SplashScreenPriv* d;
};

}   // namespace Digikam

#endif /* SPLASHSCREEN_H */

/* ============================================================
 * File  : splashscreen.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
 * Description :
 *
 * Copyright 2003 by Renchi Raju

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

#include <qwidget.h>

class QPixmap;
class QTimer;

class SplashScreen : public QWidget
{
    Q_OBJECT

public:

    SplashScreen();
    ~SplashScreen();

    void setStatus(const QString &message, int alignment = AlignLeft,
                   const QColor &color = white );
    void finish( QWidget *mainWin );
    void repaint();

protected:

    void mousePressEvent( QMouseEvent * );

private:

    QPixmap *pix_;
    QTimer  *timer_;
    bool     close_;

private slots:

    void slotClose();

};

#endif /* SPLASHSCREEN_H */

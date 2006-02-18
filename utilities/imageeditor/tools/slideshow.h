/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at kdemail dot net>
 * Date  : 2005-04-21
 * Description : slide show tool
 * 
 * Copyright 2005-2006 by Gilles Caulier
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

#ifndef SLIDESHOW_H
#define SLIDESHOW_H

// Qt includes.

#include <qobject.h>

class QTimer;

class KAction;
class KConfig;

namespace Digikam
{

class SlideShowPriv;

class SlideShow : public QObject
{
Q_OBJECT

public:
    
    SlideShow(KAction* first, KAction* next);
    ~SlideShow();

    void setStartWithCurrent(bool);
    void setLoop(bool);
    void setDelay(int);

    bool startWithCurrent() const;
    bool loop() const;
    int  delay() const;
    
    void start();
    void stop();

signals:
    
    void finished();

private slots:
    
    void slotTimeout();

private:

    SlideShowPriv* d;

};

}   // namespace Digikam

#endif // SLIDESHOW_H

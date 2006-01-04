/* ============================================================
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-04-21
 * Description : slideshow for showfoto
 * 
 * Copyright 2005 by Gilles Caulier
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

#include <qobject.h>

class QTimer;

class KAction;
class KConfig;

namespace ShowFoto
{

class SlideShow : public QObject
{
Q_OBJECT

public:
    
    SlideShow(KAction* first,KAction* next);
    
    void setStartWithCurrent(bool);
    void setLoop(bool);
    void setDelay(int);

    bool startWithCurrent() const { return m_startWithCurrent; }
    bool loop()             const { return m_loop;             }
    int  delay()            const { return m_delay;            }
    
    void start();
    void stop();

signals:
    
    void finished();

private slots:
    
    void slotTimeout();

private:
    
    QTimer  *m_timer;
    
    KAction *m_first;
    KAction *m_next;
    
    int      m_delay;
    
    bool     m_loop;
    bool     m_startWithCurrent;
};

}   // namespace ShowFoto

#endif // SLIDESHOW_H

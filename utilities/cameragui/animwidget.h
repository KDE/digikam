/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-21
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

#ifndef ANIMWIDGET_H
#define ANIMWIDGET_H

// Qt includes.

#include <qwidget.h>

class QTimer;
class QPixmap;

namespace Digikam
{

class AnimWidget : public QWidget
{
    Q_OBJECT
    
public:

    AnimWidget(QWidget* parent, int size=28);
    ~AnimWidget();

    void start();
    void stop();
    bool running() const;

protected:

    void paintEvent(QPaintEvent*);

private slots:

    void slotTimeout();
    
private:

    QTimer*  m_timer;
    QPixmap* m_pix;
    int      m_pos;
    int      m_size;
};

}  // namespace Digikam

#endif /* ANIMWIDGET_H */

/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-08-31
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

#ifndef BUSYPROGRESSBAR_H
#define BUSYPROGRESSBAR_H

#include <qprogressbar.h>

class QPainter;
class QTimer;
class QPixmap;

class BusyProgressBar : public QProgressBar
{
    Q_OBJECT

public:

    BusyProgressBar(QWidget* parent);
    ~BusyProgressBar();

    void styleChange(QStyle& old);
    
protected:

    void drawContents(QPainter* p);
    void resizeEvent(QResizeEvent* e);

private:

    QTimer*  m_timer;
    int      m_pos;
    int      m_dir;
    QPixmap* m_pix;

private slots:

    void slotMove();
};
    
#endif /* BUSYPROGRESSBAR_H */

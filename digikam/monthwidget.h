/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-02
 * Copyright 2005 by Renchi Raju
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

#ifndef MONTHWIDGET_H
#define MONTHWIDGET_H

// Qt includes.

#include <qframe.h>

// Local includes.

#include "imageinfo.h"

namespace Digikam
{

class MonthWidget : public QFrame
{
    Q_OBJECT

public:

    MonthWidget(QWidget* parent);
    ~MonthWidget();

    void setYearMonth(int year, int month);
    QSize sizeHint() const;

    void setActive(bool val);
    
protected:
    
    void resizeEvent(QResizeEvent *e);
    void drawContents(QPainter *p);
    void mousePressEvent(QMouseEvent *e);

private:

    void init();

private slots:

    void slotAddItems(const ImageInfoList& items);
    void slotDeleteItem(ImageInfo* item);
    
private:
    
    struct Month
    {
        bool active;
        bool selected;
        int  day;
        int  numImages;
    };

    bool         m_active;
    int          m_year;
    int          m_month;
    int          m_w;
    int          m_h;
    int          m_currw;
    int          m_currh;
    struct Month m_days[42];
};

}  // namespace Digikam

#endif /* MONTHWIDGET_H */

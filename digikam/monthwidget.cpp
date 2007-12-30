/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-02
 * Description : a widget to perform month selection.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qdatetime.h>
#include <qfontmetrics.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qpalette.h>

// KDE includes.

#include <klocale.h>
#include <kglobal.h>
#include <kcalendarsystem.h>
#include <kdeversion.h>

// Local includes.

#include "imageinfo.h"
#include "albumlister.h"
#include "monthwidget.h"
#include "monthwidget.moc"

namespace Digikam
{

class MonthWidgetPriv
{
public:

    struct Month
    {
        bool active;
        bool selected;

        int  day;
        int  numImages;
    };

    MonthWidgetPriv()
    {
        active = true;
    }

    bool         active;

    int          year;
    int          month;
    int          width;
    int          height;
    int          currw;
    int          currh;

    struct Month days[42];
};

MonthWidget::MonthWidget(QWidget* parent)
           : QFrame(parent, 0, Qt::WNoAutoErase)
{
    d = new MonthWidgetPriv;
    init();

    QDate date = QDate::currentDate();
    setYearMonth(date.year(), date.month());

    setActive(false);
}

MonthWidget::~MonthWidget()
{
    delete d;
}

void MonthWidget::init()
{
    QFont fn(font());
    fn.setBold(true);
    fn.setPointSize(fn.pointSize()+1);
    QFontMetrics fm(fn);
    QRect r(fm.boundingRect("XX"));
    r.setWidth(r.width() + 2);
    r.setHeight(r.height() + 4);
    d->width  = r.width();
    d->height = r.height();

    setMinimumWidth(d->width * 8);
    setMinimumHeight(d->height * 9);
}

void MonthWidget::setYearMonth(int year, int month)
{
    d->year  = year;
    d->month = month;

    for (int i=0; i<42; i++)
    {
        d->days[i].active    = false;
        d->days[i].selected  = false;
        d->days[i].day       = -1;
        d->days[i].numImages = 0;
    }

    QDate date(year, month, 1);
    int s = date.dayOfWeek();

    for (int i=s; i<(s+date.daysInMonth()); i++)
    {
        d->days[i-1].day = i-s+1;
    }

    update();
}

QSize MonthWidget::sizeHint() const
{
    return QSize(d->width * 8, d->height * 9);
}

void MonthWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    d->currw = contentsRect().width()/8;
    d->currh = contentsRect().height()/9;
}

void MonthWidget::drawContents(QPainter *)
{
    QRect cr(contentsRect());

    QPixmap pix(cr.width(), cr.height());

    QColorGroup cg = colorGroup();

    QFont fnBold(font());
    QFont fnOrig(font());
    fnBold.setBold(true);
    fnOrig.setBold(false);

    QPainter p(&pix);
    p.fillRect(0, 0, cr.width(), cr.height(), cg.background());

    QRect r(0, 0, d->currw, d->currh);
    QRect rsmall;

    int sx, sy;
    int index = 0;
    bool weekvisible;
    for (int j=3; j<9; j++)
    {
        sy = d->currh * j;
        weekvisible = false;

        for (int i=1; i<8; i++)
        {
            sx = d->currw * i;
            r.moveTopLeft(QPoint(sx,sy));
            rsmall = QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
            if (d->days[index].day != -1)
            {
                if (d->days[index].selected)
                {
                    p.fillRect(r, cg.highlight());
                    p.setPen(cg.highlightedText());

                    if (d->days[index].active)
                    {
                        p.setFont(fnBold);
                    }
                    else
                    {
                        p.setFont(fnOrig);
                    }
                }
                else
                {
                    if (d->days[index].active)
                    {
                        p.setPen(cg.text());
                        p.setFont(fnBold);
                    }
                    else
                    {
                        p.setPen(cg.mid());
                        p.setFont(fnOrig);
                    }
                }

                p.drawText(rsmall, Qt::AlignVCenter|Qt::AlignHCenter,
                           QString::number(d->days[index].day));

                if(!weekvisible)
                {
                    int weeknr = KGlobal::locale()->calendar()->weekNumber(QDate(d->year, 
                                                                           d->month, d->days[index].day));
                    p.setPen(d->active ? Qt::black : Qt::gray);
                    p.setFont(fnBold);
                    p.fillRect(1, sy, d->currw-1, d->currh-1, QColor(210,210,210));
                    p.drawText(1, sy, d->currw-1, d->currh-1, Qt::AlignVCenter|Qt::AlignHCenter,
                               QString::number(weeknr));
                    weekvisible = true;
                }

            }

            index++;
        }
    }

    p.setPen(d->active ? Qt::black : Qt::gray);

    p.setFont(fnBold);

    sy = 2*d->currh;
    for (int i=1; i<8; i++)
    {
        sx = d->currw * i;
        r.moveTopLeft(QPoint(sx+1,sy+1));
        rsmall = r;
        rsmall.setWidth(r.width() - 2);
        rsmall.setHeight(r.height() - 2);
#if KDE_IS_VERSION(3,2,0)
        p.drawText(rsmall, Qt::AlignVCenter|Qt::AlignHCenter,
                           KGlobal::locale()->calendar()->weekDayName(i, true)
                           .remove(2,1));
#else
        p.drawText(rsmall, Qt::AlignVCenter|Qt::AlignHCenter,
                           KGlobal::locale()->weekDayName(i, true).remove(2,1));
#endif
        index++;
    }

    r = QRect(0, 0, cr.width(), 2*d->currh);

    fnBold.setPointSize(fnBold.pointSize()+2);
    p.setFont(fnBold);

#if KDE_IS_VERSION(3,2,0)
    p.drawText(r, Qt::AlignCenter,
               QString("%1 %2")
                .arg(KGlobal::locale()->calendar()->monthName(d->month, false))
                .arg(KGlobal::locale()->calendar()->year(QDate(d->year,d->month,1))));
#else
    p.drawText(r, Qt::AlignCenter,
               QString("%1 %2")
                .arg(KGlobal::locale()->monthName(d->month))
                .arg(QString::number(d->year)));
#endif

    p.end();

    bitBlt(this, cr.x(), cr.y(), &pix);
}

void MonthWidget::mousePressEvent(QMouseEvent *e)
{
    int firstSelected = 0, lastSelected = 0;
    if (e->state() != Qt::ControlButton)
    {
        for (int i=0; i<42; i++)
        {
            if (d->days[i].selected)
            {
                if (firstSelected==0)
                    firstSelected = i;
                lastSelected =i;
            }
            d->days[i].selected = false;
        }
    }

    QRect r1(0, d->currh*3, d->currw, d->currh*6);
    QRect r2(d->currw, d->currh*3, d->currw*7, d->currh*6);
    QRect r3(d->currw, d->currh*2, d->currw*7, d->currh);

    // Click on a weekday
    if( r3.contains(e->pos()))
    {
        int j = (e->pos().x() - d->currw)/d->currw;
        for (int i=0; i<6; i++)
        {
            d->days[i*7+j].selected = !d->days[i*7+j].selected;
        }
    }
    // Click on a week
    else if (r1.contains(e->pos()))
    {
        int j = (e->pos().y() - 3*d->currh)/d->currh;
        for (int i=0; i<7; i++)
        {
            d->days[j*7+i].selected = !d->days[j*7+i].selected;
        }
    }
    // Click on a day.
    else if (r2.contains(e->pos()))
    {
        int i, j;
        i = (e->pos().x() - d->currw)/d->currw;
        j = (e->pos().y() - 3*d->currh)/d->currh;
        if (e->state() == Qt::ShiftButton)
        {
            int endSelection = j*7+i;
            if (endSelection > firstSelected) 
                for (int i2=firstSelected ; i2 <= endSelection; i2++)
                    d->days[i2].selected = true;
            else if (endSelection < firstSelected)
                for (int i2=lastSelected ; i2 >= endSelection; i2--)
                    d->days[i2].selected = true;
        }
        else
            d->days[j*7+i].selected = !d->days[j*7+i].selected;
    }

    QValueList<QDateTime> filterDays;
    for (int i=0; i<42; i++)
    {
        if (d->days[i].selected && d->days[i].day != -1)
            filterDays.append(QDateTime(QDate(d->year, d->month, d->days[i].day), QTime()));
    }

    AlbumLister::instance()->setDayFilter(filterDays);

    update();
}

void MonthWidget::setActive(bool val)
{
    if (d->active == val)
        return;

    d->active = val;

    if (d->active)
    {
        connect(AlbumLister::instance(), SIGNAL(signalNewItems(const ImageInfoList&)),
                this, SLOT(slotAddItems(const ImageInfoList&)));

        connect(AlbumLister::instance(), SIGNAL(signalDeleteItem(ImageInfo*)),
                this, SLOT(slotDeleteItem(ImageInfo*)));
    }
    else
    {
        QDate date = QDate::currentDate();
        setYearMonth(date.year(), date.month());
        AlbumLister::instance()->setDayFilter(QValueList<QDateTime>());

        disconnect(AlbumLister::instance());
    }
}

void MonthWidget::slotAddItems(const ImageInfoList& items)
{
    if (!d->active)
        return;

    ImageInfo* item;
    for (ImageInfoListIterator it(items); (item = it.current()); ++it)
    {
        QDateTime dt = item->dateTime();

        for (int i=0; i<42; i++)
        {
            if (d->days[i].day == dt.date().day())
            {
                d->days[i].active = true;
                d->days[i].numImages++;
                break;
            }
        }
    }

    update();
}

void MonthWidget::slotDeleteItem(ImageInfo* item)
{
    if (!d->active || !item)
        return;

    QDateTime dt = item->dateTime();

    for (int i=0; i<42; i++)
    {
        if (d->days[i].day == dt.date().day())
        {
            d->days[i].numImages--;
            if (d->days[i].numImages <= 0)
            {
                d->days[i].active = false;
                d->days[i].numImages = 0;
            }

            break;
        }
    }

    update();
}

}  // namespace Digikam

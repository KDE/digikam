/* ============================================================
 * File  : monthwidget.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-02
 * Description : 
 * 
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

namespace Digikam
{

MonthWidget::MonthWidget(QWidget* parent)
    : QFrame(parent, 0, Qt::WNoAutoErase)
{
    init();

    QDate date = QDate::currentDate();
    setYearMonth(date.year(), date.month());

    m_active = true;
    setActive(false);
}

MonthWidget::~MonthWidget()
{
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
    m_w = r.width();
    m_h = r.height();

    setMinimumWidth(m_w * 8);
    setMinimumHeight(m_h * 9);
}

void MonthWidget::setYearMonth(int year, int month)
{
    m_year  = year;
    m_month = month;

    for (int i=0; i<42; i++)
    {
        m_days[i].active   = false;
        m_days[i].selected = false;
        m_days[i].day      = -1;
        m_days[i].numImages = 0;
    }

    QDate d(year, month, 1);
    int s = d.dayOfWeek();

    for (int i=s; i<(s+d.daysInMonth()); i++)
    {
        m_days[i-1].day = i-s+1;
    }                           
    
    update();
}

QSize MonthWidget::sizeHint() const
{
    return QSize(m_w * 8, m_h * 9);    
}

void MonthWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);

    m_currw = contentsRect().width()/8;
    m_currh = contentsRect().height()/9;
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

    QRect r(0, 0, m_currw, m_currh);
    QRect rsmall;
    
    int sx, sy;
    int index = 0;
    bool weekvisible;
    for (int j=3; j<9; j++)
    {
        sy = m_currh * j;
        weekvisible = false;
      
        for (int i=1; i<8; i++)
        {
            sx = m_currw * i;
            r.moveTopLeft(QPoint(sx,sy));
            rsmall = QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
            if (m_days[index].day != -1)
            {
                if (m_days[index].selected)
                {
                    p.fillRect(r, cg.highlight());
                    p.setPen(cg.highlightedText());

                    if (m_days[index].active)
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
                    if (m_days[index].active)
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
                           QString::number(m_days[index].day));
                           
                if(!weekvisible)
                {
                    int weeknr = KGlobal::locale()->calendar()->
                                    weekNumber(QDate(m_year, m_month, m_days[index].day));
                    p.setPen(m_active ? Qt::black : Qt::gray);
                    p.setFont(fnBold);
                    p.fillRect(1, sy, m_currw-1, m_currh-1, QColor(210,210,210));
                    p.drawText(1, sy, m_currw-1, m_currh-1, Qt::AlignVCenter|Qt::AlignHCenter,
                               QString::number(weeknr));
                    weekvisible = true;
                }

            }

            index++;                                             
        }
    }

    p.setPen(m_active ? Qt::black : Qt::gray);

    p.setFont(fnBold);
    
    sy = 2*m_currh;
    for (int i=1; i<8; i++)
    {
        sx = m_currw * i;
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
    
    r = QRect(0, 0, cr.width(), 2*m_currh);

    fnBold.setPointSize(fnBold.pointSize()+2);
    p.setFont(fnBold);

#if KDE_IS_VERSION(3,2,0)
    p.drawText(r, Qt::AlignCenter,
               QString("%1 %2")
                .arg(KGlobal::locale()->calendar()->monthName(m_month, false))
                .arg(KGlobal::locale()->calendar()->
                       year(QDate(m_year,m_month,1))));
#else
    p.drawText(r, Qt::AlignCenter,
               QString("%1 %2")
                .arg(KGlobal::locale()->monthName(m_month))
                .arg(QString::number(m_year)));
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
            if (m_days[i].selected)
            {
                if (firstSelected==0)
                    firstSelected = i;
                lastSelected =i;
            }
            m_days[i].selected = false;
        }
    }

    QRect r1(0, m_currh*3, m_currw, m_currh*6);
    QRect r2(m_currw, m_currh*3, m_currw*7, m_currh*6);
    QRect r3(m_currw, m_currh*2, m_currw*7, m_currh);

    // Click on a weekday
    if( r3.contains(e->pos()))
    {
        int j = (e->pos().x() - m_currw)/m_currw;
        for (int i=0; i<6; i++)
        {
            m_days[i*7+j].selected = !m_days[i*7+j].selected;
        }
    }
    // Click on a week
    else if (r1.contains(e->pos()))
    {
        int j = (e->pos().y() - 3*m_currh)/m_currh;
        for (int i=0; i<7; i++)
        {
            m_days[j*7+i].selected = !m_days[j*7+i].selected;
        }
    }
    // Click on a day.
    else if (r2.contains(e->pos()))
    {
        int i, j;
        i = (e->pos().x() - m_currw)/m_currw;
        j = (e->pos().y() - 3*m_currh)/m_currh;
        if (e->state() == Qt::ShiftButton)
        {
            int endSelection = j*7+i;
            if (endSelection > firstSelected) 
                for (int i2=firstSelected ; i2 <= endSelection; i2++)
                    m_days[i2].selected = true;
            else if (endSelection < firstSelected)
                for (int i2=lastSelected ; i2 >= endSelection; i2--)
                    m_days[i2].selected = true;
        }
        else
            m_days[j*7+i].selected = !m_days[j*7+i].selected;
    }

    QValueList<int> filterDays;
    for (int i=0; i<42; i++)
    {
        if (m_days[i].selected && m_days[i].day != -1)
            filterDays.append(m_days[i].day);
    }

    AlbumLister::instance()->setDayFilter(filterDays);
    
    update();
}

void MonthWidget::setActive(bool val)
{
    if (m_active == val)
        return;

    m_active = val;
    
    if (m_active)
    {
        connect(AlbumLister::instance(),
                SIGNAL(signalNewItems(const ImageInfoList&)),
                SLOT(slotAddItems(const ImageInfoList&)));
        connect(AlbumLister::instance(),
                SIGNAL(signalDeleteItem(ImageInfo*)),
                SLOT(slotDeleteItem(ImageInfo*)));
    }
    else
    {
        QDate date = QDate::currentDate();
        setYearMonth(date.year(), date.month());
        AlbumLister::instance()->setDayFilter(QValueList<int>());

        disconnect(AlbumLister::instance());
    }
}

void MonthWidget::slotAddItems(const ImageInfoList& items)
{
    if (!m_active)
        return;
    
    ImageInfo* item;
    for (ImageInfoListIterator it(items); (item = it.current()); ++it)
    {
        QDateTime dt = item->dateTime();

        for (int i=0; i<42; i++)
        {
            if (m_days[i].day == dt.date().day())
            {
                m_days[i].active = true;
                m_days[i].numImages++;
                break;
            }
        }
    }

    update();
}

void MonthWidget::slotDeleteItem(ImageInfo* item)
{
    if (!m_active || !item)
        return;

    QDateTime dt = item->dateTime();

    for (int i=0; i<42; i++)
    {
        if (m_days[i].day == dt.date().day())
        {
            m_days[i].numImages--;
            if (m_days[i].numImages <= 0)
            {
                m_days[i].active = false;
                m_days[i].numImages = 0;
            }
            
            break;
        }
    }
    
    update();
}

}  // namespace Digikam

#include "monthwidget.moc"

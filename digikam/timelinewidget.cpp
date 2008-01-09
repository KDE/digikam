/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a widget to display date and time statistics of pictures
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <qpainter.h>
#include <qpixmap.h>
#include <qpen.h>

// KDE include.

#include <kcursor.h>
#include <klocale.h>
#include <kglobal.h>
#include <kcalendarsystem.h>

// Local includes.

#include "ddebug.h"
#include "timelinewidget.h"
#include "timelinewidget.moc"

namespace Digikam
{

class TimeLineWidgetPriv
{

public :

    typedef QPair<int, int>  YearRefPair;   // Year + a reference association (Month or week or day)
    typedef QPair<int, bool> StatPair;      // Statistic value + selection status.

public:

    TimeLineWidgetPriv()
    {
        validMouseEvent = false;
        selMouseEvent   = false;
        maxCountByDay   = 1;
        maxCountByWeek  = 1;
        maxCountByMonth = 1;
        maxCountByYear  = 1;
        topMargin       = 3;
        bottomMargin    = 20;
        barWidth        = 20;
        startPos        = 96;
        nbItems         = 10;
        dateMode        = TimeLineWidget::Month;
        scaleMode       = TimeLineWidget::LinScale;
    }

    bool                        validMouseEvent;   // Current mouse enter event is valid to set cursor position or selection.
    bool                        selMouseEvent;     // Current mouse enter event is about to make a selection.

    int                         maxCountByDay;
    int                         maxCountByWeek;
    int                         maxCountByMonth;
    int                         maxCountByYear;
    int                         topMargin; 
    int                         bottomMargin; 
    int                         barWidth; 
    int                         nbItems;
    int                         startPos;

    QDateTime                   refDateTime;       // Reference date-time used to draw histogram from middle of widget.
    QDateTime                   selDateTime;       // Current date-time used to draw focus cursor.
    QDateTime                   minDateTime;       // Higher date on histogram.
    QDateTime                   maxDateTime;       // Lower date on histogram.
    QDateTime                   selStartDateTime;
    QDateTime                   selMinDateTime;
    QDateTime                   selMaxDateTime;

    QPixmap                     pixmap;

    QMap<YearRefPair, StatPair> dayStatMap;
    QMap<YearRefPair, StatPair> weekStatMap;
    QMap<YearRefPair, StatPair> monthStatMap;
    QMap<int,         StatPair> yearStatMap;

    TimeLineWidget::DateMode    dateMode;
    TimeLineWidget::ScaleMode   scaleMode;
};

TimeLineWidget::TimeLineWidget(QWidget *parent)
              : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineWidgetPriv;
    setBackgroundMode(Qt::NoBackground);
    setMouseTracking(true);
    setMinimumWidth(256);
    setMinimumHeight(192);

    QDateTime ref = QDateTime::currentDateTime();
    setCurrentDateTime(ref);
    setRefDateTime(ref);
}

TimeLineWidget::~TimeLineWidget()
{
    delete d;
}

void TimeLineWidget::setDateMode(DateMode dateMode)
{
    d->dateMode   = dateMode;
    QDateTime ref = currentDateTime();
    setCurrentDateTime(ref);
    setRefDateTime(ref);
    updatePixmap();
    update();
}

TimeLineWidget::DateMode TimeLineWidget::dateMode() const
{
    return d->dateMode;
}

void TimeLineWidget::setScaleMode(ScaleMode scaleMode)
{
    d->scaleMode = scaleMode;
    updatePixmap();
    update();
}

TimeLineWidget::ScaleMode TimeLineWidget::scaleMode() const
{
    return d->scaleMode;
}

void TimeLineWidget::setCurrentDateTime(const QDateTime& dateTime)
{
    QDateTime dt = dateTime;
    dt.setTime(QTime());

    switch(d->dateMode)
    {
        case Week:
        {
            // Go to the first day of week.
            int dayWeekOffset = (-1) * (KGlobal::locale()->calendar()->dayOfWeek(dt.date()) - 1);
            dt = dt.addDays(dayWeekOffset);
            break;
        }
        case Month:
        {
            // Go to the first day of month.
            dt.setDate(QDate(dt.date().year(), dt.date().month(), 1));
            break;
        }
        case Year:
        {
            // Go to the first day of year.
            dt.setDate(QDate(dt.date().year(), 1, 1));
            break;
        }
        default:
            break;
    }

    if (d->selDateTime == dt)
        return;

    d->selDateTime = dt;
    updatePixmap();
    update();

    emit signalCursorPositionChanged();
}

QDateTime TimeLineWidget::currentDateTime() const
{
    return d->selDateTime;
}

void TimeLineWidget::setRefDateTime(const QDateTime& dateTime)
{
    QDateTime dt = dateTime;
    dt.setTime(QTime());

    switch(d->dateMode)
    {
        case Week:
        {
            // Go to the first day of week.
            int dayWeekOffset = (-1) * (KGlobal::locale()->calendar()->dayOfWeek(dt.date()) - 1);
            dt = dt.addDays(dayWeekOffset);
            break;
        }
        case Month:
        {
            // Go to the first day of month.
            dt.setDate(QDate(dt.date().year(), dt.date().month(), 1));
            break;
        }
        case Year:
        {
            // Go to the first day of year.
            dt.setDate(QDate(dt.date().year(), 1, 1));
            break;
        }
        default:
            break;
    }

    d->refDateTime = dt;
    updatePixmap();
    update();
}

int TimeLineWidget::cursorInfo(QDateTime& start, QDateTime& end)
{
    bool selected;
    start = currentDateTime();
    end   = nextDateTime(start);
    return statForDateTime(start, selected);
}

void TimeLineWidget::resetSelection()
{
    resetSelection(d->dateMode);
    updatePixmap();
    update();
}

void TimeLineWidget::resetAllSelection()
{
    for (int i=(int)Day ; i<=(int)Year ; i++)
        resetSelection((TimeLineWidget::DateMode)i);

    updatePixmap();
    update();
}

void TimeLineWidget::resetSelection(TimeLineWidget::DateMode mode)
{
    switch(mode)
    {
        case Day:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->dayStatMap.begin() ; it != d->dayStatMap.end(); ++it)
                it.data().second = false;

            break;
        }
        case Week:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->weekStatMap.begin() ; it != d->weekStatMap.end(); ++it)
                it.data().second = false;

            break;
        }
        case Month:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->monthStatMap.begin() ; it != d->monthStatMap.end(); ++it)
                it.data().second = false;

            break;
        }
        case Year:
        {
            QMap<int, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->yearStatMap.begin() ; it != d->yearStatMap.end(); ++it)
                it.data().second = false;

            break;
        }
    }
}
DateRangeList TimeLineWidget::currentSelectedDateRange(int& totalCount)
{
    totalCount = 0;
    DateRangeList list;

    switch(d->dateMode)
    {
        case Day:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->dayStatMap.begin() ; it != d->dayStatMap.end(); ++it)
            {
                if (it.data().second)  // Selected ?
                {
                    QDate date(it.key().first, 1, 1);
                    date = date.addDays(it.key().second-1);
                    QDateTime sdt(date);
                    QDateTime edt = nextDateTime(sdt); 
                    list.append(DateRange(sdt, edt));
                    totalCount += it.data().first;
                }
            }
            break;
        }
        case Week:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->weekStatMap.begin() ; it != d->weekStatMap.end(); ++it)
            {
                if (it.data().second)  // Selected ?
                {
                    QDateTime sdt = firstDayOfWeek(it.key().first, it.key().second);
                    QDateTime edt = nextDateTime(sdt); 
                    list.append(DateRange(sdt, edt));
                    totalCount += it.data().first;
                }
            }
            break;
        }
        case Month:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->monthStatMap.begin() ; it != d->monthStatMap.end(); ++it)
            {
                if (it.data().second)  // Selected ?
                {
                    QDate date(it.key().first, it.key().second, 1);
                    QDateTime sdt(date);
                    QDateTime edt = nextDateTime(sdt); 
                    list.append(DateRange(sdt, edt));
                    totalCount += it.data().first;
                }
            }
            break;
        }
        case Year:
        {
            QMap<int, TimeLineWidgetPriv::StatPair>::iterator it;

            for (it = d->yearStatMap.begin() ; it != d->yearStatMap.end(); ++it)
            {
                if (it.data().second)  // Selected ?
                {
                    QDate date(it.key(), 1, 1);
                    QDateTime sdt(date);
                    QDateTime edt = nextDateTime(sdt); 
                    list.append(DateRange(sdt, edt));
                    totalCount += it.data().first;
                }
            }
            break;
        }
    }

    DateRangeList::iterator it, it2;
    for (it = list.begin() ; it != list.end(); ++it)
        DDebug() << (*it).first.date().toString(Qt::ISODate) << " :: " 
                 << (*it).second.date().toString(Qt::ISODate) << endl;

    DDebug() << "Total Count of Items = " << totalCount << endl;

    // Group contiguous date ranges to optimize query to database.

    DateRangeList list2;
    QDateTime     first, second;

    for (it = list.begin() ; it != list.end(); ++it)
    { 
        first  = (*it).first;
        second = (*it).second;
        it2 = it;
        do
        {
            ++it2;
            if ((*it2).first == second)
            {
                second = (*it2).second;
                ++it;
            }
            else 
                break;
        }
        while(it2 != list.end());

        list2.append(DateRange(first, second));
    }

    for (it = list2.begin() ; it != list2.end(); ++it)
        DDebug() << (*it).first.date().toString(Qt::ISODate) << " :: " 
                 << (*it).second.date().toString(Qt::ISODate) << endl;

    return list2;
}

void TimeLineWidget::slotDatesMap(const QMap<QDateTime, int>& datesStatMap)
{
    // Clear all counts in all stats maps before to update it. Do not clear selections.

    QMap<int, TimeLineWidgetPriv::StatPair>::iterator it_iP;
    for ( it_iP = d->yearStatMap.begin() ; it_iP != d->yearStatMap.end(); ++it_iP )
        it_iP.data().first = 0;

    QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it_YP;
    for ( it_YP = d->monthStatMap.begin() ; it_YP != d->monthStatMap.end(); ++it_YP )
        it_YP.data().first = 0;

    for ( it_YP = d->weekStatMap.begin() ; it_YP != d->weekStatMap.end(); ++it_YP )
        it_YP.data().first = 0;

    for ( it_YP = d->dayStatMap.begin() ; it_YP != d->dayStatMap.end(); ++it_YP )
        it_YP.data().first = 0;

    // Parse all new Date stamp and store histogram stats relevant in maps.

    int count;
    QMap<QDateTime, int>::const_iterator it;
    d->minDateTime = datesStatMap.begin().key();
    d->maxDateTime = datesStatMap.begin().key();

    for ( it = datesStatMap.begin(); it != datesStatMap.end(); ++it )
    {
        if (it.key() > d->maxDateTime)
            d->maxDateTime = it.key();

        if (it.key() < d->minDateTime)
            d->minDateTime = it.key();

        int year  = it.key().date().year();
        int month = it.key().date().month();
        int day   = KGlobal::locale()->calendar()->dayOfYear(it.key().date());
        int week  = KGlobal::locale()->calendar()->weekNumber(it.key().date());

        // Stats Years values.

        it_iP = d->yearStatMap.find(year);
        if ( it_iP == d->yearStatMap.end() )
        {
            count = it.data();
            d->yearStatMap.insert( year, TimeLineWidgetPriv::StatPair(count, false) );
        }
        else
        {
            count = it_iP.data().first + it.data();
            d->yearStatMap.replace( year, TimeLineWidgetPriv::StatPair(count, it_iP.data().second) );
        }

        if (d->maxCountByYear < count) 
            d->maxCountByYear = count;

        // Stats Months values.

        it_YP = d->monthStatMap.find(TimeLineWidgetPriv::YearRefPair(year, month));
        if ( it_YP == d->monthStatMap.end() )
        {
            count = it.data();
            d->monthStatMap.insert( TimeLineWidgetPriv::YearRefPair(year, month), 
                                    TimeLineWidgetPriv::StatPair(count, false) );
        }
        else
        {
            count = it_YP.data().first + it.data();
            d->monthStatMap.replace( TimeLineWidgetPriv::YearRefPair(year, month), 
                                     TimeLineWidgetPriv::StatPair(count, it_YP.data().second) );
        }

        if (d->maxCountByMonth < count) 
            d->maxCountByMonth = count;

        // Stats Weeks values.

        it_YP = d->weekStatMap.find(TimeLineWidgetPriv::YearRefPair(year, week));
        if ( it_YP == d->weekStatMap.end() )
        {
            count = it.data();
            d->weekStatMap.insert( TimeLineWidgetPriv::YearRefPair(year, week), 
                                   TimeLineWidgetPriv::StatPair(count, false) );
        }
        else
        {
            count = it_YP.data().first + it.data();
            d->weekStatMap.replace( TimeLineWidgetPriv::YearRefPair(year, week), 
                                    TimeLineWidgetPriv::StatPair(count, it_YP.data().second) );
        }

        if (d->maxCountByWeek < count) 
            d->maxCountByWeek = count;

        // Stats Days values.

        it_YP = d->dayStatMap.find(TimeLineWidgetPriv::YearRefPair(year, day));
        if ( it_YP == d->dayStatMap.end() )
        {
            count = it.data();
            d->dayStatMap.insert( TimeLineWidgetPriv::YearRefPair(year, day), 
                                  TimeLineWidgetPriv::StatPair(count, false) );
        }
        else
        {
            count = it_YP.data().first + it.data();
            d->dayStatMap.replace( TimeLineWidgetPriv::YearRefPair(year, day), 
                                   TimeLineWidgetPriv::StatPair(count, it_YP.data().second) );
        }

        if (d->maxCountByDay < count) 
            d->maxCountByDay = count;
    }

    d->maxDateTime.setDate(QDate(d->maxDateTime.date().year()+1, 1, 1));
    d->maxDateTime.setTime(QTime());
    d->minDateTime.setDate(QDate(d->minDateTime.date().year(), 1, 1));
    d->minDateTime.setTime(QTime());

    updatePixmap();
    update();
}

void TimeLineWidget::updatePixmap()
{
    // Drawing background and image.
    d->pixmap = QPixmap(size());
    d->pixmap.fill(palette().active().background());

    QPainter p(&d->pixmap);

    d->bottomMargin = (int)(p.fontMetrics().height()*1.5); 
    d->barWidth     = p.fontMetrics().width("00"); 
    d->nbItems      = (int)((width() / 2.0) / (float)d->barWidth);
    d->startPos     = (int)((width() / 2.0) - ((float)(d->barWidth) / 2.0));
    int dim         = height() - d->bottomMargin - d->topMargin;
    QDateTime ref   = d->refDateTime;
    ref.setTime(QTime());
    int   val;
    bool  sel;
    QRect focusRect, selRect;

    // Date histogram drawing is divided in 2 parts. The current date-time 
    // is placed on the center of the view and all dates on right are computed,
    // and in second time, all dates on the left.

    // Draw all dates on the right of ref. date-time.

    for (int i = 0 ; i < d->nbItems ; i++)
    {
        val = statForDateTime(ref, sel);

        QRect barRect;
        barRect.setTop(dim + d->topMargin - ((val * dim) / maxCount()));
        barRect.setLeft(d->startPos + i*d->barWidth);
        barRect.setBottom(height() - d->bottomMargin);
        barRect.setRight(d->startPos + (i+1)*d->barWidth);

        if (ref == d->selDateTime)
            focusRect = barRect;

        p.setPen(palette().active().foreground());
        p.fillRect(barRect, QBrush(Qt::green));
        p.drawRect(barRect);
        p.drawLine(barRect.right(), barRect.bottom(), barRect.right(), barRect.bottom()+3);
        p.drawLine(barRect.left(),  barRect.bottom(), barRect.left(),  barRect.bottom()+3);

        if (sel)
        {
            selRect.setTop(height() - d->bottomMargin + 1);
            selRect.setLeft(d->startPos + i*d->barWidth);
            selRect.setBottom(height() - d->bottomMargin/2);
            selRect.setRight(d->startPos + (i+1)*d->barWidth);
            p.fillRect(selRect, QBrush(palette().active().highlight()));
        }

        switch(d->dateMode)
        {
            case Day:
            {
                {
                    p.save();
                    QFont fnt = p.font();
                    fnt.setPointSize(fnt.pointSize()-4);
                    p.setFont(fnt);
                    p.setPen(val ? palette().active().foreground() : palette().active().mid()) ;
                    QString txt = QString(KGlobal::locale()->calendar()->weekDayName(ref.date(), true)[0]);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left() + ((barRect.width()-br.width())/2),
                               barRect.bottom()+br.height(), txt);
                    p.restore();
                }

                if (KGlobal::locale()->calendar()->dayOfWeek(ref.date()) == 1)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = KGlobal::locale()->formatDate(ref.date(), true);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                break;
            }
            case Week:
            {
                int week = KGlobal::locale()->calendar()->weekNumber(ref.date());
                {
                    p.save();
                    QFont fnt = p.font();
                    fnt.setPointSize(fnt.pointSize()-4);
                    p.setFont(fnt);
                    p.setPen(val ? palette().active().foreground() : palette().active().mid()) ;
                    QString txt = QString::number(week);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left() + ((barRect.width()-br.width())/2),
                               barRect.bottom()+br.height(), txt);
                    p.restore();
                }

                if (week == 1 || week == 10 || week == 20 || week == 30 || week == 40 || week == 50)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = KGlobal::locale()->formatDate(ref.date(), true);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    if (week != 50)
                        p.drawText(barRect.left()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                else if (week == 6 || week == 16 || week == 26 || week == 36 || week == 46)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/4);
                }
                break;
            }
            case Month:
            {
                {
                    p.save();
                    QFont fnt = p.font();
                    fnt.setPointSize(fnt.pointSize()-4);
                    p.setFont(fnt);
                    p.setPen(val ? palette().active().foreground() : palette().active().mid()) ;
                    QString txt = QString(KGlobal::locale()->calendar()->monthName(ref.date(), true)[0]);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left() + ((barRect.width()-br.width())/2),
                               barRect.bottom()+br.height(), txt);
                    p.restore();
                }

                if (ref.date().month() == 1)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = QString::number(ref.date().year());
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                else if (ref.date().month() == 7)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/4);
                }
                break;
            }
            case Year:
            {
                if (ref.date().year() % 10 == 0)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = QString::number(ref.date().year());
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                else if (ref.date().year() % 5 == 0)
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/4);
                break;
            }
        }

        ref = nextDateTime(ref);
    }

    // Draw all dates on the left of ref. date-time.

    ref = d->refDateTime;
    ref.setTime(QTime());
    ref = prevDateTime(ref);

    for (int i = 0 ; i < d->nbItems-1 ; i++)
    {
        val = statForDateTime(ref, sel);

        QRect barRect;
        barRect.setTop(dim + d->topMargin - ((val * dim) / maxCount()));
        barRect.setRight(d->startPos - i*d->barWidth);
        barRect.setBottom(height() - d->bottomMargin);
        barRect.setLeft(d->startPos - (i+1)*d->barWidth);

        if (ref == d->selDateTime)
            focusRect = barRect;

        p.setPen(palette().active().foreground());
        p.fillRect(barRect, QBrush(Qt::green));
        p.drawRect(barRect);
        p.drawLine(barRect.right(), barRect.bottom(), barRect.right(), barRect.bottom()+3);
        p.drawLine(barRect.left(),  barRect.bottom(), barRect.left(),  barRect.bottom()+3);

        if (sel)
        {
            selRect.setTop(height() - d->bottomMargin + 1);
            selRect.setLeft(d->startPos - (i+1)*d->barWidth);
            selRect.setBottom(height() - d->bottomMargin/2);
            selRect.setRight(d->startPos - i*d->barWidth);
            p.fillRect(selRect, QBrush(palette().active().highlight()));
        }

        switch(d->dateMode)
        {
            case Day:
            {
                {
                    p.save();
                    QFont fnt = p.font();
                    fnt.setPointSize(fnt.pointSize()-4);
                    p.setFont(fnt);
                    p.setPen(val ? palette().active().foreground() : palette().active().mid()) ;
                    QString txt = QString(KGlobal::locale()->calendar()->weekDayName(ref.date(), true)[0]);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left() + ((barRect.width()-br.width())/2),
                               barRect.bottom()+br.height(), txt);
                    p.restore();
                }

                if (KGlobal::locale()->calendar()->dayOfWeek(ref.date()) == 1)
                {
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = KGlobal::locale()->formatDate(ref.date(), true);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.right()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                break;
            }
            case Week:
            {
                int week = KGlobal::locale()->calendar()->weekNumber(ref.date());
                {
                    p.save();
                    QFont fnt = p.font();
                    fnt.setPointSize(fnt.pointSize()-4);
                    p.setFont(fnt);
                    p.setPen(val ? palette().active().foreground() : palette().active().mid()) ;
                    QString txt = QString::number(week);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left() + ((barRect.width()-br.width())/2),
                               barRect.bottom()+br.height(), txt);
                    p.restore();
                }

                if (week == 1 || week == 10 || week == 20 || week == 30 || week == 40 || week == 50)
                {
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = KGlobal::locale()->formatDate(ref.date(), true);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    if (week != 50)
                        p.drawText(barRect.right()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                else if (week == 6 || week == 16 || week == 26 || week == 36 || week == 46)
                {
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/4);
                }
                break;
            }
            case Month:
            {
                {
                    p.save();
                    QFont fnt = p.font();
                    fnt.setPointSize(fnt.pointSize()-4);
                    p.setFont(fnt);
                    p.setPen(val ? palette().active().foreground() : palette().active().mid()) ;
                    QString txt = QString(KGlobal::locale()->calendar()->monthName(ref.date(), true)[0]);
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left() + ((barRect.width()-br.width())/2),
                               barRect.bottom()+br.height(), txt);
                    p.restore();
                }

                if (ref.date().month() == 1)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = QString::number(ref.date().year());
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                else if (ref.date().month() == 7)
                {
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/4);
                }
                break;
            }
            case Year:
            {
                if (ref.date().year() % 10 == 0)
                {
                    p.drawLine(barRect.left(), barRect.bottom(), 
                               barRect.left(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = QString::number(ref.date().year());
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.left()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
                }
                else if (ref.date().year() % 5 == 0)
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/4);
                break;
            }
        }

        ref = prevDateTime(ref);
    }

    // Draw focus rectangle over current date-time.
    if (focusRect.isValid())
    {
        focusRect.setTop(d->topMargin);
        focusRect.addCoords(-1,-1, 1, 1 + d->bottomMargin/2);
        p.setPen(palette().active().shadow());
        p.drawRect(focusRect);
        focusRect.addCoords(-1,-1, 1, 1);
        p.setPen(palette().active().background());
        p.drawRect(focusRect);
        focusRect.addCoords(-1,-1, 1, 1);
        p.setPen(palette().active().shadow());
        p.drawRect(focusRect);
    }
    p.end();
}

QDateTime TimeLineWidget::prevDateTime(const QDateTime& dt)
{
    QDateTime prev;
    switch(d->dateMode)
    {
        case Day:
        {
            prev = dt.addDays(-1);
            break;
        }
        case Week:
        {
            prev = dt.addDays(-7);
            break;
        }
        case Month:
        {
            prev = dt.addMonths(-1);
            break;
        }
        case Year:
        {
            prev = dt.addYears(-1);
            break;
        }
    }
    return prev;
}

QDateTime TimeLineWidget::nextDateTime(const QDateTime& dt)
{
    QDateTime next;
    switch(d->dateMode)
    {
        case Day:
        {
            next = dt.addDays(1);
            break;
        }
        case Week:
        {
            next = dt.addDays(7);
            break;
        }
        case Month:
        {
            next = dt.addMonths(1);
            break;
        }
        case Year:
        {
            next = dt.addYears(1);
            break;
        }
    }
    return next;
}

int TimeLineWidget::maxCount()
{
    int max = 1;
    switch(d->dateMode)
    {
        case Day:
        {
            max = d->maxCountByDay;
            break;
        }
        case Week:
        {
            max = d->maxCountByWeek;
            break;
        }
        case Month:
        {
            max = d->maxCountByMonth;
            break;
        }
        case Year:
        {
            max = d->maxCountByYear;
            break;
        }
    }
    return max;
}

int TimeLineWidget::statForDateTime(const QDateTime& dt, bool& selected)
{
    int count = 0;
    int year  = dt.date().year();
    int month = dt.date().month();
    int day   = KGlobal::locale()->calendar()->dayOfYear(dt.date());
    int week  = KGlobal::locale()->calendar()->weekNumber(dt.date());
    selected  = false;

    switch(d->dateMode)
    {
        case Day:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it =
                d->dayStatMap.find(TimeLineWidgetPriv::YearRefPair(year, day));
            if ( it != d->dayStatMap.end() )
            {
                count    = it.data().first;
                selected = it.data().second;
            }
            break;
        }
        case Week:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it =
                d->weekStatMap.find(TimeLineWidgetPriv::YearRefPair(year, week));
            if ( it != d->weekStatMap.end() )
            {
                count    = it.data().first;
                selected = it.data().second;
            }
            break;
        }
        case Month:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it =
                d->monthStatMap.find(TimeLineWidgetPriv::YearRefPair(year, month));
            if ( it != d->monthStatMap.end() )
            {
                count    = it.data().first;
                selected = it.data().second;
            }
            break;
        }
        case Year:
        {
            QMap<int, TimeLineWidgetPriv::StatPair>::iterator it = d->yearStatMap.find(year);
            if ( it != d->yearStatMap.end() )
            {
                count    = it.data().first;
                selected = it.data().second;
            }
            break;
        }
    }

    return count;
}

void TimeLineWidget::setDateTimeSelected(const QDateTime& dt, bool selected)
{
    int year  = dt.date().year();
    int month = dt.date().month();
    int day   = KGlobal::locale()->calendar()->dayOfYear(dt.date());
    int week  = KGlobal::locale()->calendar()->weekNumber(dt.date());

    switch(d->dateMode)
    {
        case Day:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it =
                d->dayStatMap.find(TimeLineWidgetPriv::YearRefPair(year, day));
            if ( it != d->dayStatMap.end() )
                it.data().second = selected;
            break;
        }
        case Week:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it =
                d->weekStatMap.find(TimeLineWidgetPriv::YearRefPair(year, week));
            if ( it != d->weekStatMap.end() )
                it.data().second = selected;
            break;
        }
        case Month:
        {
            QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it =
                d->monthStatMap.find(TimeLineWidgetPriv::YearRefPair(year, month));
            if ( it != d->monthStatMap.end() )
                it.data().second = selected;
            break;
        }
        case Year:
        {
            QMap<int, TimeLineWidgetPriv::StatPair>::iterator it = d->yearStatMap.find(year);
            if ( it != d->yearStatMap.end() )
                it.data().second = selected;
            break;
        }
    }

    updatePixmap();
    update();
}

void TimeLineWidget::paintEvent(QPaintEvent*)
{
    bitBlt(this, 0, 0, &d->pixmap);
}

void TimeLineWidget::resizeEvent(QResizeEvent*)
{
    updatePixmap();
}

void TimeLineWidget::slotBackward()
{
    QDateTime ref = d->refDateTime;

    switch(d->dateMode)
    {
        case Day:
        {
            for (int i = 0; i < 7; i++)
                ref = prevDateTime(ref);
            break;
        }
        case Week:
        {
            for (int i = 0; i < 4; i++)
                ref = prevDateTime(ref);
            break;
        }
        case Month:
        {
            for (int i = 0; i < 12; i++)
                ref = prevDateTime(ref);
            break;
        }
        case Year:
        {
            for (int i = 0; i < 5; i++)
                ref = prevDateTime(ref);
            break;
        }
    }

    if (ref < d->minDateTime)
        ref = d->minDateTime;

    setRefDateTime(ref);
}

void TimeLineWidget::slotPrevious()
{
    QDateTime ref = prevDateTime(d->refDateTime);
    if (ref < d->minDateTime)
        ref = d->minDateTime;
    setRefDateTime(ref);
}

void TimeLineWidget::slotNext()
{
    QDateTime ref = nextDateTime(d->refDateTime);
    if (ref > d->maxDateTime)
        ref = d->maxDateTime;
    setRefDateTime(ref);
}

void TimeLineWidget::slotForward()
{
    QDateTime ref = d->refDateTime;

    switch(d->dateMode)
    {
        case Day:
        {
            for (int i = 0; i < 7; i++)
                ref = nextDateTime(ref);
            break;
        }
        case Week:
        {
            for (int i = 0; i < 4; i++)
                ref = nextDateTime(ref);
            break;
        }
        case Month:
        {
            for (int i = 0; i < 12; i++)
                ref = nextDateTime(ref);
            break;
        }
        case Year:
        {
            for (int i = 0; i < 5; i++)
                ref = nextDateTime(ref);
            break;
        }
    }

    if (ref > d->maxDateTime)
        ref = d->maxDateTime;

    setRefDateTime(ref);
}

void TimeLineWidget::wheelEvent(QWheelEvent* e)
{
    if (e->delta() < 0)
    {
        if (e->state() & Qt::ShiftButton)
            slotForward();
        else
            slotNext();
    }

    if (e->delta() > 0)
    {
        if (e->state() & Qt::ShiftButton)
            slotBackward();
        else
            slotPrevious();
    }
}

void TimeLineWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton)
    {
        QPoint pt(e->x(), e->y());

        bool ctrlPressed    = e->state() & Qt::ControlButton;
        QDateTime ref       = dateTimeForPoint(pt, d->selMouseEvent);
        d->selStartDateTime = QDateTime();
        if (d->selMouseEvent) 
        {
            if (!ctrlPressed)
                resetSelection();

            d->selStartDateTime = ref;
            d->selMinDateTime   = ref;
            d->selMaxDateTime   = ref;
            setDateTimeSelected(ref, true);
        }
        else if (!ref.isNull())
        {
            setCurrentDateTime(ref);
        }

        d->validMouseEvent = true;
    }
}

void TimeLineWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (d->validMouseEvent == true)
    {
        QPoint pt(e->x(), e->y());

        bool sel;
        QDateTime selEndDateTime = dateTimeForPoint(pt, sel);

        if (!selEndDateTime.isNull() && !d->selStartDateTime.isNull())
        {
            if (selEndDateTime > d->selStartDateTime && 
                selEndDateTime > d->selMaxDateTime)
            {
                d->selMaxDateTime = selEndDateTime;
            }
            else if (selEndDateTime < d->selStartDateTime && 
                     selEndDateTime < d->selMinDateTime)
            {
                d->selMinDateTime = selEndDateTime;
            }

            QDateTime dt = d->selMinDateTime;
            do
            {
                setDateTimeSelected(dt, false);
                dt = nextDateTime(dt);
            }
            while(dt <= d->selMaxDateTime);
        }

        if (d->selMouseEvent)
        {
            if (!d->selStartDateTime.isNull() && !selEndDateTime.isNull())
            {
                QDateTime dt = d->selStartDateTime;
                if (selEndDateTime > d->selStartDateTime)
                {
                    do
                    {
                        setDateTimeSelected(dt, true);
                        dt = nextDateTime(dt);
                    }
                    while(dt <= selEndDateTime);
                }
                else
                {
                    do
                    {
                        setDateTimeSelected(dt, true);
                        dt = prevDateTime(dt);
                    }
                    while(dt >= selEndDateTime);
                }
            }
        }
        else if (!selEndDateTime.isNull())
        {
            setCurrentDateTime(selEndDateTime);
        }
    }
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent*)
{
    d->validMouseEvent = false;

    // Only dispatch changes about selection when user release mouse selection
    // to prevent multiple queries on database.
    if (d->selMouseEvent)
            emit signalSelectionChanged();

    d->selMouseEvent   = false;
}

QDateTime TimeLineWidget::dateTimeForPoint(const QPoint& pt, bool &isOnSelectionArea)
{
    QRect barRect, selRect;
    isOnSelectionArea = false;

    // Check on the left of reference date.

    QDateTime ref = d->refDateTime;
    ref.setTime(QTime());

    for (int i = 0 ; i < d->nbItems ; i++)
    {
        barRect.setTop(0);
        barRect.setLeft(d->startPos + i*d->barWidth);
        barRect.setBottom(height() - d->bottomMargin + 1);
        barRect.setRight(d->startPos + (i+1)*d->barWidth);

        selRect.setTop(height() - d->bottomMargin + 1);
        selRect.setLeft(d->startPos + i*d->barWidth);
        selRect.setBottom(height());
        selRect.setRight(d->startPos + (i+1)*d->barWidth);

        if (selRect.contains(pt))
            isOnSelectionArea = true;

        if (barRect.contains(pt) || selRect.contains(pt))
            return ref;

        ref = nextDateTime(ref);
    }

    // Check on the right of reference date.

    ref = d->refDateTime;
    ref.setTime(QTime());
    ref = prevDateTime(ref);

    for (int i = 0 ; i < d->nbItems-1 ; i++)
    {
        barRect.setTop(0);
        barRect.setRight(d->startPos - i*d->barWidth);
        barRect.setBottom(height() - d->bottomMargin + 1);
        barRect.setLeft(d->startPos - (i+1)*d->barWidth);

        selRect.setTop(height() - d->bottomMargin + 1);
        selRect.setLeft(d->startPos - (i+1)*d->barWidth);
        selRect.setBottom(height());
        selRect.setRight(d->startPos - i*d->barWidth);

        if (selRect.contains(pt))
            isOnSelectionArea = true;

        if (barRect.contains(pt) || selRect.contains(pt))
            return ref;

        ref = prevDateTime(ref);
    }

    return QDateTime();
}

QDateTime TimeLineWidget::firstDayOfWeek(int year, int weekNumber)
{
    QDateTime date(QDate(year, 1, 1));
    for (int i = 0 ; i < KGlobal::locale()->calendar()->daysInYear(date.date()) ; i++)
    {
        QDateTime dt = date.addDays(i);

        if (KGlobal::locale()->calendar()->weekNumber(dt.date()) == weekNumber)
        {
            if (weekNumber == 1 && KGlobal::locale()->calendar()->dayOfWeek(dt.date()) > 1)
            {
                int dayWeekOffset = (-1) * (KGlobal::locale()->calendar()->dayOfWeek(dt.date()) - 1);
                dt = dt.addDays(dayWeekOffset);
            }
            return dt;
        }
    }
    return QDateTime();
}

}  // NameSpace Digikam

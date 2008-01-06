/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a widget to display date and time statistics of pictures
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
        selectMode      = false;
        maxCountByDay   = 1;
        maxCountByWeek  = 1;
        maxCountByMonth = 1;
        maxCountByYear  = 1;
        dateMode        = TimeLineWidget::Month;
        topMargin       = 3;
        bottomMargin    = 20;
        barWidth        = 20;
        startPos        = 96;
        nbItems         = 10;
    }

    bool                        selectMode;

    int                         maxCountByDay;
    int                         maxCountByWeek;
    int                         maxCountByMonth;
    int                         maxCountByYear;
    int                         topMargin; 
    int                         bottomMargin; 
    int                         barWidth; 
    int                         nbItems;
    int                         startPos;

    QDateTime                   refDateTime;    // Reference date-time used to draw histogram from the central of widget.
    QDateTime                   selDateTime;    // Current date-time used to draw focus cursor.

    QPixmap                     pixmap;

    QMap<YearRefPair, StatPair> dayStatMap;
    QMap<YearRefPair, StatPair> weekStatMap;
    QMap<YearRefPair, StatPair> monthStatMap;
    QMap<int,         StatPair> yearStatMap;

    TimeLineWidget::DateMode    dateMode;
};

TimeLineWidget::TimeLineWidget(QWidget *parent)
              : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new TimeLineWidgetPriv;
    setBackgroundMode(Qt::NoBackground);
    setMouseTracking(true);
    setMinimumWidth(192);
    setMinimumHeight(80);
}

TimeLineWidget::~TimeLineWidget()
{
    delete d;
}

void TimeLineWidget::setDateMode(DateMode dateMode)
{
    d->dateMode = dateMode;
    updatePixmap();
    update();
}

TimeLineWidget::DateMode TimeLineWidget::dateMode() const
{
    return d->dateMode;
}

void TimeLineWidget::setCurrentDateTime(const QDateTime& dateTime)
{
    if (d->selDateTime == dateTime)
        return;

    d->refDateTime = dateTime;
    d->selDateTime = dateTime;
    updatePixmap();
    update();
    emit signalSelectionChanged();
}

QDateTime TimeLineWidget::currentDateTime() const
{
    return d->selDateTime;
}

void TimeLineWidget::setRefDateTime(const QDateTime& dateTime)
{
    d->refDateTime = dateTime;
    updatePixmap();
    update();
}

int TimeLineWidget::currentSelectionInfo(QDateTime& start, QDateTime& end)
{
    bool selected;
    start = currentDateTime();
    end   = nextDateTime(currentDateTime());
    return statForDateTime(currentDateTime(), selected);
}

void TimeLineWidget::slotDatesMap(const QMap<QDateTime, int>& datesStatMap)
{
    for ( QMap<QDateTime, int>::const_iterator it = datesStatMap.begin();
          it != datesStatMap.end(); ++it )
    {
        int year  = it.key().date().year();
        int month = it.key().date().month();
        int day   = KGlobal::locale()->calendar()->dayOfYear(it.key().date());
        int week  = KGlobal::locale()->calendar()->weekNumber(it.key().date());

        // Stats Years values.

        QMap<int, TimeLineWidgetPriv::StatPair>::iterator it5 = d->yearStatMap.find(year);
        if ( it5 == d->yearStatMap.end() )
        {
            d->yearStatMap.insert( year, TimeLineWidgetPriv::StatPair(it.data(), false) );
            if (d->maxCountByYear < it.data()) 
                d->maxCountByYear = it.data();
        }
        else
        {
            d->yearStatMap.replace( year, TimeLineWidgetPriv::StatPair(it5.data().first + it.data(), false) );
            if (d->maxCountByYear < it5.data().first + it.data()) 
                d->maxCountByYear = it5.data().first + it.data();
        }

        // Stats Months values.

        QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it2 = 
             d->monthStatMap.find(TimeLineWidgetPriv::YearRefPair(year, month));
        if ( it2 == d->monthStatMap.end() )
        {
            d->monthStatMap.insert( TimeLineWidgetPriv::YearRefPair(year, month), 
                                    TimeLineWidgetPriv::StatPair(it.data(), false) );
            if (d->maxCountByMonth < it.data()) 
                d->maxCountByMonth = it.data();
        }
        else
        {
            d->monthStatMap.replace( TimeLineWidgetPriv::YearRefPair(year, month), 
                                     TimeLineWidgetPriv::StatPair(it2.data().first + it.data(), false) );
            if (d->maxCountByMonth < it2.data().first + it.data()) 
                d->maxCountByMonth = it2.data().first + it.data();
        }

        // Stats Weeks values.

        QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it3 = 
             d->weekStatMap.find(TimeLineWidgetPriv::YearRefPair(year, week));
        if ( it3 == d->weekStatMap.end() )
        {
            d->weekStatMap.insert( TimeLineWidgetPriv::YearRefPair(year, week), 
                                   TimeLineWidgetPriv::StatPair(it.data(), false) );
            if (d->maxCountByWeek < it.data()) 
                d->maxCountByWeek = it.data();
        }
        else
        {
            d->weekStatMap.replace( TimeLineWidgetPriv::YearRefPair(year, week), 
                                    TimeLineWidgetPriv::StatPair(it3.data().first + it.data(), false) );
            if (d->maxCountByWeek < it3.data().first + it.data()) 
                d->maxCountByWeek = it3.data().first + it.data();
        }

        // Stats Days values.

        QMap<TimeLineWidgetPriv::YearRefPair, TimeLineWidgetPriv::StatPair>::iterator it4 =
             d->dayStatMap.find(TimeLineWidgetPriv::YearRefPair(year, day));
        if ( it4 == d->dayStatMap.end() )
        {
            d->dayStatMap.insert( TimeLineWidgetPriv::YearRefPair(year, day), 
                                  TimeLineWidgetPriv::StatPair(it.data(), false) );
            if (d->maxCountByDay < it.data()) 
                d->maxCountByDay = it.data();
        }
        else
        {
            d->dayStatMap.replace( TimeLineWidgetPriv::YearRefPair(year, day), 
                                   TimeLineWidgetPriv::StatPair(it4.data().first + it.data(), false) );
            if (d->maxCountByDay < it4.data().first + it.data()) 
                d->maxCountByDay = it4.data().first + it.data();
        }
    }

    setCurrentDateTime(QDateTime::currentDateTime());

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
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = QString::number(ref.date().year());
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.right()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
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
                    p.drawLine(barRect.right(), barRect.bottom(), 
                               barRect.right(), barRect.bottom()+d->bottomMargin/2);
                    QString txt = QString::number(ref.date().year());
                    QRect br    = p.fontMetrics().boundingRect(0, 0, width(), height(), 0, txt); 
                    p.drawText(barRect.right()-br.width()/2, barRect.bottom() + d->bottomMargin, txt);
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
    setRefDateTime(ref);
}

void TimeLineWidget::slotPrevious()
{
    setRefDateTime(prevDateTime(d->refDateTime));
}

void TimeLineWidget::slotNext()
{
    setRefDateTime(nextDateTime(d->refDateTime));
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
        checkForSelection(pt);
        d->selectMode = true;
    }
}

void TimeLineWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (d->selectMode == true)
    {
        QPoint pt(e->x(), e->y());
        checkForSelection(pt);
    }
}

void TimeLineWidget::mouseReleaseEvent(QMouseEvent*)
{
    d->selectMode = false;

}

void TimeLineWidget::checkForSelection(const QPoint& pt)
{
    QDateTime ref = d->refDateTime;
    ref.setTime(QTime());

    for (int i = 0 ; i < d->nbItems ; i++)
    {
        QRect barRect;
        barRect.setTop(0);
        barRect.setLeft(d->startPos + i*d->barWidth);
        barRect.setBottom(height());
        barRect.setRight(d->startPos + (i+1)*d->barWidth);

        if (barRect.contains(pt) && d->selDateTime != ref)
        {
            d->selDateTime = ref;
            bool sel;
            statForDateTime(ref, sel);
            setDateTimeSelected(ref, !sel);
            emit signalSelectionChanged();
            updatePixmap();
            update();
            return;
        }

        ref = nextDateTime(ref);
    }

    ref = d->refDateTime;
    ref.setTime(QTime());
    ref = prevDateTime(ref);

    for (int i = 0 ; i < d->nbItems-1 ; i++)
    {
        QRect barRect;
        barRect.setTop(0);
        barRect.setRight(d->startPos - i*d->barWidth);
        barRect.setBottom(height());
        barRect.setLeft(d->startPos - (i+1)*d->barWidth);

        if (barRect.contains(pt) && d->selDateTime != ref)
        {
            d->selDateTime = ref;
            bool sel;
            statForDateTime(ref, sel);
            setDateTimeSelected(ref, !sel);
            emit signalSelectionChanged();
            updatePixmap();
            update();
            return;
        }

        ref = prevDateTime(ref);
    }
}

}  // NameSpace Digikam

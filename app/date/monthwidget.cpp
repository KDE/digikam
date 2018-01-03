/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-02
 * Description : a widget to perform month selection.
 *
 * Copyright (C) 2005      by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "monthwidget.h"

// Qt includes

#include <QList>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QDateTime>
#include <QFontMetrics>
#include <QPainter>
#include <QPixmap>
#include <QPalette>
#include <QTimer>
#include <QLocale>
#include <QDate>

// Local includes

#include "imagefiltermodel.h"
#include "imagemodel.h"

namespace Digikam
{

class MonthWidget::Private
{
public:

    struct Month
    {
        Month()
        {
            active    = false;
            selected  = false;
            day       = 0;
            numImages = 0;
        }

        bool active;
        bool selected;

        int  day;
        int  numImages;
    };

public:

    Private() :
        active(true),
        model(0),
        timer(0),
        year(0),
        month(0),
        width(0),
        height(0),
        currw(0),
        currh(0)
    {
    }

    bool              active;

    ImageFilterModel* model;
    QTimer*           timer;

    int               year;
    int               month;
    int               width;
    int               height;
    int               currw;
    int               currh;

    struct Month      days[42];
};

MonthWidget::MonthWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    init();

    QDate date = QDate::currentDate();
    setYearMonth(date.year(), date.month());

    setActive(false);

    d->timer = new QTimer(this);
    d->timer->setSingleShot(true);
    d->timer->setInterval(150);

    connect(d->timer, &QTimer::timeout,
            this, &MonthWidget::updateDays);
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
    QRect r(fm.boundingRect(QLatin1String("XX")));
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

    for (int i=0; i<42; ++i)
    {
        d->days[i].active    = false;
        d->days[i].selected  = false;
        d->days[i].day       = -1;
        d->days[i].numImages = 0;
    }

    QDate date(year, month, 1);
    int s = date.dayOfWeek();

    for (int i=s; i<(s+date.daysInMonth()); ++i)
    {
        d->days[i-1].day = i-s+1;
    }

    update();
}

QSize MonthWidget::sizeHint() const
{
    return QSize(d->width * 8, d->height * 9);
}

void MonthWidget::resizeEvent(QResizeEvent* e)
{
    QWidget::resizeEvent(e);

    d->currw = contentsRect().width()/8;
    d->currh = contentsRect().height()/9;
}

void MonthWidget::paintEvent(QPaintEvent*)
{
    QRect cr(contentsRect());

    QPixmap pix(cr.width(), cr.height());

    QFont fnBold(font());
    QFont fnOrig(font());
    fnBold.setBold(true);
    fnOrig.setBold(false);

    QPainter p(&pix);
    p.fillRect(0, 0, cr.width(), cr.height(), palette().color(QPalette::Background));

    QRect r(0, 0, d->currw, d->currh);
    QRect rsmall;

    int sx, sy;
    int index = 0;
    bool weekvisible;

    for (int j=3; j<9; ++j)
    {
        sy          = d->currh * j;
        weekvisible = false;

        for (int i=1; i<8; ++i)
        {
            sx     = d->currw * i;
            r.moveTopLeft(QPoint(sx,sy));
            rsmall = QRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);

            if (d->days[index].day != -1)
            {
                if (d->days[index].selected)
                {
                    p.fillRect(r, palette().color(QPalette::Highlight));
                    p.setPen(palette().color(QPalette::HighlightedText));

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
                        p.setPen(palette().color(QPalette::Text));
                        p.setFont(fnBold);
                    }
                    else
                    {
                        p.setPen(palette().color(QPalette::Mid));
                        p.setFont(fnOrig);
                    }
                }

                p.drawText(rsmall, Qt::AlignVCenter|Qt::AlignHCenter,
                           QString::number(d->days[index].day));

                if (!weekvisible)
                {
                    int weeknr = QDate(d->year, d->month, d->days[index].day).weekNumber();
                    p.setPen(d->active ? Qt::black : Qt::gray);
                    p.setFont(fnBold);
                    p.fillRect(1, sy, d->currw-1, d->currh-1, QColor(210, 210, 210));
                    p.drawText(1, sy, d->currw-1, d->currh-1, Qt::AlignVCenter|Qt::AlignHCenter,
                               QString::number(weeknr));
                    weekvisible = true;
                }

            }

            ++index;
        }
    }

    p.setPen(d->active ? Qt::black : Qt::gray);
    p.setFont(fnBold);

    sy = 2*d->currh;

    for (int i = 1; i < 8; ++i)
    {
        sx     = d->currw * i;
        r.moveTopLeft(QPoint(sx+1,sy+1));
        rsmall = r;
        rsmall.setWidth(r.width() - 2);
        rsmall.setHeight(r.height() - 2);
        p.drawText(rsmall, Qt::AlignVCenter|Qt::AlignHCenter,
                   QLocale().dayName(i, QLocale::ShortFormat).remove(2, 1));
        ++index;
    }

    r = QRect(0, 0, cr.width(), 2*d->currh);

    fnBold.setPointSize(fnBold.pointSize()+2);
    p.setFont(fnBold);

    p.drawText(r, Qt::AlignCenter, QString::fromUtf8("%1 %2")
               .arg(QLocale().monthName(d->month, QLocale::LongFormat))
               .arg(QDate(d->year, d->month, 1).year()));

    p.end();

    QPainter p2(this);
    p2.drawPixmap(cr.x(), cr.y(), pix);
    p2.end();
}

void MonthWidget::mousePressEvent(QMouseEvent* e)
{
    int firstSelected = 0, lastSelected = 0;

    if (e->modifiers() != Qt::ControlModifier)
    {
        for (int i=0; i<42; ++i)
        {
            if (d->days[i].selected)
            {
                if (firstSelected==0)
                {
                    firstSelected = i;
                }

                lastSelected =i;
            }

            d->days[i].selected = false;
        }
    }

    QRect r1(0, d->currh*3, d->currw, d->currh*6);
    QRect r2(d->currw, d->currh*3, d->currw*7, d->currh*6);
    QRect r3(d->currw, d->currh*2, d->currw*7, d->currh);

    // Click on a weekday
    if ( r3.contains(e->pos()))
    {
        int j = (e->pos().x() - d->currw)/d->currw;

        for (int i=0; i<6; ++i)
        {
            d->days[i*7+j].selected = !d->days[i*7+j].selected;
        }
    }
    // Click on a week
    else if (r1.contains(e->pos()))
    {
        int j = (e->pos().y() - 3*d->currh)/d->currh;

        for (int i=0; i<7; ++i)
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

        if (e->modifiers() == Qt::ShiftModifier)
        {
            int endSelection = j*7+i;

            if (endSelection > firstSelected)
                for (int i2=firstSelected ; i2 <= endSelection; ++i2)
                {
                    d->days[i2].selected = true;
                }
            else if (endSelection < firstSelected)
                for (int i2=lastSelected ; i2 >= endSelection; --i2)
                {
                    d->days[i2].selected = true;
                }
        }
        else
        {
            d->days[j*7+i].selected = !d->days[j*7+i].selected;
        }
    }

    QList<QDateTime> filterDays;

    for (int i=0; i<42; ++i)
    {
        if (d->days[i].selected && d->days[i].day != -1)
        {
            filterDays.append(QDateTime(QDate(d->year, d->month, d->days[i].day), QTime()));
        }
    }

    if (d->model)
    {
        d->model->setDayFilter(filterDays);
    }

    update();
}

void MonthWidget::setActive(bool val)
{
    if (d->active == val)
    {
        return;
    }

    d->active = val;

    if (d->active)
    {
        connectModel();
        triggerUpdateDays();
    }
    else
    {
        QDate date = QDate::currentDate();
        setYearMonth(date.year(), date.month());

        if (d->model)
        {
            d->model->setDayFilter(QList<QDateTime>());
            disconnect(d->model, 0, this, 0);
        }
    }
}

void MonthWidget::setImageModel(ImageFilterModel* model)
{
    if (d->model)
    {
        disconnect(d->model, 0, this, 0);
    }

    d->model = model;
    connectModel();

    triggerUpdateDays();
}

void MonthWidget::connectModel()
{
    if (d->model)
    {
        connect(d->model, &ImageFilterModel::destroyed,
                this, &MonthWidget::slotModelDestroyed);

        connect(d->model, &ImageFilterModel::rowsInserted,
                this, &MonthWidget::triggerUpdateDays);

        connect(d->model, &ImageFilterModel::rowsRemoved,
                this, &MonthWidget::triggerUpdateDays);

        connect(d->model, &ImageFilterModel::modelReset,
                this, &MonthWidget::triggerUpdateDays);
/*
        connect(d->model, SIGNAL(triggerUpdateDays()),
                this, SLOT(triggerUpdateDays()));

        connect(d->model, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
                this, SLOT(triggerUpdateDays()));
*/
    }
}

void MonthWidget::triggerUpdateDays()
{
    if (!d->timer->isActive())
    {
        d->timer->start();
    }
}

void MonthWidget::resetDayCounts()
{
    for (int i=0; i<42; ++i)
    {
        d->days[i].active    = false;
        d->days[i].numImages = 0;
    }
}

void MonthWidget::updateDays()
{
    if (!d->active)
    {
        return;
    }

    resetDayCounts();

    if (!d->model)
    {
        return;
    }

    const int size = d->model->rowCount();

    for (int i=0; i<size; ++i)
    {
        QModelIndex index = d->model->index(i, 0);

        if (!index.isValid())
        {
            continue;
        }

        QDateTime dt = d->model->data(index, ImageModel::CreationDateRole).toDateTime();

        if (dt.isNull())
        {
            continue;
        }

        for (int i=0; i<42; ++i)
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

void MonthWidget::slotModelDestroyed()
{
    d->model = 0;
    resetDayCounts();
    update();
}

}  // namespace Digikam

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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

// Qt includes.

#include <qstring.h>
#include <qwidget.h>
#include <qdatetime.h>

// Local includes.

#include "ddebug.h"

namespace Digikam
{

typedef QPair<QDateTime, QDateTime> DateRange;    // Range of a contigue dates selection <start date, end date>.
typedef QValueList<DateRange> DateRangeList;      // List of dates range selected.

class TimeLineWidgetPriv;

class TimeLineWidget : public QWidget
{
Q_OBJECT

public:

    enum DateMode 
    {
        Day = 0,
        Week,
        Month,
        Year
    };

public:

    TimeLineWidget(QWidget *parent=0);
    ~TimeLineWidget();

    void setDateMode(DateMode dateMode);
    DateMode dateMode() const;

    void resetSelection();
    void resetSelection(TimeLineWidget::DateMode mode);
    void resetAllSelection();

    void setRefDateTime(const QDateTime& dateTime);

    void setCurrentDateTime(const QDateTime& dateTime);
    QDateTime currentDateTime() const;

    int cursorInfo(QDateTime& start, QDateTime& end);
    DateRangeList currentSelectedDateRange(int& totalCoun);

signals:

    void signalSelectionChanged();

public slots:

    void slotDatesMap(const QMap<QDateTime, int>&);
    void slotBackward();
    void slotPrevious();
    void slotNext();
    void slotForward();

private:

    QDateTime prevDateTime(const QDateTime& dt);
    QDateTime nextDateTime(const QDateTime& dt);

    int       maxCount();
    int       statForDateTime(const QDateTime& dt, bool& selected);
    void      setDateTimeSelected(const QDateTime& dt, bool selected);

    void      updatePixmap();
    void      paintEvent(QPaintEvent*);
    void      resizeEvent(QResizeEvent*);
    void      wheelEvent(QWheelEvent* e); 

    void      mousePressEvent(QMouseEvent*);
    void      mouseMoveEvent(QMouseEvent*);
    void      mouseReleaseEvent(QMouseEvent*);
    void      checkForSelection(const QPoint& pt);

    QDateTime firstDayOfWeek(int year, int weekNumber);

private:

    TimeLineWidgetPriv* d;
};

}  // NameSpace Digikam

#endif // TIMELINEWIDGET_H

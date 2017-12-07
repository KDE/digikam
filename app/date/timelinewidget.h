/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a widget to display date and time statistics of pictures
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

// Qt includes

#include <QString>
#include <QWidget>
#include <QDateTime>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QMouseEvent>

// Local inclues

#include "searchmodificationhelper.h"

namespace Digikam
{

class TimeLineWidget : public QWidget
{
    Q_OBJECT

public:

    enum TimeUnit
    {
        Day = 0,
        Week,
        Month,
        Year
    };

    enum SelectionMode
    {
        Unselected = 0,    // No selection.
        FuzzySelection,    // Partially selected.
        Selected           // Fully selected.
    };

    enum ScaleMode
    {
        LinScale = 0,      // Linear scale.
        LogScale           // Logarithmic scale.
    };

public:

    explicit TimeLineWidget(QWidget* const parent = 0);
    ~TimeLineWidget();

    void      setTimeUnit(TimeUnit timeUnit);
    TimeUnit  timeUnit() const;

    void      setScaleMode(ScaleMode scaleMode);
    ScaleMode scaleMode() const;

    void      setCursorDateTime(const QDateTime& dateTime);
    QDateTime cursorDateTime() const;
    int       cursorInfo(QString& infoDate) const;

    /** Return a list of Date-Range based on selection performed on days-map */
    DateRangeList selectedDateRange(int& totalCount) const;
    void          setSelectedDateRange(const DateRangeList& list);

    int  totalIndex()             const;
    int  indexForRefDateTime()    const;
    int  indexForCursorDateTime() const;
    void setCurrentIndex(int index);

Q_SIGNALS:

    void signalCursorPositionChanged();
    void signalSelectionChanged();
    void signalRefDateTimeChanged();
    void signalDateMapChanged();

public Q_SLOTS:

    void slotDatesMap(const QMap<QDateTime, int>&);
    void slotPrevious();
    void slotNext();
    void slotBackward();
    void slotForward();
    void slotResetSelection();

private Q_SLOTS:

    void slotThemeChanged();

private:

    QDateTime     prevDateTime(const QDateTime& dt) const;
    QDateTime     nextDateTime(const QDateTime& dt) const;

    int           maxCount() const;
    int           indexForDateTime(const QDateTime& date) const;
    int           statForDateTime(const QDateTime& dt, SelectionMode& selected) const;
    void          setRefDateTime(const QDateTime& dateTime);

    void          paintEvent(QPaintEvent*);
    void          wheelEvent(QWheelEvent*);

    void          mousePressEvent(QMouseEvent*);
    void          mouseMoveEvent(QMouseEvent*);
    void          mouseReleaseEvent(QMouseEvent*);

    void          keyPressEvent(QKeyEvent *e);
    void          keyReleaseEvent(QKeyEvent *);
    void          keyScroll(bool isScrollNext);

    QDateTime     dateTimeForPoint(const QPoint& pt, bool& isOnSelectionArea);
    QDateTime     firstDayOfWeek(int year, int weekNumber) const;

    void          resetSelection();
    void          setDateTimeSelected(const QDateTime& dt, SelectionMode selected);
    void          setDaysRangeSelection(const QDateTime& dts, const QDateTime& dte, SelectionMode selected);
    SelectionMode checkSelectionForDaysRange(const QDateTime& dts, const QDateTime& dte) const;
    void          updateWeekSelection(const QDateTime& dts, const QDateTime& dte);
    void          updateMonthSelection(const QDateTime& dts, const QDateTime& dte);
    void          updateYearSelection(const QDateTime& dts, const QDateTime& dte);
    void          updateAllSelection();

    // helper methods for painting
    int           calculateTop(int& val) const;
    void          paintItem(QPainter& p, const QRect& barRect,
                            const QDateTime& ref, const int& separatorPosition,
                            const QColor& dateColor, const QColor& subDateColor);

    void          handleSelectionRange(QDateTime& selEndDateTime);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // TIMELINEWIDGET_H

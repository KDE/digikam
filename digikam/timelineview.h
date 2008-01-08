/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a time line control view
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

#ifndef TIMELINEVIEW_H
#define TIMELINEVIEW_H

// Qt includes.

#include <qdatetime.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qdatetime.h>
#include <qframe.h>

namespace Digikam
{

class TimeLineViewPriv;

class TimeLineView : public QFrame
{
    Q_OBJECT

public:

    TimeLineView(QWidget *parent=0);
    ~TimeLineView();

    void setCurrentDateTime(const QDateTime& dateTime);

private slots:

    void slotScaleChanged(int);
    void slotSelectionChanged();
    void slotDatesMap(const QMap<QDateTime, int>&);
    void slotQuerySearchKIOSlave();

private:

    TimeLineViewPriv* d;
};

}  // NameSpace Digikam

#endif /* TIMELINEVIEW_H */

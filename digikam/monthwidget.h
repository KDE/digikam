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

#ifndef MONTHWIDGET_H
#define MONTHWIDGET_H

// Qt includes.

#include <qframe.h>

// Local includes.

#include "imageinfo.h"

namespace Digikam
{
class MonthWidgetPriv;

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

    MonthWidgetPriv *d;
};

}  // namespace Digikam

#endif /* MONTHWIDGET_H */

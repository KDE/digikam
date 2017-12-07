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

// Qt includes

#include <QWidget>

// Local includes

#include "imageinfo.h"

class QResizeEvent;
class QPaintEvent;
class QMouseEvent;

namespace Digikam
{
class ImageFilterModel;

class MonthWidget : public QWidget
{
    Q_OBJECT

public:

    explicit MonthWidget(QWidget* const parent);
    ~MonthWidget();

    void setImageModel(ImageFilterModel* const model);

    void setYearMonth(int year, int month);
    QSize sizeHint() const;

    void setActive(bool val);

protected:

    void resizeEvent(QResizeEvent* e);
    void paintEvent(QPaintEvent*);
    void mousePressEvent(QMouseEvent* e);

private:

    void init();

private Q_SLOTS:

    void triggerUpdateDays();
    void updateDays();
    void slotModelDestroyed();

private:

    void resetDayCounts();
    void connectModel();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* MONTHWIDGET_H */

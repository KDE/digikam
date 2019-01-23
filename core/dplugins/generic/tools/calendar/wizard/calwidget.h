/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2003-11-03
 * Description : calendar widget used to preview the active template.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2007-2008 by Orgad Shaneh <orgads at gmail dot com>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_CAL_WIDGET_H
#define DIGIKAM_CAL_WIDGET_H

// Qt includes

#include <QWidget>

class QPaintEvent;

namespace GenericCalendarPlugin
{

class CalWidget : public QWidget
{
    Q_OBJECT

public:

    explicit CalWidget(QWidget* const parent);
    ~CalWidget();

    void setCurrent(int curr);
    int current() const;

public Q_SLOTS:

    void recreate();

protected:

    void paintEvent(QPaintEvent* e);

protected:

    int m_current;
};

} // Namespace Digikam

#endif // DIGIKAM_CAL_WIDGET_H

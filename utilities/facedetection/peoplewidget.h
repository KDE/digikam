/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-10
 * Description : Face detection widget
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef PEOPLEWIDGET_H
#define PEOPLEWIDGET_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class PeopleWidget : public QWidget
{
    Q_OBJECT

public:

    enum SelectionMode
    {
        Unselected=0,      // No selection.
        FuzzySelection,    // Partially selected.
        Selected           // Fully selected.
    };

public:

    PeopleWidget(QWidget* parent=0, int numPeople = 0);
    ~PeopleWidget();

    int  cursorInfo(QString& infoDate);
    void setCurrentIndex(int index);

private:

    class PeopleWidgetPriv;
    PeopleWidgetPriv* const d;
};

}  // Namespace Digikam

#endif // PEOPLEWIDGET_H

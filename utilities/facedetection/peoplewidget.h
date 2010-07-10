/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef PEOPLEWIDGET_H
#define PEOPLEWIDGET_H

#include <QWidget>

namespace Digikam
{

class PeopleWidgetPriv;

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

    PeopleWidget(QWidget *parent=0, int numPeople = 0);
    ~PeopleWidget();

    int  cursorInfo(QString& infoDate);
    void setCurrentIndex(int index);
    
private:

private:

    PeopleWidgetPriv* const d;
};




}  // Namespace Digikam


#endif // PEOPLEWIDGET_H

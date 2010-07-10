/**
    This file is part of the digiKam project.
    Copyright (C) 2010  Aditya Bhatt

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

// Qt includes

#include <QPair>


#include "peoplewidget.h"
#include <QLabel>

namespace Digikam{

class PeopleWidgetPriv
{

public:

    PeopleWidgetPriv()
    {
        peopleCount = new QString;
        peopleCount->setNum(0);
        label = new QLabel;
        label->setText("Number of People");
    }

    QLabel *label;
    QString *peopleCount;
};


PeopleWidget::PeopleWidget(QWidget* parent, int numPeople)
             :QWidget(parent), d(new PeopleWidgetPriv)
{
    d->peopleCount->setNum(numPeople);
    d->label->setText("Number of People");
}


PeopleWidget::~PeopleWidget()
{

}


}; // Namespace Digikam
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

#include "peoplewidget.moc"

// Qt includes

#include <QPair>
#include <QLabel>

namespace Digikam
{

class PeopleWidget::PeopleWidgetPriv
{

public:

    PeopleWidgetPriv()
    {
        peopleCount = new QString;
        peopleCount->setNum(0);
        label       = new QLabel;
        label->setText("Number of People");
    }

    QLabel*  label;
    QString* peopleCount;
};

PeopleWidget::PeopleWidget(QWidget* parent, int numPeople)
    : QWidget(parent), d(new PeopleWidgetPriv)
{
    d->peopleCount->setNum(numPeople);
    d->label->setText("Number of People");
}

PeopleWidget::~PeopleWidget()
{
    delete d;
}

int PeopleWidget::cursorInfo(QString& /*infoDate*/)
{
}

void PeopleWidget::setCurrentIndex(int /*index*/)
{
}

}; // Namespace Digikam

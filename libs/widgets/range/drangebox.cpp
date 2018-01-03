/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-08-09
 * Description : Generic range boxes, i.e. ranges where a minimum and maximum can be given.
 *
 * Copyright (C) 2017 by Mario Frank <mario dot frank at uni minus potsdam dot de>
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

#include "drangebox.h"

// Qt includes

#include <QSpinBox>
#include <QLabel>
#include <QHBoxLayout>

namespace Digikam
{

class Q_DECL_HIDDEN DIntRangeBox::Private
{

public:

    Private()
    {
        intervalLabel = 0;
        minValueBox   = 0;
        maxValueBox   = 0;
    }

    QLabel*   intervalLabel;
    QSpinBox* minValueBox;
    QSpinBox* maxValueBox;
};

DIntRangeBox::DIntRangeBox(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    d->minValueBox   = new QSpinBox(this);
    d->minValueBox->setRange(0, 100);
    d->minValueBox->setValue(0);
    d->minValueBox->setSingleStep(1);

    d->intervalLabel = new QLabel(QLatin1String("-"), this);
    d->intervalLabel->setAlignment(Qt::AlignCenter);

    d->maxValueBox   = new QSpinBox(this);
    d->maxValueBox->setRange(d->minValueBox->value(), 100);
    d->maxValueBox->setValue(100);
    d->maxValueBox->setSingleStep(1);

    //const int spacing = QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing);

    QHBoxLayout* const mainLayout = new QHBoxLayout(this);
    mainLayout->addWidget(d->minValueBox);
    mainLayout->addWidget(d->intervalLabel);
    mainLayout->addWidget(d->maxValueBox);
    //mainLayout->setContentsMargins(spacing, spacing, spacing, spacing);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    setLayout(mainLayout);

    connect(d->minValueBox, SIGNAL(valueChanged(int)),
            this, SLOT(slotMinimumChanged(int)));

    connect(d->minValueBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(minChanged(int)));

    connect(d->maxValueBox, SIGNAL(valueChanged(int)),
            this, SIGNAL(maxChanged(int)));
}

DIntRangeBox::~DIntRangeBox()
{
    delete d;
}


void DIntRangeBox::setRange(int min, int max)
{
    if (min <= max)
    {
        d->minValueBox->setMinimum(min);
        d->maxValueBox->setMaximum(max);
    }
}

void DIntRangeBox::setInterval(int min, int max)
{
    if (min <= max)
    {
        d->minValueBox->setValue(min);
        d->maxValueBox->setValue(max);
    }
}

void DIntRangeBox::setSuffix(const QString& suffix)
{
    d->minValueBox->setSuffix(suffix);
    d->maxValueBox->setSuffix(suffix);
}

void DIntRangeBox::setEnabled(bool enabled)
{
    d->intervalLabel->setEnabled(enabled);
    d->minValueBox->setEnabled(enabled);
    d->maxValueBox->setEnabled(enabled);
}

int DIntRangeBox::minValue()
{
    return d->minValueBox->value();
}

int DIntRangeBox::maxValue()
{
    return d->maxValueBox->value();
}

void DIntRangeBox::slotMinimumChanged(int newValue)
{
    // Set the new minimum value of the maximum similarity
    d->maxValueBox->setMinimum(newValue);

    // If the new value of the mimimum is now higher than the maximum similarity,
    // set the maximum similarity to the new value.
    if (newValue > d->maxValueBox->value())
    {
        d->maxValueBox->setValue(d->minValueBox->value());
    }
}

}  // namespace Digikam

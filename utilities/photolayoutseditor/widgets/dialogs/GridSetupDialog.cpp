/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "GridSetupDialog.h"
#include "PLEConfigSkeleton.h"

#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>

#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

GridSetupDialog::GridSetupDialog(QWidget * parent) :
    QDialog(parent)
{
    PLEConfigSkeleton * skeleton = PLEConfigSkeleton::self();

    setWindowTitle(i18n("Setup grid lines"));
    setModal(true);

    centralWidget = new QWidget(this);

    QFormLayout * layout = new QFormLayout(centralWidget);
    layout->setSizeConstraint( QLayout::SetFixedSize );

    setLayout(layout);

    x = new QDoubleSpinBox(centralWidget);
    KConfigSkeletonItem * hgi = skeleton->findItem(QLatin1String("horizontalGrid"));

    if (hgi)
    {
        x->setMinimum(hgi->minValue().toDouble());
        x->setMaximum(hgi->maxValue().toDouble());
    }

    x->setSingleStep(1.0);
    x->setValue(PLEConfigSkeleton::horizontalGrid());
    connect(skeleton, SIGNAL(horizontalGridChanged(double)), x, SLOT(setValue(double)));
    layout->addRow(i18n("Horizontal distance"), x);

    y = new QDoubleSpinBox(centralWidget);
    KConfigSkeletonItem * vgi = skeleton->findItem(QLatin1String("verticalGrid"));

    if (vgi && hgi)
    {
        y->setMinimum(hgi->minValue().toDouble());
        y->setMaximum(hgi->maxValue().toDouble());
    }

    y->setSingleStep(1.0);
    y->setValue(PLEConfigSkeleton::verticalGrid());
    connect(skeleton, SIGNAL(verticalGridChanged(double)), y, SLOT(setValue(double)));
    layout->addRow(i18n("Vertical distance"), y);

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);
    layout->addRow(buttons);

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    connect(buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(reject()));
}

void GridSetupDialog::setHorizontalDistance(qreal value)
{
    x->setValue(value);
}

void GridSetupDialog::setVerticalDistance(qreal value)
{
    y->setValue(value);
}

qreal GridSetupDialog::horizontalDistance() const
{
    return x->value();
}

qreal GridSetupDialog::verticalDistance() const
{
    return y->value();
}

int GridSetupDialog::exec()
{
    int result = QDialog::exec();
    if (result == Accepted)
    {
        PLEConfigSkeleton::setHorizontalGrid( this->horizontalDistance() );
        PLEConfigSkeleton::setVerticalGrid( this->verticalDistance() );
        PLEConfigSkeleton::self()->save();
    }
    return result;
}

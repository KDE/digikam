/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "PLEConfigViewWidget.h"
#include "PLEConfigSkeleton.h"

#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>

#include <kconfigdialog.h>
#include <klocalizedstring.h>

using namespace PhotoLayoutsEditor;

class PhotoLayoutsEditor::PLEConfigViewWidget::PLEConfigViewWidgetPrivate
{
    QCheckBox      * antialiasing;
    QDoubleSpinBox * xGrid;
    QDoubleSpinBox * yGrid;
    QCheckBox      * showGrid;

    friend class PLEConfigViewWidget;
};

PLEConfigViewWidget::PLEConfigViewWidget(QWidget * parent, const QString & caption) :
    QWidget(parent),
    d(new PLEConfigViewWidgetPrivate)
{
    this->setupGUI();
    this->setWindowTitle(caption);
}

PLEConfigViewWidget::~PLEConfigViewWidget()
{
    delete d;
}

void PLEConfigViewWidget::updateSettings()
{
    PLEConfigSkeleton::setAntialiasing(d->antialiasing->isChecked());
    PLEConfigSkeleton::setShowGrid(d->showGrid->isChecked());
    PLEConfigSkeleton::setHorizontalGrid(d->xGrid->value());
    PLEConfigSkeleton::setVerticalGrid(d->yGrid->value());
    PLEConfigSkeleton::self()->save();
}

void PLEConfigViewWidget::updateWidgets()
{
    d->antialiasing->setChecked( PLEConfigSkeleton::antialiasing() );
    d->showGrid->setChecked(PLEConfigSkeleton::showGrid());
    d->xGrid->setValue(PLEConfigSkeleton::horizontalGrid());
    d->yGrid->setValue(PLEConfigSkeleton::verticalGrid());
}

void PLEConfigViewWidget::setupGUI()
{
    QVBoxLayout * layout = new QVBoxLayout();
    this->setLayout(layout);

    PLEConfigSkeleton * skeleton = PLEConfigSkeleton::self();

    QFormLayout * generalLayout = new QFormLayout();
    layout->addLayout(generalLayout);
    d->antialiasing = new QCheckBox(this);
    connect(skeleton, SIGNAL(antialiasingChanged(bool)), d->antialiasing, SLOT(setChecked(bool)));
    generalLayout->addRow(i18n("Antialiasing"), d->antialiasing);

    QGroupBox * gridBox = new QGroupBox(i18n("Grid"), this);
    layout->addWidget(gridBox);
    QFormLayout * gridLayout = new QFormLayout();
    gridBox->setLayout(gridLayout);

    d->showGrid = new QCheckBox(gridBox);
    connect(skeleton, SIGNAL(showGridChanged(bool)), d->showGrid, SLOT(setChecked(bool)));
    gridLayout->addRow(i18n("Show grid lines"), d->showGrid);

    d->xGrid = new QDoubleSpinBox(gridBox);
    KConfigSkeletonItem * hgi = skeleton->findItem(QLatin1String("horizontalGrid"));
    if (hgi)
    {
        d->xGrid->setMinimum(hgi->minValue().toDouble());
        d->xGrid->setMaximum(hgi->maxValue().toDouble());
    }
    d->xGrid->setSingleStep(1.0);
    connect(skeleton, SIGNAL(horizontalGridChanged(double)), d->xGrid, SLOT(setValue(double)));
    gridLayout->addRow(i18n("Horizontal distance"), d->xGrid);

    d->yGrid = new QDoubleSpinBox(gridBox);
    KConfigSkeletonItem * vgi = skeleton->findItem(QLatin1String("verticalGrid"));
    if (hgi)
    {
        d->yGrid->setMinimum(vgi->minValue().toDouble());
        d->yGrid->setMaximum(vgi->maxValue().toDouble());
    }
    d->yGrid->setSingleStep(1.0);
    connect(skeleton, SIGNAL(verticalGridChanged(double)), d->yGrid, SLOT(setValue(double)));
    gridLayout->addRow(i18n("Vertical distance"), d->yGrid);

//    KConfigDialog * dialog = KConfigDialog::exists( "settings" );
//    qCDebug(DIGIKAM_GENERAL_LOG) << dialog;
//    if (dialog)
//    {
//        connect(d->antialiasing, SIGNAL(stateChanged(int)), dialog, SLOT(updateButtons()));
//        connect(d->showGrid, SIGNAL(stateChanged(int)), dialog, SLOT(updateButtons()));
//        connect(d->xGrid, SIGNAL(valueChanged(double)), dialog, SLOT(updateButtons()));
//        connect(d->yGrid, SIGNAL(valueChanged(double)), dialog, SLOT(updateButtons()));
//        connect(d->antialiasing, SIGNAL(stateChanged(int)), dialog, SLOT(settingsChangedSlot()));
//        connect(d->showGrid, SIGNAL(stateChanged(int)), dialog, SLOT(settingsChangedSlot()));
//        connect(d->xGrid, SIGNAL(valueChanged(double)), dialog, SLOT(settingsChangedSlot()));
//        connect(d->yGrid, SIGNAL(valueChanged(double)), dialog, SLOT(settingsChangedSlot()));
//    }

    this->updateWidgets();
}


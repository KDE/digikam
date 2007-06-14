/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : a zoom bar used in status bar.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qtoolbutton.h>
#include <qtimer.h>
#include <qslider.h>
#include <qtooltip.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "thumbnailsize.h"
#include "dcursortracker.h"
#include "statuszoombar.h"
#include "statuszoombar.moc"

namespace Digikam
{

class StatusZoomBarPriv
{

public:

    StatusZoomBarPriv()
    {
        zoomTracker     = 0;
        zoomMinusButton = 0;
        zoomPlusButton  = 0;
        zoomSlider      = 0;
        zoomTimer       = 0;
    }

    QToolButton *zoomPlusButton;
    QToolButton *zoomMinusButton;

    QTimer      *zoomTimer;

    QSlider     *zoomSlider;

    DTipTracker *zoomTracker;
};

StatusZoomBar::StatusZoomBar(QWidget *parent)
             : Q3HBox(parent, 0, Qt::WDestructiveClose)
{
    d = new StatusZoomBarPriv;

    d->zoomMinusButton = new QToolButton(this);
    d->zoomMinusButton->setAutoRaise(true);
    d->zoomMinusButton->setIconSet(SmallIconSet("viewmag-"));
    QToolTip::add(d->zoomMinusButton, i18n("Zoom Out"));

    d->zoomSlider = new QSlider(ThumbnailSize::Small, ThumbnailSize::Huge,
                                ThumbnailSize::Step, ThumbnailSize::Medium, 
                                Qt::Horizontal, this);
    d->zoomSlider->setLineStep(ThumbnailSize::Step);
    d->zoomSlider->setMaximumHeight(fontMetrics().height()+2);    
    d->zoomSlider->setFixedWidth(120);

    d->zoomPlusButton = new QToolButton(this);
    d->zoomPlusButton->setAutoRaise(true);
    d->zoomPlusButton->setIconSet(SmallIconSet("viewmag+"));
    QToolTip::add(d->zoomPlusButton, i18n("Zoom In"));

    d->zoomTracker = new DTipTracker("", d->zoomSlider);

    // -------------------------------------------------------------

    connect(d->zoomMinusButton, SIGNAL(clicked()),
            this, SIGNAL(signalZoomMinusClicked()));

    connect(d->zoomPlusButton, SIGNAL(clicked()),
            this, SIGNAL(signalZoomPlusClicked()));

    connect(d->zoomSlider, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalZoomSliderChanged(int)));

    connect(d->zoomSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomSlider, SIGNAL(sliderReleased()),
            this, SLOT(slotZoomSliderReleased()));
}

StatusZoomBar::~StatusZoomBar()
{
    if (d->zoomTimer)
        delete d->zoomTimer;

    delete d->zoomTracker;
    delete d;
}

void StatusZoomBar::slotZoomSliderChanged(int)
{
    if (d->zoomTimer)
    {
        d->zoomTimer->stop();
        delete d->zoomTimer;
    }

    d->zoomTimer = new QTimer( this );
    connect(d->zoomTimer, SIGNAL(timeout()),
            this, SLOT(slotDelayedZoomSliderChanged()) );
    d->zoomTimer->start(300, true);    
}

void StatusZoomBar::slotDelayedZoomSliderChanged()
{
    emit signalDelayedZoomSliderChanged(d->zoomSlider->value());
}

void StatusZoomBar::slotZoomSliderReleased()
{
    emit signalZoomSliderReleased(d->zoomSlider->value());
}

void StatusZoomBar::setZoomSliderValue(int v)
{
    d->zoomSlider->blockSignals(true);
    d->zoomSlider->setValue(v);
    d->zoomSlider->blockSignals(false);
}

void StatusZoomBar::setZoomTrackerText(const QString& text)
{
    d->zoomTracker->setText(text);
}

void StatusZoomBar::setEnableZoomPlus(bool e)
{
    d->zoomPlusButton->setEnabled(e);
}

void StatusZoomBar::setEnableZoomMinus(bool e)
{
    d->zoomMinusButton->setEnabled(e);
}

}  // namespace Digikam


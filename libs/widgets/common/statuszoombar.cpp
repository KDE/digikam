/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : a zoom bar used in status bar.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// #include "statuszoombar.h"
#include "statuszoombar.moc"

// Qt includes

#include <QLayout>
#include <QSlider>
#include <QTimer>
#include <QToolButton>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>

// Local includes

#include "dcursortracker.h"
#include "thumbnailsize.h"

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
             : KHBox(parent), d(new StatusZoomBarPriv)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->zoomMinusButton = new QToolButton(this);
    d->zoomMinusButton->setAutoRaise(true);
    d->zoomMinusButton->setFocusPolicy(Qt::NoFocus);
    d->zoomMinusButton->setIcon(SmallIcon("zoom-out"));
    d->zoomMinusButton->setToolTip( i18n("Zoom Out"));

    d->zoomSlider = new QSlider(Qt::Horizontal, this);
    d->zoomSlider->setRange(ThumbnailSize::Small, ThumbnailSize::Huge);
    d->zoomSlider->setSingleStep(ThumbnailSize::Step);
    d->zoomSlider->setValue(ThumbnailSize::Medium);
    d->zoomSlider->setFixedWidth(120);
    d->zoomSlider->setFocusPolicy(Qt::NoFocus);
    d->zoomSlider->setInvertedControls(true);       // See B.K.O #161087

    d->zoomPlusButton = new QToolButton(this);
    d->zoomPlusButton->setAutoRaise(true);
    d->zoomPlusButton->setIcon(SmallIcon("zoom-in"));
    d->zoomPlusButton->setToolTip( i18n("Zoom In"));
    d->zoomPlusButton->setFocusPolicy(Qt::NoFocus);

    d->zoomTracker = new DTipTracker("", d->zoomSlider);

    layout()->setMargin(0);
    layout()->setSpacing(0);

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
    d->zoomTimer->setSingleShot(true);
    d->zoomTimer->start(300);
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

void StatusZoomBar::triggerZoomTrackerToolTip()
{
    d->zoomTracker->triggerAutoShow();
}

void StatusZoomBar::setEnableZoomPlus(bool e)
{
    d->zoomPlusButton->setEnabled(e);
}

void StatusZoomBar::setEnableZoomMinus(bool e)
{
    d->zoomMinusButton->setEnabled(e);
}

void StatusZoomBar::slotUpdateTrackerPos()
{
    d->zoomTracker->refresh();
}

}  // namespace Digikam

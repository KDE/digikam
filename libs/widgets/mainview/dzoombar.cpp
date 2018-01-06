/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-04-15
 * Description : a zoom bar used in status bar.
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dzoombar.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QLineEdit>
#include <QAction>
#include <QLayout>
#include <QSlider>
#include <QTimer>
#include <QToolButton>
#include <QList>
#include <QComboBox>
#include <QLocale>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "digikam_debug.h"
#include "dcursortracker.h"
#include "thumbnailsize.h"

namespace Digikam
{

class DZoomBar::Private
{

public:

    Private()
    {
        zoomToFitButton = 0;
        zoomTo100Button = 0;
        zoomTracker     = 0;
        zoomMinusButton = 0;
        zoomPlusButton  = 0;
        zoomSlider      = 0;
        zoomTimer       = 0;
        zoomCombo       = 0;
    }

    QToolButton*    zoomToFitButton;
    QToolButton*    zoomTo100Button;
    QToolButton*    zoomPlusButton;
    QToolButton*    zoomMinusButton;

    QTimer*         zoomTimer;

    QSlider*        zoomSlider;

    QComboBox*      zoomCombo;

    DCursorTracker* zoomTracker;
};

DZoomBar::DZoomBar(QWidget* const parent)
    : DHBox(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->zoomToFitButton = new QToolButton(this);
    d->zoomToFitButton->setAutoRaise(true);
    d->zoomToFitButton->setFocusPolicy(Qt::NoFocus);

    d->zoomTo100Button = new QToolButton(this);
    d->zoomTo100Button->setAutoRaise(true);
    d->zoomTo100Button->setFocusPolicy(Qt::NoFocus);

    d->zoomMinusButton = new QToolButton(this);
    d->zoomMinusButton->setAutoRaise(true);
    d->zoomMinusButton->setFocusPolicy(Qt::NoFocus);

    d->zoomSlider  = new QSlider(Qt::Horizontal, this);
    d->zoomTracker = new DCursorTracker(QLatin1String(""), d->zoomSlider);
    d->zoomSlider->setRange(ThumbnailSize::Small, ThumbnailSize::maxThumbsSize());
    d->zoomSlider->setSingleStep(ThumbnailSize::Step);
    d->zoomSlider->setValue(ThumbnailSize::Medium);
    d->zoomSlider->setFixedWidth(120);
    d->zoomSlider->setFocusPolicy(Qt::NoFocus);
    d->zoomSlider->setInvertedControls(true);       // See bug #161087

    d->zoomPlusButton = new QToolButton(this);
    d->zoomPlusButton->setAutoRaise(true);
    d->zoomPlusButton->setFocusPolicy(Qt::NoFocus);

    d->zoomCombo = new QComboBox(this);
    d->zoomCombo->setEditable(true);
    d->zoomCombo->setDuplicatesEnabled(false);
    d->zoomCombo->setFocusPolicy(Qt::ClickFocus);
    d->zoomCombo->setInsertPolicy(QComboBox::NoInsert);

    QList<double> zoomLevels;
    zoomLevels << 10.0;
    zoomLevels << 25.0;
    zoomLevels << 50.0;
    zoomLevels << 75.0;
    zoomLevels << 100.0;
    zoomLevels << 150.0;
    zoomLevels << 200.0;
    zoomLevels << 300.0;
    zoomLevels << 450.0;
    zoomLevels << 600.0;
    zoomLevels << 800.0;
    zoomLevels << 1200.0;

    foreach(const double zoom, zoomLevels)
    {
        d->zoomCombo->addItem(QString::fromLatin1("%1%").arg((int)zoom), QVariant(zoom));
    }

    layout()->setContentsMargins(QMargins());
    layout()->setSpacing(0);

    // -------------------------------------------------------------

    connect(d->zoomSlider, SIGNAL(valueChanged(int)),
            this, SIGNAL(signalZoomSliderChanged(int)));

    connect(d->zoomSlider, SIGNAL(valueChanged(int)),
            this, SLOT(slotZoomSliderChanged(int)));

    connect(d->zoomSlider, SIGNAL(sliderReleased()),
            this, SLOT(slotZoomSliderReleased()));

    connect(d->zoomCombo, SIGNAL(activated(int)),
            this, SLOT(slotZoomSelected(int)));

    connect(d->zoomCombo->lineEdit(), SIGNAL(returnPressed()),
            this, SLOT(slotZoomTextChanged()));

    // -------------------------------------------------------------

    setBarMode(PreviewZoomCtrl);
}

DZoomBar::~DZoomBar()
{
    delete d->zoomTimer;
    delete d;
}

void DZoomBar::setZoomToFitAction(QAction* const action)
{
    d->zoomToFitButton->setDefaultAction(action);
}

void DZoomBar::setZoomTo100Action(QAction* const action)
{
    d->zoomTo100Button->setDefaultAction(action);
}

void DZoomBar::setZoomPlusAction(QAction* const action)
{
    d->zoomPlusButton->setDefaultAction(action);
}

void DZoomBar::setZoomMinusAction(QAction* const action)
{
    d->zoomMinusButton->setDefaultAction(action);
}

void DZoomBar::slotZoomSliderChanged(int)
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

void DZoomBar::slotDelayedZoomSliderChanged()
{
    emit signalDelayedZoomSliderChanged(d->zoomSlider->value());
}

void DZoomBar::slotZoomSliderReleased()
{
    emit signalZoomSliderReleased(d->zoomSlider->value());
}

void DZoomBar::setZoom(double zoom, double zmin, double zmax)
{
    int size = sizeFromZoom(zoom, zmin, zmax);

    d->zoomSlider->blockSignals(true);
    d->zoomSlider->setValue(size);
    d->zoomSlider->blockSignals(false);

    QString ztxt = QString::number(lround(zoom*100.0)) + QLatin1String("%");
    d->zoomCombo->blockSignals(true);
    d->zoomCombo->setCurrentIndex(-1);
    d->zoomCombo->setEditText(ztxt);
    d->zoomCombo->blockSignals(false);
}

void DZoomBar::setThumbsSize(int size)
{
    d->zoomSlider->blockSignals(true);
    d->zoomSlider->setValue(size);
    d->zoomSlider->blockSignals(false);

    d->zoomTracker->setText(i18n("Size: %1", size));
    triggerZoomTrackerToolTip();
}

int DZoomBar::sizeFromZoom(double zoom, double zmin, double zmax)
{
    double h = (double)ThumbnailSize::maxThumbsSize();
    double s = (double)ThumbnailSize::Small;
    double zoomN = log(zoom)/log(2);
    double zminN = log(zmin)/log(2);
    double zmaxN = log(zmax)/log(2);
    double zval = (zoomN-zminN)/(zmaxN-zminN);
    return (int)(zval*(h - s) + s);
}

double DZoomBar::zoomFromSize(int size, double zmin, double zmax)
{
    double h = (double)ThumbnailSize::maxThumbsSize();
    double s = (double)ThumbnailSize::Small;
    double zminN = log(zmin)/log(2);
    double zmaxN = log(zmax)/log(2);
    double zval = (size - s)/(h - s);
    return pow(2, zval*(zmaxN-zminN) + zminN);
}

void DZoomBar::triggerZoomTrackerToolTip()
{
    d->zoomTracker->triggerAutoShow();
}

void DZoomBar::slotUpdateTrackerPos()
{
    d->zoomTracker->refresh();
}

void DZoomBar::slotZoomSelected(int index)
{
    bool ok     = false;
    double zoom = d->zoomCombo->itemData(index).toDouble(&ok) / 100.0;

    if (ok && zoom > 0.0)
    {
        emit signalZoomValueEdited(zoom);
    }
}

void DZoomBar::slotZoomTextChanged()
{
    QString txt = d->zoomCombo->currentText();
    bool ok     = false;
    double zoom = QLocale().toDouble(txt, &ok) / 100.0;

    if (ok && zoom > 0.0 && zoom <= 48.0)
    {
        emit signalZoomValueEdited(zoom);
    }
}

void DZoomBar::setBarMode(BarMode mode)
{
    QAction* const zfitAction = d->zoomToFitButton->defaultAction();
    QAction* const z100Action = d->zoomTo100Button->defaultAction();

    switch (mode)
    {
        case PreviewZoomCtrl:
        {
            d->zoomToFitButton->show();

            if (zfitAction)
            {
                zfitAction->setEnabled(true);
            }

            d->zoomTo100Button->show();

            if (z100Action)
            {
                z100Action->setEnabled(true);
            }

            d->zoomCombo->show();
            d->zoomCombo->setEnabled(true);
            d->zoomTracker->setEnable(false);
            break;
        }
        case ThumbsSizeCtrl:
        {
            d->zoomToFitButton->show();

            if (zfitAction)
            {
                zfitAction->setEnabled(true);
            }

            d->zoomTo100Button->show();

            if (z100Action)
            {
                z100Action->setEnabled(false);
            }

            d->zoomCombo->show();
            d->zoomCombo->setEnabled(false);
            d->zoomTracker->setEnable(true);
            break;
        }
        default:   // NoPreviewZoomCtrl
        {
            d->zoomToFitButton->hide();
            d->zoomTo100Button->hide();
            d->zoomCombo->hide();
            d->zoomTracker->setEnable(true);
            break;
        }
    }
}

} // namespace Digikam

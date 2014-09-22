/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow OSD widget.
 *
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slideosd.moc"

// Qt includes

#include <QLayout>
#include <QDesktopWidget>
#include <QEvent>

// KDE includes

#include <kdebug.h>
#include <kapplication.h>
#include <khbox.h>

// Local includes

#include "slideshow.h"
#include "slideinfowidget.h"
#include "ratingwidget.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"

namespace Digikam
{

class SlideOSD::Private
{

public:

    Private() :
        labelsBox(0),
        parent(0),
        slideInfo(0),
        ratingWidget(0),
        clWidget(0),
        plWidget(0)

    {
    }

    KHBox*              labelsBox;

    SlideShow*          parent;
    SlideInfoWidget*    slideInfo;
    RatingWidget*       ratingWidget;
    ColorLabelSelector* clWidget;
    PickLabelSelector*  plWidget;
    SlideShowSettings   settings;
};

SlideOSD::SlideOSD(const SlideShowSettings& settings, SlideShow* const parent)
    : QWidget(parent),
      d(new Private)
{
    Qt::WindowFlags flags = Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint |
                            Qt::X11BypassWindowManagerHint;

    setWindowFlags(flags);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_X11NetWmWindowTypeNotification, true);
    setAttribute(Qt::WA_ShowWithoutActivating, true);

#ifdef Q_OS_WIN32
    // Don't show the window in the taskbar.  Qt::ToolTip does this too, but it
    // adds an extra ugly shadow.
    int ex_style = GetWindowLong(winId(), GWL_EXSTYLE);
    ex_style |= WS_EX_NOACTIVATE;
    SetWindowLong(winId(), GWL_EXSTYLE, ex_style);
#endif

    QGridLayout* const grid = new QGridLayout(this);
    d->slideInfo            = new SlideInfoWidget(settings, this);
    d->settings             = settings;
    d->parent               = parent;

    // ---------------------------------------------------------------

    d->labelsBox    = new KHBox(this);
    d->clWidget     = new ColorLabelSelector(d->labelsBox);
    d->clWidget->installEventFilter(this);
    d->clWidget->colorLabelWidget()->installEventFilter(this);
    d->plWidget     = new PickLabelSelector(d->labelsBox);
    d->plWidget->installEventFilter(this);
    d->plWidget->pickLabelWidget()->installEventFilter(this);
    d->ratingWidget = new RatingWidget(d->labelsBox);
    d->ratingWidget->setTracking(false);
    d->ratingWidget->setFading(false);
    d->ratingWidget->installEventFilter(this);
    QWidget* const space = new QWidget(d->labelsBox);
    d->labelsBox->setVisible(false);
    d->labelsBox->layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignLeft);
    d->labelsBox->setStretchFactor(space, 10);

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            parent, SLOT(slotAssignRating(int)));

    connect(d->clWidget, SIGNAL(signalColorLabelChanged(int)),
            parent, SLOT(slotAssignColorLabel(int)));

    connect(d->plWidget, SIGNAL(signalPickLabelChanged(int)),
            parent, SLOT(slotAssignPickLabel(int)));

    // ---------------------------------------------------------------

    grid->addWidget(d->slideInfo, 1, 0, 1, 1);
    grid->addWidget(d->labelsBox, 2, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setColumnStretch(0, 10);
    grid->setMargin(0);
}

SlideOSD::~SlideOSD()
{
    delete d;
}

void SlideOSD::setCurrentInfo(const SlidePictureInfo& info, const KUrl& url)
{
    // Update info text.

    d->slideInfo->setCurrentInfo(info, url);

    // Display Labels.

    d->labelsBox->setVisible(d->settings.printLabels);

    if (d->settings.printLabels)
    {
        d->ratingWidget->blockSignals(true);
        d->clWidget->blockSignals(true);
        d->plWidget->blockSignals(true);
        d->ratingWidget->setRating(info.rating);
        d->clWidget->setColorLabel((ColorLabel)info.colorLabel);
        d->plWidget->setPickLabel((PickLabel)info.pickLabel);
        d->ratingWidget->blockSignals(false);
        d->clWidget->blockSignals(false);
        d->plWidget->blockSignals(false);
    }

    reposition();
}

void SlideOSD::reposition()
{
    // Make the OSD the proper size
    layout()->activate();
    resize(sizeHint());

    QRect geometry(QApplication::desktop()->availableGeometry(parentWidget()));

    move(10, geometry.bottom() - height());

    show();
}

bool SlideOSD::eventFilter(QObject* obj, QEvent* ev)
{
    if (obj == d->ratingWidget                 ||
        obj == d->clWidget                     ||
        obj == d->plWidget                     ||
        obj == d->clWidget->colorLabelWidget() ||
        obj == d->plWidget->pickLabelWidget())
    {
        if (ev->type() == QEvent::Enter)
        {
            d->parent->setPaused(true);
            return false;
        }

        if (ev->type() == QEvent::Leave)
        {
            d->parent->setPaused(false);
            return false;
        }
    }

    // pass the event on to the parent class
    return QWidget::eventFilter(obj, ev);
}

}  // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-18
 * Description : slideshow OSD widget
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

#include <QTimer>
#include <QLayout>
#include <QDesktopWidget>
#include <QEvent>
#include <QProgressBar>

// KDE includes

#include <kdebug.h>
#include <kapplication.h>
#include <khbox.h>
#include <kdialog.h>

// Local includes

#include "slideshow.h"
#include "slidetoolbar.h"
#include "slideproperties.h"
#include "ratingwidget.h"
#include "colorlabelwidget.h"
#include "picklabelwidget.h"

namespace Digikam
{

class SlideOSD::Private
{

public:

    Private() :
        blink(false),
        delay(500),         // Progress bar refresh timer in ms
        progressBar(0),
        progressTimer(0),
        timer(0),           // Slide timer
        labelsBox(0),
        parent(0),
        slideProps(0),
        toolBar(0),
        ratingWidget(0),
        clWidget(0),
        plWidget(0)

    {
    }

    bool                blink;
    int const           delay;

    QProgressBar*       progressBar;
    QTimer*             progressTimer;
    QTimer*             timer;

    KHBox*              labelsBox;
    KHBox*              progressBox;

    SlideShow*          parent;
    SlideProperties*    slideProps;
    SlideToolBar*       toolBar;
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
    ex_style    |= WS_EX_NOACTIVATE;
    SetWindowLong(winId(), GWL_EXSTYLE, ex_style);
#endif

    d->slideProps = new SlideProperties(settings, this);
    d->settings   = settings;
    d->parent     = parent;

    // ---------------------------------------------------------------

    d->labelsBox    = new KHBox(this);
    d->clWidget     = new ColorLabelSelector(d->labelsBox);
    d->clWidget->installEventFilter(this);
    d->clWidget->colorLabelWidget()->installEventFilter(this);
    d->clWidget->setFocusPolicy(Qt::NoFocus);
    d->plWidget     = new PickLabelSelector(d->labelsBox);
    d->plWidget->installEventFilter(this);
    d->plWidget->setFocusPolicy(Qt::NoFocus);
    d->plWidget->pickLabelWidget()->installEventFilter(this);
    d->ratingWidget = new RatingWidget(d->labelsBox);
    d->ratingWidget->setTracking(false);
    d->ratingWidget->setFading(false);
    d->ratingWidget->installEventFilter(this);
    d->ratingWidget->setFocusPolicy(Qt::NoFocus);
    d->labelsBox->layout()->setAlignment(d->ratingWidget, Qt::AlignVCenter | Qt::AlignLeft);
    d->labelsBox->setVisible(d->settings.printLabels);

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            parent, SLOT(slotAssignRating(int)));

    connect(d->clWidget, SIGNAL(signalColorLabelChanged(int)),
            parent, SLOT(slotAssignColorLabel(int)));

    connect(d->plWidget, SIGNAL(signalPickLabelChanged(int)),
            parent, SLOT(slotAssignPickLabel(int)));

    // ---------------------------------------------------------------

    d->progressBox   = new KHBox(this);
    d->progressBox->setVisible(d->settings.showProgressIndicator);

    d->progressBar   = new QProgressBar(d->progressBox);
    d->progressBar->setMinimum(0);
    d->progressBar->setMaximum(d->settings.delay*(1000/d->delay));
    d->progressBar->setFocusPolicy(Qt::NoFocus);

    d->toolBar       = new SlideToolBar(d->progressBox);
    d->toolBar->setEnabledPrev(!d->settings.loop);

    connect(d->toolBar, SIGNAL(signalPause()),
            this, SLOT(slotPause()));

    connect(d->toolBar, SIGNAL(signalPlay()),
            this, SLOT(slotPlay()));

    connect(d->toolBar, SIGNAL(signalNext()),
            d->parent, SLOT(slotLoadNextItem()));

    connect(d->toolBar, SIGNAL(signalPrev()),
            d->parent, SLOT(slotLoadPrevItem()));

    connect(d->toolBar, SIGNAL(signalClose()),
            d->parent, SLOT(close()));

    // ---------------------------------------------------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->slideProps,  1, 0, 1, 2);
    grid->addWidget(d->labelsBox,   2, 0, 1, 1);
    grid->addWidget(d->progressBox, 3, 0, 1, 1);
    grid->setRowStretch(0, 10);
    grid->setColumnStretch(1, 10);
    grid->setSpacing(KDialog::spacingHint());
    grid->setMargin(0);

    // ---------------------------------------------------------------

    d->progressTimer = new QTimer(this);
    d->progressTimer->setSingleShot(false);

    connect(d->progressTimer, SIGNAL(timeout()),
            this, SLOT(slotTimer()));

    d->timer         = new QTimer(this);

    connect(d->timer, SIGNAL(timeout()),
            d->parent, SLOT(slotLoadNextItem()));

    d->timer->setSingleShot(true);
    d->timer->start(10);
}

SlideOSD::~SlideOSD()
{
    d->timer->stop();
    d->progressTimer->stop();

    delete d->timer;
    delete d->progressTimer;

    delete d;
}

SlideToolBar* SlideOSD::toolBar() const
{
    return d->toolBar;
}

void SlideOSD::setCurrentInfo(const SlidePictureInfo& info, const KUrl& url)
{
    // Update info text.

    d->slideProps->setCurrentInfo(info, url);

    // Display Labels.

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
    if (obj == d->labelsBox                    ||
        obj == d->ratingWidget                 ||
        obj == d->clWidget                     ||
        obj == d->plWidget                     ||
        obj == d->clWidget->colorLabelWidget() ||
        obj == d->plWidget->pickLabelWidget())
    {
        if (ev->type() == QEvent::Enter)
        {
            pause(true);
            return false;
        }

        if (ev->type() == QEvent::Leave)
        {
            pause(false);
            return false;
        }
    }

    // pass the event on to the parent class
    return QWidget::eventFilter(obj, ev);
}

void SlideOSD::slotTimer()
{
    QString str = QString("(%1/%2)")
                    .arg(QString::number(d->settings.fileList.indexOf(d->parent->currentItem()) + 1))
                    .arg(QString::number(d->settings.fileList.count()));

    if (d->toolBar->isPaused())
    {
        d->blink = !d->blink;

        if (d->blink)
            str = QString();

        d->progressBar->setFormat(str);
    }
    else
    {
        d->progressBar->setFormat(str);
        d->progressBar->setValue(d->progressBar->value()-1);
    }
}

void SlideOSD::pause(bool b)
{
    d->toolBar->pause(b);

    if (b)
    {
        d->timer->stop();
    }
    else
    {
        d->progressBar->setValue(d->settings.delay*(1000/d->delay));
        d->progressTimer->start(d->delay);
        d->timer->setSingleShot(true);
        d->timer->start(d->settings.delay * 1000);
    }
}

bool SlideOSD::isPaused() const
{
    return d->toolBar->isPaused();
}

void SlideOSD::slotPause()
{
    pause(true);
}

void SlideOSD::slotPlay()
{
    pause(false);
}
}  // namespace Digikam

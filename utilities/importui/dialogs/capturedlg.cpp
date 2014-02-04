/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-06
 * Description : a dialog to control camera capture.
 *
 * Copyright (C) 2007-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "capturedlg.moc"

// Qt includes

#include <QTimer>

// KDE includes

#include <kconfig.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "cameracontroller.h"
#include "capturewidget.h"

namespace Digikam
{

class CaptureDlg::Private
{
public:

    Private() :
        stopPreview(false),
        timer(0),
        controller(0),
        captureWidget(0)
    {
    }

    bool              stopPreview;

    QTimer*           timer;

    CameraController* controller;

    CaptureWidget*    captureWidget;
};

CaptureDlg::CaptureDlg(QWidget* const parent, CameraController* const controller,
                       const QString& cameraTitle)
    : KDialog(parent), d(new Private)
{
    d->controller = controller;
    setCaption(i18nc("@title:window %1: name of the camera", "Capture from %1", cameraTitle));
    setButtons(Help | Cancel | Ok);
    setDefaultButton(Ok);
    setButtonText(Ok, i18nc("@action:button", "Capture"));
    setModal(true);
    setHelp("camerainterface.anchor", "digikam");

    d->captureWidget = new CaptureWidget(this);
    setMainWidget(d->captureWidget);
    restoreDialogSize(KGlobal::config()->group("Capture Tool Dialog"));


    // -------------------------------------------------------------

    connect(this, SIGNAL(cancelClicked()),
            this, SLOT(slotCancel()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotCapture()));

    connect(d->controller, SIGNAL(signalPreview(QImage)),
            this, SLOT(slotPreviewDone(QImage)));

    // -------------------------------------------------------------

    if(d->controller->cameraCaptureImagePreviewSupport()) {
        d->timer = new QTimer(this);

        connect( d->timer, SIGNAL(timeout()),
                this, SLOT(slotPreview()) );

        d->timer->setSingleShot(true);

        d->timer->start(0);
    }
}

CaptureDlg::~CaptureDlg()
{
    delete d->timer; // TODO is there a need to call this even separately? As parent is set to this widget in any case, so it should be destroyed?
    delete d;
}

void CaptureDlg::closeEvent(QCloseEvent* e)
{
    d->stopPreview = true;
    if(d->timer) {
        d->timer->stop();
    }
    KConfigGroup group = KGlobal::config()->group("Capture Tool Dialog");
    saveDialogSize(group);
    e->accept();
}

void CaptureDlg::slotCancel()
{
    d->stopPreview = true;
    if(d->timer) {
        d->timer->stop();
    }
    KConfigGroup group = KGlobal::config()->group("Capture Tool Dialog");
    saveDialogSize(group);
    done(Cancel);
}

void CaptureDlg::slotPreview()
{
    d->controller->getPreview();
}

void CaptureDlg::slotCapture()
{
    d->stopPreview = true;
    if(d->timer) {
        d->timer->stop();
    }

    disconnect(d->controller, SIGNAL(signalPreview(QImage)),
               this, SLOT(slotPreviewDone(QImage)));

    KConfigGroup group = KGlobal::config()->group("Capture Tool Dialog");
    saveDialogSize(group);
    d->controller->capture();
    done(Ok);
}

void CaptureDlg::slotPreviewDone(const QImage& preview)
{
    d->captureWidget->setPreview(preview);

    if (!d->stopPreview && d->timer)
    {
        d->timer->start(0);
    }
}

}  // namespace Digikam

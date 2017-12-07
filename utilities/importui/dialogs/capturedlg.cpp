/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-06
 * Description : a dialog to control camera capture.
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

#include "capturedlg.h"

// Qt includes

#include <QTimer>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>
#include <ksharedconfig.h>

// Local includes

#include "cameracontroller.h"
#include "capturewidget.h"
#include "dxmlguiwindow.h"

namespace Digikam
{

class CaptureDlg::Private
{
public:

    Private() :
        stopPreview(false),
        timer(0),
        buttons(0),
        controller(0),
        captureWidget(0)
    {
    }

    bool              stopPreview;

    QTimer*           timer;
    QDialogButtonBox* buttons;

    CameraController* controller;

    CaptureWidget*    captureWidget;
};

CaptureDlg::CaptureDlg(QWidget* const parent, CameraController* const controller,
                       const QString& cameraTitle)
    : QDialog(parent),
      d(new Private)
{
    d->controller = controller;

    setWindowTitle(i18nc("@title:window %1: name of the camera", "Capture from %1", cameraTitle));
    setModal(true);

    d->buttons = new QDialogButtonBox(QDialogButtonBox::Help | QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    d->buttons->button(QDialogButtonBox::Cancel)->setDefault(true);
    d->buttons->button(QDialogButtonBox::Ok)->setText(i18nc("@action:button", "Capture"));

    d->captureWidget = new CaptureWidget(this);

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(d->captureWidget);
    vbx->addWidget(d->buttons);
    setLayout(vbx);

    KConfigGroup group = KSharedConfig::openConfig()->group("Capture Tool Dialog");

    winId();
    DXmlGuiWindow::restoreWindowSize(windowHandle(), group);
    resize(windowHandle()->size());

    // -------------------------------------------------------------

    connect(d->buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(slotCapture()));

    connect(d->buttons->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
            this, SLOT(slotCancel()));

    connect(d->buttons->button(QDialogButtonBox::Help), SIGNAL(clicked()),
            this, SLOT(slotHelp()));

    connect(d->controller, SIGNAL(signalPreview(QImage)),
            this, SLOT(slotPreviewDone(QImage)));

    // -------------------------------------------------------------

    if (d->controller->cameraCaptureImagePreviewSupport())
    {
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

    if (d->timer)
    {
        d->timer->stop();
    }

    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Capture Tool Dialog"));
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    e->accept();
}

void CaptureDlg::slotCancel()
{
    d->stopPreview = true;

    if (d->timer)
    {
        d->timer->stop();
    }

    KConfigGroup group = KSharedConfig::openConfig()->group(QLatin1String("Capture Tool Dialog"));
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);

    reject();
}

void CaptureDlg::slotPreview()
{
    d->controller->getPreview();
}

void CaptureDlg::slotCapture()
{
    d->stopPreview = true;

    if (d->timer)
    {
        d->timer->stop();
    }

    disconnect(d->controller, SIGNAL(signalPreview(QImage)),
               this, SLOT(slotPreviewDone(QImage)));

    KConfigGroup group = KSharedConfig::openConfig()->group("Capture Tool Dialog");
    DXmlGuiWindow::saveWindowSize(windowHandle(), group);
    d->controller->capture();

    accept();
}

void CaptureDlg::slotPreviewDone(const QImage& preview)
{
    d->captureWidget->setPreview(preview);

    if (!d->stopPreview && d->timer)
    {
        d->timer->start(0);
    }
}

void CaptureDlg::slotHelp()
{
    DXmlGuiWindow::openHandbook();
}

}  // namespace Digikam

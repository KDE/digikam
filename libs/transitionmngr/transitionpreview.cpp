/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show transition preview
 *
 * Copyright (C) 2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "transitionpreview.h"

// Qt includes

#include <QTimer>
#include <QImage>
#include <QPixmap>

// Local includes

#include "frameutils.h"
#include "digikam_debug.h"

namespace Digikam
{

class TransitionPreview::Private
{
public:

    Private()
    {
        mngr          = 0;
        curTransition = TransitionMngr::None;
        previewSize   = QSize(192, 144);
    }

    QTimer                    restartTimer;
    QTimer                    transTimer;
    TransitionMngr*           mngr;
    TransitionMngr::TransType curTransition;
    QSize                     previewSize;
};

TransitionPreview::TransitionPreview(QWidget* const parent)
    : QLabel(parent),
      d(new Private)
{
    setFixedSize(d->previewSize);
    setContentsMargins(QMargins());
    setScaledContents(false);
    setOpenExternalLinks(false);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    d->mngr = new TransitionMngr;
    d->mngr->setOutputSize(d->previewSize);

    connect(&d->transTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressTransition()));

    connect(&d->restartTimer, SIGNAL(timeout()),
            this, SLOT(slotRestart()));
}

TransitionPreview::~TransitionPreview()
{
    delete d;
}

void TransitionPreview::setImagesList(const QList<QUrl>& images)
{
    if (!images.isEmpty())
    {
        d->mngr->setInImage(FrameUtils::makeFramedImage(images[0].toLocalFile(), d->previewSize));

        if (images.count() > 1)
        {
            d->mngr->setOutImage(FrameUtils::makeFramedImage(images[1].toLocalFile(), d->previewSize));
        }
        else
        {
            QImage blank(d->previewSize, QImage::Format_ARGB32);
            blank.fill(Qt::black);
            d->mngr->setOutImage(blank);
        }
    }
}

void TransitionPreview::startPreview(TransitionMngr::TransType eff)
{
    stopPreview();
    d->curTransition = eff;
    d->mngr->setTransition(eff);
    d->transTimer.start(100);
}

void TransitionPreview::slotProgressTransition()
{
    int tmout  = -1;
    QImage img = d->mngr->currentFrame(tmout);
    setPixmap(QPixmap::fromImage(img));

    if (tmout == -1)
    {
        stopPreview();
        d->restartTimer.start(1000);
    }
}

void TransitionPreview::stopPreview()
{
    d->transTimer.stop();
    d->restartTimer.stop();
}

void TransitionPreview::slotRestart()
{
    startPreview(d->curTransition);
}

} // namespace Digikam

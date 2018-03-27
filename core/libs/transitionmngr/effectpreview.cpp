/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-04
 * Description : A label to show video frame effect preview
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "effectpreview.h"

// Qt includes

#include <QTimer>
#include <QImage>
#include <QPixmap>

// Local includes

#include "frameutils.h"
#include "digikam_debug.h"

namespace Digikam
{

class EffectPreview::Private
{
public:

    Private()
    {
        mngr        = 0;
        curEffect   = EffectMngr::None;
        previewSize = QSize(192, 144);
    }

    QTimer                 restartTimer;
    QTimer                 effTimer;
    EffectMngr*            mngr;
    EffectMngr::EffectType curEffect;
    QSize                  previewSize;
};

EffectPreview::EffectPreview(QWidget* const parent)
    : QLabel(parent),
      d(new Private)
{
    setFixedSize(d->previewSize);
    setContentsMargins(QMargins());
    setScaledContents(false);
    setOpenExternalLinks(false);
    setFocusPolicy(Qt::NoFocus);
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    d->mngr = new EffectMngr;
    d->mngr->setOutputSize(d->previewSize);

    connect(&d->effTimer, SIGNAL(timeout()),
            this, SLOT(slotProgressEffect()));

    connect(&d->restartTimer, SIGNAL(timeout()),
            this, SLOT(slotRestart()));
}

EffectPreview::~EffectPreview()
{
    delete d;
}

void EffectPreview::setImagesList(const QList<QUrl>& images)
{
    if (!images.isEmpty())
    {
        d->mngr->setImage(FrameUtils::makeFramedImage(images[0].toLocalFile(), d->previewSize));
    }
    else
    {
        QImage blank(d->previewSize, QImage::Format_ARGB32);
        blank.fill(Qt::black);
        d->mngr->setImage(blank);
    }
}

void EffectPreview::startPreview(EffectMngr::EffectType eff)
{
    stopPreview();
    d->curEffect = eff;
    d->mngr->setEffect(eff);
    d->effTimer.start(50);
}

void EffectPreview::slotProgressEffect()
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

void EffectPreview::stopPreview()
{
    d->effTimer.stop();
    d->restartTimer.stop();
}

void EffectPreview::slotRestart()
{
    startPreview(d->curEffect);
}

} // namespace Digikam

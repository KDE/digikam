/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-22
 * Description : Slideshow video viewer
 *
 * Copyright (C) 2014-2017 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "slidevideo.h"

// Qt includes

#include <QApplication>
#include <QProxyStyle>
#include <QGridLayout>
#include <QWidget>
#include <QString>
#include <QSlider>
#include <QStyle>
#include <QLabel>

// KDE includes

#include <klocalizedstring.h>

// QtAV includes

#include <QtAV/AVPlayer.h>               // krazy:exclude=includes
#include <QtAVWidgets/WidgetRenderer.h>  // krazy:exclude=includes

// Local includes

#include "metaenginesettings.h"
#include "digikam_debug.h"
#include "dlayoutbox.h"
#include "dmetadata.h"

using namespace QtAV;

namespace Digikam
{

class Q_DECL_HIDDEN VideoStyle : public QProxyStyle
{
public:

    using QProxyStyle::QProxyStyle;

    int styleHint(QStyle::StyleHint hint, const QStyleOption* option = 0,
                  const QWidget* widget = 0, QStyleHintReturn* returnData = 0) const
    {
        if (hint == QStyle::SH_Slider_AbsoluteSetButtons)
        {
            return (Qt::LeftButton | Qt::MidButton | Qt::RightButton);
        }

        return QProxyStyle::styleHint(hint, option, widget, returnData);
    }
};

class Q_DECL_HIDDEN SlideVideo::Private
{

public:

    explicit Private()
      : iface(0),
        videoWidget(0),
        player(0),
        slider(0),
        tlabel(0),
        indicator(0)
    {
    }

    DInfoInterface*      iface;

    WidgetRenderer*      videoWidget;
    AVPlayer*            player;

    QSlider*             slider;
    QLabel*              tlabel;

    DHBox*               indicator;
};

SlideVideo::SlideVideo(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMouseTracking(true);

    d->videoWidget    = new WidgetRenderer(this);
    d->videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->videoWidget->setOutAspectRatioMode(VideoRenderer::VideoAspectRatio);
    d->videoWidget->setMouseTracking(true);

    d->player         = new AVPlayer(this);
    d->player->setRenderer(d->videoWidget);

    d->indicator      = new DHBox(this);
    d->slider         = new QSlider(Qt::Horizontal, d->indicator);
    d->slider->setStyle(new VideoStyle(d->slider->style()));
    d->slider->setRange(0, 0);
    d->slider->setAutoFillBackground(true);
    d->tlabel         = new QLabel(d->indicator);
    d->tlabel->setText(QLatin1String("00:00:00 / 00:00:00"));
    d->tlabel->setAutoFillBackground(true);
    d->indicator->setStretchFactor(d->slider, 10);
    d->indicator->setAutoFillBackground(true);

    QGridLayout* const grid = new QGridLayout(this);
    grid->addWidget(d->videoWidget, 0, 0, 2, 1);
    grid->addWidget(d->indicator,   0, 0, 1, 1); // Widget will be over player to not change layout when visibility is changed.
    grid->setRowStretch(0, 1);
    grid->setRowStretch(1, 100);
    grid->setContentsMargins(QMargins());

    // --------------------------------------------------------------------------

    connect(d->slider, SIGNAL(sliderMoved(int)),
            this, SLOT(slotPosition(int)));

    connect(d->slider, SIGNAL(valueChanged(int)),
            this, SLOT(slotPosition(int)));

    connect(d->player, SIGNAL(mediaStatusChanged(QtAV::MediaStatus)),
            this, SLOT(slotPlayerStateChanged(QtAV::MediaStatus)));

    connect(d->player, SIGNAL(positionChanged(qint64)),
            this, SLOT(slotPositionChanged(qint64)));

    connect(d->player, SIGNAL(durationChanged(qint64)),
            this, SLOT(slotDurationChanged(qint64)));

    connect(d->player, SIGNAL(error(QtAV::AVError)),
            this, SLOT(slotHandlePlayerError(QtAV::AVError)));

    // --------------------------------------------------------------------------

    layout()->activate();
    resize(sizeHint());
    show();
}

SlideVideo::~SlideVideo()
{
    d->player->stop();
    delete d;
}

void SlideVideo::setInfoInterface(DInfoInterface* const iface)
{
    d->iface = iface;
}

void SlideVideo::setCurrentUrl(const QUrl& url)
{
    d->player->stop();

    if (MetaEngineSettings::instance()->settings().exifRotate)
    {
        int orientation = 0;

        if (d->iface)
        {
            DItemInfo info(d->iface->itemInfo(url));
            orientation = info.orientation();
        }
        else
        {
            DMetadata meta(url.toLocalFile());
            orientation = meta.getItemOrientation();
        }

        switch (orientation)
        {
            case MetaEngine::ORIENTATION_ROT_90:
                orientation = 90;
                break;
            case MetaEngine::ORIENTATION_ROT_180:
                orientation = 180;
                break;
            case MetaEngine::ORIENTATION_ROT_270:
                orientation = 270;
                break;
            default:
                break;
        }

        qCDebug(DIGIKAM_GENERAL_LOG) << "Found video orientation:" << orientation;
        d->videoWidget->setOrientation(orientation);
    }

    d->player->setFile(url.toLocalFile());
    d->player->play();

    showIndicator(false);
}

void SlideVideo::showIndicator(bool b)
{
    d->indicator->setVisible(b);
}

void SlideVideo::slotPlayerStateChanged(QtAV::MediaStatus newState)
{
    switch (newState)
    {
        case EndOfMedia:
            emit signalVideoFinished();
            break;
        case LoadedMedia:
            emit signalVideoLoaded(true);
            break;
        case InvalidMedia:
            emit signalVideoLoaded(false);
            break;
        default:
            break;
    }
}

void SlideVideo::pause(bool b)
{
    if (!b && !d->player->isPlaying())
    {
        d->player->play();
        return;
    }

    d->player->pause(b);
}

void SlideVideo::stop()
{
    d->player->stop();
}

void SlideVideo::slotPositionChanged(qint64 position)
{
    if (!d->slider->isSliderDown())
    {
        d->slider->blockSignals(true);
        d->slider->setValue(position);
        d->slider->blockSignals(false);
    }

    d->tlabel->setText(QString::fromLatin1("%1 / %2")
                       .arg(QTime(0, 0, 0).addMSecs(position).toString(QLatin1String("HH:mm:ss")))
                       .arg(QTime(0, 0, 0).addMSecs(d->slider->maximum()).toString(QLatin1String("HH:mm:ss"))));

    emit signalVideoPosition(position);
}

void SlideVideo::slotDurationChanged(qint64 duration)
{
    qint64 max = qMax((qint64)1, duration);
    d->slider->setRange(0, max);

    emit signalVideoDuration(duration);
}

void SlideVideo::slotPosition(int position)
{
    if (d->player->isSeekable())
    {
        d->player->setPosition((qint64)position);
    }
}

void SlideVideo::slotHandlePlayerError(const QtAV::AVError& err)
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Error: " << err.string();
}

} // namespace Digikam

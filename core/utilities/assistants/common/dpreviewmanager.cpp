/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-12-23
 * Description : a widget to manage preview.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2012      by Benjamin Girault <benjamin dot girault at gmail dot com>
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

#include "dpreviewmanager.h"

// Qt includes

#include <QStyle>
#include <QLabel>
#include <QTimer>
#include <QImage>
#include <QPixmap>
#include <QFrame>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "dpreviewimage.h"
#include "dlayoutbox.h"
#include "dworkingpixmap.h"

namespace Digikam
{

class DPreviewManager::Private
{
public:

    explicit Private()
    {
        progressPix   = DWorkingPixmap();
        progressCount = 0;
        progressTimer = 0;
        progressLabel = 0;
        thumbLabel    = 0;
        busy          = false;
        textLabel     = 0;
        preview       = 0;
        button        = 0;
    }

    bool           busy;

    QLabel*        textLabel;
    QLabel*        thumbLabel;

    QPushButton*   button;

    int            progressCount;
    DWorkingPixmap progressPix;
    QTimer*        progressTimer;
    QLabel*        progressLabel;

    DPreviewImage* preview;
};

DPreviewManager::DPreviewManager(QWidget* const parent)
    : QStackedWidget(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setMinimumSize(QSize(400, 300));
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFrame* const vbox         = new QFrame(this);
    QVBoxLayout* const vboxLay = new QVBoxLayout(vbox);
    vbox->setFrameStyle( QFrame::StyledPanel | QFrame::Sunken );
    vbox->setLineWidth( style()->pixelMetric(QStyle::PM_DefaultFrameWidth) );
    QLabel* const space1       = new QLabel(vbox);
    d->progressLabel           = new QLabel(vbox);
    d->progressLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QLabel* const space2       = new QLabel(vbox);
    d->thumbLabel              = new QLabel(vbox);
    d->thumbLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    QLabel* const space3       = new QLabel(vbox);
    d->textLabel               = new QLabel(vbox);
    d->textLabel->setScaledContents(true);
    d->textLabel->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    QWidget* const hbox        = new QWidget(vbox);
    QHBoxLayout* const hboxLay = new QHBoxLayout(hbox);
    QLabel* const space4       = new QLabel(hbox);
    d->button                  = new QPushButton(hbox);
    d->button->hide();
    QLabel* const space5       = new QLabel(hbox);

    hboxLay->addWidget(space4);
    hboxLay->addWidget(d->button);
    hboxLay->addWidget(space5);
    hboxLay->setStretchFactor(space4, 10);
    hboxLay->setStretchFactor(space5, 10);

    QLabel* const space6       = new QLabel(vbox);

    vboxLay->addWidget(space1);
    vboxLay->addWidget(d->progressLabel);
    vboxLay->addWidget(space2);
    vboxLay->addWidget(d->thumbLabel);
    vboxLay->addWidget(space3);
    vboxLay->addWidget(d->textLabel);
    vboxLay->addWidget(hbox);
    vboxLay->addWidget(space6);
    vboxLay->setStretchFactor(space1, 10);
    vboxLay->setStretchFactor(d->progressLabel, 5);
    vboxLay->setStretchFactor(space2, 1);
    vboxLay->setStretchFactor(d->thumbLabel, 5);
    vboxLay->setStretchFactor(space3, 1);
    vboxLay->setStretchFactor(d->textLabel, 5);
    vboxLay->setStretchFactor(space3, 1);
    vboxLay->setStretchFactor(hbox, 5);
    vboxLay->setStretchFactor(space6, 10);

    d->preview = new DPreviewImage(this);

    insertWidget(MessageMode, vbox);
    insertWidget(PreviewMode, d->preview);

    d->progressTimer = new QTimer(this);

    connect(d->progressTimer, &QTimer::timeout,
            this, &DPreviewManager::slotProgressTimerDone);

    connect(d->button, &QPushButton::clicked,
            this, &DPreviewManager::signalButtonClicked);
}

DPreviewManager::~DPreviewManager()
{
    delete d;
}

void DPreviewManager::slotLoad(const QUrl& url)
{
    load(url);
}

void DPreviewManager::setImage(const QImage& img, bool fit)
{
    setBusy(false);

    if (!d->preview->setImage(img))
    {
        setText(i18n( "Failed to load image" ));
        return;
    }

    setCurrentIndex(PreviewMode);

    if (fit)
    {
        d->preview->slotZoom2Fit();
    }
}

bool DPreviewManager::load(const QUrl& file, bool fit)
{
    setBusy(false);

    if (!d->preview->load(file))
    {
        setText(i18n( "Failed to load image" ));
        return false;
    }

    setCurrentIndex(PreviewMode);

    if (fit)
    {
        d->preview->slotZoom2Fit();
    }

    return true;
}

void DPreviewManager::setThumbnail(const QPixmap& thumbnail)
{
    d->thumbLabel->setPixmap(thumbnail);
    setCurrentIndex(MessageMode);
}

void DPreviewManager::setButtonText(const QString& text)
{
    d->button->setText(text);
}

void DPreviewManager::setButtonVisible(bool b)
{
    d->button->setVisible(b);
}

void DPreviewManager::setSelectionAreaPossible(bool b)
{
    d->preview->enableSelectionArea(b);
}

QRectF DPreviewManager::getSelectionArea() const
{
    return d->preview->getSelectionArea();
}

void DPreviewManager::setSelectionArea(const QRectF& rectangle)
{
    d->preview->setSelectionArea(rectangle);
}

void DPreviewManager::setText(const QString& text, const QColor& color)
{
    d->textLabel->setText(QString::fromLatin1("<qt text=\"%1\">%2</qt>").arg(color.name()).arg(text));
    setCurrentIndex(MessageMode);
}

void DPreviewManager::setBusy(bool b, const QString& text)
{
    d->busy = b;

    if (d->busy)
    {
        setCursor( Qt::WaitCursor );
        d->progressTimer->start(300);
        setText(text);
    }
    else
    {
        unsetCursor();
        d->progressTimer->stop();
        setText(text);
        d->progressLabel->setPixmap(QPixmap());
    }
}

void DPreviewManager::slotProgressTimerDone()
{
    d->progressLabel->setPixmap(d->progressPix.frameAt(d->progressCount));
    d->progressCount++;

    if (d->progressCount == 8)
        d->progressCount = 0;

    d->progressTimer->start(300);
}

} // namespace Digikam

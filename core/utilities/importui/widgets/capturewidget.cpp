/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-06
 * Description : a widget to display camera capture preview.
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

#include "capturewidget.h"

// Qt includes

#include <QPainter>
#include <QImage>
#include <QPixmap>
#include <QRect>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class CaptureWidget::Private
{
public:

    Private()
    {
    }

    QPixmap pixmap;
    QImage  preview;
};

CaptureWidget::CaptureWidget(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

CaptureWidget::~CaptureWidget()
{
    delete d;
}

void CaptureWidget::setPreview(const QImage& preview)
{
    d->preview = preview;
    d->pixmap  = QPixmap(contentsRect().size());

    updatePixmap();
    repaint();
}

void CaptureWidget::updatePixmap()
{
    d->pixmap.fill(palette().background().color());
    QPainter p(&(d->pixmap));

    if (!d->preview.isNull())
    {
        QPixmap pix = QPixmap::fromImage(d->preview.scaled(contentsRect().size(),
                                                           Qt::KeepAspectRatio, Qt::SmoothTransformation));
        p.drawPixmap((contentsRect().width()  - pix.width())  / 2,
                     (contentsRect().height() - pix.height()) / 2, pix,
                     0, 0, pix.width(), pix.height());
    }
    else
    {
        p.setPen(QPen(palette().text().color()));
        p.drawText(0, 0, d->pixmap.width(), d->pixmap.height(),
                   Qt::AlignCenter | Qt::TextWordWrap,
                   i18n("Cannot display camera preview"));
    }

    p.end();
}

void CaptureWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(contentsRect().top(), contentsRect().left(), d->pixmap);
    p.end();
}

void CaptureWidget::resizeEvent(QResizeEvent*)
{
    blockSignals(true);
    d->pixmap = QPixmap(contentsRect().size());
    updatePixmap();
    repaint();
    blockSignals(false);
}

}  // namespace Digikam

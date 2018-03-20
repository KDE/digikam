/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-08-15
 * Description : a widget to draw stars rating
 *
 * Copyright (C) 2005      by Owen Hirst <n8rider@sbcglobal.net>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ratingwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QTimeLine>
#include <QFont>
#include <QAction>
#include <QWidgetAction>

// KDE includes

#include <klocalizedstring.h>
#include <kactioncollection.h>

// Local includes

#include "digikam_debug.h"
#include "digikam_globals.h"
#include "thememanager.h"
#include "dxmlguiwindow.h"
#include "dexpanderbox.h"

namespace Digikam
{

class RatingWidget::Private
{
public:

    Private()
    {
        tracking       = true;
        isHovered      = false;
        fading         = false;
        rating         = 0;
        fadingTimeLine = 0;
        fadingValue    = 0;
        offset         = 0;
        duration       = 600;   // ms
    }

    bool       tracking;
    bool       isHovered;
    bool       fading;

    int        rating;
    int        fadingValue;
    int        duration;
    int        offset;

    QTimeLine* fadingTimeLine;

    QPixmap    selPixmap;      // Selected star.
    QPixmap    regPixmap;      // Regular star.
    QPixmap    disPixmap;      // Disable star.
};

RatingWidget::RatingWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    slotThemeChanged();

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

RatingWidget::~RatingWidget()
{
    delete d;
}

void RatingWidget::setupTimeLine()
{
    delete d->fadingTimeLine;
    d->fadingTimeLine = new QTimeLine(d->duration, this);
    d->fadingTimeLine->setFrameRange(0, 255);

    connect(d->fadingTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(setFadingValue(int)));

    d->fadingTimeLine->start();
}

int RatingWidget::regPixmapWidth() const
{
    return d->regPixmap.width();
}

void RatingWidget::setRating(int val)
{
    if (((val < RatingMin) || (val > RatingMax)) && (val != NoRating))
    {
        return;
    }

    d->rating = val;

    if (d->tracking)
    {
        emit signalRatingChanged(d->rating);
    }

    emit signalRatingModified(d->rating);
    update();
}

int RatingWidget::rating() const
{
    return d->rating;
}

void RatingWidget::setTracking(bool tracking)
{
    d->tracking = tracking;
}

bool RatingWidget::hasTracking() const
{
    return d->tracking;
}

void RatingWidget::setFading(bool fading)
{
    d->fading = fading;
}

bool RatingWidget::hasFading() const
{
    return d->fading;
}

void RatingWidget::setFadingValue(int value)
{
    d->fadingValue = value;

    if (d->fadingValue >= 255 && d->fadingTimeLine)
    {
        d->fadingTimeLine->stop();
    }

    update();
}

void RatingWidget::setVisible(bool visible)
{
    QWidget::setVisible(visible);

    if (visible)
    {
        startFading();
    }
    else
    {
        stopFading();
    }
}

int RatingWidget::maximumVisibleWidth() const
{
    return RatingMax * (d->disPixmap.width()+1);
}

void RatingWidget::startFading()
{
    if (!hasFading())
    {
        return;
    }

    if (!d->isHovered)
    {
        d->isHovered   = true;
        d->fadingValue = 0;
        setupTimeLine();
    }
}

void RatingWidget::stopFading()
{
    if (!hasFading())
    {
        return;
    }

    if (d->fadingTimeLine)
    {
        d->fadingTimeLine->stop();
    }

    d->isHovered   = false;
    d->fadingValue = 0;
    update();
}

void RatingWidget::setVisibleImmediately()
{
    setFadingValue(255);
}

QPixmap RatingWidget::starPixmapDisabled() const
{
    return d->disPixmap;
}

QPixmap RatingWidget::starPixmapFilled() const
{
    return d->selPixmap;
}

QPixmap RatingWidget::starPixmap() const
{
    return d->regPixmap;
}

void RatingWidget::regeneratePixmaps()
{
    slotThemeChanged();
}

void RatingWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        return;
    }

    if (hasFading() && d->fadingValue < 255)
    {
        return;
    }

    int pos = (e->x() - d->offset) / d->regPixmap.width() +1;

    if (d->rating == pos)
    {
        d->rating--;
    }
    else
    {
        d->rating = pos;
    }

    if (d->rating > RatingMax)
    {
        d->rating = RatingMax;
    }

    if (d->rating < RatingMin)
    {
        d->rating = RatingMin;
    }

    if (d->tracking)
    {
        emit signalRatingChanged(d->rating);
    }

    emit signalRatingModified(d->rating);
    update();
}

void RatingWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (!(e->buttons() & Qt::LeftButton))
    {
        return;
    }

    if (hasFading() && d->fadingValue < 255)
    {
        return;
    }

    int pos = (e->x() - d->offset) / d->regPixmap.width() +1;

    if (d->rating != pos)
    {
        if (pos > RatingMax)       // NOTE: bug. #151357
        {
            pos = RatingMax;
        }

        if (pos < RatingMin)
        {
            pos = RatingMin;
        }

        d->rating = pos;

        if (d->tracking)
        {
            emit signalRatingChanged(d->rating);
        }

        emit signalRatingModified(d->rating);
        update();
    }
}

void RatingWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() != Qt::LeftButton)
    {
        return;
    }

    if (hasFading() && d->fadingValue < 255)
    {
        return;
    }

    emit signalRatingChanged(d->rating);
}

void RatingWidget::slotThemeChanged()
{
    d->regPixmap = QPixmap(15, 15);
    d->regPixmap.fill(Qt::transparent);
    d->selPixmap = QPixmap(15, 15);
    d->selPixmap.fill(Qt::transparent);
    d->disPixmap = QPixmap(15, 15);
    d->disPixmap.fill(Qt::transparent);

    QPainter p1(&d->regPixmap);
    p1.setRenderHint(QPainter::Antialiasing, true);
    p1.setBrush(palette().color(QPalette::Active, backgroundRole()));
    p1.setPen(palette().color(QPalette::Active, foregroundRole()));
    p1.drawPolygon(starPolygon(), Qt::WindingFill);
    p1.end();

    QPainter p2(&d->selPixmap);
    p2.setRenderHint(QPainter::Antialiasing, true);
    p2.setBrush(qApp->palette().color(QPalette::Link));
    p2.setPen(palette().color(QPalette::Active, foregroundRole()));
    p2.drawPolygon(starPolygon(), Qt::WindingFill);
    p2.end();

    QPainter p3(&d->disPixmap);
    p3.setRenderHint(QPainter::Antialiasing, true);
    p3.setBrush(palette().color(QPalette::Disabled, backgroundRole()));
    p3.setPen(palette().color(QPalette::Disabled, foregroundRole()));
    p3.drawPolygon(starPolygon(), Qt::WindingFill);
    p3.end();

    setMinimumSize(QSize((d->regPixmap.width()+1)*RatingMax, d->regPixmap.height()));
    update();
}

QPolygon RatingWidget::starPolygon()
{
    QPolygon star;
    star << QPoint(0,  6);
    star << QPoint(5,  5);
    star << QPoint(7,  0);
    star << QPoint(9,  5);
    star << QPoint(14, 6);
    star << QPoint(10, 9);
    star << QPoint(11, 14);
    star << QPoint(7,  11);
    star << QPoint(3,  14);
    star << QPoint(4,  9);
    return star;
}

QIcon RatingWidget::buildIcon(int rate, int size)
{
    QPixmap pix(size, size);
    pix.fill(Qt::transparent);
    QPainter p(&pix);
    QMatrix matrix;
    matrix.scale(size/15.0, size/15.0);
    p.setMatrix(matrix);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(qApp->palette().color(QPalette::Active, QPalette::ButtonText));

    if (rate > 0) p.setBrush(qApp->palette().color(QPalette::Link));

    p.drawPolygon(starPolygon(), Qt::WindingFill);
    p.end();

    return QIcon(pix);
}

void RatingWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    d->offset = (width() - RatingMax * (d->disPixmap.width()+1)) / 2;

    // Widget is disable : drawing grayed frame.
    if (!isEnabled())
    {
        int x = d->offset;

        for (int i = 0; i < RatingMax; ++i)
        {
            p.drawPixmap(x, 0, d->disPixmap);
            x += d->disPixmap.width()+1;
        }
    }
    else
    {
        int x       = d->offset;
        int rate    = d->rating != NoRating ? d->rating : 0;
        QPixmap sel = d->selPixmap;
        applyFading(sel);

        for (int i = 0; i < rate; ++i)
        {
            p.drawPixmap(x, 0, sel);
            x += sel.width()+1;
        }

        QPixmap reg = d->regPixmap;
        applyFading(reg);

        for (int i = rate; i < RatingMax; ++i)
        {
            p.drawPixmap(x, 0, reg);
            x += reg.width()+1;
        }
    }

    p.end();
}

void RatingWidget::applyFading(QPixmap& pix)
{
    if (hasFading() && d->fadingValue < 255)
    {
        QPainter p(&pix);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.fillRect(pix.rect(), QColor(0, 0, 0, d->fadingValue));
        p.end();
    }
}

// -------------------------------------------------------------------------------

class RatingBox::Private
{

public:

    Private()
    {
        shortcut     = 0;
        ratingWidget = 0;
    }

    DAdjustableLabel* shortcut;

    RatingWidget*     ratingWidget;
};

RatingBox::RatingBox(QWidget* const parent)
    : DVBox(parent),
      d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setFocusPolicy(Qt::NoFocus);

    d->ratingWidget = new RatingWidget(this);
    d->ratingWidget->setTracking(false);

    d->shortcut = new DAdjustableLabel(this);
    QFont fnt   = d->shortcut->font();
    fnt.setItalic(true);
    d->shortcut->setFont(fnt);
    d->shortcut->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->shortcut->setWordWrap(false);

    setContentsMargins(QMargins());
    setSpacing(0);

    // -------------------------------------------------------------

    connect(d->ratingWidget, SIGNAL(signalRatingModified(int)),
            this, SLOT(slotUpdateDescription(int)));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalRatingChanged(int)));
}

RatingBox::~RatingBox()
{
    delete d;
}

void RatingBox::slotUpdateDescription(int rating)
{
    DXmlGuiWindow* const app = dynamic_cast<DXmlGuiWindow*>(qApp->activeWindow());

    if (app)
    {
        QAction* const ac = app->actionCollection()->action(QString::fromLatin1("rateshortcut-%1").arg(rating));

        if (ac)
        {
            d->shortcut->setAdjustedText(ac->shortcut().toString());
        }
    }
}

// -------------------------------------------------------------------------------

RatingMenuAction::RatingMenuAction(QMenu* const parent)
    : QMenu(parent)
{
    setTitle(i18n("Rating"));
    QWidgetAction* const wa = new QWidgetAction(this);
    RatingBox* const rb     = new RatingBox(parent);
    wa->setDefaultWidget(rb);
    addAction(wa);

    connect(rb, SIGNAL(signalRatingChanged(int)),
            this, SIGNAL(signalRatingChanged(int)));

    connect(rb, SIGNAL(signalRatingChanged(int)),
            parent, SLOT(close()));
}

RatingMenuAction::~RatingMenuAction()
{
}

}  // namespace Digikam

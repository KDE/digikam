/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a generic widget to display a panel to choose
 *               a rectangular image area.
 *
 * Copyright (C) 1997      by Tim D. Gilman (tdgilman at best dot org)
 * Copyright (C) 1998-2001 by Mirko Boehm (mirko at kde dot org)
 * Copyright (C) 2007      by John Layt <john at layt dot net>
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "paniconwidget.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QPainter>
#include <QPixmap>
#include <QPen>
#include <QTimer>
#include <QTimerEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QHideEvent>
#include <QToolButton>
#include <QIcon>
#include <QApplication>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QKeyEvent>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

class PanIconFrame::Private
{
public:

    Private(PanIconFrame* const qq);
    ~Private();

public:

    PanIconFrame*        q;

    /**
     * The result. It is returned from exec() when the popup window closes.
     */
    int                  result;

    /**
     * The only subwidget that uses the whole dialog window.
     */
    QWidget*             main;

    class OutsideClickCatcher;
    OutsideClickCatcher* outsideClickCatcher;
};

// -------------------------------------------------------------------

class PanIconFrame::Private::OutsideClickCatcher
    : public QObject
{
public:

    OutsideClickCatcher(QObject* const parent = 0)
        : QObject(parent),
          m_popup(0)
    {
    }

    ~OutsideClickCatcher()
    {
    }

    void setPopupFrame(PanIconFrame* const popup)
    {
        m_popup = popup;
        popup->installEventFilter(this);
    }

    bool eventFilter(QObject* object, QEvent* event)
    {
        Q_UNUSED(object);

        // To catch outside clicks, it is sufficient to check for
        // hide events on Qt::Popup type widgets
        if (event->type() == QEvent::Hide && m_popup)
        {
            // do not set d->result here, because the popup
            // hides itself after leaving the event loop.
            emit m_popup->leaveModality();
        }

        return false;
    }

public:

    PanIconFrame* m_popup;
};

// -------------------------------------------------------------------

PanIconFrame::Private::Private(PanIconFrame* const qq)
    : q(qq),
      result(0), // rejected
      main(0),
      outsideClickCatcher(new OutsideClickCatcher)
{
    outsideClickCatcher->setPopupFrame(q);
}

PanIconFrame::Private::~Private()
{
    delete outsideClickCatcher;
}

// -------------------------------------------------------------------

PanIconFrame::PanIconFrame(QWidget* const parent)
    : QFrame(parent, Qt::Popup),
      d(new Private(this))
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setMidLineWidth(2);
}

PanIconFrame::~PanIconFrame()
{
    delete d;
}

void PanIconFrame::keyPressEvent(QKeyEvent* e)
{
    if( e->key() == Qt::Key_Escape )
    {
        d->result = 0; // rejected
        emit leaveModality();
    }
}

void PanIconFrame::close(int r)
{
    d->result = r;
    emit leaveModality();
}

void PanIconFrame::setMainWidget(QWidget* const main)
{
    d->main = main;

    if( d->main )
    {
        resize( d->main->width() + 2 * frameWidth(), d->main->height() + 2 * frameWidth() );
    }
}

void PanIconFrame::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED( e );

    if( d->main )
    {
        d->main->setGeometry(frameWidth(), frameWidth(),
                             width() - 2 * frameWidth(), height() - 2 * frameWidth());
    }
}

void PanIconFrame::popup(const QPoint& pos)
{
    // Make sure the whole popup is visible.
    QRect desktopGeometry = QApplication::desktop()->screenGeometry( pos );

    int x = pos.x();
    int y = pos.y();
    int w = width();
    int h = height();

    if ( x + w > desktopGeometry.x() + desktopGeometry.width() )
    {
        x = desktopGeometry.width() - w;
    }

    if ( y + h > desktopGeometry.y() + desktopGeometry.height() )
    {
        y = desktopGeometry.height() - h;
    }

    if ( x < desktopGeometry.x() )
    {
        x = 0;
    }

    if ( y < desktopGeometry.y() )
    {
        y = 0;
    }

    // Pop the thingy up.
    move(x, y);
    show();
    d->main->setFocus();
}

int PanIconFrame::exec(const QPoint& pos)
{
    popup(pos);
    repaint();
    d->result = 0; // rejected
    QEventLoop eventLoop;

    connect(this, SIGNAL(leaveModality()),
            &eventLoop, SLOT(quit()));

    eventLoop.exec();
    hide();

    return d->result;
}

int PanIconFrame::exec(int x, int y)
{
    return exec(QPoint(x, y));
}

// -------------------------------------------------------------------

class PanIconWidget::Private
{

public:

    Private() :
        moveSelection(false),
        flicker(false),
        width(0),
        height(0),
        zoomedOrgWidth(0),
        zoomedOrgHeight(0),
        orgWidth(0),
        orgHeight(0),
        xpos(0),
        ypos(0),
        zoomFactor(1.0),
        timer(0)
    {
    }

    bool    moveSelection;
    bool    flicker;

    int     width;
    int     height;
    int     zoomedOrgWidth;
    int     zoomedOrgHeight;
    int     orgWidth;
    int     orgHeight;
    int     xpos;
    int     ypos;

    double  zoomFactor;

    QRect   regionSelection;         // Original size image selection.
    QTimer* timer;

    QRect   rect;
    QRect   localRegionSelection;    // Thumbnail size selection.

    QPixmap pixmap;
};

PanIconWidget::PanIconWidget(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    d->timer = new QTimer(this);
    d->timer->setInterval(800);

    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);

    connect(d->timer, SIGNAL(timeout()),
            this, SLOT(slotFlickerTimer()));
}

PanIconWidget::~PanIconWidget()
{
    delete d;
}

QToolButton* PanIconWidget::button()
{
    QToolButton* const btn = new QToolButton;
    btn->setToolButtonStyle(Qt::ToolButtonIconOnly);
    btn->setIcon(QIcon::fromTheme(QLatin1String("transform-move")));
    btn->hide();
    btn->setToolTip(i18n("Pan the image to a region"));

    return btn;
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const QImage& image)
{
    QSize sz(image.width(), image.height());
    sz.scale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    QImage scaledImg = image.scaled(sz.width(), sz.height(),
                                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

    setImage(scaledImg, image.size());
}

void PanIconWidget::setImage(const QImage& scaledPreviewImage, const QSize& fullImageSize)
{
    d->width           = scaledPreviewImage.width();
    d->height          = scaledPreviewImage.height();
    d->orgWidth        = fullImageSize.width();
    d->orgHeight       = fullImageSize.height();
    d->zoomedOrgWidth  = fullImageSize.width();
    d->zoomedOrgHeight = fullImageSize.height();
    d->pixmap          = QPixmap(d->width, d->height);
    d->pixmap.fill(palette().color(QPalette::Background));
    QPainter p(&d->pixmap);
    p.drawImage(0, 0, scaledPreviewImage);

    setFixedSize(d->width, d->height);

    d->rect = QRect(width()/2-d->width/2, height()/2-d->height/2, d->width, d->height);
    update();
}

void PanIconWidget::setImage(int previewWidth, int previewHeight, const DImg& image)
{
    DImg img = DImg(image).smoothScale(previewWidth, previewHeight, Qt::KeepAspectRatio);
    setImage(img.copyQImage(), image.size());
}

void PanIconWidget::slotZoomFactorChanged(double factor)
{
    if (d->zoomFactor == factor)
    {
        return;
    }

    d->zoomFactor      = factor;
    d->zoomedOrgWidth  = (int)(d->orgWidth  * factor);
    d->zoomedOrgHeight = (int)(d->orgHeight * factor);
    update();
}

void PanIconWidget::setRegionSelection(const QRect& regionSelection)
{
    d->regionSelection = regionSelection;
    d->localRegionSelection.setX( d->rect.x() + (int)((float)d->regionSelection.x() *
                                  ((float)d->width / (float)d->zoomedOrgWidth)) );

    d->localRegionSelection.setY( d->rect.y() + (int)((float)d->regionSelection.y() *
                                  ((float)d->height / (float)d->zoomedOrgHeight)) );

    d->localRegionSelection.setWidth( (int)((float)d->regionSelection.width() *
                                            ((float)d->width / (float)d->zoomedOrgWidth)) );

    d->localRegionSelection.setHeight( (int)((float)d->regionSelection.height() *
                                       ((float)d->height / (float)d->zoomedOrgHeight)) );

    update();
}

QRect PanIconWidget::getRegionSelection() const
{
    return (d->regionSelection);
}

void PanIconWidget::setCursorToLocalRegionSelectionCenter()
{
    QCursor::setPos(mapToGlobal(d->localRegionSelection.center()));
}

void PanIconWidget::setCenterSelection()
{
    setRegionSelection(QRect((int)(((float)d->zoomedOrgWidth / 2.0) - ((float)d->regionSelection.width()  / 2.0)),
                            (int)(((float)d->zoomedOrgHeight / 2.0) - ((float)d->regionSelection.height() / 2.0)),
                            d->regionSelection.width(),
                            d->regionSelection.height()));
}

void PanIconWidget::regionSelectionMoved(bool targetDone)
{
    if (targetDone)
    {
        update();
    }

    int x = (int)lround( ((float)d->localRegionSelection.x() - (float)d->rect.x() ) *
                         ((float)d->zoomedOrgWidth / (float)d->width) );

    int y = (int)lround( ((float)d->localRegionSelection.y() - (float)d->rect.y() ) *
                         ((float)d->zoomedOrgHeight / (float)d->height) );

    int w = (int)lround( (float)d->localRegionSelection.width() *
                         ((float)d->zoomedOrgWidth / (float)d->width) );

    int h = (int)lround( (float)d->localRegionSelection.height() *
                         ((float)d->zoomedOrgHeight / (float)d->height) );

    d->regionSelection.setX(x);
    d->regionSelection.setY(y);
    d->regionSelection.setWidth(w);
    d->regionSelection.setHeight(h);

    emit signalSelectionMoved(d->regionSelection, targetDone);
}

void PanIconWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);

    p.drawPixmap(d->rect.x(), d->rect.y(), d->pixmap);

    // Drawing selection border

    if (d->flicker)
    {
        p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
    }
    else
    {
        p.setPen(QPen(Qt::red, 1, Qt::SolidLine));
    }

    QRect r(d->localRegionSelection);

    // Clamp to widget size. Selection area must always be visible
    if (r.left() < 0)
    {
        r.setLeft(0);
    }

    if (r.top() < 0)
    {
        r.setTop(0);
    }

    if (r.right() > width()-2)
    {
        r.setRight(width()-2);
    }

    if (r.bottom() > height()-2)
    {
        r.setBottom(height()-2);
    }

    p.drawRect(r.x(), r.y(), r.width(), r.height());

    if (d->flicker)
    {
        p.setPen(QPen(Qt::red, 1, Qt::DotLine));
    }
    else
    {
        p.setPen(QPen(Qt::white, 1, Qt::DotLine));
    }

    p.drawRect(r.x(), r.y(), r.width(), r.height());
}

void PanIconWidget::setMouseFocus()
{
    raise();
    d->xpos          = d->localRegionSelection.center().x();
    d->ypos          = d->localRegionSelection.center().y();
    d->moveSelection = true;
    setCursor(Qt::SizeAllCursor);
    emit signalSelectionTakeFocus();
}

void PanIconWidget::showEvent(QShowEvent* e)
{
    QWidget::showEvent(e);

    d->timer->start();
}

void PanIconWidget::hideEvent(QHideEvent* e)
{
    QWidget::hideEvent(e);

    d->timer->stop();

    if ( d->moveSelection )
    {
        d->moveSelection = false;
        setCursor(Qt::ArrowCursor);
        emit signalHidden();
    }
}

void PanIconWidget::mousePressEvent(QMouseEvent* e)
{
    if ( (e->button() == Qt::LeftButton || e->button() == Qt::MidButton) &&
         d->localRegionSelection.contains( e->x(), e->y() ) )
    {
        d->xpos          = e->x();
        d->ypos          = e->y();
        d->moveSelection = true;
        setCursor(Qt::SizeAllCursor);
        emit signalSelectionTakeFocus();
    }
}

void PanIconWidget::mouseMoveEvent(QMouseEvent* e)
{
    if ( d->moveSelection &&
         (e->buttons() == Qt::LeftButton || e->buttons() == Qt::MidButton) )
    {
        int newxpos = e->x();
        int newypos = e->y();

        d->localRegionSelection.translate(newxpos - d->xpos, newypos - d->ypos);

        d->xpos = newxpos;
        d->ypos = newypos;

        // Perform normalization of selection area.

        if (d->localRegionSelection.left() < d->rect.left())
        {
            d->localRegionSelection.moveLeft(d->rect.left());
        }

        if (d->localRegionSelection.top() < d->rect.top())
        {
            d->localRegionSelection.moveTop(d->rect.top());
        }

        if (d->localRegionSelection.right() > d->rect.right())
        {
            d->localRegionSelection.moveRight(d->rect.right());
        }

        if (d->localRegionSelection.bottom() > d->rect.bottom())
        {
            d->localRegionSelection.moveBottom(d->rect.bottom());
        }

        update();
        regionSelectionMoved(false);
        return;
    }
    else
    {
        if (d->localRegionSelection.contains(e->x(), e->y()))
        {
            setCursor(Qt::PointingHandCursor);
        }
        else
        {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void PanIconWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (d->moveSelection)
    {
        d->moveSelection = false;
        setCursor(Qt::ArrowCursor);
        regionSelectionMoved(true);
    }
}

void PanIconWidget::slotFlickerTimer()
{
    d->flicker = !d->flicker;
    update();
}

}  // namespace Digikam

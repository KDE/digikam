/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 1997-04-21
 * Description : Frame with popup menu behavior.
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 1997      by Tim D. Gilman <tdgilman at best dot org>
 * Copyright (C) 1998-2001 by Mirko Boehm <mirko at kde dot org>
 * Copyright (C) 2007      by John Layt <john at layt dot net>
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

#include "dpopupframe.h"

// Qt includes

#include <QApplication>
#include <QDesktopWidget>
#include <QEventLoop>
#include <QKeyEvent>

namespace Digikam
{

class DPopupFrame::Private
{
public:

    explicit Private(DPopupFrame* const qq);
    ~Private();

public:

    DPopupFrame*         q;

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

class DPopupFrame::Private::OutsideClickCatcher : public QObject
{
public:

    explicit OutsideClickCatcher(QObject* const parent = 0)
        : QObject(parent),
          m_popup(0)
    {
    }

    ~OutsideClickCatcher()
    {
    }

    void setPopupFrame(DPopupFrame* const popup)
    {
        m_popup = popup;
        popup->installEventFilter(this);
    }


    bool eventFilter(QObject* object, QEvent* event) Q_DECL_OVERRIDE
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

    DPopupFrame* m_popup;
};

DPopupFrame::Private::Private(DPopupFrame* const qq)
    : q(qq),
      result(0),   // rejected
      main(0),
      outsideClickCatcher(new OutsideClickCatcher)
{
    outsideClickCatcher->setPopupFrame(q);
}

DPopupFrame::Private::~Private()
{
    delete outsideClickCatcher;
}

DPopupFrame::DPopupFrame(QWidget* const parent)
    : QFrame(parent, Qt::Popup),
      d(new Private(this))
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setMidLineWidth(2);
}

DPopupFrame::~DPopupFrame()
{
    delete d;
}

void DPopupFrame::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Escape)
    {
        d->result = 0; // rejected
        emit leaveModality();
    }
}

void DPopupFrame::hideEvent(QHideEvent *e)
{
    QFrame::hideEvent(e);
}

void DPopupFrame::close(int r)
{
    d->result = r;
    emit leaveModality();
}

void DPopupFrame::setMainWidget(QWidget* const m)
{
    d->main = m;

    if (d->main)
    {
        resize(d->main->width() + 2 * frameWidth(), d->main->height() + 2 * frameWidth());
    }
}

void DPopupFrame::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e);

    if (d->main)
    {
        d->main->setGeometry(frameWidth(), frameWidth(),
                             width() - 2 * frameWidth(), height() - 2 * frameWidth());
    }
}

void DPopupFrame::popup(const QPoint& p)
{
    // Make sure the whole popup is visible.
    QRect desktopGeometry = QApplication::desktop()->screenGeometry(p);

    int x = p.x();
    int y = p.y();
    int w = width();
    int h = height();

    if (x + w > desktopGeometry.x() + desktopGeometry.width())
    {
        x = desktopGeometry.width() - w;
    }

    if (y + h > desktopGeometry.y() + desktopGeometry.height())
    {
        y = desktopGeometry.height() - h;
    }

    if (x < desktopGeometry.x())
    {
        x = 0;
    }

    if (y < desktopGeometry.y())
    {
        y = 0;
    }

    // Pop the thingy up.
    move(x, y);
    show();
    d->main->setFocus();
}

int DPopupFrame::exec(const QPoint& p)
{
    popup(p);
    repaint();
    d->result = 0; // rejected
    QEventLoop eventLoop;

    connect(this, &DPopupFrame::leaveModality,
            &eventLoop, &QEventLoop::quit);

    eventLoop.exec();

    hide();
    return d->result;
}

int DPopupFrame::exec(int x, int y)
{
    return exec(QPoint(x, y));
}

}  // namespace Digikam

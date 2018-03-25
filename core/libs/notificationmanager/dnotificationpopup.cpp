/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-07-03
 * Description : dialog-like popup that displays messages without interrupting the user
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2001-2006 by Richard Moore <rich at kde dot org>
 * Copyright (C) 2004-2005 by Sascha Cunz <sascha.cunz at tiscali dot de>
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

#include "dnotificationpopup.h"
#include "digikam_config.h"

// Qt includes

#include <QGuiApplication>
#include <QBitmap>
#include <QScreen>
#include <QLabel>
#include <QLayout>
#include <QBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QStyle>
#include <QTimer>
#include <QToolTip>
#include <QSystemTrayIcon>

namespace Digikam
{

static const int DEFAULT_POPUP_TYPE      = DNotificationPopup::Boxed;
static const int DEFAULT_POPUP_TIME      = 6 * 1000;
static const Qt::WindowFlags POPUP_FLAGS = Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint;

class DNotificationPopup::Private
{
public:

    Private(DNotificationPopup* const q, WId winId)
        : q(q),
          popupStyle(DEFAULT_POPUP_TYPE),
          window(winId),
          msgView(0),
          topLayout(0),
          hideDelay(DEFAULT_POPUP_TIME),
          hideTimer(new QTimer(q)),
          ttlIcon(0),
          ttl(0),
          msg(0),
          autoDelete(false)
    {
        q->setWindowFlags(POPUP_FLAGS);
        q->setFrameStyle(QFrame::Box | QFrame::Plain);
        q->setLineWidth(2);

        if (popupStyle == Boxed)
        {
            q->setFrameStyle(QFrame::Box | QFrame::Plain);
            q->setLineWidth(2);
        }
        else if (popupStyle == Balloon)
        {
            q->setPalette(QToolTip::palette());
        }

        connect(hideTimer, SIGNAL(timeout()),
                q, SLOT(hide()));

        connect(q, SIGNAL(clicked()),
                q, SLOT(hide()));
    }

public:

    DNotificationPopup* q;

    int                 popupStyle;
    QPolygon            surround;
    QPoint              anchor;
    QPoint              fixedPosition;

    WId                 window;
    QWidget*            msgView;
    QBoxLayout*         topLayout;
    int                 hideDelay;
    QTimer*             hideTimer;

    QLabel*             ttlIcon;
    QLabel*             ttl;
    QLabel*             msg;

    bool                autoDelete;

    /**
     * Updates the transparency mask. Unused if PopupStyle == Boxed
     */
    void updateMask()
    {
        // get screen-geometry for screen our anchor is on
        // (geometry can differ from screen to screen!
        QRect deskRect   = desktopRectForPoint(anchor);

        const int width  = q->width();
        const int height = q->height();

        int xh = 70, xl = 40;

        if (width < 80)
        {
            xh = xl = 40;
        }
        else if (width < 110)
        {
            xh = width - 40;
        }

        bool bottom = (anchor.y() + height) > ((deskRect.y() + deskRect.height() - 48));
        bool right  = (anchor.x() + width)  > ((deskRect.x() + deskRect.width()  - 48));

        QPoint corners[4] =
        {
            QPoint(width - 50, 10),
            QPoint(10, 10),
            QPoint(10, height - 50),
            QPoint(width - 50, height - 50)
        };

        QBitmap mask(width, height);
        mask.clear();
        QPainter p(&mask);
        QBrush brush(Qt::color1, Qt::SolidPattern);
        p.setBrush(brush);

        int i = 0, z = 0;

        for (; i < 4; ++i)
        {
            QPainterPath path;
            path.moveTo(corners[i].x(), corners[i].y());
            path.arcTo(corners[i].x(), corners[i].y(), 40, 40, i * 90, 90);
            QPolygon corner = path.toFillPolygon().toPolygon();

            surround.resize(z + corner.count() - 1);

            for (int s = 1; s < corner.count() - 1; s++, z++)
            {
                surround.setPoint(z, corner[s]);
            }

            if (bottom && i == 2)
            {
                if (right)
                {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(width - xh, height - 10));
                    surround.setPoint(z++, QPoint(width - 20, height));
                    surround.setPoint(z++, QPoint(width - xl, height - 10));
                }
                else
                {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(xl, height - 10));
                    surround.setPoint(z++, QPoint(20, height));
                    surround.setPoint(z++, QPoint(xh, height - 10));
                }
            }
            else if (!bottom && i == 0)
            {
                if (right)
                {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(width - xl, 10));
                    surround.setPoint(z++, QPoint(width - 20, 0));
                    surround.setPoint(z++, QPoint(width - xh, 10));
                }
                else
                {
                    surround.resize(z + 3);
                    surround.setPoint(z++, QPoint(xh, 10));
                    surround.setPoint(z++, QPoint(20, 0));
                    surround.setPoint(z++, QPoint(xl, 10));
                }
            }
        }

        surround.resize(z + 1);
        surround.setPoint(z, surround[0]);
        p.drawPolygon(surround);
        q->setMask(mask);

        q->move(right ? anchor.x() - width + 20 : (anchor.x() < 11 ? 11 : anchor.x() - 20),
                bottom ? anchor.y() - height : (anchor.y() < 11 ? 11 : anchor.y()));

        q->update();
    }

    /**
     * Calculates the position to place the popup near the specified rectangle.
     */
    QPoint calculateNearbyPoint(const QRect& target)
    {
        QPoint pos = target.topLeft();
        int x      = pos.x();
        int y      = pos.y();
        int w      = q->minimumSizeHint().width();
        int h      = q->minimumSizeHint().height();

        QRect r    = desktopRectForPoint(QPoint(x + w / 2, y + h / 2));

        if (popupStyle == Balloon)
        {
            // find a point to anchor to
            if (x + w > r.width())
            {
                x = x + target.width();
            }

            if (y + h > r.height())
            {
                y = y + target.height();
            }
        }
        else
        {
            if (x < r.center().x())
            {
                x = x + target.width();
            }
            else
            {
                x = x - w;
            }

            // It's apparently trying to go off screen, so display it ALL at the bottom.
            if ((y + h) > r.bottom())
            {
                y = r.bottom() - h;
            }

            if ((x + w) > r.right())
            {
                x = r.right() - w;
            }
        }

        if (y < r.top())
        {
            y = r.top();
        }

        if (x < r.left())
        {
            x = r.left();
        }

        return QPoint(x, y);
    }

    QRect desktopRectForPoint(const QPoint& point)
    {
        QList<QScreen*> screens = QGuiApplication::screens();

        foreach(const QScreen* screen, screens)
        {
            if (screen->geometry().contains(point))
            {
                return screen->geometry();
            }
        }

        // If no screen was found, return the primary screen's geometry
        return QGuiApplication::primaryScreen()->geometry();
    }
};

DNotificationPopup::DNotificationPopup(QWidget* parent, Qt::WindowFlags f)
    : QFrame(0, f ? f : POPUP_FLAGS),
      d(new Private(this, parent ? parent->effectiveWinId() : 0L))
{
}

DNotificationPopup::DNotificationPopup(WId win)
    : QFrame(0),
      d(new Private(this, win))
{
}

DNotificationPopup::~DNotificationPopup()
{
    delete d;
}

void DNotificationPopup::setPopupStyle(int popupstyle)
{
    if (d->popupStyle == popupstyle)
    {
        return;
    }

    d->popupStyle = popupstyle;

    if (d->popupStyle == Boxed)
    {
        setFrameStyle(QFrame::Box | QFrame::Plain);
        setLineWidth(2);
    }
    else if (d->popupStyle == Balloon)
    {
        setPalette(QToolTip::palette());
    }
}

void DNotificationPopup::setView(QWidget* child)
{
    delete d->msgView;
    d->msgView = child;

    delete d->topLayout;
    d->topLayout = new QVBoxLayout(this);

    if (d->popupStyle == Balloon)
    {
        const int marginHint = style()->pixelMetric(QStyle::PM_DefaultChildMargin);
        d->topLayout->setMargin(2 * marginHint);
    }

    d->topLayout->addWidget(d->msgView);
    d->topLayout->activate();
}

void DNotificationPopup::setView(const QString& caption, const QString& text,
                                 const QPixmap& icon)
{
    // qCDebug(LOG_KNOTIFICATIONS) << "DNotificationPopup::setView " << caption << ", " << text;
    setView(standardView(caption, text, icon, this));
}

QWidget* DNotificationPopup::standardView(const QString& caption,
                                          const QString& text,
                                          const QPixmap& icon,
                                          QWidget* parent)
{
    QWidget* top = new QWidget(parent ? parent : this);
    QVBoxLayout *vb = new QVBoxLayout(top);
    vb->setMargin(0);
    top->setLayout(vb);

    QHBoxLayout* hb = 0;

    if (!icon.isNull())
    {
        hb = new QHBoxLayout(top);
        hb->setMargin(0);
        vb->addLayout(hb);
        d->ttlIcon = new QLabel(top);
        d->ttlIcon->setPixmap(icon);
        d->ttlIcon->setAlignment(Qt::AlignLeft);
        hb->addWidget(d->ttlIcon);
    }

    if (!caption.isEmpty())
    {
        d->ttl = new QLabel(caption, top);
        QFont fnt = d->ttl->font();
        fnt.setBold(true);
        d->ttl->setFont(fnt);
        d->ttl->setAlignment(Qt::AlignHCenter);

        if (hb)
        {
            hb->addWidget(d->ttl);
            hb->setStretchFactor(d->ttl, 10);   // enforce centering
        }
        else
        {
            vb->addWidget(d->ttl);
        }
    }

    if (!text.isEmpty())
    {
        d->msg = new QLabel(text, top);
        d->msg->setAlignment(Qt::AlignLeft);
        d->msg->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        d->msg->setOpenExternalLinks(true);
        vb->addWidget(d->msg);
    }

    return top;
}

void DNotificationPopup::setView(const QString& caption, const QString& text)
{
    setView(caption, text, QPixmap());
}

QWidget* DNotificationPopup::view() const
{
    return d->msgView;
}

int DNotificationPopup::timeout() const
{
    return d->hideDelay;
}

void DNotificationPopup::setTimeout(int delay)
{
    d->hideDelay = delay < 0 ? DEFAULT_POPUP_TIME : delay;

    if (d->hideTimer->isActive())
    {
        if (delay)
        {
            d->hideTimer->start(delay);
        }
        else
        {
            d->hideTimer->stop();
        }
    }
}

bool DNotificationPopup::autoDelete() const
{
    return d->autoDelete;
}

void DNotificationPopup::setAutoDelete(bool autoDelete)
{
    d->autoDelete = autoDelete;
}

void DNotificationPopup::mouseReleaseEvent(QMouseEvent* e)
{
    emit clicked();
    emit clicked(e->pos());
}

void DNotificationPopup::setVisible(bool visible)
{
    if (! visible)
    {
        QFrame::setVisible(visible);
        return;
    }

    if (size() != sizeHint())
    {
        resize(sizeHint());
    }

    if (d->fixedPosition.isNull())
    {
        positionSelf();
    }
    else
    {
        if (d->popupStyle == Balloon)
        {
            setAnchor(d->fixedPosition);
        }
        else
        {
            move(d->fixedPosition);
        }
    }

    QFrame::setVisible(/*visible=*/ true);

    int delay = d->hideDelay;

    if (delay < 0)
    {
        delay = DEFAULT_POPUP_TIME;
    }

    if (delay > 0)
    {
        d->hideTimer->start(delay);
    }
}

void DNotificationPopup::show(const QPoint& p)
{
    d->fixedPosition = p;
    show();
}

void DNotificationPopup::hideEvent(QHideEvent*)
{
    d->hideTimer->stop();

    if (d->autoDelete)
    {
        deleteLater();
    }
}

QPoint DNotificationPopup::defaultLocation() const
{
    const QRect r = QGuiApplication::primaryScreen()->availableGeometry();
    return QPoint(r.left(), r.top());
}

void DNotificationPopup::positionSelf()
{
    QRect target;

    if (d->window)
    {
        // Put it by the window itself

        if (target.isNull())
        {
            // Avoid making calls to the window system if we can
            QWidget* const widget = QWidget::find(d->window);

            if (widget)
            {
                target = widget->geometry();
            }
        }
    }

    if (target.isNull())
    {
        target = QRect(defaultLocation(), QSize(0, 0));
    }

    moveNear(target);
}

void DNotificationPopup::moveNear(const QRect& target)
{
    QPoint pos = d->calculateNearbyPoint(target);

    if (d->popupStyle == Balloon)
    {
        setAnchor(pos);
    }
    else
    {
        move(pos.x(), pos.y());
    }
}

QPoint DNotificationPopup::anchor() const
{
    return d->anchor;
}

void DNotificationPopup::setAnchor(const QPoint& anchor)
{
    d->anchor = anchor;
    d->updateMask();
}

void DNotificationPopup::paintEvent(QPaintEvent* pe)
{
    if (d->popupStyle == Balloon)
    {
        QPainter p;
        p.begin(this);
        p.drawPolygon(d->surround);
    }
    else
    {
        QFrame::paintEvent(pe);
    }
}

DNotificationPopup* DNotificationPopup::message(const QString& caption, const QString& text,
                                                const QPixmap& icon, QWidget* parent, int timeout, const QPoint& p)
{
    return message(DEFAULT_POPUP_TYPE, caption, text, icon, parent, timeout, p);
}

DNotificationPopup* DNotificationPopup::message(const QString& text, QWidget* parent, const QPoint& p)
{
    return message(DEFAULT_POPUP_TYPE, QString(), text, QPixmap(), parent, -1, p);
}

DNotificationPopup* DNotificationPopup::message(const QString& caption, const QString& text,
                                                QWidget* parent, const QPoint& p)
{
    return message(DEFAULT_POPUP_TYPE, caption, text, QPixmap(), parent, -1, p);
}

DNotificationPopup* DNotificationPopup::message(const QString& caption, const QString& text,
                                                const QPixmap& icon, WId parent, int timeout, const QPoint& p)
{
    return message(DEFAULT_POPUP_TYPE, caption, text, icon, parent, timeout, p);
}

DNotificationPopup* DNotificationPopup::message(const QString& caption, const QString& text,
                                                const QPixmap& icon, QSystemTrayIcon* parent, int timeout)
{
    return message(DEFAULT_POPUP_TYPE, caption, text, icon, parent, timeout);
}

DNotificationPopup* DNotificationPopup::message(const QString& text, QSystemTrayIcon* parent)
{
    return message(DEFAULT_POPUP_TYPE, QString(), text, QPixmap(), parent);
}

DNotificationPopup* DNotificationPopup::message(const QString& caption, const QString& text,
                                                QSystemTrayIcon* parent)
{
    return message(DEFAULT_POPUP_TYPE, caption, text, QPixmap(), parent);
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& caption, const QString& text,
                                                const QPixmap& icon, QWidget* parent, int timeout, const QPoint& p)
{
    DNotificationPopup* const pop = new DNotificationPopup(parent);
    pop->setPopupStyle(popupStyle);
    pop->setAutoDelete(true);
    pop->setView(caption, text, icon);
    pop->d->hideDelay = timeout < 0 ? DEFAULT_POPUP_TIME : timeout;

    if (p.isNull())
    {
        pop->show();
    }
    else
    {
        pop->show(p);
    }

    return pop;
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& text, QWidget* parent, const QPoint& p)
{
    return message(popupStyle, QString(), text, QPixmap(), parent, -1, p);
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& caption, const QString& text,
                                                QWidget* parent, const QPoint& p)
{
    return message(popupStyle, caption, text, QPixmap(), parent, -1, p);
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& caption, const QString& text,
                                                const QPixmap& icon, WId parent, int timeout, const QPoint& p)
{
    DNotificationPopup* const pop = new DNotificationPopup(parent);
    pop->setPopupStyle(popupStyle);
    pop->setAutoDelete(true);
    pop->setView(caption, text, icon);
    pop->d->hideDelay = timeout < 0 ? DEFAULT_POPUP_TIME : timeout;

    if (p.isNull())
    {
        pop->show();
    }
    else
    {
        pop->show(p);
    }

    return pop;
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& caption, const QString& text,
                                                const QPixmap& icon, QSystemTrayIcon* parent, int timeout)
{
    DNotificationPopup* const pop = new DNotificationPopup();
    pop->setPopupStyle(popupStyle);
    pop->setAutoDelete(true);
    pop->setView(caption, text, icon);
    pop->d->hideDelay = timeout < 0 ? DEFAULT_POPUP_TIME : timeout;
    QPoint pos = pop->d->calculateNearbyPoint(parent->geometry());
    pop->show(pos);
    pop->moveNear(parent->geometry());

    return pop;
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& text, QSystemTrayIcon* parent)
{
    return message(popupStyle, QString(), text, QPixmap(), parent);
}

DNotificationPopup* DNotificationPopup::message(int popupStyle, const QString& caption, const QString& text,
                                                QSystemTrayIcon* parent)
{
    return message(popupStyle, caption, text, QPixmap(), parent);
}

} // namespace Digikam

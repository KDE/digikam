/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-02
 * Description : a pop-up menu to show stars rating selection.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ratingpopupmenu.h"
#include "ratingpopupmenu.moc"

// Qt includes.

#include <QString>
#include <QPainter>
#include <QPixmap>
#include <QPolygon>
#include <QWidgetAction>
#include <QLabel>
#include <QMouseEvent>
#include <QSize>
#include <QPaintEvent>
#include <QEvent>
#include <QObject>

// KDE includes.

#include <klocale.h>
#include <kstandarddirs.h>

// Local includes.

#include "constants.h"
#include "themeengine.h"

namespace Digikam
{

class RatingMenuItem : public QWidget
{

  public:

    RatingMenuItem(int star, QWidget *parent=0) : QWidget(parent)
    {
        m_hightlighted = false;
        m_star         = star;

        QPolygon starPolygon;

        // Pre-computed star polygon for a 15x15 pixmap.
        starPolygon << QPoint(0,  6);
        starPolygon << QPoint(5,  5);
        starPolygon << QPoint(7,  0);
        starPolygon << QPoint(9,  5);
        starPolygon << QPoint(14, 6);
        starPolygon << QPoint(10, 9);
        starPolygon << QPoint(11, 14);
        starPolygon << QPoint(7,  11);
        starPolygon << QPoint(3,  14);
        starPolygon << QPoint(4,  9);

        // Template of one star.
        QPixmap starPix(15, 15);
        starPix.fill(Qt::transparent);
        QPainter p1(&starPix);
        p1.setRenderHint(QPainter::Antialiasing, true);
        p1.setBrush(ThemeEngine::instance()->textSpecialRegColor());
        p1.setPen(palette().color(QPalette::Active, QPalette::Foreground));
        p1.drawPolygon(starPolygon, Qt::WindingFill);
        p1.end();

        // Set widget properties.
        setMouseTracking(true);
        setAttribute(Qt::WA_DeleteOnClose);
        setFixedSize(QSize(4 + starPix.width()*RatingMax, starPix.height()+4));

        // Regular rating pixmap of m_star.
        m_ratingPixReg = QPixmap(contentsRect().size());
        m_ratingPixReg.fill(Qt::transparent);
        QPainter p2(&m_ratingPixReg);
        p2.drawTiledPixmap(2, 2, m_star*starPix.width(), starPix.height(), starPix);
        p2.end();

        // Selected rating pixmap of m_star.
        m_ratingPixSel = QPixmap(contentsRect().size());
        m_ratingPixSel.fill(palette().color(QPalette::Active, QPalette::Highlight));
        QPainter p3(&m_ratingPixSel);
        p3.drawTiledPixmap(2, 2, m_star*starPix.width(), starPix.height(), starPix);
        p3.end();
    }

    void setHightLighted(bool h)
    {
        m_hightlighted = h;
        update();
    }

  private:

    void paintEvent(QPaintEvent *)
    {
        QPainter p(this);
        p.drawPixmap(contentsRect(), m_hightlighted ? m_ratingPixSel : m_ratingPixReg);
        p.end();
    }

  private:

    bool     m_hightlighted;
    int      m_star;

    QPixmap  m_ratingPixReg;
    QPixmap  m_ratingPixSel;
};

RatingPopupMenu::RatingPopupMenu(QWidget* parent)
               : KMenu(parent)
{
    QAction *action = addAction(i18nc("assign no rating", "None"), this, SLOT(slotRatingTriggered()));
    action->setData(0);

    for (int i = 1 ; i <= RatingMax ; i++)
    {
        QWidgetAction *action2 = new QWidgetAction(this);
        RatingMenuItem *item   = new RatingMenuItem(i);
        action2->setDefaultWidget(item);
        action2->setData(i);
        connect(action2, SIGNAL(triggered()), this, SLOT(slotRatingTriggered()));
        addAction(action2);
    }

    connect(this, SIGNAL(hovered(QAction *)),
            this, SLOT(slotHovered(QAction *)));
}

RatingPopupMenu::~RatingPopupMenu()
{
}

void RatingPopupMenu::slotRatingTriggered()
{
    int r = qobject_cast<QAction*>(sender())->data().toInt();
    emit signalRatingChanged(r);
}

void RatingPopupMenu::slotHovered(QAction *current)
{
    QList<QAction*> list = actions();
    foreach(QAction *action, list)
    {
        if (action->data().toInt() > 0)
        {
            QWidget *w         = qobject_cast<QWidgetAction*>(action)->defaultWidget();
            RatingMenuItem *mi = dynamic_cast<RatingMenuItem*>(w);
            mi->setHightLighted(false);
        }
    }

    if (current->data().toInt() > 0)
    {
        QWidget *w         = qobject_cast<QWidgetAction*>(current)->defaultWidget();
        RatingMenuItem *mi = dynamic_cast<RatingMenuItem*>(w);
        mi->setHightLighted(true);
    }
}

}  // namespace Digikam

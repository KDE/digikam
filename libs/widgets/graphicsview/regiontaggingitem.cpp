/** ===========================================================
 *
 * This file is a part of digiKam project
 * <a href="http://www.digikam.org">http://www.digikam.org</a>
 *
 * @date   2010-06-21
 * @brief  Region tagging widget
 *
 * @author Copyright (C) 2010 by Alex Jironkin
 *         <a href="mailto:alexjironkin at gmail dot com">alexjironkin at gmail dot com</a>
 * @author Copyright (C) 2010 by Aditya Bhatt
 *         <a href="mailto:adityabhatt1991 at gmail dot com">adityabhatt1991 at gmail dot com</a>
 * @author Copyright (C) 2010 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "regiontaggingitem.moc"

// Qt includes

#include <QWidget>
#include <QPainter>
#include <QGraphicsOpacityEffect>
#include <QTextDocument>
#include <QGraphicsScene>

// KDE include

#include <kdebug.h>
#include <kstandarddirs.h>

// Local includes

#include "button.h"

namespace Digikam{

class RegionTaggingItemPriv
{
public:

    RegionTaggingItemPriv()
    {
        faceRect      = 0;
        faceName      = 0;
        nameRect      = 0;
        approveButton = 0;
        rejectButton  = 0;
    }

    int                sceneWidth, sceneHeight;
    int                x1, x2, y1, y2;
    QGraphicsRectItem* faceRect;
    QGraphicsTextItem* faceName;
    QGraphicsRectItem* nameRect;
    Button*            approveButton;
    Button*            rejectButton;
};

RegionTaggingItem::RegionTaggingItem(QGraphicsItem* parent, QGraphicsScene* scene, const QRect& rect, double scale, const QString& name)
        : QGraphicsObject(parent), d(new RegionTaggingItemPriv)
{
    setAcceptHoverEvents(true);

    d->faceRect    = new QGraphicsRectItem( 0, scene);
    d->sceneWidth  = scene->width();
    d->sceneHeight = scene->height();

    // Scale all coordinates to fit the initial size of the scene
    d->x1 = rect.topLeft().x()*scale;
    d->y1 = rect.topLeft().y()*scale;
    d->x2 = rect.bottomRight().x()*scale;
    d->y2 = rect.bottomRight().y()*scale;

    // Top-left Coordinates for the name
    int x = d->x1 + 20;
    int y = d->y2 + 10;

    // A QRect containing coordinates for the face rectangle
    QRect scaledRect;
    scaledRect.setTopLeft(QPoint(d->x1, d->y1));
    scaledRect.setBottomRight(QPoint(d->x2, d->y2));

    // Draw the Face Rectangle as a QGraphicsRectItem using the dimensions of the scaledRect
    d->faceRect->setRect(scaledRect);
    QPen pen(QColor(QString("red")));
    pen.setWidth(3);
    d->faceRect->setPen(pen);
    d->faceRect->setOpacity(1);

    // Make a new QGraphicsTextItem for writing the name text, and a new QGraphicsRectItem to draw a good-looking, semi-transparent bounding box.
    d->nameRect = new QGraphicsRectItem( 0, scene);
    d->faceName = new QGraphicsTextItem (name, 0, scene);

    // Make the bounding box for the name update itself to cover all the text whenever contents are changed
    QTextDocument* doc;
    doc = d->faceName->document();
    QTextOption o;
    o.setAlignment(Qt::AlignCenter);
    doc->setDefaultTextOption(o);

    // ----------------
    // Set coordinates of the name
    d->faceName->setPos(x,y);

    // Get coordinates of the name relative the the scene
    QRectF r = d->faceName->mapRectToScene(d->faceName->boundingRect());

    // Draw the bounding name rectangle with the scene coordinates
    d->nameRect->setRect(r);
    QPen p(QColor(QString("black")));
    p.setWidth(3);
    d->nameRect->setPen(p);
    d->nameRect->setBrush(QBrush(QColor(QString("black"))));
    d->nameRect->setOpacity(0.8);

    // Draw the name input item
    d->faceName->setDefaultTextColor(QColor(QString("white")));
    d->faceName->setFont(QFont("Helvetica", 8));
    d->faceName->setTextInteractionFlags(Qt::TextEditorInteraction);
    d->faceName->setOpacity(1);

    //---------------------

    d->approveButton = new Button( KStandardDirs::locate("data", "regiontaggingwidget/icons/button-approve-normal.png"),
                                KStandardDirs::locate("data", "regiontaggingwidget/icons/button-approve-pressed.png") );

    d->rejectButton  = new Button( KStandardDirs::locate("data", "regiontaggingwidget/icons/button-reject-normal.png"),
                                KStandardDirs::locate("data", "regiontaggingwidget/icons/button-reject-pressed.png") );

    d->approveButton->hide();
    d->rejectButton->hide();

    scene->addItem(d->approveButton);
    scene->addItem(d->rejectButton);

    d->approveButton->setPos(x-40, y);
    d->rejectButton->setPos(x-20, y);


    connect(d->rejectButton, SIGNAL(clicked()),
            this, SLOT(clearText()) );

    connect(doc, SIGNAL(contentsChanged()),
            this, SLOT(update()));
}

RegionTaggingItem::~RegionTaggingItem()
{
    delete d;
}

QRectF RegionTaggingItem::boundingRect() const
{
    qreal adjust = 0.5;
    return QRectF(-18 - adjust, -22 - adjust,
                  36 + adjust, 60 + adjust);
}

void RegionTaggingItem::paint(QPainter* /*painter*/, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
}

void RegionTaggingItem::setText(const QString& newName)
{
    d->faceName->setPlainText(newName);
}

QString RegionTaggingItem::text() const
{
    return d->faceName->toPlainText();
}

void RegionTaggingItem::update()
{
    if(d->faceName->toPlainText() == QString())
    {
        d->approveButton->hide();
        d->rejectButton->hide();
    }
    else
    {
        d->approveButton->show();
        d->rejectButton->show();
    }

    QRectF r = d->faceName->mapRectToScene(d->faceName->boundingRect());
    d->nameRect->setRect(r);
}

void RegionTaggingItem::setVisible(bool visible)
{
    d->faceRect->setVisible(visible);
    setControlsVisible(visible);
}

void RegionTaggingItem::setControlsVisible(bool visible)
{
    d->nameRect->setVisible(visible);
    d->faceName->setVisible(visible);
    d->approveButton->setVisible(visible);
    d->rejectButton->setVisible(visible);
}

void RegionTaggingItem::clearText()
{
    d->faceName->setPlainText(QString());
}

} // Namespace Digikam
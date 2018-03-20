/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-21
 * Description : GUI test program for FacesEngine
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Alex Jironkin <alexjironkin at gmail dot com>
 * Copyright (C)      2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef FACEITEM_H
#define FACEITEM_H

// Qt includes

#include <QtCore/QtGlobal>
#include <QtCore/QObject>
#include <QtWidgets/QGraphicsObject>
#include <QtWidgets/QGraphicsRectItem>
#include <QtGui/QBrush>
#include <QtGui/QFont>

// Local includes

#include "button.h"

namespace Digikam
{

class FaceItem : public QGraphicsObject
{
    Q_OBJECT

public:

    explicit FaceItem(QGraphicsItem* const parent = 0, QGraphicsScene* const scene = 0,
                      const QRect& rect = QRect(0, 0, 0, 0),
                      double scale = 1, const QString& name = QString(), double originalscale = 1);
    ~FaceItem();

    QRectF boundingRect() const;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

    void setText(const QString& newName);
    QString text() const;

    QRect originalRect()   const;
    double originalScale() const ;

    void setVisible(bool);
    void setControlsVisible(bool);

Q_SIGNALS:

    void acceptButtonClicked(const QString&, const QRect&);
    void rejectButtonClicked(const QString&, const QRect&);

    void suggestionAcceptButtonClicked(const QString&, const QRect&);
    void suggestionRejectButtonClicked(const QString&, const QRect&);

public Q_SLOTS:

    void update();
    void clearText();
    void clearAndHide();
    void accepted();
    void reject();

    void suggest(const QString& name);
    void switchToEditMode();
    void switchToSuggestionMode();

    void slotSuggestionAccepted();
    void slotSuggestionRejected();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FACEITEM_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-17
 * Description : Managing of face tag region items on a GraphicsDImgView
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FACEGROUP_H
#define FACEGROUP_H

// Qt includes

#include <QObject>
#include <QPointF>

// Local includes

#include "itemvisibilitycontroller.h"

class QGraphicsSceneHoverEvent;

namespace Digikam
{

class ImageInfo;
class GraphicsDImgView;
class RegionFrameItem;
class TaggingAction;

class FaceGroup : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)

public:

    /**
     * Constructs a new face group, managing RegionFrameItems for faces
     * of a particular image, displayed on a GraphicsDImgView.
     */
    explicit FaceGroup(GraphicsDImgView* const view);
    ~FaceGroup();

    bool isVisible() const;
    ImageInfo info() const;
    bool hasVisibleItems() const;

    QList<RegionFrameItem*> items() const;

    /**
     * Auto suggest applies if an image has not been scanned,
     * or an unknown face is registered.
     * In this case, a new scan will be triggered.
     */
    void setAutoSuggest(bool doAutoSuggest);
    bool autoSuggest() const;

    /**
     * Even if visible() is false, show the item under the mouse cursor
     */
    void setShowOnHover(bool show);
    bool showOnHover() const;

    /**
     * Returns the item in this group closest to scene position p.
     * If given, manhattanLength is set to the manhattan length between
     * p and the closest point of the returned item's bounding rectangle.
     * In particular, if p is inside the item's rectangle, manhattanLength is 0.
     */
    RegionFrameItem* closestItem(const QPointF& p, qreal* const manhattanLength = 0) const;

    bool acceptsMouseClick(const QPointF& scenePos);
    void itemHoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void itemHoverMoveEvent(QGraphicsSceneHoverEvent* event);
    void itemHoverLeaveEvent(QGraphicsSceneHoverEvent* event);
    void leaveEvent(QEvent*);
    void enterEvent(QEvent*);

public Q_SLOTS:

    /** Shows or hides the frames */
    void setVisible(bool visible);
    void setVisibleItem(RegionFrameItem* item);

    /** Sets the current ImageInfo */
    void setInfo(const ImageInfo& info);

    /** Prepares to load a new info.
     *  Closes the face group for editing.
     *  Pass a null info if about to close. */
    void aboutToSetInfo(const ImageInfo& info);

    void aboutToSetInfoAfterRotate(const ImageInfo& info);

    /** Enters a special state where by click + drag a new face can be created */
    void addFace();

    /** Rejects (clears) all faces on the current image */
    void rejectAll();

Q_SIGNALS:

protected:

    void load();
    void clear();
    void applyItemGeometryChanges();

protected Q_SLOTS:

    void itemStateChanged(int);
    void startAutoSuggest();
    void slotAssigned(const TaggingAction& action, const ImageInfo& info, const QVariant& faceIdentifier);
    void slotRejected(const ImageInfo& info, const QVariant& faceIdentifier);
    void slotLabelClicked(const ImageInfo& info, const QVariant& faceIdentifier);
    void slotAddItemStarted(const QPointF& pos);
    void slotAddItemMoving(const QRectF& rect);
    void slotAddItemFinished(const QRectF& rect);
    void cancelAddItem();

private:

    class Private;
    Private* const d;
};

// ---------------------------------------------------------------------------------

class FaceItem;

class AssignNameWidgetHidingStateChanger : public HidingStateChanger
{
    Q_OBJECT

public:

    explicit AssignNameWidgetHidingStateChanger(FaceItem* const item);

protected Q_SLOTS:

    void slotStateChanged();
};

} // namespace Digikam

#endif // FACEGROUP_H

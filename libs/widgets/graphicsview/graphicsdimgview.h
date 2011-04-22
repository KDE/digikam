/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View for DImg preview
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

#ifndef GRAPHICSDIMGVIEW_H
#define GRAPHICSDIMGVIEW_H

// Qt includes

#include <QGraphicsView>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DImgPreviewItem;
class SinglePhotoPreviewLayout;

class DIGIKAM_EXPORT GraphicsDImgView : public QGraphicsView
{
    Q_OBJECT

public:

    GraphicsDImgView(QWidget* parent = 0);
    ~GraphicsDImgView();

    DImgPreviewItem*          previewItem() const;
    SinglePhotoPreviewLayout* layout()      const;

    /** Scrolls the view such that scenePos (in scene coordinates
     *  is displayed on the viewport at viewportPos (in viewport coordinates).
     *  E.g., calling scrollPointOnPoint(scenePos, viewport()->rect().center()) is
     *  equivalent to calling centerOn(scenePos). */
    void scrollPointOnPoint(const QPointF& scenePos, const QPoint& viewportPos);

    int contentsX() const;
    int contentsY() const;

Q_SIGNALS:

    void contentsMoving(int, int);
    void rightButtonClicked();
    void leftButtonClicked();
    void leftButtonDoubleClicked();
    void activated();
    void toNextImage();
    void toPreviousImage();
    void contentsMoved(bool panningFinished);
    void resized();
    //void contentTakeFocus();

    void viewportRectChanged(const QRectF& viewportRect);

protected:

    void drawForeground(QPainter* painter, const QRectF& rect);
    void drawText(QPainter* p, const QRectF& rect, const QString& text);

    void setItem(DImgPreviewItem* item);
    void installPanIcon();

    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void wheelEvent(QWheelEvent*);
    void resizeEvent(QResizeEvent*);

    void startPanning(const QPoint& pos);
    void continuePanning(const QPoint& pos);
    void finishPanning();

    virtual bool acceptsMouseClick(QMouseEvent* e);

    virtual void scrollContentsBy(int dx, int dy);

protected Q_SLOTS:

    void         slotContentsMoved();
    void         slotCornerButtonPressed();
    void         slotPanIconHidden();
    virtual void slotPanIconSelectionMoved(const QRect&, bool);

private:

    class GraphicsDImgViewPriv;
    GraphicsDImgViewPriv* const d;
};

} // namespace Digikam

#endif // GRAPHICSDIMGVIEW_H

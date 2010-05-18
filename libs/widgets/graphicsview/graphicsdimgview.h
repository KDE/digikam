/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View for DImg preview
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

class GraphicsDImgViewPriv;
class DImgPreviewItem;
class SinglePhotoPreviewLayout;

class DIGIKAM_EXPORT GraphicsDImgView : public QGraphicsView
{
    Q_OBJECT

public:

    GraphicsDImgView(QWidget* parent = 0);
    ~GraphicsDImgView();

    DImgPreviewItem* previewItem() const;
    SinglePhotoPreviewLayout* layout() const;

Q_SIGNALS:

    void rightButtonClicked();
    void leftButtonClicked();
    void leftButtonDoubleClicked();
    void activated();
    void toNextImage();
    void toPreviousImage();
    void contentsMoved(bool panningFinished);
    void resized();
    //void contentTakeFocus();

protected:

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

protected Q_SLOTS:

    void slotCornerButtonPressed();
    void slotPanIconHidden();
    virtual void slotPanIconSelectionMoved(const QRect&, bool);

private:

    GraphicsDImgViewPriv* const d;
};

/*
class DIGIKAM_EXPORT ImagePreviewView : public GraphicsDImgView
{
    void setImageInfo(const ImageInfo& info = ImageInfo(),
                      const ImageInfo& previous = ImageInfo(),
                      const ImageInfo& next = ImageInfo());

    void setImagePath(const QString& path,
                      const QString& previous = QString(),
                      const QString& next = QString());
};
*/

} // namespace Digikam

#endif // GRAPHICSDIMGVIEW_H

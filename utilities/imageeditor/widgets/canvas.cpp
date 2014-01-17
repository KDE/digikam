/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-04
 * Description : image editor canvas management class
 *
 * Copyright (C) 2013-2014 by Yiou Wang <geow812 at gmail dot com>
 * Copyright (C) 2004-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2004-2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "canvas.moc"

// Qt includes

#include <QFileInfo>
#include <QClipboard>
#include <QToolButton>
#include <QScrollBar>

// KDE includes

#include <kcursor.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdatetable.h>
#include <kglobalsettings.h>
#include <kapplication.h>

// Local includes

#include "autocrop.h"
#include "imagehistogram.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "exposurecontainer.h"
#include "iofilesettings.h"
#include "loadingcacheinterface.h"
#include "imagepreviewitem.h"
#include "previewlayout.h"
#include "imagezoomsettings.h"
#include "clickdragreleaseitem.h"
#include "rubberitem.h"

namespace Digikam
{

class Canvas::Private
{

public:

    Private()
    {
        rubber     = 0;
        wrapItem   = 0;
        canvasItem = 0;
        im         = 0;
    }

    QString               errorMessage;

    ImagePreviewItem*     canvasItem;

    RubberItem*           rubber;
    ClickDragReleaseItem* wrapItem;
    EditorCore*           im;
};

Canvas::Canvas(QWidget* const parent)
    : GraphicsDImgView(parent), d(new Private)
{
    d->im         = new EditorCore();
    d->canvasItem = new ImagePreviewItem;
    setItem(d->canvasItem);

    setFrameStyle(QFrame::NoFrame);
    addRubber();
    layout()->fitToWindow();
    installPanIcon();

    // ------------------------------------------------------------

    connect(d->im, SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d->im, SIGNAL(signalLoadingStarted(QString)),
            this, SIGNAL(signalLoadingStarted(QString)));

    connect(d->im, SIGNAL(signalImageLoaded(QString,bool)),
            this, SLOT(slotImageLoaded(QString,bool)));

    connect(d->im, SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    connect(d->im, SIGNAL(signalLoadingProgress(QString,float)),
            this, SIGNAL(signalLoadingProgress(QString,float)));

    connect(d->im, SIGNAL(signalSavingStarted(QString)),
            this, SIGNAL(signalSavingStarted(QString)));

    connect(d->im, SIGNAL(signalSavingProgress(QString,float)),
            this, SIGNAL(signalSavingProgress(QString,float)));

    connect(this, SIGNAL(signalSelected(bool)),
            this, SLOT(slotSelected()));

    connect(d->canvasItem, SIGNAL(showContextMenu(QGraphicsSceneContextMenuEvent*)),
            this, SIGNAL(signalRightButtonClicked()));

    connect(layout(), SIGNAL(zoomFactorChanged(double)),
            this, SIGNAL(signalZoomChanged(double)));
}

Canvas::~Canvas()
{
    delete d->im;
    delete d->canvasItem;
    delete d;
}

void Canvas::resetImage()
{
    reset();
    d->im->resetImage();
}

void Canvas::reset()
{
    if (d->rubber && d->rubber->isVisible())
    {
        d->rubber->setVisible(false);

        if (d->im->imageValid())
        {
            emit signalSelected(false);
        }
    }

    addRubber();
    d->errorMessage.clear();
}

void Canvas::load(const QString& filename, IOFileSettings* const IOFileSettings)
{
    reset();
    emit signalPrepareToLoad();
    d->im->load(filename, IOFileSettings);
}

void Canvas::slotImageLoaded(const QString& filePath, bool success)
{
    d->canvasItem->setImage(d->im->getImg()->copyImageData());

    // Note: in showFoto, we using a null filename to clear canvas.
    if (!success && !filePath.isEmpty())
    {
        QFileInfo info(filePath);
        d->errorMessage = i18n("Failed to load image\n\"%1\"", info.fileName());
    }
    else
    {
        d->errorMessage.clear();
    }

    viewport()->update();

    emit signalLoadingFinished(filePath, success);
}

void Canvas::fitToSelect()
{
    QRect sel = d->im->getSelectedArea();

    if (!sel.size().isNull())
    {
        // If selected area, use center of selection
        // and recompute zoom factor accordingly.
        //double cpx       = sel.x() + sel.width()  / 2.0;
        //double cpy       = sel.y() + sel.height() / 2.0;
        double srcWidth  = sel.width();
        double srcHeight = sel.height();
        double dstWidth  = contentsRect().width();
        double dstHeight = contentsRect().height();
        double zoom      = qMin(dstWidth / srcWidth, dstHeight / srcHeight);

        emit signalToggleOffFitToWindow();

        layout()->setZoomFactor(zoom);
        //emit layout()->zoomFactorChanged(zoom);

        //centerOn(cpx * d->zoom, cpy * d->zoom);
        viewport()->update();
    }
}
void Canvas::applyTransform(const IccTransform& t)
{
    IccTransform transform(t);

    if (transform.willHaveEffect())
    {
        d->im->applyTransform(transform);
    }
    else
    {
        d->im->updateColorManagement();
        viewport()->update();
    }
}

void Canvas::preload(const QString& /*filename*/)
{
    //    d->im->preload(filename);
}

void Canvas::slotImageSaved(const QString& filePath, bool success)
{
    emit signalSavingFinished(filePath, success);
}

void Canvas::abortSaving()
{
    d->im->abortSaving();
}

void Canvas::setModified()
{
    d->im->setModified();
}

QString Canvas::ensureHasCurrentUuid() const
{
    return d->im->ensureHasCurrentUuid();
}

DImg Canvas::currentImage() const
{
    DImg* const image = d->im->getImg();

    if (image)
    {
        return DImg(*image);
    }

    return DImg();
}

QString Canvas::currentImageFileFormat() const
{
    return d->im->getImageFormat();
}

QString Canvas::currentImageFilePath() const
{
    return d->im->getImageFilePath();
}

int Canvas::imageWidth() const
{
    return d->im->origWidth();
}

int Canvas::imageHeight() const
{
    return d->im->origHeight();
}

bool Canvas::isReadOnly() const
{
    return d->im->isReadOnly();
}

QRect Canvas::getSelectedArea() const
{
    return d->im->getSelectedArea();
}

QRect Canvas::visibleArea() const
{
    return mapToScene(viewport()->geometry()).boundingRect().toRect();
}

EditorCore* Canvas::interface() const
{
    return d->im;
}

void Canvas::makeDefaultEditingCanvas()
{
    EditorCore::setDefaultInstance(d->im);
}

bool Canvas::exifRotated() const
{
    return d->im->exifRotated();
}

void Canvas::slotRotate90()
{
    d->canvasItem->clearCache();
    d->im->rotate90();
}

void Canvas::slotRotate180()
{
    d->canvasItem->clearCache();
    d->im->rotate180();
}

void Canvas::slotRotate270()
{
    d->canvasItem->clearCache();
    d->im->rotate270();
}

void Canvas::slotFlipHoriz()
{
    d->canvasItem->clearCache();
    d->im->flipHoriz();
}

void Canvas::slotFlipVert()
{
    d->canvasItem->clearCache();
    d->im->flipVert();
}

void Canvas::slotAutoCrop()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    d->canvasItem->clearCache();
    AutoCrop ac(d->im->getImg());
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();
    d->im->crop(rect);
    QApplication::restoreOverrideCursor();

    if (d->rubber && d->rubber->isVisible())
    {
        d->rubber->setVisible(false);
    }
}

void Canvas::slotCrop()
{
    d->canvasItem->clearCache();
    QRect sel = d->im->getSelectedArea();

    if (sel.size().isNull())   // No current selection.
    {
        return;
    }

    d->im->crop(sel);

    if (d->rubber && d->rubber->isVisible())
    {
        d->rubber->setVisible(false);
    }
}

void Canvas::setICCSettings(const ICCSettingsContainer& cmSettings)
{
    d->canvasItem->clearCache();
    d->im->setICCSettings(cmSettings);
    viewport()->update();
}

void Canvas::setSoftProofingEnabled(bool enable)
{
    d->canvasItem->clearCache();
    d->im->setSoftProofingEnabled(enable);
    viewport()->update();
}

void Canvas::setExposureSettings(ExposureSettingsContainer* const expoSettings)
{
    d->canvasItem->clearCache();
    d->im->setExposureSettings(expoSettings);
    viewport()->update();
}

void Canvas::setExifOrient(bool exifOrient)
{
    d->canvasItem->clearCache();
    d->im->setExifOrient(exifOrient);
    viewport()->update();
}

void Canvas::slotRestore()
{
    d->im->restore();
    viewport()->update();
}

void Canvas::slotUndo(int steps)
{
    emit signalUndoSteps(steps);

    d->canvasItem->clearCache();

    while (steps > 0)
    {
        d->im->undo();
        --steps;
    }
}

void Canvas::slotRedo(int steps)
{
    emit signalRedoSteps(steps);

    d->canvasItem->clearCache();

    while (steps > 0)
    {
        d->im->redo();
        --steps;
    }
}

void Canvas::slotCopy()
{
    QRect sel = d->im->getSelectedArea();

    if (sel.size().isNull())   // No current selection.
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    DImg selDImg              = d->im->getImgSelection();
    QImage selImg             = selDImg.copyQImage();
    QMimeData* const mimeData = new QMimeData();
    mimeData->setImageData(selImg);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    QApplication::restoreOverrideCursor();
}

void Canvas::slotSelected()
{
    QRect sel = QRect(0, 0, 0, 0);

    if (d->rubber)
    {
        sel = calcSelectedArea();
    }

    d->im->setSelectedArea(sel);
}

QRect Canvas::calcSelectedArea() const
{
    int x = 0, y = 0, w = 0, h = 0;

    if (d->rubber && d->rubber->isVisible())
    {
        QRect r(d->rubber->boundingRect().toRect());

        if (r.isValid())
        {
            r.translate((int)d->rubber->x(), (int)d->rubber->y());

            x = (int)((double)r.x()      / layout()->zoomFactor());
            y = (int)((double)r.y()      / layout()->zoomFactor());
            w = (int)((double)r.width()  / layout()->zoomFactor());
            h = (int)((double)r.height() / layout()->zoomFactor());

            x = qMin(imageWidth(),  qMax(x, 0));
            y = qMin(imageHeight(), qMax(y, 0));
            w = qMin(imageWidth(),  qMax(w, 0));
            h = qMin(imageHeight(), qMax(h, 0));

            // Avoid empty selection by rubberband - at least mark one pixel
            // At high zoom factors, the rubberband may operate at subpixel level!
            if (w == 0)
            {
                w = 1;
            }

            if (h == 0)
            {
                h = 1;
            }
        }
    }

    return QRect(x, y, w, h);
}

void Canvas::slotModified()
{
    d->canvasItem->setImage(currentImage());

    emit signalChanged();
}

void Canvas::slotSelectAll()
{
    if (!d->rubber)
    {
        d->rubber = new RubberItem(d->canvasItem);
        d->rubber->setCanvas(this);
    }
    d->rubber->setRectInSceneCoordinates(d->canvasItem->boundingRect());
    viewport()->setMouseTracking(true);
    viewport()->update();

    if (d->im->imageValid())
    {
        emit signalSelected(true);
    }
}

void Canvas::slotSelectNone()
{
    reset();
    viewport()->update();
}

void Canvas::keyPressEvent(QKeyEvent* event)
{
    if (!event)
    {
        return;
    }

    int mult = 1;

    if ((event->modifiers() & Qt::ControlModifier))
    {
        mult = 10;
    }

    switch (event->key())
    {
        case Qt::Key_Right:
        {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() + horizontalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Left:
        {
            horizontalScrollBar()->setValue(horizontalScrollBar()->value() - horizontalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Up:
        {
            verticalScrollBar()->setValue(verticalScrollBar()->value() - verticalScrollBar()->singleStep()*mult);
            break;
        }

        case Qt::Key_Down:
        {
            verticalScrollBar()->setValue(verticalScrollBar()->value() + verticalScrollBar()->singleStep()*mult);
            break;
        }

        default:
        {
            event->ignore();
            break;
        }
    }
}

void Canvas::addRubber()
{
    if (!d->wrapItem)
    {
        d->wrapItem = new ClickDragReleaseItem(d->canvasItem);
    }

    d->wrapItem->setFocus();
    setFocus();

    connect(d->wrapItem, SIGNAL(started(QPointF)),
            this, SLOT(slotAddItemStarted(QPointF)));

    connect(d->wrapItem, SIGNAL(moving(QRectF)),
            this, SLOT(slotAddItemMoving(QRectF)));

    connect(d->wrapItem, SIGNAL(finished(QRectF)),
            this, SLOT(slotAddItemFinished(QRectF)));

    connect(d->wrapItem, SIGNAL(cancelled()),
            this, SLOT(cancelAddItem()));
}

void Canvas::slotAddItemStarted(const QPointF& pos)
{
    Q_UNUSED(pos);
}

void Canvas::slotAddItemMoving(const QRectF& rect)
{
    if (d->rubber)
    {
        delete d->rubber;
    }
    d->rubber = new RubberItem(d->canvasItem);
    d->rubber->setCanvas(this);
    d->rubber->setRectInSceneCoordinates(rect);
}

void Canvas::slotAddItemFinished(const QRectF& rect)
{
    if (d->rubber)
    {
        d->rubber->setRectInSceneCoordinates(rect);
        //d->wrapItem->stackBefore(d->canvasItem);
    }

    cancelAddItem();
}

void Canvas::cancelAddItem()
{
    if (d->wrapItem)
    {
        this->scene()->removeItem(d->wrapItem);
        d->wrapItem->deleteLater();
        d->wrapItem = 0;
    }

    emit signalSelected(true);
}

void Canvas::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        GraphicsDImgItem* const item = dynamic_cast<GraphicsDImgItem*>(itemAt(event->pos()));

        if (item)
        {
            QString className(item->metaObject()->className());

            if (!(className == "Digikam::RubberItem" || className == "Digikam::ClickDragReleaseItem"))
            {
                if (d->rubber && d->rubber->isVisible())
                {
                    d->rubber->setVisible(false);
                }

                emit signalSelected(false);
                addRubber();
            }
        }
    }

    GraphicsDImgView::mousePressEvent(event);
}

}  // namespace Digikam

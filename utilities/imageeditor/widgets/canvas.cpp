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
 * Copyright (C) 2004-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include "paniconwidget.h"
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

    Private() :
       minZoom(0.1), maxZoom(12.0), zoomMultiplier(1.2)
    {
        panIconPopup     = 0;
        cornerButton     = 0;
        parent           = 0;
        autoZoom         = false;
        fullScreen       = false;
        isWheelEvent     = false;
        zoom             = 1.0;
        initialZoom      = true;
        rubber           = 0;
        wrapItem         = 0;
        canvasItem       = 0;
        im               = 0;
    }

    bool                     autoZoom;
    bool                     fullScreen;
    bool                     initialZoom;
    bool                     isWheelEvent;

    QPoint                   wheelEventPoint;

    double                   zoom;
    const double             minZoom;
    const double             maxZoom;
    const double             zoomMultiplier;

    QToolButton*             cornerButton; // Have to leave the cornerButton here in the view in order to use setCornerWidget

    QPoint                   dragStart;
    QRect                    dragStartRect;

    QColor                   bgColor;

    QWidget*                 parent;
    QPixmap                  qcheck;

    KPopupFrame*             panIconPopup;

    QString                  errorMessage;
    
    ImagePreviewItem*        canvasItem;

    RubberItem*              rubber;
    ClickDragReleaseItem*    wrapItem;
    EditorCore*              im;
};

Canvas::Canvas(QWidget* const parent)
    : GraphicsDImgView(parent), d(new Private)
{
    d->im     = new EditorCore();
    d->parent = parent;
    d->bgColor.setRgb(0, 0, 0);
    d->canvasItem = new ImagePreviewItem;
    setItem(d->canvasItem);

    d->qcheck = QPixmap(16, 16);
    QPainter p(&d->qcheck);
    p.fillRect(0, 0, 8, 8, QColor(144, 144, 144));
    p.fillRect(8, 8, 8, 8, QColor(144, 144, 144));
    p.fillRect(0, 8, 8, 8, QColor(100, 100, 100));
    p.fillRect(8, 0, 8, 8, QColor(100, 100, 100));
    p.end();

    d->cornerButton = PanIconWidget::button();
    setCornerWidget(d->cornerButton);

    QPalette palette;
    palette.setColor(viewport()->backgroundRole(), Qt::WA_NoBackground);
    viewport()->setPalette(palette);
    viewport()->setMouseTracking(false);
    setFrameStyle(QFrame::NoFrame);
    setFocusPolicy(Qt::ClickFocus);

    addRubber();

    layout()->fitToWindow();

    // ------------------------------------------------------------

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

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
    d->canvasItem->zoomSettings()->setZoomFactor(d->zoom);
    d->im->zoom(d->zoom);

    if (d->autoZoom || d->initialZoom)
    {
        d->initialZoom = false;
        updateAutoZoom();
    }

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

    updateContentsSize(true);

    viewport()->update();

    emit signalZoomChanged(d->zoom);

    emit signalLoadingFinished(filePath, success);
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
    DImg* image = d->im->getImg();

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

double Canvas::calcAutoZoomFactor() const
{
    if (!d->im->imageValid())
    {
        return d->zoom;
    }

    double srcWidth  = d->im->origWidth();
    double srcHeight = d->im->origHeight();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();

    return (qMin(dstWidth / srcWidth, dstHeight / srcHeight));
}

void Canvas::updateAutoZoom()
{
    d->zoom = calcAutoZoomFactor();
    d->canvasItem->zoomSettings()->setZoomFactor(d->zoom);
    d->im->zoom(d->zoom);

    emit signalZoomChanged(d->zoom);
}

void Canvas::updateContentsSize(bool deleteRubber)
{
    viewport()->setUpdatesEnabled(false);

    if (deleteRubber && d->rubber && d->rubber->isVisible())
    {
        d->rubber->setVisible(false);
        viewport()->unsetCursor();
        viewport()->setMouseTracking(false);
        addRubber();

        if (d->im->imageValid())
        {
            emit signalSelected(false);
        }
    }

    QPoint center(d->canvasItem->boundingRect().center().toPoint());
    QSize canvasItemSize = QSize(imageWidth()*d->zoom, imageHeight()*d->zoom);
    QPoint topLeft       = QPoint(center.x()- canvasItemSize.width()/2, center.y()-canvasItemSize.height()/2);
    setSceneRect(QRect(topLeft, canvasItemSize));

    if (!d->isWheelEvent)
    {
        centerOn(center);
    }
    else
    {
        centerOn(mapToScene(d->wheelEventPoint));
        d->isWheelEvent = false;
    }

    viewport()->setUpdatesEnabled(true); // necessary
}

bool Canvas::maxZoom() const
{
    return ((d->zoom * d->zoomMultiplier) >= d->maxZoom);
}

bool Canvas::minZoom() const
{
    return ((d->zoom / d->zoomMultiplier) <= d->minZoom);
}

double Canvas::zoomMax() const
{
    return d->maxZoom;
}

double Canvas::zoomMin() const
{
    return d->minZoom;
}

bool Canvas::exifRotated() const
{
    return d->im->exifRotated();
}

double Canvas::snapZoom(double zoom) const
{
    // If the zoom value gets changed from d->zoom to zoom
    // across 50%, 100% or fit-to-window, then return the
    // the corresponding special value. Otherwise zoom is returned unchanged.
    double fit = calcAutoZoomFactor();
    QList<double> snapValues;
    snapValues.append(0.5);
    snapValues.append(1.0);
    snapValues.append(fit);

    if (d->zoom < zoom)
    {
        qStableSort(snapValues);
    }
    else
    {
        qStableSort(snapValues.begin(), snapValues.end(), qGreater<double>());
    }

    for (QList<double>::const_iterator it = snapValues.constBegin(); it != snapValues.constEnd(); ++it)
    {
        double z = *it;

        if ((d->zoom < z) && (zoom > z))
        {
            zoom = z;
            break;
        }
    }

    return zoom;
}

void Canvas::slotIncreaseZoom()
{
    if (maxZoom())
    {
        return;
    }

    double zoom = d->zoom * d->zoomMultiplier;
    zoom        = snapZoom(zoom);
    setZoomFactor(zoom);
}

void Canvas::slotDecreaseZoom()
{
    if (minZoom())
    {
        return;
    }

    double zoom = d->zoom / d->zoomMultiplier;
    zoom        = snapZoom(zoom);
    setZoomFactor(zoom);
}

void Canvas::setZoomFactorSnapped(double zoom)
{
    double fit = calcAutoZoomFactor();

    if (fabs(zoom - fit) < 0.05)
    {
        // If 1.0 or 0.5 are even closer to zoom than fit, then choose these.
        if (fabs(zoom - fit) > fabs(zoom - 1.0))
        {
            zoom = 1.0;
        }
        else if (fabs(zoom - fit) > fabs(zoom - 0.5))
        {
            zoom = 0.5;
        }
        else
        {
            zoom = fit;
        }
    }
    else
    {
        if (fabs(zoom - 1.0) < 0.05)
        {
            zoom = 1.0;
        }

        if (fabs(zoom - 0.5) < 0.05)
        {
            zoom = 0.5;
        }
    }

    setZoomFactor(zoom);
}

double Canvas::zoomFactor() const
{
    return d->zoom;
}

void Canvas::setZoomFactor(double zoom)
{
    if (d->autoZoom)
    {
        d->autoZoom = false;
        emit signalToggleOffFitToWindow();
    }

    d->canvasItem->clearCache();
    d->zoom = zoom;

    d->canvasItem->zoomSettings()->setZoomFactor(d->zoom);

    updateContentsSize(false);

    emit signalZoomChanged(d->zoom);
}

void Canvas::fitToSelect()
{
    QRect sel = d->im->getSelectedArea();

    if (!sel.size().isNull())
    {
        // If selected area, use center of selection
        // and recompute zoom factor accordingly.
        double cpx       = sel.x() + sel.width()  / 2.0;
        double cpy       = sel.y() + sel.height() / 2.0;
        double srcWidth  = sel.width();
        double srcHeight = sel.height();
        double dstWidth  = contentsRect().width();
        double dstHeight = contentsRect().height();
        d->zoom          = qMin(dstWidth / srcWidth, dstHeight / srcHeight);
        d->autoZoom      = false;

        emit signalToggleOffFitToWindow();

        d->canvasItem->zoomSettings()->setZoomFactor(d->zoom);

        updateContentsSize(true);

        centerOn(cpx * d->zoom, cpy * d->zoom);
        viewport()->update();

        emit signalZoomChanged(d->zoom);
    }
}

bool Canvas::fitToWindow() const
{
    return d->autoZoom;
}

void Canvas::toggleFitToWindow()
{
    setFitToWindow(!d->autoZoom);
}

void Canvas::setFitToWindow(bool fit)
{
    d->autoZoom = fit;

    if (d->autoZoom)
    {
        updateAutoZoom();
    }
    else
    {
        d->zoom = 1.0;
        d->canvasItem->zoomSettings()->setZoomFactor(d->zoom);

        emit signalZoomChanged(d->zoom);
    }

    updateContentsSize(false);
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
}

void Canvas::setBackgroundColor(const QColor& color)
{
    if (d->bgColor == color)
    {
        return;
    }

    d->bgColor = color;
    setBackgroundBrush(QBrush(color));
    //viewport()->update();
}

void Canvas::setICCSettings(const ICCSettingsContainer& cmSettings)
{
    d->canvasItem->clearCache();
    ICCSettingsContainer old = d->im->getICCSettings();
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

    DImg selDImg        = d->im->getImgSelection();
    QImage selImg       = selDImg.copyQImage();
    QMimeData* mimeData = new QMimeData();
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

            x = (int)((double)r.x()      / d->zoom);
            y = (int)((double)r.y()      / d->zoom);
            w = (int)((double)r.width()  / d->zoom);
            h = (int)((double)r.height() / d->zoom);

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
    if (d->autoZoom)
    {
        updateAutoZoom();
    }

    d->canvasItem->setImage(currentImage());
    updateContentsSize(true);

    emit signalChanged();
}

void Canvas::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
    }

    d->panIconPopup          = new KPopupFrame(this);
    PanIconWidget* const pan = new PanIconWidget(d->panIconPopup);

    connect(pan, SIGNAL(signalSelectionMoved(QRect,bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect,bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHidden()));

    QRect visibleRect = visibleArea();
    QRect r((int)visibleRect.topLeft().x()/d->zoom, (int)visibleRect.topLeft().y()/d->zoom,
            (int)visibleRect.width()/d->zoom, (int)visibleRect.height()/d->zoom);
    pan->setImage(180, 120, d->im->getImg()->copyQImage());
    pan->setRegionSelection(r);
    pan->setMouseFocus();
    d->panIconPopup->setMainWidget(pan);

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x() + viewport()->size().width());
    g.setY(g.y() + viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(),
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void Canvas::slotPanIconHidden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void Canvas::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*d->zoom), (int)(r.y()*d->zoom));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
        d->panIconPopup = 0;
        slotPanIconHidden();
    }
}

void Canvas::slotSelectAll()
{
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
    GraphicsDImgView::mousePressEvent(event);
    GraphicsDImgItem* const item = (GraphicsDImgItem*)itemAt(event->pos());

    if (item)
    {
        QString className(item->metaObject()->className());
        if (className == "Digikam::RubberItem" || className == "Digikam::ClickDragReleaseItem")
        {
            return;
        }
        else
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

void Canvas::wheelEvent(QWheelEvent* event)
{
    GraphicsDImgView::wheelEvent(event);
    event->accept();

    if (event->modifiers() & Qt::ShiftModifier)
    {
        if (event->delta() < 0)
        {
            emit signalShowNextImage();
        }
        else if (event->delta() > 0)
        {
            emit signalShowPrevImage();
        }

        return;
    }
    else if (event->modifiers() & Qt::ControlModifier)
    {

        d->isWheelEvent    = true;
        d->wheelEventPoint = event->pos();

        if (event->delta() < 0)
        {
            slotDecreaseZoom();
        }
        else if (event->delta() > 0)
        {
            slotIncreaseZoom();
        }

        return;
    }
}

}  // namespace Digikam

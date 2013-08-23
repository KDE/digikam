/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-08-04
 * Description : image editor canvas management class
 *
 * Copyright (C) 2004-2005 by Yiou Wang <geow812 at gmail dot com>
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

// C++ includes


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
#include "canvasitem.h"
#include "previewlayout.h"
#include "imagezoomsettings.h"

namespace Digikam
{

class Canvas::Private
{

public:

    Private() :
       minZoom(0.1), maxZoom(12.0), zoomMultiplier(1.2)
    {
        pressedMoved     = false;
        pressedMoving    = false;
        ltActive         = false;
        rtActive         = false;
        lbActive         = false;
        rbActive         = false;
        lsActive         = false;
        rsActive         = false;
        bsActive         = false;
        tsActive         = false;
        dragActive       = false;
        midButtonPressed = false;
        midButtonX       = 0;
        midButtonY       = 0;
        panIconPopup     = 0;
        cornerButton     = 0;
        parent           = 0;
        //rubber           = 0;
        autoZoom         = false;
        fullScreen       = false;
        zoom             = 1.0;
        initialZoom      = true;
    }

    bool                     autoZoom;
    bool                     fullScreen;
    bool                     pressedMoved;
    bool                     pressedMoving;
    bool                     ltActive;
    bool                     rtActive;
    bool                     lbActive;
    bool                     rbActive;
    bool                     lsActive;
    bool                     rsActive;
    bool                     bsActive;
    bool                     tsActive;
    bool                     dragActive;
    bool                     midButtonPressed;
    bool                     initialZoom;

    int                      midButtonX;
    int                      midButtonY;

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
    
    CanvasItem*              canvasItem;
};

Canvas::Canvas(QWidget* const parent)
    : GraphicsDImgView(parent), d_ptr(new Private)
{
    d_ptr->parent = parent;
    d_ptr->bgColor.setRgb(0, 0, 0);
    d_ptr->canvasItem = new CanvasItem(this);
    setItem(d_ptr->canvasItem);

    d_ptr->qcheck = QPixmap(16, 16);
    QPainter p(&d_ptr->qcheck);
    p.fillRect(0, 0, 8, 8, QColor(144, 144, 144));
    p.fillRect(8, 8, 8, 8, QColor(144, 144, 144));
    p.fillRect(0, 8, 8, 8, QColor(100, 100, 100));
    p.fillRect(8, 0, 8, 8, QColor(100, 100, 100));
    p.end();

    d_ptr->cornerButton = PanIconWidget::button();
    setCornerWidget(d_ptr->cornerButton);

    QPalette palette;
    palette.setColor(viewport()->backgroundRole(), Qt::WA_NoBackground);
    viewport()->setPalette(palette);
    viewport()->setMouseTracking(false);
    setFrameStyle(QFrame::NoFrame);
    setFocusPolicy(Qt::ClickFocus);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalZoomChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d_ptr->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalModified()),
            this, SLOT(slotModified()));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalLoadingStarted(QString)),
            this, SIGNAL(signalLoadingStarted(QString)));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalImageLoaded(QString,bool)),
            this, SLOT(slotImageLoaded(QString,bool)));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalImageSaved(QString,bool)),
            this, SLOT(slotImageSaved(QString,bool)));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalLoadingProgress(QString,float)),
            this, SIGNAL(signalLoadingProgress(QString,float)));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalSavingStarted(QString)),
            this, SIGNAL(signalSavingStarted(QString)));

    connect(d_ptr->canvasItem->im(), SIGNAL(signalSavingProgress(QString,float)),
            this, SIGNAL(signalSavingProgress(QString,float)));

//     connect(this, SIGNAL(signalSelected(bool)),
//             this, SLOT(slotSelected()));

    layout()->fitToWindow();
}

Canvas::~Canvas()
{
    delete d_ptr->canvasItem;
    delete d_ptr;
}

void Canvas::resetImage()
{
    //reset();
    d_ptr->canvasItem->im()->resetImage();
}

// void Canvas::reset()
// {
//     if (d_ptr->rubber->isActive())
//     {
//         d_ptr->rubber->setActive(false);
// 
//         if (d_ptr->canvasItem->im()->imageValid())
//         {
//             emit signalSelected(false);
//         }
//     }
// 
//     d_ptr->errorMessage.clear();
// }

void Canvas::load(const QString& filename, IOFileSettings* const IOFileSettings)
{
    //reset();
    qDebug()<<"Canvas::load()";
    emit signalPrepareToLoad();
    d_ptr->canvasItem->im()->load(filename, IOFileSettings);
}

void Canvas::slotImageLoaded(const QString& filePath, bool success)
{
    qDebug()<<"Canvas::slotImageLoaded()";
    d_ptr->canvasItem->im()->zoom(d_ptr->zoom);

    if (d_ptr->autoZoom || d_ptr->initialZoom)
    {
        d_ptr->initialZoom = false;
        updateAutoZoom();
    }

    // Note: in showFoto, we using a null filename to clear canvas.
    if (!success && !filePath.isEmpty())
    {
        QFileInfo info(filePath);
        d_ptr->errorMessage = i18n("Failed to load image\n\"%1\"", info.fileName());
    }
    else
    {
        d_ptr->errorMessage.clear();
    }

    updateContentsSize(true);

//     viewport()->update();

    emit signalZoomChanged(d_ptr->zoom);

    emit signalLoadingFinished(filePath, success);
}

void Canvas::applyTransform(const IccTransform& t)
{
    IccTransform transform(t);

    if (transform.willHaveEffect())
    {
        d_ptr->canvasItem->im()->applyTransform(transform);
    }
    else
    {
        d_ptr->canvasItem->im()->updateColorManagement();
//         viewport()->update();
    }
}

void Canvas::preload(const QString& /*filename*/)
{
    //    d_ptr->canvasItem->im()->preload(filename);
}

void Canvas::slotImageSaved(const QString& filePath, bool success)
{
    emit signalSavingFinished(filePath, success);
}

void Canvas::abortSaving()
{
    d_ptr->canvasItem->im()->abortSaving();
}

void Canvas::setModified()
{
    d_ptr->canvasItem->im()->setModified();
}

QString Canvas::ensureHasCurrentUuid() const
{
    return d_ptr->canvasItem->im()->ensureHasCurrentUuid();
}

DImg Canvas::currentImage() const
{
    DImg* image = d_ptr->canvasItem->im()->getImg();

    if (image)
    {
        return DImg(*image);
    }

    return DImg();
}

QString Canvas::currentImageFileFormat() const
{
    return d_ptr->canvasItem->im()->getImageFormat();
}

QString Canvas::currentImageFilePath() const
{
    return d_ptr->canvasItem->im()->getImageFilePath();
}

int Canvas::imageWidth() const
{
    return d_ptr->canvasItem->im()->origWidth();
}

int Canvas::imageHeight() const
{
    return d_ptr->canvasItem->im()->origHeight();
}

bool Canvas::isReadOnly() const
{
    return d_ptr->canvasItem->im()->isReadOnly();
}

QRect Canvas::getSelectedArea() const
{
    return (d_ptr->canvasItem->im()->getSelectedArea());
}

QRect Canvas::visibleArea() const
{
    //return (QRect(contentsX(), contentsY(), visibleWidth(), visibleHeight()));
    return (d_ptr->canvasItem->boundingRect().toRect());
}

EditorCore* Canvas::interface() const
{
    return d_ptr->canvasItem->im();
}

void Canvas::makeDefaultEditingCanvas()
{
    EditorCore::setDefaultInstance(d_ptr->canvasItem->im());
}

double Canvas::calcAutoZoomFactor() const
{
    if (!d_ptr->canvasItem->im()->imageValid())
    {
        return d_ptr->zoom;
    }

    double srcWidth  = d_ptr->canvasItem->im()->origWidth();
    double srcHeight = d_ptr->canvasItem->im()->origHeight();
    double dstWidth  = contentsRect().width();
    double dstHeight = contentsRect().height();
    return qMin(dstWidth / srcWidth, dstHeight / srcHeight);
}

void Canvas::updateAutoZoom()
{
    d_ptr->zoom = calcAutoZoomFactor();
    d_ptr->canvasItem->zoomSettings()->setZoomFactor(d_ptr->zoom);
    d_ptr->canvasItem->im()->zoom(d_ptr->zoom);
    emit signalZoomChanged(d_ptr->zoom);
}

void Canvas::updateContentsSize(bool deleteRubber)
{
    Q_UNUSED(deleteRubber);
    viewport()->setUpdatesEnabled(false);

    //if (deleteRubber && d_ptr->rubber->isActive())
    //{
    //    d_ptr->rubber->setActive(false);
        d_ptr->ltActive     = false;
        d_ptr->rtActive     = false;
        d_ptr->lbActive     = false;
        d_ptr->rbActive     = false;
        d_ptr->lsActive     = false;
        d_ptr->rsActive     = false;
        d_ptr->bsActive     = false;
        d_ptr->tsActive     = false;
        d_ptr->dragActive   = false;
        d_ptr->pressedMoved = false;
        viewport()->unsetCursor();
        viewport()->setMouseTracking(false);

        if (d_ptr->canvasItem->im()->imageValid())
        {
            emit signalSelected(false);
        }
    //}

    /*int wZ = d_ptr->canvasItem->im()->width();
    int hZ = d_ptr->canvasItem->im()->height();

    if (visibleWidth() > wZ || visibleHeight() > hZ)
    {
        // Center the image
        int centerx   = contentsRect().width()  / 2;
        int centery   = contentsRect().height() / 2;
        int xoffset   = int(centerx - wZ / 2);
        int yoffset   = int(centery - hZ / 2);
        xoffset       = qMax(xoffset, 0);
        yoffset       = qMax(yoffset, 0);
        d_ptr->pixmapRect = QRect(xoffset, yoffset, wZ, hZ);
    }
    else
    {
        d_ptr->pixmapRect = QRect(0, 0, wZ, hZ);
    }

    // always restrict rubberband to area covered by image
    //d_ptr->rubber->setRestrictionOnContents(d_ptr->pixmapRect);

    if (!deleteRubber && d_ptr->rubber->isActive())
    {
        QRect sel = d_ptr->canvasItem->im()->getSelectedArea();
        int xSel  = (int)(sel.x()      *  d_ptr->zoom);
        int ySel  = (int)(sel.y()      *  d_ptr->zoom);
        int wSel  = (int)(sel.width()  *  d_ptr->zoom);
        int hSel  = (int)(sel.height() *  d_ptr->zoom);
        QRect rubberRect(xSel, ySel, wSel, hSel);
        rubberRect.translate(d_ptr->pixmapRect.x(), d_ptr->pixmapRect.y());
        //d_ptr->rubber->setRectOnViewport(rubberRect);
    }*/

    //resizeContents(wZ, hZ);
    viewport()->setUpdatesEnabled(true);
}

bool Canvas::maxZoom() const
{
    return ((d_ptr->zoom * d_ptr->zoomMultiplier) >= d_ptr->maxZoom);
}

bool Canvas::minZoom() const
{
    return ((d_ptr->zoom / d_ptr->zoomMultiplier) <= d_ptr->minZoom);
}

double Canvas::zoomMax() const
{
    return d_ptr->maxZoom;
}

double Canvas::zoomMin() const
{
    return d_ptr->minZoom;
}

bool Canvas::exifRotated() const
{
    return d_ptr->canvasItem->im()->exifRotated();
}

double Canvas::snapZoom(double zoom) const
{
    // If the zoom value gets changed from d_ptr->zoom to zoom
    // across 50%, 100% or fit-to-window, then return the
    // the corresponding special value. Otherwise zoom is returned unchanged.
    double fit = calcAutoZoomFactor();
    QList<double> snapValues;
    snapValues.append(0.5);
    snapValues.append(1.0);
    snapValues.append(fit);

    if (d_ptr->zoom < zoom)
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

        if ((d_ptr->zoom < z) && (zoom > z))
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

    double zoom = d_ptr->zoom * d_ptr->zoomMultiplier;
    zoom        = snapZoom(zoom);
    setZoomFactor(zoom);
}

void Canvas::slotDecreaseZoom()
{
    if (minZoom())
    {
        return;
    }

    double zoom = d_ptr->zoom / d_ptr->zoomMultiplier;
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
    return d_ptr->zoom;
}

void Canvas::setZoomFactor(double zoom)
{
    if (d_ptr->autoZoom)
    {
        d_ptr->autoZoom = false;
        emit signalToggleOffFitToWindow();
    }

    // Zoom using center of canvas and given zoom factor.

    /*double cpx = contentsX() + visibleWidth()  / 2.0;
    double cpy = contentsY() + visibleHeight() / 2.0;

    cpx        = cpx * d_ptr->zoom;
    cpy        = cpy * d_ptr->zoom;*/

    d_ptr->zoom    = zoom;

    d_ptr->canvasItem->zoomSettings()->setZoomFactor(d_ptr->zoom);
    d_ptr->canvasItem->im()->zoom(d_ptr->zoom);
    updateContentsSize(false);

//     viewport()->setUpdatesEnabled(false);
//    center((int)(cpx * d_ptr->zoom),
//           (int)(cpy * d_ptr->zoom));
//     viewport()->setUpdatesEnabled(true);
//     viewport()->update();

    emit signalZoomChanged(d_ptr->zoom);
}

void Canvas::fitToSelect()
{
    QRect sel = d_ptr->canvasItem->im()->getSelectedArea();

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
        d_ptr->zoom          = qMin(dstWidth / srcWidth, dstHeight / srcHeight);
        d_ptr->autoZoom      = false;

        emit signalToggleOffFitToWindow();
        d_ptr->canvasItem->im()->zoom(d_ptr->zoom);
        updateContentsSize(true);

//         viewport()->setUpdatesEnabled(false);
        centerOn(cpx * d_ptr->zoom, cpy * d_ptr->zoom);
//         viewport()->setUpdatesEnabled(true);
//         viewport()->update();

        emit signalZoomChanged(d_ptr->zoom);
    }
}

bool Canvas::fitToWindow() const
{
    return d_ptr->autoZoom;
}

void Canvas::toggleFitToWindow()
{
    setFitToWindow(!d_ptr->autoZoom);
}

void Canvas::setFitToWindow(bool fit)
{
    d_ptr->autoZoom = fit;

    if (d_ptr->autoZoom)
    {
        updateAutoZoom();
    }
    else
    {
        d_ptr->zoom = 1.0;
        emit signalZoomChanged(d_ptr->zoom);
    }

    d_ptr->canvasItem->im()->zoom(d_ptr->zoom);
    updateContentsSize(false);
    slotZoomChanged(d_ptr->zoom);
//     viewport()->update();
}

void Canvas::slotRotate90()
{
    d_ptr->canvasItem->im()->rotate90();
}

void Canvas::slotRotate180()
{
    d_ptr->canvasItem->im()->rotate180();
}

void Canvas::slotRotate270()
{
    d_ptr->canvasItem->im()->rotate270();
}

void Canvas::slotFlipHoriz()
{
    d_ptr->canvasItem->im()->flipHoriz();
}

void Canvas::slotFlipVert()
{
    d_ptr->canvasItem->im()->flipVert();
}

void Canvas::slotAutoCrop()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);

    AutoCrop ac(d_ptr->canvasItem->im()->getImg());
    ac.startFilterDirectly();
    QRect rect = ac.autoInnerCrop();
    d_ptr->canvasItem->im()->crop(rect);

    QApplication::restoreOverrideCursor();
}

void Canvas::slotCrop()
{
    QRect sel = d_ptr->canvasItem->im()->getSelectedArea();

    if (sel.size().isNull())   // No current selection.
    {
        return;
    }

    d_ptr->canvasItem->im()->crop(sel);
}

void Canvas::setBackgroundColor(const QColor& color)
{
    if (d_ptr->bgColor == color)
    {
        return;
    }

    d_ptr->bgColor = color;
//     viewport()->update();
}

void Canvas::setICCSettings(const ICCSettingsContainer& cmSettings)
{
    ICCSettingsContainer old = d_ptr->canvasItem->im()->getICCSettings();
    d_ptr->canvasItem->im()->setICCSettings(cmSettings);
//     viewport()->update();
}

void Canvas::setSoftProofingEnabled(bool enable)
{
    d_ptr->canvasItem->im()->setSoftProofingEnabled(enable);
//     viewport()->update();
}

void Canvas::setExposureSettings(ExposureSettingsContainer* const expoSettings)
{
    d_ptr->canvasItem->im()->setExposureSettings(expoSettings);
//     viewport()->update();
}

void Canvas::setExifOrient(bool exifOrient)
{
    d_ptr->canvasItem->im()->setExifOrient(exifOrient);
//     viewport()->update();
}

void Canvas::slotRestore()
{
    d_ptr->canvasItem->im()->restore();
}

void Canvas::slotUndo(int steps)
{
    emit signalUndoSteps(steps);

    while (steps > 0)
    {
        d_ptr->canvasItem->im()->undo();
        --steps;
    }
}

void Canvas::slotRedo(int steps)
{
    emit signalRedoSteps(steps);

    while (steps > 0)
    {
        d_ptr->canvasItem->im()->redo();
        --steps;
    }
}

void Canvas::slotCopy()
{
    QRect sel = d_ptr->canvasItem->im()->getSelectedArea();

    if (sel.size().isNull())   // No current selection.
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);

    DImg selDImg        = d_ptr->canvasItem->im()->getImgSelection();
    QImage selImg       = selDImg.copyQImage();
    QMimeData* mimeData = new QMimeData();
    mimeData->setImageData(selImg);
    QApplication::clipboard()->setMimeData(mimeData, QClipboard::Clipboard);

    QApplication::restoreOverrideCursor();
}

// void Canvas::slotSelected()
// {
//     QRect sel = QRect(0, 0, 0, 0);
// 
//     if (d_ptr->rubber->isActive() && d_ptr->pressedMoved)
//     {
//         sel = calcSelectedArea();
//     }
// 
//     d_ptr->canvasItem->im()->setSelectedArea(sel);
// }

// QRect Canvas::calcSelectedArea() const
// {
//     int x = 0, y = 0, w = 0, h = 0;
//     QRect r(d_ptr->rubber->rubberBandAreaOnContents());
// 
//     if (r.isValid())
//     {
//         r.translate(- d_ptr->pixmapRect.x(), - d_ptr->pixmapRect.y());
// 
//         x = (int)((double)r.x()  / d_ptr->zoom);
//         y = (int)((double)r.y()  / d_ptr->zoom);
//         w = (int)((double)r.width()  / d_ptr->zoom);
//         h = (int)((double)r.height()  / d_ptr->zoom);
// 
//         x = qMin(imageWidth(),  qMax(x, 0));
//         y = qMin(imageHeight(), qMax(y, 0));
//         w = qMin(imageWidth(),  qMax(w, 0));
//         h = qMin(imageHeight(), qMax(h, 0));
// 
//         // Avoid empty selection by rubberband - at least mark one pixel
//         // At high zoom factors, the rubberband may operate at subpixel level!
//         if (w == 0)
//         {
//             w = 1;
//         }
// 
//         if (h == 0)
//         {
//             h = 1;
//         }
//     }
// 
//     return QRect(x, y, w, h);
// }

void Canvas::slotModified()
{
    if (d_ptr->autoZoom)
    {
        updateAutoZoom();
    }

    d_ptr->canvasItem->zoomSettings()->setZoomFactor(d_ptr->zoom);
    d_ptr->canvasItem->im()->zoom(d_ptr->zoom);

    updateContentsSize(true);
//     viewport()->update();

    // To be sure than corner widget used to pan image will be hide/show
    // accordingly with new image size (if changed).
    slotZoomChanged(d_ptr->zoom);

    emit signalChanged();
}

void Canvas::slotCornerButtonPressed()
{
    if (d_ptr->panIconPopup)
    {
        d_ptr->panIconPopup->hide();
        d_ptr->panIconPopup->deleteLater();
        d_ptr->panIconPopup = 0;
    }

    d_ptr->panIconPopup          = new KPopupFrame(this);
    PanIconWidget* const pan = new PanIconWidget(d_ptr->panIconPopup);

    connect(pan, SIGNAL(signalSelectionMoved(QRect,bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect,bool)));

    connect(pan, SIGNAL(signalHidden()),
            this, SLOT(slotPanIconHidden()));

    QRect r = d_ptr->canvasItem->boundingRect().toRect();
    pan->setImage(180, 120, d_ptr->canvasItem->im()->getImg()->copyQImage());
    pan->setRegionSelection(r);
    pan->setMouseFocus();
    d_ptr->panIconPopup->setMainWidget(pan);

    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x() + viewport()->size().width());
    g.setY(g.y() + viewport()->size().height());
    d_ptr->panIconPopup->popup(QPoint(g.x() - d_ptr->panIconPopup->width(),
                                  g.y() - d_ptr->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void Canvas::slotPanIconHidden()
{
    d_ptr->cornerButton->blockSignals(true);
    d_ptr->cornerButton->animateClick();
    d_ptr->cornerButton->blockSignals(false);
}

void Canvas::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*d_ptr->zoom), (int)(r.y()*d_ptr->zoom));

    if (b)
    {
        d_ptr->panIconPopup->hide();
        d_ptr->panIconPopup->deleteLater();
        d_ptr->panIconPopup = 0;
        slotPanIconHidden();
    }
}

void Canvas::slotZoomChanged(double /*zoom*/)
{
    //updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
    {
        d_ptr->cornerButton->show();
    }
    else
    {
        d_ptr->cornerButton->hide();
    }
}

void Canvas::slotSelectAll()
{
    //d_ptr->rubber->setRectOnContents(d_ptr->pixmapRect);
    d_ptr->pressedMoved = true;
    viewport()->setMouseTracking(true);
//     viewport()->update();

    if (d_ptr->canvasItem->im()->imageValid())
    {
        emit signalSelected(true);
    }
}

void Canvas::slotSelectNone()
{
    //reset();
//     viewport()->update();
}

void Canvas::keyPressEvent(QKeyEvent* e)
{
    if (!e)
    {
        return;
    }

    int mult = 1;

    if ((e->modifiers() & Qt::ControlModifier))
    {
        mult = 10;
    }

    switch (e->key())
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
            e->ignore();
            break;
        }
    }
}

}  // namespace Digikam

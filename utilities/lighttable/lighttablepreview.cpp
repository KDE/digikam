/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes.

#include <qpainter.h>
#include <qcursor.h>
#include <qstring.h>
#include <qvaluevector.h>
#include <qfileinfo.h>
#include <qtoolbutton.h>
#include <qtooltip.h>
#include <qpixmap.h>
#include <qdrawutil.h>

// KDE includes.

#include <kdialogbase.h>
#include <klocale.h>
#include <kservice.h>
#include <krun.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <kcursor.h>
#include <kdatetbl.h>
#include <kiconloader.h>
#include <kprocess.h>
#include <kapplication.h>

// Local includes.

#include "dimg.h"
#include "ddebug.h"
#include "albumdb.h"
#include "constants.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "dragobjects.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "metadatahub.h"
#include "paniconwidget.h"
#include "previewloadthread.h"
#include "loadingdescription.h"
#include "tagspopupmenu.h"
#include "ratingpopupmenu.h"
#include "themeengine.h"
#include "lighttablepreview.h"
#include "lighttablepreview.moc"

namespace Digikam
{

class LightTablePreviewPriv
{
public:

    LightTablePreviewPriv()
    {
        panIconPopup         = 0;
        panIconWidget        = 0;
        cornerButton         = 0;
        previewThread        = 0;
        previewPreloadThread = 0;
        imageInfo            = 0;
        hasPrev              = false;
        hasNext              = false;
        selected             = false;
        dragAndDropEnabled   = true;
        loadFullImageSize    = false;
        currentFitWindowZoom = 0;
        previewSize          = 1024;
    }

    bool               hasPrev;
    bool               hasNext;
    bool               selected;
    bool               dragAndDropEnabled;
    bool               loadFullImageSize;

    int                previewSize;

    double             currentFitWindowZoom;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    QToolButton       *cornerButton;

    KPopupFrame       *panIconPopup;

    PanIconWidget     *panIconWidget;

    DImg               preview;

    ImageInfo         *imageInfo;

    PreviewLoadThread *previewThread;
    PreviewLoadThread *previewPreloadThread;
};
    
LightTablePreview::LightTablePreview(QWidget *parent)
                 : PreviewWidget(parent)
{
    d = new LightTablePreviewPriv;

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = QMAX(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;
    
    viewport()->setAcceptDrops(true);
    setAcceptDrops(true); 

    slotThemeChanged();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIconSet(SmallIcon("move"));
    d->cornerButton->hide();
    QToolTip::add(d->cornerButton, i18n("Pan the image"));
    setCornerWidget(d->cornerButton);

    setLineWidth(5);
    setSelected(false);

    // ------------------------------------------------------------

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ------------------------------------------------------------

    slotReset(); 
}

LightTablePreview::~LightTablePreview()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void LightTablePreview::setLoadFullImageSize(bool b)
{
    d->loadFullImageSize = b;
    reload();
}

void LightTablePreview::setDragAndDropEnabled(bool b)
{
    d->dragAndDropEnabled = b;
}

void LightTablePreview::setDragAndDropMessage()
{
    if (d->dragAndDropEnabled)
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::WordBreak, 
                   i18n("Drag and drop an image here"));
        p.end();
        setImage(pix.convertToImage());
    }
}

void LightTablePreview::setImage(const DImg& image)
{   
    d->preview = image;

    updateZoomAndSize(true);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

DImg& LightTablePreview::getImage() const
{
    return d->preview;
}

QSize LightTablePreview::getImageSize()
{
    return d->preview.size();
}

void LightTablePreview::reload()
{
    // cache is cleaned from AlbumIconView::refreshItems
    setImagePath(d->path);
}

void LightTablePreview::setPreviousNextPaths(const QString& previous, const QString &next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void LightTablePreview::setImagePath(const QString& path)
{
    setCursor( KCursor::waitCursor() );

    d->path         = path;
    d->nextPath     = QString();
    d->previousPath = QString();

    if (d->path.isEmpty())
    {
        slotReset();
        unsetCursor();
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        connect(d->previewThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotGotImagePreview(const LoadingDescription &, const DImg&)));
    }
    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        connect(d->previewPreloadThread, SIGNAL(signalImageLoaded(const LoadingDescription &, const DImg &)),
                this, SLOT(slotNextPreload()));
    }

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(LoadingDescription(path, 0, AlbumSettings::instance()->getExifRotate()));
    else
        d->previewThread->load(LoadingDescription(path, d->previewSize, AlbumSettings::instance()->getExifRotate()));
}

void LightTablePreview::slotGotImagePreview(const LoadingDescription &description, const DImg& preview)
{
    if (description.filePath != d->path)
        return;   

    if (preview.isNull())
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::WordBreak, 
                   i18n("Unable to display preview for\n\"%1\"")
                   .arg(info.fileName()));
        p.end();
        setImage(DImg(pix.convertToImage()));

        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);
        if (AlbumSettings::instance()->getExifRotate())
            d->previewThread->exifRotate(img, description.filePath);
        setImage(img);
        emit signalPreviewLoaded(true);
    }

    unsetCursor();
    slotNextPreload();
}

void LightTablePreview::slotNextPreload()
{
    QString loadPath;
    if (!d->nextPath.isNull())
    {
        loadPath    = d->nextPath;
        d->nextPath = QString();
    }
    else if (!d->previousPath.isNull())
    {
        loadPath        = d->previousPath;
        d->previousPath = QString();
    }
    else
        return;

    d->previewPreloadThread->load(LoadingDescription(loadPath, d->previewSize,
                                  AlbumSettings::instance()->getExifRotate()));
}

void LightTablePreview::setImageInfo(ImageInfo* info, ImageInfo *previous, ImageInfo *next)
{
    d->imageInfo = info;
    d->hasPrev   = previous;
    d->hasNext   = next;

    if (d->imageInfo)
        setImagePath(info->filePath());
    else
    {
        setImagePath();
        setSelected(false);
    }

    setPreviousNextPaths(previous ? previous->filePath() : QString(),
                         next     ? next->filePath()     : QString());
}

ImageInfo* LightTablePreview::getImageInfo() const
{
    return d->imageInfo;
}

void LightTablePreview::slotContextMenu()
{
    RatingPopupMenu *ratingMenu     = 0;
    TagsPopupMenu   *assignTagsMenu = 0;
    TagsPopupMenu   *removeTagsMenu = 0;

    if (!d->imageInfo)
        return;

    //-- Open With Actions ------------------------------------

    KURL url(d->imageInfo->kurl().path());
    KMimeType::Ptr mimePtr = KMimeType::findByURL(url, 0, true, true);

    QValueVector<KService::Ptr> serviceVector;
    KTrader::OfferList offers = KTrader::self()->query(mimePtr->name(), "Type == 'Application'");

    QPopupMenu openWithMenu;

    KTrader::OfferList::Iterator iter;
    KService::Ptr ptr;
    int index = 100;

    for( iter = offers.begin(); iter != offers.end(); ++iter )
    {
        ptr = *iter;
        openWithMenu.insertItem( ptr->pixmap(KIcon::Small), ptr->name(), index++);
        serviceVector.push_back(ptr);
    }

    DPopupMenu popmenu(this);
    
    //-- Zoom actions -----------------------------------------------

    popmenu.insertItem(SmallIcon("viewmag"), i18n("Zoom in"), 17);
    popmenu.insertItem(SmallIcon("viewmag-"), i18n("Zoom out"), 18);
    popmenu.insertItem(SmallIcon("view_fit_window"), i18n("Fit to &Window"), 19);

    //-- Edit actions -----------------------------------------------

    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("slideshow"), i18n("SlideShow"), 16);
    popmenu.insertItem(SmallIcon("editimage"), i18n("Edit..."), 12);
    popmenu.insertItem(i18n("Open With"), &openWithMenu, 13);

    //-- Trash action -------------------------------------------

    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("edittrash"), i18n("Move to Trash"), 14);

    // Bulk assignment/removal of tags --------------------------

    Q_LLONG id = d->imageInfo->id();
    QValueList<Q_LLONG> idList;
    idList.append(id);

    assignTagsMenu = new TagsPopupMenu(idList, 1000, TagsPopupMenu::ASSIGN);
    removeTagsMenu = new TagsPopupMenu(idList, 2000, TagsPopupMenu::REMOVE);

    popmenu.insertSeparator();

    popmenu.insertItem(i18n("Assign Tag"), assignTagsMenu);
    int i = popmenu.insertItem(i18n("Remove Tag"), removeTagsMenu);

    connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    AlbumDB* db = AlbumManager::instance()->albumDB();
    if (!db->hasTags( idList ))
        popmenu.setItemEnabled(i, false);

    popmenu.insertSeparator();

    // Assign Star Rating -------------------------------------------

    ratingMenu = new RatingPopupMenu();
    
    connect(ratingMenu, SIGNAL(activated(int)),
            this, SLOT(slotAssignRating(int)));

    popmenu.insertItem(i18n("Assign Rating"), ratingMenu);

    // --------------------------------------------------------

    int idm = popmenu.exec(QCursor::pos());

    switch(idm) 
    {
        case 12:     // Edit...
        {
            emit signalEditItem(d->imageInfo);
            break;
        }

        case 14:     // Move to trash
        {
            emit signalDeleteItem(d->imageInfo);
            break;
        }

        case 16:     // SlideShow
        {
            emit signalSlideShow();
            break;
        }

        case 17:     // Zoom in
        {
            slotIncreaseZoom();
            break;
        }

        case 18:     // Zoom out
        {
            slotDecreaseZoom();
            break;
        }

        case 19:     // Fit to window
        {
            fitToWindow();
            break;
        }

        default:
            break;
    }

    // Open With...
    if (idm >= 100 && idm < 1000) 
    {
        KService::Ptr imageServicePtr = serviceVector[idm-100];
        KRun::run(*imageServicePtr, url);
    }

    serviceVector.clear();
    delete assignTagsMenu;
    delete removeTagsMenu;
    delete ratingMenu;
}

void LightTablePreview::slotAssignTag(int tagID)
{
    if (d->imageInfo)
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, true);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo->filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotRemoveTag(int tagID)
{
    if (d->imageInfo)
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, false);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo->filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotAssignRating(int rating)
{
    rating = QMIN(RatingMax, QMAX(RatingMin, rating));
    if (d->imageInfo)
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setRating(rating);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo->filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotThemeChanged()
{
    setBackgroundColor(ThemeEngine::instance()->baseColor());
    frameChanged();
}

void LightTablePreview::slotCornerButtonPressed()
{    
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        delete d->panIconPopup;
        d->panIconPopup = 0;
    }

    d->panIconPopup    = new KPopupFrame(this);
    PanIconWidget *pan = new PanIconWidget(d->panIconPopup);
    pan->setImage(180, 120, getImage());
    d->panIconPopup->setMainWidget(pan);

    QRect r((int)(contentsX()    / zoomFactor()), (int)(contentsY()     / zoomFactor()),
            (int)(visibleWidth() / zoomFactor()), (int)(visibleHeight() / zoomFactor()));
    pan->setRegionSelection(r);
    pan->setMouseFocus();

    connect(pan, SIGNAL(signalSelectionMoved(const QRect&, bool)),
            this, SLOT(slotPanIconSelectionMoved(const QRect&, bool)));
    
    connect(pan, SIGNAL(signalHiden()),
            this, SLOT(slotPanIconHiden()));
    
    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(), 
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void LightTablePreview::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void LightTablePreview::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*zoomFactor()), (int)(r.y()*zoomFactor()));

    if (b)
    {
        d->panIconPopup->hide();
        delete d->panIconPopup;
        d->panIconPopup = 0;
        slotPanIconHiden();
    }
}

void LightTablePreview::zoomFactorChanged(double zoom)
{
    updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
        d->cornerButton->show();
    else
        d->cornerButton->hide();        

    PreviewWidget::zoomFactorChanged(zoom);
}

void LightTablePreview::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    QScrollView::resizeEvent(e);

    if (!d->imageInfo)
    {
        d->cornerButton->hide();
        setDragAndDropMessage();
    }

    updateZoomAndSize(false);
}

void LightTablePreview::updateZoomAndSize(bool alwaysFitToWindow)
{
    // Set zoom for fit-in-window as minimum, but dont scale up images
    // that are smaller than the available space, only scale down.
    double zoom = calcAutoZoomFactor(ZoomInOnly);
    setZoomMin(zoom);
    setZoomMax(zoom*12.0);

    // Is currently the zoom factor set to fit to window? Then set it again to fit the new size.
    if (zoomFactor() < zoom || alwaysFitToWindow || zoomFactor() == d->currentFitWindowZoom)
    {
        setZoomFactor(zoom);
    }

    // store which zoom factor means it is fit to window
    d->currentFitWindowZoom = zoom;

    updateContentsSize();
}

int LightTablePreview::previewWidth()
{
    return d->preview.width();
}

int LightTablePreview::previewHeight()
{
    return d->preview.height();
}

bool LightTablePreview::previewIsNull()
{
    return d->preview.isNull();
}

void LightTablePreview::resetPreview()
{
    d->preview   = DImg();
    d->path      = QString(); 
    d->imageInfo = 0;

    setDragAndDropMessage();
    updateZoomAndSize(true);
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
    emit signalPreviewLoaded(false);
}

void LightTablePreview::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->preview.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());    
    QPixmap pix2 = img.convertToPixmap();
    bitBlt(pix, 0, 0, &pix2, 0, 0);
}

void LightTablePreview::contentsDragMoveEvent(QDragMoveEvent *e)
{
    if (d->dragAndDropEnabled)
    {
        int             albumID;
        QValueList<int> albumIDs;
        QValueList<int> imageIDs;
        KURL::List      urls;
        KURL::List      kioURLs;        
    
        if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs) ||
            AlbumDrag::decode(e, urls, albumID) ||
            TagDrag::canDecode(e))
        {
            e->accept();
            return;
        }
    }

    e->ignore();
}

void LightTablePreview::contentsDropEvent(QDropEvent *e)
{
    if (d->dragAndDropEnabled)
    {
        int             albumID;
        QValueList<int> albumIDs;
        QValueList<int> imageIDs;
        KURL::List      urls;
        KURL::List      kioURLs;  
        ImageInfoList   list;
    
        if (ItemDrag::decode(e, urls, kioURLs, albumIDs, imageIDs))
        {
            for (QValueList<int>::const_iterator it = imageIDs.begin();
                 it != imageIDs.end(); ++it)
            {
                list.append(new ImageInfo(*it));
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
        }
        else if (AlbumDrag::decode(e, urls, albumID))
        {
            QValueList<Q_LLONG> itemIDs = AlbumManager::instance()->albumDB()->getItemIDsInAlbum(albumID);
    
            for (QValueList<Q_LLONG>::const_iterator it = itemIDs.begin();
                it != itemIDs.end(); ++it)
            {
                list.append(new ImageInfo(*it));
            }

            emit signalDroppedItems(list);
            e->accept();
            return;
        }
        else if(TagDrag::canDecode(e))
        {
            QByteArray ba = e->encodedData("digikam/tag-id");
            QDataStream ds(ba, IO_ReadOnly);
            int tagID;
            ds >> tagID;
    
            AlbumManager* man           = AlbumManager::instance();
            QValueList<Q_LLONG> itemIDs = man->albumDB()->getItemIDsInTag(tagID, true);
            ImageInfoList imageInfoList;
    
            for (QValueList<Q_LLONG>::const_iterator it = itemIDs.begin();
                it != itemIDs.end(); ++it)
            {
                list.append(new ImageInfo(*it));
            }
        
            emit signalDroppedItems(list);
            e->accept();
            return;
        }   
    }

    e->ignore();
}

void LightTablePreview::setSelected(bool sel)
{
    if (d->selected != sel)
    {
        d->selected = sel;
        frameChanged();
    }
}

bool LightTablePreview::isSelected()
{
    return d->selected;
}

void LightTablePreview::drawFrame(QPainter *p)
{
    if (d->selected)
    {
        qDrawPlainRect(p, frameRect(), ThemeEngine::instance()->thumbSelColor(), lineWidth());
        qDrawPlainRect(p, frameRect(), ThemeEngine::instance()->textSelColor(), 2);
    }
    else 
        qDrawPlainRect(p, frameRect(), ThemeEngine::instance()->baseColor(), lineWidth());
}

}  // NameSpace Digikam

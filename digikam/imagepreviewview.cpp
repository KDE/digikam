/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 * 
 * Copyright (C) 2006-2007 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <Q3ValueList>
#include <Q3PopupMenu>
#include <Q3ValueVector>
#include <QPainter>
#include <QCursor>
#include <QString>
#include <QFileInfo>
#include <QToolButton>
#include <QDesktopWidget>
#include <QPixmap>

// KDE includes.

#include <kservicetypetrader.h>
#include <kdialog.h>
#include <klocale.h>
#include <kservice.h>
#include <krun.h>
#include <kaction.h>
#include <kmimetype.h>
#include <kcursor.h>
#include <kdatetable.h>
#include <kiconloader.h>
#include <kapplication.h>

// LibKipi includes.

#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "dimg.h"
#include "ddebug.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "databaseaccess.h"
#include "fastscale.h"
#include "imageinfo.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "metadatahub.h"
#include "paniconwidget.h"
#include "previewloadthread.h"
#include "loadingdescription.h"
#include "tagspopupmenu.h"
#include "ratingpopupmenu.h"
#include "themeengine.h"
#include "imagepreviewview.h"
#include "imagepreviewview.moc"

namespace Digikam
{

class ImagePreviewViewPriv
{
public:

    ImagePreviewViewPriv()
    {
        panIconPopup         = 0;
        panIconWidget        = 0;
        cornerButton         = 0;
        previewThread        = 0;
        previewPreloadThread = 0;
        parent               = 0;
        hasPrev              = false;
        hasNext              = false;
        loadFullImageSize    = false;
        currentFitWindowZoom = 0;
        previewSize          = 1024;
    }

    bool               hasPrev;
    bool               hasNext;
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

    ImageInfo          imageInfo;

    PreviewLoadThread *previewThread;
    PreviewLoadThread *previewPreloadThread;

    AlbumWidgetStack  *parent;
};
    
ImagePreviewView::ImagePreviewView(AlbumWidgetStack *parent)
                : PreviewWidget(parent)
{
    d = new ImagePreviewViewPriv;
    d->parent = parent;

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIcon(SmallIcon("move"));
    d->cornerButton->hide();
    d->cornerButton->setToolTip( i18n("Pan the image to a region"));
    setCornerWidget(d->cornerButton);

    // ------------------------------------------------------------

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(this, SIGNAL(signalShowNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(signalShowPrevImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(this, SIGNAL(signalLeftButtonClicked()),
            this, SIGNAL(signalBack2Album()));

    connect(ThemeEngine::componentData(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    // ------------------------------------------------------------

    slotReset(); 
}

ImagePreviewView::~ImagePreviewView()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void ImagePreviewView::setLoadFullImageSize(bool b)
{
    d->loadFullImageSize = b;
    reload();
}

void ImagePreviewView::setImage(const DImg& image)
{
    d->preview = image;

    updateZoomAndSize(true);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

DImg& ImagePreviewView::getImage() const
{
    return d->preview;
}

void ImagePreviewView::reload()
{
    // cache is cleaned from AlbumIconView::refreshItems
    setImagePath(d->path);
}

void ImagePreviewView::setPreviousNextPaths(const QString& previous, const QString &next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void ImagePreviewView::setImagePath(const QString& path)
{
    setCursor( Qt::WaitCursor );

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
        d->previewThread->loadHighQuality(LoadingDescription(path, 0, AlbumSettings::componentData()->getExifRotate()));
    else
        d->previewThread->load(LoadingDescription(path, d->previewSize, AlbumSettings::componentData()->getExifRotate()));
}

void ImagePreviewView::slotGotImagePreview(const LoadingDescription &description, const DImg& preview)
{
    if (description.filePath != d->path)
        return;   

    if (preview.isNull())
    {
        d->parent->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::componentData()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::componentData()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap, 
                   i18n("Cannot display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        // three copies - but the image is small
        setImage(DImg(pix.toImage()));
        d->parent->previewLoaded();
        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);
        if (AlbumSettings::componentData()->getExifRotate())
            d->previewThread->exifRotate(img, description.filePath);
        d->parent->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        setImage(img);
        d->parent->previewLoaded();
        emit signalPreviewLoaded(true);
    }

    unsetCursor();
    slotNextPreload();
}

void ImagePreviewView::slotNextPreload()
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
                                  AlbumSettings::componentData()->getExifRotate()));
}

void ImagePreviewView::setImageInfo(const ImageInfo & info, const ImageInfo &previous, const ImageInfo &next)
{
    d->imageInfo = info;
    d->hasPrev   = !previous.isNull();
    d->hasNext   = !next.isNull();

    if (!d->imageInfo.isNull())
        setImagePath(info.filePath());
    else
        setImagePath();

    setPreviousNextPaths(d->hasPrev ? QString() : previous.filePath(),
                         d->hasNext ? QString() : next.filePath());
}

ImageInfo ImagePreviewView::getImageInfo() const
{
    return d->imageInfo;
}

void ImagePreviewView::slotContextMenu()
{
    RatingPopupMenu *ratingMenu     = 0;
    TagsPopupMenu   *assignTagsMenu = 0;
    TagsPopupMenu   *removeTagsMenu = 0;

    if (d->imageInfo.isNull())
        return;

    //-- Open With Actions ------------------------------------

    KUrl url(d->imageInfo.fileUrl().path());
    KMimeType::Ptr mimePtr = KMimeType::findByUrl(url, 0, true, true);

    Q3ValueVector<KService::Ptr> serviceVector;

    const KService::List offers = KServiceTypeTrader::self()->query(mimePtr->name(), "Type == 'Application'");
    KService::List::ConstIterator iter;
    KService::Ptr ptr;

    Q3PopupMenu openWithMenu;

    int index = 100;

    for( iter = offers.begin(); iter != offers.end(); ++iter )
    {
        ptr = *iter;
        openWithMenu.insertItem(SmallIcon(ptr->icon()), ptr->name(), index++);
        serviceVector.push_back(ptr); 
    }

    //-- Navigate actions -------------------------------------------

    DPopupMenu popmenu(this);
    popmenu.insertItem(SmallIcon("back"), i18n("Back"), 10);
    if (!d->hasPrev) popmenu.setItemEnabled(10, false);

    popmenu.insertItem(SmallIcon("forward"), i18n("Forward"), 11);
    if (!d->hasNext) popmenu.setItemEnabled(11, false);

    popmenu.insertItem(SmallIcon("folder_image"), i18n("Back to Album"), 15);
    
    //-- Edit actions -----------------------------------------------

    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("datashow"), i18n("SlideShow"), 16);
    popmenu.insertItem(SmallIcon("editimage"), i18n("Edit..."), 12);
    popmenu.insertItem(SmallIcon("lighttable"), i18n("Add to Light Table"), 17);
    popmenu.insertItem(i18n("Open With"), &openWithMenu, 13);

    // Merge in the KIPI plugins actions ----------------------------

    KIPI::PluginLoader* kipiPluginLoader      = KIPI::PluginLoader::componentData();
    KIPI::PluginLoader::PluginList pluginList = kipiPluginLoader->pluginList();
    
    for (KIPI::PluginLoader::PluginList::const_iterator it = pluginList.begin();
        it != pluginList.end(); ++it)
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if (plugin && (*it)->name() == "JPEGLossless")
        {
            DDebug() << "Found JPEGLossless plugin" << endl;

            QList<KAction*> actionList = plugin->actions();
            
            for (QList<KAction*>::const_iterator iter = actionList.begin();
                iter != actionList.end(); ++iter)
            {
                KAction* action = *iter;
                
                if (action->objectName().toLatin1() == QString::fromLatin1("jpeglossless_rotate"))
                {
                    popmenu.addAction(action);
                }
            }
        }
    }

    //-- Trash action -------------------------------------------

    popmenu.insertSeparator();
    popmenu.insertItem(SmallIcon("edittrash"), i18n("Move to Trash"), 14);

    // Bulk assignment/removal of tags --------------------------

    QList<qlonglong> idList;
    idList << d->imageInfo.id();

    assignTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::ASSIGN);
    removeTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::REMOVE);

    popmenu.insertSeparator();

    popmenu.insertItem(i18n("Assign Tag"), assignTagsMenu);
    int i = popmenu.insertItem(i18n("Remove Tag"), removeTagsMenu);

    connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    if (!DatabaseAccess().db()->hasTags( idList ))
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
        case 10:     // Back
        {
            emit signalPrevItem();
            break;
        }

        case 11:     // Forward
        {
            emit signalNextItem();
            break;
        }

        case 12:     // Edit...
        {
            emit signalEditItem();
            break;
        }

        case 14:     // Move to trash
        {
            emit signalDeleteItem();
            break;
        }

        case 15:     // Back to album
        {
            emit signalBack2Album();
            break;
        }

        case 16:     // SlideShow
        {
            emit signalSlideShow();
            break;
        }

        case 17:     // Place onto Light Table
        {
            emit signalInsert2LightTable();
            break;
        }

        default:
            break;
    }

    // Open With...
    if (idm >= 100 && idm < 1000) 
    {
        KService::Ptr imageServicePtr = serviceVector[idm-100];
        KRun::run(*imageServicePtr, url,this);
    }

    serviceVector.clear();
    delete assignTagsMenu;
    delete removeTagsMenu;
    delete ratingMenu;
}

void ImagePreviewView::slotAssignTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, true);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotRemoveTag(int tagID)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setTag(tagID, false);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotAssignRating(int rating)
{
    rating = qMin(5, qMax(0, rating));
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setRating(rating);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void ImagePreviewView::slotThemeChanged()
{
    setBackgroundColor(ThemeEngine::componentData()->baseColor());
}

void ImagePreviewView::slotCornerButtonPressed()
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

    connect(pan, SIGNAL(signalSelectionMoved(QRect, bool)),
            this, SLOT(slotPanIconSelectionMoved(QRect, bool)));
    
    connect(pan, SIGNAL(signalHiden()),
            this, SLOT(slotPanIconHiden()));
    
    QPoint g = mapToGlobal(viewport()->pos());
    g.setX(g.x()+ viewport()->size().width());
    g.setY(g.y()+ viewport()->size().height());
    d->panIconPopup->popup(QPoint(g.x() - d->panIconPopup->width(), 
                                  g.y() - d->panIconPopup->height()));

    pan->setCursorToLocalRegionSelectionCenter();
}

void ImagePreviewView::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void ImagePreviewView::slotPanIconSelectionMoved(QRect r, bool b)
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

void ImagePreviewView::zoomFactorChanged(double zoom)
{
    updateScrollBars();

    if (horizontalScrollBar()->isVisible() || verticalScrollBar()->isVisible())
        d->cornerButton->show();
    else
        d->cornerButton->hide();        

    PreviewWidget::zoomFactorChanged(zoom);
}

void ImagePreviewView::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    Q3ScrollView::resizeEvent(e);

    if (d->imageInfo.isNull())
        d->cornerButton->hide();

    updateZoomAndSize(false);
}

void ImagePreviewView::updateZoomAndSize(bool alwaysFitToWindow)
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

int ImagePreviewView::previewWidth()
{
    return d->preview.width();
}

int ImagePreviewView::previewHeight()
{
    return d->preview.height();
}

bool ImagePreviewView::previewIsNull()
{
    return d->preview.isNull();
}

void ImagePreviewView::resetPreview()
{
    d->preview   = DImg();
    d->path      = QString();
    d->imageInfo = ImageInfo();

    updateZoomAndSize(true);
    emit signalPreviewLoaded(false);
}

void ImagePreviewView::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->preview.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());    
    QPixmap pix2 = img.convertToPixmap();
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2);
    p.end();
}

}  // NameSpace Digikam

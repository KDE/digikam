/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-12
 * Description : digiKam light table preview item.
 *
 * Copyright (C) 2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// KDE includes.

#include <kdialogbase.h>
#include <klocale.h>
#include <kservice.h>
#include <krun.h>
#include <ktrader.h>
#include <kmimetype.h>
#include <kiconloader.h>
#include <kcursor.h>
#include <kdatetbl.h>
#include <kiconloader.h>
#include <kprocess.h>

// LibKipi includes.

#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "ddebug.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
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
        currentFitWindowZoom = 0;
    }

    bool               hasPrev;
    bool               hasNext;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    QToolButton       *cornerButton;

    QImage             preview;

    KPopupFrame       *panIconPopup;

    PanIconWidget     *panIconWidget;

    double             currentFitWindowZoom;

    ImageInfo         *imageInfo;

    PreviewLoadThread *previewThread;
    PreviewLoadThread *previewPreloadThread;
};
    
LightTablePreview::LightTablePreview(QWidget *parent)
                 : PreviewWidget(parent)
{
    d = new LightTablePreviewPriv;
    
    slotThemeChanged();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIconSet(SmallIcon("move"));
    d->cornerButton->hide();
    QToolTip::add(d->cornerButton, i18n("Pan the image to a region"));
    setCornerWidget(d->cornerButton);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalZoomFactorChanged(double)),
            this, SLOT(slotZoomChanged(double)));

    connect(d->cornerButton, SIGNAL(pressed()),
            this, SLOT(slotCornerButtonPressed()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

LightTablePreview::~LightTablePreview()
{
    delete d->previewThread;
    delete d->previewPreloadThread;
    delete d;
}

void LightTablePreview::setImage(const QImage& image)
{   
    d->preview = image;

    updateZoomAndSize(true);

    viewport()->setUpdatesEnabled(true);
    viewport()->update();
}

QImage& LightTablePreview::getImage() const
{
    return d->preview;
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
        unsetCursor();
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        connect(d->previewThread, SIGNAL(signalPreviewLoaded(const LoadingDescription &, const QImage &)),
                this, SLOT(slotGotImagePreview(const LoadingDescription &, const QImage&)));
    }
    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        connect(d->previewPreloadThread, SIGNAL(signalPreviewLoaded(const LoadingDescription &, const QImage &)),
                this, SLOT(slotNextPreload()));
    }

    d->previewThread->load(LoadingDescription(path, 1024, AlbumSettings::instance()->getExifRotate()));
}

void LightTablePreview::slotGotImagePreview(const LoadingDescription &description, const QImage& preview)
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
                   i18n("Cannot display preview for\n\"%1\"")
                   .arg(info.fileName()));
        p.end();
        setImage(pix.convertToImage());
    }
    else
    {
        setImage(preview);
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

    d->previewPreloadThread->load(LoadingDescription(loadPath, 1024,
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
        setImagePath();

    setPreviousNextPaths(previous ? previous->filePath() : QString(),
                         next     ? next->filePath()     : QString());
}

ImageInfo* LightTablePreview::getImageInfo()
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

    // Merge in the KIPI plugins actions ----------------------------

    KIPI::PluginLoader* kipiPluginLoader      = KIPI::PluginLoader::instance();
    KIPI::PluginLoader::PluginList pluginList = kipiPluginLoader->pluginList();
    
    for (KIPI::PluginLoader::PluginList::const_iterator it = pluginList.begin();
        it != pluginList.end(); ++it)
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if (plugin && (*it)->name() == "JPEGLossless")
        {
            DDebug() << "Found JPEGLossless plugin" << endl;

            KActionPtrList actionList = plugin->actions();
            
            for (KActionPtrList::const_iterator iter = actionList.begin();
                iter != actionList.end(); ++iter)
            {
                KAction* action = *iter;
                
                if (QString::fromLatin1(action->name())
                    == QString::fromLatin1("jpeglossless_rotate"))
                {
                    action->plug(&popmenu);
                }
            }
        }
    }

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
            emit signalEditItem();
            break;
        }

        case 14:     // Move to trash
        {
            emit signalDeleteItem();
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
    rating = QMIN(5, QMAX(0, rating));
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

void LightTablePreview::slotPanIconHiden()
{
    d->cornerButton->blockSignals(true);
    d->cornerButton->animateClick();
    d->cornerButton->blockSignals(false);
}

void LightTablePreview::slotPanIconSelectionMoved(QRect r, bool b)
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

void LightTablePreview::slotZoomChanged(double zoom)
{
    if (zoom > calcAutoZoomFactor())
        d->cornerButton->show();
    else
        d->cornerButton->hide();        
}

void LightTablePreview::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    if (previewIsNull())
        return;

    QScrollView::resizeEvent(e);

    updateZoomAndSize(false);
    //emit signalZoomFactorChanged(zoomFactor());
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
    d->preview.reset();
}

void LightTablePreview::paintPreview(QPixmap *pix, int sx, int sy, int sw, int sh)
{
    // Fast smooth scale method from Antonio.   
    QImage img = FastScale::fastScaleQImage(d->preview.copy(sx, sy, sw, sh),
                                            tileSize(), tileSize());
    bitBlt(pix, 0, 0, &img, 0, 0);
}

}  // NameSpace Digikam

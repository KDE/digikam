/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2008 Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagepreviewview.h"
#include "imagepreviewview.moc"

// Qt includes.

#include <QPainter>
#include <QCursor>
#include <QString>
#include <QFileInfo>
#include <QToolButton>
#include <QDesktopWidget>
#include <QPixmap>

// KDE includes.

#include <kdebug.h>
#include <kmimetypetrader.h>
#include <kdialog.h>
#include <klocale.h>
#include <kservice.h>
#include <krun.h>
#include <kaction.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kcursor.h>
#include <kdatetable.h>
#include <kiconloader.h>
#include <kapplication.h>

// LibKIPI includes.

#include <libkipi/pluginloader.h>
#include <libkipi/plugin.h>

// Local includes.

#include "dimg.h"
#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "databaseaccess.h"
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
        stack                = 0;
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

    AlbumWidgetStack  *stack;
};

ImagePreviewView::ImagePreviewView(QWidget *parent, AlbumWidgetStack *stack)
                : PreviewWidget(parent), d(new ImagePreviewViewPriv)
{
    d->stack = stack;

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    d->cornerButton = new QToolButton(this);
    d->cornerButton->setIcon(SmallIcon("transform-move"));
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

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
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
        d->previewThread->loadHighQuality(LoadingDescription(path, 0, AlbumSettings::instance()->getExifRotate()));
    else
        d->previewThread->load(LoadingDescription(path, d->previewSize, AlbumSettings::instance()->getExifRotate()));
}

void ImagePreviewView::slotGotImagePreview(const LoadingDescription &description, const DImg& preview)
{
    if (description.filePath != d->path)
        return;

    if (preview.isNull())
    {
        d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Cannot display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        // three copies - but the image is small
        setImage(DImg(pix.toImage()));
        d->stack->previewLoaded();
        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);
        if (AlbumSettings::instance()->getExifRotate())
            d->previewThread->exifRotate(img, description.filePath);
        d->stack->setPreviewMode(AlbumWidgetStack::PreviewImageMode);
        setImage(img);
        d->stack->previewLoaded();
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

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(LoadingDescription(loadPath, 0,
                                          AlbumSettings::instance()->getExifRotate()));
    else
        d->previewPreloadThread->load(LoadingDescription(loadPath, d->previewSize,
                                      AlbumSettings::instance()->getExifRotate()));
}

void ImagePreviewView::setImageInfo(const ImageInfo & info, const ImageInfo &previous, const ImageInfo &next)
{
    d->imageInfo = info;
    d->hasPrev   = previous.isNull() ? false : true;
    d->hasNext   = next.isNull()     ? false : true;

    if (!d->imageInfo.isNull())
        setImagePath(info.filePath());
    else
        setImagePath();

    setPreviousNextPaths(previous.filePath(), next.filePath());
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

    QMap<QAction *,KService::Ptr> serviceMap;

    const KService::List offers = KMimeTypeTrader::self()->query(mimePtr->name());
    KService::List::ConstIterator iter;
    KService::Ptr ptr;

    KMenu openWithMenu;

    for( iter = offers.begin(); iter != offers.end(); ++iter )
    {
        ptr = *iter;
        QAction *serviceAction = openWithMenu.addAction(SmallIcon(ptr->icon()), ptr->name());
        serviceMap[serviceAction] = ptr;
    }

    if (openWithMenu.isEmpty())
        openWithMenu.menuAction()->setEnabled(false);

    //-- Navigate actions -------------------------------------------

    DPopupMenu popmenu(this);
    QAction *backAction        = popmenu.addAction(SmallIcon("go-previous"), i18nc("go to previous image", "Back"));
    backAction->setEnabled(d->hasPrev);

    QAction *forwardAction     = popmenu.addAction(SmallIcon("go-next"), i18nc("go to next image", "Forward"));
    forwardAction->setEnabled(d->hasNext);

    QAction *backToAlbumAction = popmenu.addAction(SmallIcon("folder-image"), i18n("Back to Album"));

    //-- Edit actions -----------------------------------------------

    popmenu.addSeparator();
    QAction *slideshowAction    = popmenu.addAction(SmallIcon("view-presentation"), i18n("Slideshow"));
    QAction *editAction         = popmenu.addAction(SmallIcon("editimage"),         i18n("Edit..."));
    QAction *lighttableAction   = popmenu.addAction(SmallIcon("lighttableadd"),     i18n("Add to Light Table"));
    QAction *findSimilarAction  = popmenu.addAction(SmallIcon("tools-wizard"),      i18n("Find Similar"));
    popmenu.addMenu(&openWithMenu);
    openWithMenu.menuAction()->setText(i18n("Open With"));

    // Merge in the KIPI plugins actions ----------------------------

    KIPI::PluginLoader* kipiPluginLoader      = KIPI::PluginLoader::instance();
    KIPI::PluginLoader::PluginList pluginList = kipiPluginLoader->pluginList();

    for (KIPI::PluginLoader::PluginList::const_iterator it = pluginList.constBegin();
        it != pluginList.constEnd(); ++it)
    {
        KIPI::Plugin* plugin = (*it)->plugin();

        if (plugin && (*it)->name() == "JPEGLossless")
        {
            //kDebug(50003) << "Found JPEGLossless plugin" << endl;

            QList<KAction*> actionList = plugin->actions();

            for (QList<KAction*>::const_iterator iter = actionList.constBegin();
                iter != actionList.constEnd(); ++iter)
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

    popmenu.addSeparator();
    QAction *trashAction = popmenu.addAction(SmallIcon("user-trash"), i18n("Move to Trash"));

    // Bulk assignment/removal of tags --------------------------

    QList<qlonglong> idList;
    idList << d->imageInfo.id();

    assignTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::ASSIGN);
    removeTagsMenu = new TagsPopupMenu(idList, TagsPopupMenu::REMOVE);

    popmenu.addSeparator();

    popmenu.addMenu(assignTagsMenu);
    assignTagsMenu->menuAction()->setText(i18n("Assign Tag"));

    popmenu.addMenu(removeTagsMenu);
    removeTagsMenu->menuAction()->setText(i18n("Remove Tag"));

    connect(assignTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotAssignTag(int)));

    connect(removeTagsMenu, SIGNAL(signalTagActivated(int)),
            this, SLOT(slotRemoveTag(int)));

    if (!DatabaseAccess().db()->hasTags( idList ))
        removeTagsMenu->menuAction()->setEnabled(false);

    popmenu.addSeparator();

    // Assign Star Rating -------------------------------------------

    ratingMenu = new RatingPopupMenu();

    connect(ratingMenu, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotAssignRating(int)));

    popmenu.addMenu(ratingMenu);
    ratingMenu->menuAction()->setText(i18n("Assign Rating"));

    // --------------------------------------------------------

    QAction *choice = popmenu.exec(QCursor::pos());

    if (choice)
    {
        if (choice == backAction)                // Back
        {
            emit signalPrevItem();
        }
        else if (choice == forwardAction)        // Forward
        {
            emit signalNextItem();
        }
        else if (choice == editAction)           // Edit...
        {
            emit signalEditItem();
        }
        else if (choice == trashAction)          // Move to trash
        {
            emit signalDeleteItem();
        }
        else if (choice == backToAlbumAction)    // Back to album
        {
            emit signalBack2Album();
        }
        else if (choice == slideshowAction)      // SlideShow
        {
            emit signalSlideShow();
        }
        else if (choice == lighttableAction)     // Place onto Light Table
        {
            emit signalInsert2LightTable();
        }
        else if (choice == findSimilarAction)    // Find Similar
        {
            emit signalFindSimilar();
        }
        else if (serviceMap.contains(choice))
        {
            KService::Ptr imageServicePtr = serviceMap[choice];
            KRun::run(*imageServicePtr, url,this);
        }
    }

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
    QPalette plt(palette());
    plt.setColor(backgroundRole(), ThemeEngine::instance()->baseColor());
    setPalette(plt);
}

void ImagePreviewView::slotCornerButtonPressed()
{
    if (d->panIconPopup)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
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

    connect(pan, SIGNAL(signalHidden()),
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

void ImagePreviewView::slotPanIconSelectionMoved(const QRect& r, bool b)
{
    setContentsPos((int)(r.x()*zoomFactor()), (int)(r.y()*zoomFactor()));

    if (b)
    {
        d->panIconPopup->hide();
        d->panIconPopup->deleteLater();
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
    // Set zoom for fit-in-window as minimum, but don't scale up images
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

}  // namespace Digikam

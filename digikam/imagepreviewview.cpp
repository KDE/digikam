/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : a embedded view to show the image preview widget.
 *
 * Copyright (C) 2006-2010 Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "imagepreviewview.moc"

// Qt includes

#include <QCursor>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QToolBar>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kapplication.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>

// LibKIPI includes

#include <libkipi/plugin.h>
#include <libkipi/pluginloader.h>

// Local includes

#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "albumwidgetstack.h"
#include "contextmenuhelper.h"
#include "databaseaccess.h"
#include "digikamapp.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "imageinfo.h"
#include "loadingdescription.h"
#include "metadatahub.h"
#include "previewloadthread.h"
#include "ratingpopupmenu.h"
#include "tagspopupmenu.h"
#include "themeengine.h"

namespace Digikam
{

class ImagePreviewViewPriv
{
public:

    ImagePreviewViewPriv()
    {
        previewThread        = 0;
        previewPreloadThread = 0;
        stack                = 0;
        loadFullImageSize    = false;
        previewSize          = 1024;
        isLoaded             = false;
        toolBar              = 0;
        back2AlbumAction     = 0;
        prevAction           = 0;
        nextAction           = 0;
        rotLeftAction        = 0;
        rotRightAction       = 0;
    }

    bool               loadFullImageSize;
    bool               isLoaded;

    int                previewSize;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    QAction*           back2AlbumAction;
    QAction*           prevAction;
    QAction*           nextAction;
    QAction*           rotLeftAction;
    QAction*           rotRightAction;

    QToolBar*          toolBar;

    DImg               preview;

    ImageInfo          imageInfo;

    PreviewLoadThread* previewThread;
    PreviewLoadThread* previewPreloadThread;

    AlbumWidgetStack*  stack;
};

ImagePreviewView::ImagePreviewView(AlbumWidgetStack* parent)
                : PreviewWidget(parent), d(new ImagePreviewViewPriv)
{
    d->stack            = parent;
    d->back2AlbumAction = new QAction(SmallIcon("folder-image"),        i18n("Back to Album"),                  this);
    d->prevAction       = new QAction(SmallIcon("go-previous"),         i18nc("go to previous image", "Back"),  this);
    d->nextAction       = new QAction(SmallIcon("go-next"),             i18nc("go to next image", "Forward"),   this);
    d->rotLeftAction    = new QAction(SmallIcon("object-rotate-left"),  i18nc("@info:tooltip", "Rotate Left"),  this);
    d->rotRightAction   = new QAction(SmallIcon("object-rotate-right"), i18nc("@info:tooltip", "Rotate Right"), this);

    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());
    if (d->previewSize < 640)
        d->previewSize = 640;
    if (d->previewSize > 2560)
        d->previewSize = 2560;

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalShowNextImage()),
            this, SIGNAL(signalNextItem()));

    connect(this, SIGNAL(signalShowPrevImage()),
            this, SIGNAL(signalPrevItem()));

    connect(this, SIGNAL(signalActivated()),
            this, SIGNAL(signalBack2Album()));

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(d->prevAction, SIGNAL(triggered()),
            this, SIGNAL(signalPrevItem()));

    connect(d->nextAction, SIGNAL(triggered()),
            this, SIGNAL(signalNextItem()));

    connect(d->back2AlbumAction, SIGNAL(triggered()),
            this, SIGNAL(signalBack2Album()));

    connect(d->rotLeftAction, SIGNAL(triggered()),
            this, SLOT(slotRotateLeft()));

    connect(d->rotRightAction, SIGNAL(triggered()),
            this, SLOT(slotRotateRight()));

    // ------------------------------------------------------------

    d->toolBar = new QToolBar(this);
    d->toolBar->addAction(d->prevAction);
    d->toolBar->addAction(d->nextAction);
    d->toolBar->addAction(d->back2AlbumAction);
    d->toolBar->addAction(d->rotLeftAction);
    d->toolBar->addAction(d->rotRightAction);

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

void ImagePreviewView::setPreviousNextPaths(const QString& previous, const QString& next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void ImagePreviewView::setImagePath(const QString& path)
{
    setCursor( Qt::WaitCursor );

    d->path         = path;
    d->nextPath.clear();
    d->previousPath.clear();

    if (d->path.isEmpty())
    {
        slotReset();
        unsetCursor();
        d->isLoaded = false;
        return;
    }

    if (!d->previewThread)
    {
        d->previewThread = new PreviewLoadThread();
        d->previewThread->setDisplayingWidget(this);
        connect(d->previewThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
                this, SLOT(slotGotImagePreview(const LoadingDescription&, const DImg&)));
    }

    if (!d->previewPreloadThread)
    {
        d->previewPreloadThread = new PreviewLoadThread();
        d->previewPreloadThread->setDisplayingWidget(this);
        connect(d->previewPreloadThread, SIGNAL(signalImageLoaded(const LoadingDescription&, const DImg&)),
                this, SLOT(slotNextPreload()));
    }

    if (d->loadFullImageSize)
        d->previewThread->loadHighQuality(path, AlbumSettings::instance()->getExifRotate());
    else
        d->previewThread->load(path, d->previewSize, AlbumSettings::instance()->getExifRotate());
}

void ImagePreviewView::slotGotImagePreview(const LoadingDescription& description, const DImg& preview)
{
    if (description.filePath != d->path || description.isThumbnail())
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
        d->isLoaded = false;
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
        d->isLoaded = true;
        emit signalPreviewLoaded(true);
    }

    d->rotLeftAction->setEnabled(d->isLoaded);
    d->rotRightAction->setEnabled(d->isLoaded);

    unsetCursor();
    slotNextPreload();
}

void ImagePreviewView::slotNextPreload()
{
    QString loadPath;
    if (!d->nextPath.isNull())
    {
        loadPath = d->nextPath;
        d->nextPath.clear();
    }
    else if (!d->previousPath.isNull())
    {
        loadPath = d->previousPath;
        d->previousPath.clear();
    }
    else
    {
        return;
    }

    if (d->loadFullImageSize)
        d->previewPreloadThread->loadHighQuality(loadPath, AlbumSettings::instance()->getExifRotate());
    else
        d->previewPreloadThread->load(loadPath, d->previewSize, AlbumSettings::instance()->getExifRotate());
}

void ImagePreviewView::setImageInfo(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    if (d->imageInfo != info)
    {
        d->imageInfo = info;
        if (!d->imageInfo.isNull())
            setImagePath(info.filePath());
        else
            setImagePath();
    }

    d->prevAction->setEnabled(!previous.isNull());
    d->nextAction->setEnabled(!next.isNull());

    setPreviousNextPaths(previous.filePath(), next.filePath());
}

ImageInfo ImagePreviewView::getImageInfo() const
{
    return d->imageInfo;
}

void ImagePreviewView::slotContextMenu()
{
    if (d->imageInfo.isNull())
        return;

    QList<qlonglong> idList;
    idList << d->imageInfo.id();
    KUrl::List selectedItems;
    selectedItems << d->imageInfo.fileUrl();

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(d->prevAction, true);
    cmhelper.addAction(d->nextAction, true);
    cmhelper.addAction(d->back2AlbumAction);
    cmhelper.addGotoMenu(idList);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addKipiActions(idList);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionItemDelete(this, SLOT(slotDeleteItem()));
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addRatingMenu();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTag(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(const ImageInfo&)),
            this, SIGNAL(signalGotoAlbumAndItem(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(const ImageInfo&)),
            this, SIGNAL(signalGotoDateAndItem(const ImageInfo&)));

    cmhelper.exec(QCursor::pos());
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

void ImagePreviewView::slotDeleteItem()
{
    emit signalDeleteItem();
}

void ImagePreviewView::resizeEvent(QResizeEvent* e)
{
    if (!e) return;

    Q3ScrollView::resizeEvent(e);

    updateZoomAndSize(false);
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
    d->path.clear();
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

void ImagePreviewView::viewportPaintExtraData()
{
    if (!m_movingInProgress && d->isLoaded)
    {
        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setBackgroundMode(Qt::TransparentMode);
        QFontMetrics fontMt = p.fontMetrics();

        QString text;
        QRect textRect, fontRect;
        QRect region = contentsRect();
        p.translate(region.topLeft());

        if (!d->loadFullImageSize)
        {
            if (d->imageInfo.format().startsWith(QLatin1String("RAW")))
                text = i18n("Embedded JPEG Preview");
            else
                text = i18n("Reduced Size Preview");
        }
        else
        {
            if (d->imageInfo.format().startsWith(QLatin1String("RAW")))
                text = i18n("Half Size Raw Preview");
            else
                text = i18n("Full Size Preview");
        }

        fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
        drawText(&p, QPoint(region.topRight().x()-fontRect.width()-10, region.topRight().y()+5), text);
        p.end();
    }
}

void ImagePreviewView::slotGotoTag(int tagID)
{
    emit signalGotoTagAndItem(tagID);
}

QImage ImagePreviewView::previewToQImage() const
{
    return d->preview.copyQImage();
}

void ImagePreviewView::slotRotateLeft()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());
    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_ccw"))
                ac->trigger();
        }
    }
}

void ImagePreviewView::slotRotateRight()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());
    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_cw"))
                ac->trigger();
        }
    }
}

}  // namespace Digikam

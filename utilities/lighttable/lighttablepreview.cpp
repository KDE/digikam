/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-21-12
 * Description : digiKam light table preview item.
 *
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablepreview.moc"

// Qt includes

#include <QList>
#include <QPainter>
#include <QCursor>
#include <QString>
#include <QFileInfo>
#include <QPixmap>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QResizeEvent>
#include <QDesktopWidget>

// KDE includes

#include <kapplication.h>
#include <kcursor.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <kmimetypetrader.h>
#include <kdebug.h>

// Local includes

#include "albumdb.h"
#include "albummanager.h"
#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "ddragobjects.h"
#include "dimg.h"
#include "dmetadata.h"
#include "dpopupmenu.h"
#include "loadingdescription.h"
#include "metadatahub.h"
#include "metadatasettings.h"
#include "previewloadthread.h"
#include "tagspopupmenu.h"
#include "thememanager.h"
#include "globals.h"

namespace Digikam
{

class LightTablePreview::LightTablePreviewPriv
{
public:

    LightTablePreviewPriv() :
        isLoaded(false),
        hasPrev(false),
        hasNext(false),
        selected(false),
        dragAndDropEnabled(true),
        loadFullImageSize(false),
        previewSize(1024),
        previewThread(0),
        previewPreloadThread(0)
    {
    }

    bool               isLoaded;
    bool               hasPrev;
    bool               hasNext;
    bool               selected;
    bool               dragAndDropEnabled;
    bool               loadFullImageSize;

    int                previewSize;

    QString            path;
    QString            nextPath;
    QString            previousPath;

    DImg               preview;

    ImageInfo          imageInfo;

    PreviewLoadThread* previewThread;
    PreviewLoadThread* previewPreloadThread;
};

LightTablePreview::LightTablePreview(QWidget* parent)
    : PreviewWidget(parent), d(new LightTablePreviewPriv)
{
    // get preview size from screen size, but limit from VGA to WQXGA
    d->previewSize = qMax(KApplication::desktop()->height(),
                          KApplication::desktop()->width());

    if (d->previewSize < 640)
    {
        d->previewSize = 640;
    }

    if (d->previewSize > 2560)
    {
        d->previewSize = 2560;
    }

    viewport()->setAcceptDrops(true);
    setAcceptDrops(true);

    slotThemeChanged();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    setLineWidth(5);
    setSelected(false);

    // ------------------------------------------------------------

    connect(this, SIGNAL(signalRightButtonClicked()),
            this, SLOT(slotContextMenu()));

    connect(ThemeManager::instance(), SIGNAL(signalThemeChanged()),
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
        pix.fill(kapp->palette().color(QPalette::Base));
        QPainter p(&pix);
        p.setPen(QPen(kapp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Drag and drop an image here"));
        p.end();
        setImage(DImg(pix.toImage()));
    }
}

void LightTablePreview::setImage(const DImg& image)
{
    d->preview = image;

    updateZoomAndSize(false);

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

void LightTablePreview::setPreviousNextPaths(const QString& previous, const QString& next)
{
    d->nextPath     = next;
    d->previousPath = previous;
}

void LightTablePreview::setImagePath(const QString& path)
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
    {
        d->previewThread->loadHighQuality(path, MetadataSettings::instance()->settings().exifRotate);
    }
    else
    {
        d->previewThread->load(path, d->previewSize, MetadataSettings::instance()->settings().exifRotate);
    }
}

void LightTablePreview::slotGotImagePreview(const LoadingDescription& description, const DImg& preview)
{
    if (description.filePath != d->path)
    {
        return;
    }

    if (preview.isNull())
    {
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(kapp->palette().color(QPalette::Base));
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(kapp->palette().color(QPalette::Text)));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Unable to display preview for\n\"%1\"",
                        info.fileName()));
        p.end();
        setImage(DImg(pix.toImage()));
        d->isLoaded = false;
        emit signalPreviewLoaded(false);
    }
    else
    {
        DImg img(preview);

        if (MetadataSettings::instance()->settings().exifRotate)
        {
            d->previewThread->exifRotate(img, description.filePath);
        }

        setImage(img);
        d->isLoaded = true;
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
    {
        d->previewThread->loadHighQuality(loadPath, MetadataSettings::instance()->settings().exifRotate);
    }
    else
    {
        d->previewThread->load(loadPath, d->previewSize, MetadataSettings::instance()->settings().exifRotate);
    }
}

void LightTablePreview::setImageInfo(const ImageInfo& info, const ImageInfo& previous, const ImageInfo& next)
{
    d->imageInfo = info;
    d->hasPrev   = !previous.isNull();
    d->hasNext   = !next.isNull();

    if (!d->imageInfo.isNull())
    {
        setImagePath(info.filePath());
    }
    else
    {
        setImagePath();
        setSelected(false);
    }

    setPreviousNextPaths(previous.isNull() ? QString() : previous.filePath(),
                         next.isNull()     ? QString() : next.filePath());
}

ImageInfo LightTablePreview::getImageInfo() const
{
    return d->imageInfo;
}

void LightTablePreview::slotContextMenu()
{
    if (d->imageInfo.isNull())
    {
        return;
    }

    QList<qlonglong> idList;
    idList << d->imageInfo.id();
    KUrl::List selectedItems;
    selectedItems << d->imageInfo.fileUrl();

    //-- temporary actions -----------------------------------------------

    QAction* zoomInAction    = new QAction(SmallIcon("zoom-in"),           i18n("Zoom in"), this);
    QAction* zoomOutAction   = new QAction(SmallIcon("zoom-out"),          i18n("Zoom out"), this);
    QAction* fitWindowAction = new QAction(SmallIcon("zoom-fit-best"),     i18n("Fit to &Window"), this);
    QAction* slideshowAction = new QAction(SmallIcon("view-presentation"), i18n("Slideshow"), this);
    QAction* editAction      = new QAction(SmallIcon("editimage"),         i18n("Edit..."), this);
    QAction* trashAction     = new QAction(SmallIcon("user-trash"),        i18nc("Non-pluralized", "Move to Trash"), this);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(zoomInAction);
    cmhelper.addAction(zoomOutAction);
    cmhelper.addAction(fitWindowAction);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(slideshowAction);
    cmhelper.addAction(editAction);
    cmhelper.addServicesMenu(selectedItems);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(trashAction);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(idList);
    cmhelper.addRemoveTagsMenu(idList);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addLabelsAction();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(slotAssignPickLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(slotAssignColorLabel(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    QAction* choice = cmhelper.exec(QCursor::pos());

    if (choice)
    {
        if (choice == editAction)
        {
            emit signalEditItem(d->imageInfo);
        }
        else if (choice == trashAction)
        {
            emit signalDeleteItem(d->imageInfo);
        }
        else if (choice == slideshowAction)
        {
            emit signalSlideShow();
        }
        else if (choice == zoomInAction)
        {
            slotIncreaseZoom();
        }
        else if (choice == zoomOutAction)
        {
            slotDecreaseZoom();
        }
        else if (choice == fitWindowAction)
        {
            fitToWindow();
        }
    }
}

void LightTablePreview::slotAssignTag(int tagID)
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

void LightTablePreview::slotRemoveTag(int tagID)
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

void LightTablePreview::slotAssignPickLabel(int pickId)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setPickLabel(pickId);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotAssignColorLabel(int colorId)
{
    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setColorLabel(colorId);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotAssignRating(int rating)
{
    rating = qMin(RatingMax, qMax(RatingMin, rating));

    if (!d->imageInfo.isNull())
    {
        MetadataHub hub;
        hub.load(d->imageInfo);
        hub.setRating(rating);
        hub.write(d->imageInfo, MetadataHub::PartialWrite);
        hub.write(d->imageInfo.filePath(), MetadataHub::FullWriteIfChanged);
    }
}

void LightTablePreview::slotThemeChanged()
{
    setBackgroundColor(kapp->palette().color(QPalette::Base));
    frameChanged();
    update();
}

void LightTablePreview::resizeEvent(QResizeEvent* e)
{
    if (!e)
    {
        return;
    }

    Q3ScrollView::resizeEvent(e);

    if (d->imageInfo.isNull())
    {
        setDragAndDropMessage();
    }

    updateZoomAndSize(false);
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
    d->path.clear();
    d->imageInfo = ImageInfo();

    setDragAndDropMessage();
    updateZoomAndSize(true);
    viewport()->setUpdatesEnabled(true);
    viewport()->update();
    emit signalPreviewLoaded(false);
}

void LightTablePreview::paintPreview(QPixmap* pix, int sx, int sy, int sw, int sh)
{
    DImg img     = d->preview.smoothScaleSection(sx, sy, sw, sh, tileSize(), tileSize());
    QPixmap pix2 = img.convertToPixmap();
    QPainter p(pix);
    p.drawPixmap(0, 0, pix2, 0, 0, pix2.width(), pix2.height());
    p.end();
}

void LightTablePreview::viewportPaintExtraData()
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
        //p.translate(region.topLeft());

        if (!d->loadFullImageSize)
        {
            if (d->imageInfo.format().startsWith(QLatin1String("RAW")))
            {
                text = i18n("Embedded JPEG Preview");
            }
            else
            {
                text = i18n("Reduced Size Preview");
            }
        }
        else
        {
            if (d->imageInfo.format().startsWith(QLatin1String("RAW")))
            {
                text = i18n("Half Size Raw Preview");
            }
            else
            {
                text = i18n("Full Size Preview");
            }
        }

        fontRect = fontMt.boundingRect(0, 0, contentsWidth(), contentsHeight(), 0, text);
        textRect.setTopLeft(QPoint(region.topRight().x()-fontRect.width()-20, region.topRight().y()+20));
        textRect.setSize( QSize(fontRect.width()+2, fontRect.height()+2) );

        drawText(&p, QPoint(region.topRight().x()-fontRect.width()-20, region.topRight().y()+20), text);
        p.end();
    }
}

void LightTablePreview::contentsDragEnterEvent(QDragEnterEvent* e)
{
    if (d->dragAndDropEnabled)
    {
        int        albumID;
        QList<int> albumIDs;
        QList<qlonglong> imageIDs;
        KUrl::List urls;
        KUrl::List kioURLs;

        if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs) ||
            DAlbumDrag::decode(e->mimeData(), urls, albumID)                    ||
            DTagDrag::canDecode(e->mimeData()))
        {
            e->accept();
            return;
        }
    }

    e->ignore();
}

void LightTablePreview::contentsDropEvent(QDropEvent* e)
{
    if (d->dragAndDropEnabled)
    {
        int           albumID;
        QList<int>    albumIDs;
        QList<qlonglong> imageIDs;
        KUrl::List    urls;
        KUrl::List    kioURLs;

        if (DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            emit signalDroppedItems(ImageInfoList(imageIDs));
            e->accept();
            return;
        }
        else if (DAlbumDrag::decode(e->mimeData(), urls, albumID))
        {
            QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInAlbum(albumID);

            emit signalDroppedItems(ImageInfoList(itemIDs));
            e->accept();
            return;
        }
        else if (DTagDrag::canDecode(e->mimeData()))
        {
            int tagID;

            if (!DTagDrag::decode(e->mimeData(), tagID))
            {
                return;
            }

            QList<qlonglong> itemIDs = DatabaseAccess().db()->getItemIDsInTag(tagID, true);
            ImageInfoList imageInfoList;

            emit signalDroppedItems(ImageInfoList(itemIDs));
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
        update();
    }
}

bool LightTablePreview::isSelected()
{
    return d->selected;
}

void LightTablePreview::drawFrame(QPainter* p)
{
    if (d->selected)
    {
        qDrawPlainRect(p, frameRect(), kapp->palette().color(QPalette::Highlight),       lineWidth());
        qDrawPlainRect(p, frameRect(), kapp->palette().color(QPalette::HighlightedText), 2);
    }
    else
    {
        qDrawPlainRect(p, frameRect(), kapp->palette().color(QPalette::Base), lineWidth());
    }
}

QImage LightTablePreview::previewToQImage() const
{
    return d->preview.copyQImage();
}

}  // namespace Digikam

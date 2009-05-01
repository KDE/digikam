/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikamimageview.h"
#include "digikamimageview.moc"

// Qt includes

#include <QClipboard>
#include <QFileInfo>

// KDE includes

#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kmenu.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kwindowsystem.h>

// Local includes

#include "albumsettings.h"
#include "contextmenuhelper.h"
#include "ddragobjects.h"
#include "dio.h"
#include "dpopupmenu.h"
#include "imagealbummodel.h"
#include "imagealbumfiltermodel.h"
#include "imagedragdrop.h"
#include "imagewindow.h"
#include "thumbnailloadthread.h"

namespace Digikam
{

class DigikamImageViewPriv
{
public:
    DigikamImageViewPriv()
    {
    }
};

DigikamImageView::DigikamImageView(QWidget *parent)
                : ImageCategorizedView(parent), d(new DigikamImageViewPriv)
{
    imageFilterModel()->setCategorizationMode(ImageFilterModel::CategoryByAlbum);
    imageModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    setThumbnailSize((ThumbnailSize::Size)AlbumSettings::instance()->getDefaultIconSize());
    imageModel()->setDragDropHandler(new ImageDragDropHandler(imageModel()));
}

DigikamImageView::~DigikamImageView()
{
    delete d;
}

void DigikamImageView::activated(const ImageInfo& info)
{
    if (info.isNull())
        return;

    if (AlbumSettings::instance()->getItemRightClickAction() == AlbumSettings::ShowPreview)
        previewRequested(info);
    else
        openInEditor(info);
}

void DigikamImageView::openInEditor(const ImageInfo &info)
{
    if (info.isNull())
        return;

    QFileInfo fi(info.filePath());
    QString imagefilter = AlbumSettings::instance()->getImageFileFilter();
    imagefilter += AlbumSettings::instance()->getRawFileFilter();

    // If the current item is not an image file.
    if ( !imagefilter.contains(fi.suffix().toLower()) )
    {
        KMimeType::Ptr mimePtr = KMimeType::findByUrl(info.fileUrl(), 0, true, true);
        const KService::List offers = KServiceTypeTrader::self()->query(mimePtr->name(), "Type == 'Application'");

        if (offers.isEmpty())
            return;

        KService::Ptr ptr = offers.first();
        // Run the dedicated app to show the item.
        KRun::run(*ptr, info.fileUrl(), this);
        return;
    }

    // Run digiKam ImageEditor with all image from current Album.

    ImageWindow *imview = ImageWindow::imagewindow();

    imview->disconnect(this);
    connect(imview, SIGNAL(signalURLChanged(const KUrl&)),
            this, SLOT(setCurrentUrl(const KUrl &)));

    imview->loadImageInfos(imageInfos(), info,
                           currentAlbum() ? i18n("Album \"%1\"", currentAlbum()->title()) : QString(),
                           true);

    if (imview->isHidden())
        imview->show();
    if (imview->isMinimized())
        KWindowSystem::unminimizeWindow(imview->winId());
    KWindowSystem::activateWindow(imview->winId());
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event, const ImageInfo& info)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;
    foreach (ImageInfo info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    // Temporary actions --------------------------------------

    QAction  *viewAction = new QAction(SmallIcon("viewimage"), i18nc("View the selected image", "View"),  this);
    viewAction->setEnabled(selectedImageIDs.count() == 1);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction("album_new_from_selection");
    cmhelper.addAction(viewAction);
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedUrls());
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addGotoMenu(selectedImageIDs);
    cmhelper.addKipiActions();
    cmhelper.addAction("image_rename");
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionCopy(this, SLOT(copy()));
    cmhelper.addActionPaste(this, SLOT(paste()));
    cmhelper.addActionItemDelete(this, SLOT(slotDeleteSelectedItems()), selectedImageIDs.count());
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addActionThumbnail(selectedImageIDs, currentAlbum());
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(selectedImageIDs);
    cmhelper.addRemoveTagsMenu(selectedImageIDs);
    popmenu.addSeparator();
    // --------------------------------------------------------
    cmhelper.addRatingMenu();

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(slotAssignTag(int)));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(slotRemoveTag(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SLOT(slotGotoTag(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(ImageInfo&)),
            this, SIGNAL(signalGotoAlbumAndItem(ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(ImageInfo&)),
            this, SIGNAL(signalGotoDateAndItem(ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(slotAssignRating(int)));

    connect(&cmhelper, SIGNAL(signalSetThumbnail(ImageInfo&)),
            this, SLOT(slotSetAlbumThumbnail(ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SIGNAL(signalAddToExistingQueue(int)));

    // --------------------------------------------------------

    QAction *choice = cmhelper.exec(event->pos());
    if (choice)
    {
        if (choice == viewAction)
            emit previewRequested(info);
    }

    // cleanup -----------------------

    popmenu.deleteLater();
    delete viewAction;
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event)
{
    Album *album = currentAlbum();

    if (!album ||
         album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
    {
        return;
    }

    KMenu popmenu(this);
    KAction *paste        = KStandardAction::paste(this, SLOT(slotPaste()), 0);
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    //TODO
    if(!data || !KUrl::List::canDecode(data))
        paste->setEnabled(false);

    popmenu.addAction(paste);
    popmenu.exec(event->pos());
    delete paste;
}

void DigikamImageView::copy()
{
    QMimeData *data = imageModel()->dragDropHandler()->createMimeData(selectedImageInfos());
    if (data)
        kapp->clipboard()->setMimeData(data);
}

void DigikamImageView::paste()
{
    const QMimeData *data = kapp->clipboard()->mimeData(QClipboard::Clipboard);
    if (!data)
        return;
    QDropEvent event(mapFromGlobal(QCursor::pos()), Qt::CopyAction, data, Qt::NoButton, Qt::ControlModifier);
    QModelIndex index = indexAt(event.pos());
    if (!imageModel()->dragDropHandler()->accepts(&event, index))
        return;
    imageModel()->dragDropHandler()->dropEvent(this, &event, index);
}

/*
//TODO
slotDeleteSelectedItems
slotAssignTag
slotRemoveTag
slotAssignRating
slotSetAlbumThumbnail
slotGotoTag
slotDIOResult

changeTagOnImageInfos

signalGotoAlbumAndItem
signalGotoDateAndItem
signalAddToExistingQueue
signalPreviewItem
*/

} // namespace Digikam

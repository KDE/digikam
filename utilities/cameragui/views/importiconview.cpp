/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-22-07
 * Description : Icon view for import tool items
 *
 * Copyright (C) 2012 by Islam Wazery <wazery at ubuntu dot com>
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

#include "importiconview.moc"
#include "importiconview_p.h"

// Qt includes

#include <QPointer>
#include <QAction>

// KDE includes

#include <KLocale>
#include <KMenu>

// Local includes

#include "importsettings.h"
#include "camitemsortsettings.h"
#include "fileactionmngr.h"
#include "importdelegate.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "imageviewutilities.h"
#include "contextmenuhelper.h"

namespace Digikam
{

ImportIconView::ImportIconView(QWidget* parent)
    : ImportCategorizedView(parent), d(new ImportIconViewPriv(this))
{
}

void ImportIconView::init(CameraController* controller)
{
    installDefaultModels(controller);

    d->normalDelegate = new ImportNormalDelegate(this);
    if(d->normalDelegate)
        setItemDelegate(d->normalDelegate);
    setSpacing(10);

    ImportSettings* settings = ImportSettings::instance();

    importFilterModel()->setCategorizationMode(CamItemSortSettings::CategoryByFolder);

    setThumbnailSize((ThumbnailSize::Size)settings->getDefaultIconSize());

    //importImageModel()->setDragDropHandler(new ImageDragDropHandler(importImageModel()));
    //setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());
    importFilterModel()->setSortRole((CamItemSortSettings::SortRole)settings->getImageSortOrder());
    importFilterModel()->setSortOrder((CamItemSortSettings::SortOrder)settings->getImageSorting());
    importFilterModel()->setCategorizationMode((CamItemSortSettings::CategorizationMode)settings->getImageGroupMode());

    // selection overlay
    addSelectionOverlay(d->normalDelegate);
    //TODO: addSelectionOverlay(d->faceDelegate);

    // rotation overlays
    //TODO: d->rotateLeftOverlay = ImageRotateOverlay::left(this);
    //TODO: d->rotateRightOverlay = ImageRotateOverlay::right(this);
    //TODO: d->updateOverlays();

    // rating overlay
    //TODO: ImageRatingOverlay* ratingOverlay = new ImageRatingOverlay(this);
    //TODO: addOverlay(ratingOverlay);

    //TODO: GroupIndicatorOverlay* groupOverlay = new GroupIndicatorOverlay(this);
    //TODO: addOverlay(groupOverlay);

    //TODO: connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            //this, SLOT(assignRating(QList<QModelIndex>,int)));

    //TODO: connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            //this, SLOT(groupIndicatorClicked(QModelIndex)));

    //TODO: connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            //this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    //TODO: connect(importImageModel()->dragDropHandler(), SIGNAL(assignTags(QList<CamItemInfo>,QList<int>)),
            //FileActionMngr::instance(), SLOT(assignTags(QList<CamItemInfo>,QList<int>)));

    connect(importImageModel()->dragDropHandler(), SIGNAL(addToGroup(CamItemInfo,QList<CamItemInfo>)),
            FileActionMngr::instance(), SLOT(addToGroup(CamItemInfo,QList<CamItemInfo>)));

    connect(settings, SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

ImportIconView::~ImportIconView()
{
    delete d;
}

ImageViewUtilities* ImportIconView::utilities() const
{
    return d->utilities;
}

void ImportIconView::setThumbnailSize(const ThumbnailSize& size)
{
    importThumbnailModel()->setThumbnailSize(size);
    ImportCategorizedView::setThumbnailSize(size);
}

int ImportIconView::fitToWidthIcons()
{
    return delegate()->calculatethumbSizeToFit(viewport()->size().width());
}

void ImportIconView::slotSetupChanged()
{
    setToolTipEnabled(ImportSettings::instance()->showToolTipsIsValid());
    setFont(ImportSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ImportCategorizedView::slotSetupChanged();
}

void ImportIconView::rename()
{
    KUrl::List urls = selectedUrls();
    NewNamesList newNamesList;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == KDialog::Accepted)
    {
        newNamesList = dlg->newNames();
    }

    delete dlg;

    if (!newNamesList.isEmpty())
    {
        QPointer<AdvancedRenameProcessDialog> dlg = new AdvancedRenameProcessDialog(newNamesList);
        dlg->exec();
        delete dlg;
    }
}

void ImportIconView::deleteSelected(bool /*permanently*/)
{
    CamItemInfoList camItemInfoList = selectedCamItemInfos();

    //FIXME: This way of deletion may not working with camera items.
    //if (d->utilities->deleteImages(camItemInfoList, permanently))
    //{
    //    awayFromSelection();
    //}
}

void ImportIconView::deleteSelectedDirectly(bool /*permanently*/)
{
    CamItemInfoList camItemInfoList = selectedCamItemInfos();
    //FIXME: This way of deletion may not working with camera items.
    //d->utilities->deleteImagesDirectly(camItemInfoList, permanently);
    awayFromSelection();
}

void ImportIconView::createGroupFromSelection()
{
    //TODO: Impelemnt grouping in import tool.
    //QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();
    //CamItemInfo groupLeader          = selectedInfos.takeFirst();
    //FileActionMngr::instance()->addToGroup(groupLeader, selectedInfos);
}

void ImportIconView::createGroupByTimeFromSelection()
{
    //TODO: Impelemnt grouping in import tool.
    //QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();

    //while (selectedInfos.size() > 0)
    //{
        //QList<CamItemInfo> group;
        //CamItemInfo groupLeader = selectedInfos.takeFirst();
        //QDateTime dateTime    = groupLeader.dateTime();

        //while (selectedInfos.size() > 0 && abs(dateTime.secsTo(selectedInfos.first().dateTime())) < 2)
        //{
        //    group.push_back(selectedInfos.takeFirst());
        //}

        //FileActionMngr::instance()->addToGroup(groupLeader, group);
    //}
}

void ImportIconView::ungroupSelected()
{
    //TODO: Impelemnt grouping in import tool.
    //FileActionMngr::instance()->ungroup(selectedCamItemInfos());
}

void ImportIconView::removeSelectedFromGroup()
{
    //TODO: Impelemnt grouping in import tool.
    //FileActionMngr::instance()->removeFromGroup(selectedCamItemInfos());
}

void ImportIconView::activated(const CamItemInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    if (ImportSettings::instance()->getItemLeftClickAction() == ImportSettings::ShowPreview)
    {
        emit previewRequested(info);
    }
    else
    {
        //TODO: openInEditor(info);
    }
}

//TODO: Implement context menu
void ImportIconView::showContextMenuOnInfo(QContextMenuEvent* /*event*/, const CamItemInfo& /*info*/)
{
//    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();
//    QList<qlonglong> selectedImageIDs;
//    foreach(const CamItemInfo& info, selectedInfos)
//    {
//        selectedImageIDs << info.id;
//    }

//    // Temporary actions --------------------------------------

//    QAction* viewAction = new QAction(SmallIcon("viewimage"), i18nc("View the selected image", "Preview"), this);
//    viewAction->setEnabled(selectedImageIDs.count() == 1);

//    // --------------------------------------------------------

//    KMenu popmenu(this);
//    ContextMenuHelper cmhelper(&popmenu);
//    cmhelper.setImageFilterModel(importFilterModel());

//    cmhelper.addAction("full_screen");
//    cmhelper.addSeparator();
//    // --------------------------------------------------------
//    cmhelper.addAction("move_selection_to_album");
//    cmhelper.addAction(viewAction);
//    cmhelper.addAction("image_edit");
//    cmhelper.addServicesMenu(selectedUrls());
//    cmhelper.addGotoMenu(selectedImageIDs);
//    cmhelper.addRotateMenu(selectedImageIDs);
//    cmhelper.addSeparator();
//    // --------------------------------------------------------
//    cmhelper.addAction("image_find_similar");
//    cmhelper.addStandardActionLightTable();
//    cmhelper.addQueueManagerMenu();
//    cmhelper.addSeparator();
//    // --------------------------------------------------------
//    cmhelper.addAction("image_rename");
//    cmhelper.addAction("cut_album_selection");
//    cmhelper.addAction("copy_album_selection");
//    cmhelper.addAction("paste_album_selection");
//    cmhelper.addStandardActionItemDelete(this, SLOT(deleteSelected()), selectedImageIDs.count());
//    cmhelper.addSeparator();
//    // --------------------------------------------------------
//    cmhelper.addStandardActionThumbnail(selectedImageIDs, currentAlbum());
//    // --------------------------------------------------------
//    cmhelper.addAssignTagsMenu(selectedImageIDs);
//    cmhelper.addRemoveTagsMenu(selectedImageIDs);
//    cmhelper.addSeparator();
//    // --------------------------------------------------------
//    cmhelper.addLabelsAction();
//    if (!d->faceMode)
//    {
//        cmhelper.addGroupMenu(selectedImageIDs);
//    }

//    // special action handling --------------------------------

//    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
//            this, SLOT(assignTagToSelected(int)));

//    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
//            this, SIGNAL(signalPopupTagsView()));

//    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
//            this, SLOT(removeTagFromSelected(int)));

//    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
//            this, SIGNAL(gotoTagAndImageRequested(int)));

//    connect(&cmhelper, SIGNAL(signalGotoAlbum(ImageInfo)),
//            this, SIGNAL(gotoAlbumAndImageRequested(ImageInfo)));

//    connect(&cmhelper, SIGNAL(signalGotoDate(ImageInfo)),
//            this, SIGNAL(gotoDateAndImageRequested(ImageInfo)));

//    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
//            this, SLOT(assignPickLabelToSelected(int)));

//    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
//            this, SLOT(assignColorLabelToSelected(int)));

//    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
//            this, SLOT(assignRatingToSelected(int)));

//    connect(&cmhelper, SIGNAL(signalSetThumbnail(ImageInfo)),
//            this, SLOT(setAsAlbumThumbnail(ImageInfo)));

//    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
//            this, SLOT(insertSelectedToExistingQueue(int)));

//    connect(&cmhelper, SIGNAL(signalCreateGroup()),
//            this, SLOT(createGroupFromSelection()));

//    connect(&cmhelper, SIGNAL(signalCreateGroupByTime()),
//            this, SLOT(createGroupByTimeFromSelection()));

//    connect(&cmhelper, SIGNAL(signalUngroup()),
//            this, SLOT(ungroupSelected()));

//    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
//            this, SLOT(removeSelectedFromGroup()));

//    // --------------------------------------------------------

//    QAction* choice = cmhelper.exec(event->globalPos());

//    if (choice && (choice == viewAction))
//    {
//        emit previewRequested(info);
//    }
}

void ImportIconView::showContextMenu(QContextMenuEvent* /*event*/)
{
//    Album* album = currentAlbum();

//    if (!album ||
//        album->isRoot() ||
//        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
//    {
//        return;
//    }

//    KMenu popmenu(this);
//    ContextMenuHelper cmhelper(&popmenu);
//    cmhelper.setImageFilterModel(imageFilterModel());

//    cmhelper.addAction("full_screen");
//    cmhelper.addSeparator();
//    // --------------------------------------------------------
//    cmhelper.addStandardActionPaste(this, SLOT(paste()));
//    // --------------------------------------------------------

//    cmhelper.exec(event->globalPos());
}

} // namespace Digikam

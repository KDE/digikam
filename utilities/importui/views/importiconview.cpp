/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-22-07
 * Description : Icon view for import tool items
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "importiconview.h"
#include "importiconview_p.h"

// Qt includes

#include <QPointer>
#include <QAction>
#include <QMenu>

// Local includes

#include "importcategorizedview.h"
#include "importoverlays.h"
#include "importsettings.h"
#include "camitemsortsettings.h"
#include "fileactionmngr.h"
#include "importdelegate.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "imageviewutilities.h"
#include "importcontextmenu.h"
#include "importdragdrop.h"

namespace Digikam
{

ImportIconView::ImportIconView(QWidget* const parent)
    : ImportCategorizedView(parent),
      d(new Private(this))
{
    ImportThumbnailModel* const model    = new ImportThumbnailModel(this);
    ImportFilterModel* const filterModel = new ImportFilterModel(this);

    filterModel->setSourceImportModel(model);
    filterModel->sort(0); // an initial sorting is necessary

    setModels(model, filterModel);

    d->normalDelegate              = new ImportNormalDelegate(this);

    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    ImportSettings* const settings = ImportSettings::instance();

    setThumbnailSize((ThumbnailSize::Size)settings->getDefaultIconSize());

    importImageModel()->setDragDropHandler(new ImportDragDropHandler(importImageModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());

    // selection overlay
    addSelectionOverlay(d->normalDelegate);
    //TODO: addSelectionOverlay(d->faceDelegate);

    // rotation overlays
    d->rotateLeftOverlay  = ImportRotateOverlay::left(this);
    d->rotateRightOverlay = ImportRotateOverlay::right(this);

    addOverlay(new ImportDownloadOverlay(this));
    addOverlay(new ImportLockOverlay(this));
    addOverlay(new ImportCoordinatesOverlay(this));

    d->updateOverlays();

    // rating overlay
    ImportRatingOverlay* const ratingOverlay = new ImportRatingOverlay(this);
    addOverlay(ratingOverlay);

    //TODO: GroupIndicatorOverlay* groupOverlay = new GroupIndicatorOverlay(this);
    //TODO: addOverlay(groupOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(QList<QModelIndex>,int)),
            this, SLOT(assignRating(QList<QModelIndex>,int)));

    //TODO: connect(groupOverlay, SIGNAL(toggleGroupOpen(QModelIndex)),
            //this, SLOT(groupIndicatorClicked(QModelIndex)));

    //TODO: connect(groupOverlay, SIGNAL(showButtonContextMenu(QModelIndex,QContextMenuEvent*)),
            //this, SLOT(showGroupContextMenu(QModelIndex,QContextMenuEvent*)));

    //TODO: connect(importImageModel()->dragDropHandler(), SIGNAL(assignTags(QList<CamItemInfo>,QList<int>)),
            //FileActionMngr::instance(), SLOT(assignTags(QList<CamItemInfo>,QList<int>)));

    //TODO: connect(importImageModel()->dragDropHandler(), SIGNAL(addToGroup(CamItemInfo,QList<CamItemInfo>)),
            //FileActionMngr::instance(), SLOT(addToGroup(CamItemInfo,QList<CamItemInfo>)));

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
    ImportCategorizedView::setThumbnailSize(size);
}

int ImportIconView::fitToWidthIcons()
{
    return delegate()->calculatethumbSizeToFit(viewport()->size().width());
}

CamItemInfo ImportIconView::camItemInfo(const QString& folder, const QString& file)
{
    QUrl url = QUrl::fromLocalFile(folder);
    url      = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (file));
    QModelIndex indexForCamItemInfo = importFilterModel()->indexForPath(url.toLocalFile());

    if (indexForCamItemInfo.isValid())
    {
        return importFilterModel()->camItemInfo(indexForCamItemInfo);
    }

    return CamItemInfo();
}

CamItemInfo& ImportIconView::camItemInfoRef(const QString& folder, const QString& file)
{
    QUrl url = QUrl::fromLocalFile(folder);
    url      = url.adjusted(QUrl::StripTrailingSlash);
    url.setPath(url.path() + QLatin1Char('/') + (file));
    QModelIndex indexForCamItemInfo = importFilterModel()->indexForPath(url.toLocalFile());
    QModelIndex mappedIndex         = importFilterModel()->mapToSource(indexForCamItemInfo);

    return importImageModel()->camItemInfoRef(mappedIndex);
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
    QList<QUrl>  urls = selectedUrls();
    NewNamesList newNamesList;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == QDialog::Accepted)
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
/*
    if (d->utilities->deleteImages(camItemInfoList, permanently))
    {
       awayFromSelection();
    }
*/
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
/*
    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();
    CamItemInfo groupLeader          = selectedInfos.takeFirst();
    FileActionMngr::instance()->addToGroup(groupLeader, selectedInfos);
*/
}

void ImportIconView::createGroupByTimeFromSelection()
{
    //TODO: Impelemnt grouping in import tool.
/*
    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();

    while (selectedInfos.size() > 0)
    {
        QList<CamItemInfo> group;
        CamItemInfo groupLeader = selectedInfos.takeFirst();
        QDateTime dateTime    = groupLeader.dateTime();

        while (selectedInfos.size() > 0 && abs(dateTime.secsTo(selectedInfos.first().dateTime())) < 2)
        {
           group.push_back(selectedInfos.takeFirst());
        }

        FileActionMngr::instance()->addToGroup(groupLeader, group);
    }
*/
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

void ImportIconView::slotRotateLeft(const QList<QModelIndex>& /*indexes*/)
{
/*
    QList<ImageInfo> imageInfos;

    foreach(const QModelIndex& index, indexes)
    {
        ImageInfo imageInfo(importFilterModel()->camItemInfo(index).url());
        imageInfos << imageInfo;
    }

    FileActionMngr::instance()->transform(imageInfos, MetaEngineRotation::Rotate270);
*/
}

void ImportIconView::slotRotateRight(const QList<QModelIndex>& /*indexes*/)
{
/*
    QList<ImageInfo> imageInfos;

    foreach(const QModelIndex& index, indexes)
    {
        ImageInfo imageInfo(importFilterModel()->camItemInfo(index).url());
        imageInfos << imageInfo;
    }

    FileActionMngr::instance()->transform(imageInfos, MetaEngineRotation::Rotate90);
*/
}

void ImportIconView::activated(const CamItemInfo& info, Qt::KeyboardModifiers)
{
    if (info.isNull())
    {
        return;
    }

    if (ImportSettings::instance()->getItemLeftClickAction() == ImportSettings::ShowPreview)
    {
        emit previewRequested(info, false);
    }
    else
    {
        //TODO: openFile(info);
    }
}

void ImportIconView::showContextMenuOnInfo(QContextMenuEvent* event, const CamItemInfo& /*info*/)
{
    QList<CamItemInfo> selectedInfos = selectedCamItemInfosCurrentFirst();
    QList<qlonglong>   selectedItemIDs;

    foreach(const CamItemInfo& info, selectedInfos)
    {
        selectedItemIDs << info.id;
    }

    // --------------------------------------------------------

    QMenu popmenu(this);
    ImportContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(QLatin1String("importui_fullscreen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addAction(QLatin1String("import_zoomfit2window"));
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(QLatin1String("importui_imagedownload"));
    cmhelper.addAction(QLatin1String("importui_imagemarkasdownloaded"));
    cmhelper.addAction(QLatin1String("importui_imagelock"));
    cmhelper.addAction(QLatin1String("importui_delete"));
    cmhelper.addSeparator();
    cmhelper.addAction(QLatin1String("importui_item_view"));
    cmhelper.addServicesMenu(selectedUrls());
    //TODO: cmhelper.addRotateMenu(selectedItemIDs);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction(QLatin1String("importui_selectall"));
    cmhelper.addAction(QLatin1String("importui_selectnone"));
    cmhelper.addAction(QLatin1String("importui_selectinvert"));
    cmhelper.addSeparator();
    // --------------------------------------------------------
    //cmhelper.addAssignTagsMenu(selectedItemIDs);
    //cmhelper.addRemoveTagsMenu(selectedItemIDs);
    //cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addLabelsAction();
    //if (!d->faceMode)
    //{
    //    cmhelper.addGroupMenu(selectedItemIDs);
    //}

    // special action handling --------------------------------

    //connect(&cmhelper, SIGNAL(signalAssignTag(int)),
    //        this, SLOT(assignTagToSelected(int)));

    //TODO: Implement tag view for import tool.
    //connect(&cmhelper, SIGNAL(signalPopupTagsView()),
    //        this, SIGNAL(signalPopupTagsView()));

    //connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
    //        this, SLOT(removeTagFromSelected(int)));

    //connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            //this, SIGNAL(gotoTagAndImageRequested(int)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(assignPickLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(assignColorLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(assignRatingToSelected(int)));

    //connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            //this, SLOT(insertSelectedToExistingQueue(int)));

    //FIXME: connect(&cmhelper, SIGNAL(signalCreateGroup()),
            //this, SLOT(createGroupFromSelection()));

    //connect(&cmhelper, SIGNAL(signalUngroup()),
            //this, SLOT(ungroupSelected()));

    //connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            //this, SLOT(removeSelectedFromGroup()));

    // --------------------------------------------------------

    cmhelper.exec(event->globalPos());
}

void ImportIconView::showContextMenu(QContextMenuEvent* event)
{
    QMenu popmenu(this);
    ImportContextMenuHelper cmhelper(&popmenu);

    cmhelper.addAction(QLatin1String("importui_fullscreen"));
    cmhelper.addAction(QLatin1String("options_show_menubar"));
    cmhelper.addSeparator();
    cmhelper.addAction(QLatin1String("importui_close"));

    // --------------------------------------------------------

    cmhelper.exec(event->globalPos());
}

void ImportIconView::assignTagToSelected(int tagID)
{
    CamItemInfoList infos = selectedCamItemInfos();

    foreach(const CamItemInfo& info, infos)
    {
        importImageModel()->camItemInfoRef(importImageModel()->indexForCamItemInfo(info)).tagIds.append(tagID);
    }
}

void ImportIconView::removeTagFromSelected(int tagID)
{
    CamItemInfoList infos = selectedCamItemInfos();

    foreach(const CamItemInfo& info, infos)
    {
        importImageModel()->camItemInfoRef(importImageModel()->indexForCamItemInfo(info)).tagIds.removeAll(tagID);
    }
}

void ImportIconView::assignPickLabel(const QModelIndex& index, int pickId)
{
    importImageModel()->camItemInfoRef(index).pickLabel = pickId;
}

void ImportIconView::assignPickLabelToSelected(int pickId)
{
    CamItemInfoList infos = selectedCamItemInfos();

    foreach(const CamItemInfo& info, infos)
    {
        importImageModel()->camItemInfoRef(importImageModel()->indexForCamItemInfo(info)).pickLabel = pickId;
    }
}

void ImportIconView::assignColorLabel(const QModelIndex& index, int colorId)
{
    importImageModel()->camItemInfoRef(index).colorLabel = colorId;
}

void ImportIconView::assignColorLabelToSelected(int colorId)
{
    CamItemInfoList infos = selectedCamItemInfos();

    foreach(const CamItemInfo& info, infos)
    {
        importImageModel()->camItemInfoRef(importImageModel()->indexForCamItemInfo(info)).colorLabel = colorId;
    }
}

void ImportIconView::assignRating(const QList<QModelIndex>& indexes, int rating)
{
    foreach(const QModelIndex& index, indexes)
    {
        if (index.isValid())
        {
            importImageModel()->camItemInfoRef(index).rating = rating;
        }
    }
}

void ImportIconView::assignRatingToSelected(int rating)
{
    CamItemInfoList infos = selectedCamItemInfos();

    foreach(const CamItemInfo& info, infos)
    {
        importImageModel()->camItemInfoRef(importImageModel()->indexForCamItemInfo(info)).rating = rating;
    }
}

} // namespace Digikam

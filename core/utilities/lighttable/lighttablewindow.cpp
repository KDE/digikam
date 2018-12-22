/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablewindow.h"
#include "lighttablewindow_p.h"

namespace Digikam
{

LightTableWindow* LightTableWindow::m_instance = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_instance)
    {
        new LightTableWindow();
    }

    return m_instance;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_instance;
}

LightTableWindow::LightTableWindow()
    : DXmlGuiWindow(0),
      d(new Private)
{
    setConfigGroupName(QLatin1String("LightTable Settings"));
    setXMLFile(QLatin1String("lighttablewindowui5.rc"));

    m_instance = this;

    setWindowFlags(Qt::Window);
    setCaption(i18n("Light Table"));
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);
    setFullScreenOptions(FS_LIGHTTABLE);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();

    // ------------------------------------------------

    setupConnections();
    slotColorManagementOptionsChanged();

    readSettings();

    d->leftSideBar->populateTags();
    d->rightSideBar->populateTags();

    applySettings();
    setAutoSaveSettings(configGroupName(), true);
}

LightTableWindow::~LightTableWindow()
{
    m_instance = 0;

    delete d->thumbView;
    delete d->rightSideBar;
    delete d->leftSideBar;
    delete d;
}

void LightTableWindow::refreshView()
{
    d->leftSideBar->refreshTagsView();
    d->rightSideBar->refreshTagsView();
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (d->clearOnCloseAction->isChecked())
    {
        slotClearItemsList();
    }

    // There is one nasty habit with the thumbnail bar if it is floating: it
    // doesn't close when the parent window does, so it needs to be manually
    // closed. If the light table is opened again, its original state needs to
    // be restored.
    // This only needs to be done when closing a visible window and not when
    // destroying a closed window, since the latter case will always report that
    // the thumbnail bar isn't visible.
    if (isVisible())
    {
        d->barViewDock->hide();
    }

    writeSettings();

    DXmlGuiWindow::closeEvent(e);
}

void LightTableWindow::showEvent(QShowEvent*)
{
    // Restore the visibility of the thumbbar and start autosaving again.
    d->barViewDock->restoreVisibility();
}

// Deal with items dropped onto the thumbbar (e.g. from the Album view)
void LightTableWindow::slotThumbbarDroppedItems(const QList<ItemInfo>& list)
{
    // Setting the third parameter of loadItemInfos to true
    // means that the images are added to the presently available images.
    loadItemInfos(ItemInfoList(list), ItemInfo(), true);
}

// We get here either
// - via CTRL+L (from the albumview)
//     a) digikamapp.cpp:  CTRL+key_L leads to slotImageLightTable())
//     b) digikamview.cpp: void ItemIconView::slotImageLightTable()
//          calls d->iconView->insertToLightTable(list, info);
//     c) albumiconview.cpp: AlbumIconView::insertToLightTable
//          calls ltview->loadItemInfos(list, current);
// - via drag&drop, i.e. calls issued by the ...Dropped... routines
void LightTableWindow::loadItemInfos(const ItemInfoList& list,
                                      const ItemInfo& givenItemInfoCurrent,
                                      bool  addTo)
{
    // Clear all items before adding new images to the light table.
    qCDebug(DIGIKAM_GENERAL_LOG) << "Clearing LT" << (!addTo);

    if (!addTo)
    {
        slotClearItemsList();
    }

    ItemInfoList l            = list;
    ItemInfo imageInfoCurrent = givenItemInfoCurrent;

    if (imageInfoCurrent.isNull() && !l.isEmpty())
    {
        imageInfoCurrent = l.first();
    }

    d->thumbView->setItems(l);

    QModelIndex index = d->thumbView->findItemByInfo(imageInfoCurrent);

    if (index.isValid())
    {
        d->thumbView->setCurrentIndex(index);
    }
    else
    {
        d->thumbView->setCurrentWhenAvailable(imageInfoCurrent.id());
    }
}

bool LightTableWindow::isEmpty() const
{
    return (d->thumbView->countItems() == 0);
}

void LightTableWindow::slotRefreshStatusBar()
{
    d->statusProgressBar->setProgressBarMode(StatusProgressBar::TextMode,
                                             i18np("%1 item on Light Table",
                                                   "%1 items on Light Table",
                                             d->thumbView->countItems()));
}

void LightTableWindow::slotFileChanged(const QString& path)
{
    QUrl url = QUrl::fromLocalFile(path);
    // NOTE: Thumbbar handle change through ItemCategorizedView

    if (!d->previewView->leftItemInfo().isNull())
    {
        if (d->previewView->leftItemInfo().fileUrl() == url)
        {
            d->previewView->leftReload();
            d->leftSideBar->itemChanged(d->previewView->leftItemInfo());
        }
    }

    if (!d->previewView->rightItemInfo().isNull())
    {
        if (d->previewView->rightItemInfo().fileUrl() == url)
        {
            d->previewView->rightReload();
            d->rightSideBar->itemChanged(d->previewView->rightItemInfo());
        }
    }
}

void LightTableWindow::slotLeftPanelLeftButtonClicked()
{
    if (d->navigateByPairAction->isChecked())
    {
        return;
    }

    d->thumbView->setCurrentInfo(d->previewView->leftItemInfo());
}

void LightTableWindow::slotRightPanelLeftButtonClicked()
{
    // With navigate by pair option, only the left panel can be selected.
    if (d->navigateByPairAction->isChecked())
    {
        return;
    }

    d->thumbView->setCurrentInfo(d->previewView->rightItemInfo());
}

void LightTableWindow::slotLeftPreviewLoaded(bool b)
{
    d->leftZoomBar->setEnabled(b);
    d->leftFileName->setAdjustedText();

    if (b)
    {
        d->leftFileName->setAdjustedText(d->previewView->leftItemInfo().name());
        d->previewView->checkForSelection(d->thumbView->currentInfo());
        d->thumbView->setOnLeftPanel(d->previewView->leftItemInfo());

        QModelIndex index = d->thumbView->findItemByInfo(d->previewView->leftItemInfo());

        if (d->navigateByPairAction->isChecked() && index.isValid())
        {
            QModelIndex next = d->thumbView->nextIndex(index);

            if (next.isValid())
            {
                d->thumbView->setOnRightPanel(d->thumbView->findItemByIndex(next));
                slotSetItemOnRightPanel(d->thumbView->findItemByIndex(next));
            }
            else
            {
                QModelIndex first = d->thumbView->firstIndex();
                slotSetItemOnRightPanel(first.isValid() ? d->thumbView->findItemByIndex(first) : ItemInfo());
            }
        }
    }
}

void LightTableWindow::slotRightPreviewLoaded(bool b)
{
    d->rightZoomBar->setEnabled(b);
    d->rightFileName->setAdjustedText();

    if (b)
    {
        d->rightFileName->setAdjustedText(d->previewView->rightItemInfo().name());
        d->previewView->checkForSelection(d->thumbView->currentInfo());
        d->thumbView->setOnRightPanel(d->previewView->rightItemInfo());

        QModelIndex index = d->thumbView->findItemByInfo(d->previewView->rightItemInfo());

        if (index.isValid())
        {
            d->thumbView->setOnRightPanel(d->thumbView->findItemByIndex(index));
        }
    }
}

void LightTableWindow::slotItemSelected(const ItemInfo& info)
{
    bool hasInfo = !info.isNull();

    d->setItemLeftAction->setEnabled(hasInfo);
    d->setItemRightAction->setEnabled(hasInfo);
    d->editItemAction->setEnabled(hasInfo);
    d->removeItemAction->setEnabled(hasInfo);
    d->clearListAction->setEnabled(hasInfo);
    d->fileDeleteAction->setEnabled(hasInfo);
    d->fileDeleteFinalAction->setEnabled(hasInfo);
    d->backwardAction->setEnabled(hasInfo);
    d->forwardAction->setEnabled(hasInfo);
    d->firstAction->setEnabled(hasInfo);
    d->lastAction->setEnabled(hasInfo);
    d->syncPreviewAction->setEnabled(hasInfo);
    d->navigateByPairAction->setEnabled(hasInfo);
    d->slideShowAction->setEnabled(hasInfo);

    if (hasInfo)
    {
        QModelIndex curr = d->thumbView->findItemByInfo(info);

        if (curr.isValid())
        {
            if (!d->thumbView->previousIndex(curr).isValid())
            {
                d->firstAction->setEnabled(false);
            }

            if (!d->thumbView->nextIndex(curr).isValid())
            {
                d->lastAction->setEnabled(false);
            }

            if (d->navigateByPairAction->isChecked())
            {
                d->setItemLeftAction->setEnabled(false);
                d->setItemRightAction->setEnabled(false);

                d->thumbView->setOnLeftPanel(info);
                slotSetItemOnLeftPanel(info);
            }
            else if (d->autoLoadOnRightPanel && !d->thumbView->isOnLeftPanel(info))
            {
                d->thumbView->setOnRightPanel(info);
                slotSetItemOnRightPanel(info);
            }
        }
    }

    d->previewView->checkForSelection(info);
}

// Deal with one (or more) items dropped onto the left panel
void LightTableWindow::slotLeftDroppedItems(const ItemInfoList& list)
{
    ItemInfo info = list.first();
    // add the image to the existing images
    loadItemInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ItemInfo reference
    // in memory for preview object.
    QModelIndex index = d->thumbView->findItemByInfo(info);

    if (index.isValid())
    {
        slotSetItemOnLeftPanel(info);
    }
}

// Deal with one (or more) items dropped onto the right panel
void LightTableWindow::slotRightDroppedItems(const ItemInfoList& list)
{
    ItemInfo info = list.first();
    // add the image to the existing images
    loadItemInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ItemInfo reference
    // in memory for preview object.
    QModelIndex index = d->thumbView->findItemByInfo(info);

    if (index.isValid())
    {
        slotSetItemOnRightPanel(info);
        // Make this item the current one.
        d->thumbView->setCurrentInfo(info);
    }
}

// Set the images for the left and right panel.
void LightTableWindow::setLeftRightItems(const ItemInfoList& list, bool addTo)
{
    ItemInfoList l = list;

    if (l.count() == 0)
    {
        return;
    }

    ItemInfo info    = l.first();
    QModelIndex index = d->thumbView->findItemByInfo(info);

    if (l.count() == 1 && !addTo)
    {
        // Just one item; this is used for the left panel.
        d->thumbView->setOnLeftPanel(info);
        slotSetItemOnLeftPanel(info);
        d->thumbView->setCurrentInfo(info);
        return;
    }

    if (index.isValid())
    {
        // The first item is used for the left panel.
        if (!addTo)
        {
            d->thumbView->setOnLeftPanel(info);
            slotSetItemOnLeftPanel(info);
        }

        // The subsequent item is used for the right panel.
        QModelIndex next = d->thumbView->nextIndex(index);

        if (next.isValid() && !addTo)
        {
            ItemInfo nextInf = d->thumbView->findItemByIndex(next);
            d->thumbView->setOnRightPanel(nextInf);
            slotSetItemOnRightPanel(nextInf);

            if (!d->navigateByPairAction->isChecked())
            {
                d->thumbView->setCurrentInfo(nextInf);
            }
        }

        // If navigate by pairs is active, the left panel item is selected.
        // (Fixes parts of bug #150296)
        if (d->navigateByPairAction->isChecked())
        {
            d->thumbView->setCurrentInfo(info);
        }
    }
}

void LightTableWindow::slotSetItemLeft()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotSetItemOnLeftPanel(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotSetItemRight()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotSetItemOnRightPanel(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotSetItemOnLeftPanel(const ItemInfo& info)
{
    d->previewView->setLeftItemInfo(info);

    if (!info.isNull())
    {
        d->leftSideBar->itemChanged(info);
    }
    else
    {
        d->leftSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotSetItemOnRightPanel(const ItemInfo& info)
{
    d->previewView->setRightItemInfo(info);

    if (!info.isNull())
    {
        d->rightSideBar->itemChanged(info);
    }
    else
    {
        d->rightSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotClearItemsList()
{
    if (!d->previewView->leftItemInfo().isNull())
    {
        d->previewView->setLeftItemInfo();
        d->leftSideBar->slotNoCurrentItem();
    }

    if (!d->previewView->rightItemInfo().isNull())
    {
        d->previewView->setRightItemInfo();
        d->rightSideBar->slotNoCurrentItem();
    }

    d->thumbView->clear();
}

void LightTableWindow::slotDeleteItem()
{
    deleteItem(false);
}

void LightTableWindow::slotDeleteItem(const ItemInfo& info)
{
    deleteItem(info, false);
}

void LightTableWindow::slotDeleteFinalItem()
{
    deleteItem(true);
}

void LightTableWindow::slotDeleteFinalItem(const ItemInfo& info)
{
    deleteItem(info, true);
}

void LightTableWindow::deleteItem(bool permanently)
{
    if (!d->thumbView->currentInfo().isNull())
    {
        deleteItem(d->thumbView->currentInfo(), permanently);
    }
}

void LightTableWindow::deleteItem(const ItemInfo& info, bool permanently)
{
    QUrl u               = info.fileUrl();
    PAlbum* const palbum = AlbumManager::instance()->findPAlbum(u.adjusted(QUrl::RemoveFilename));

    if (!palbum)
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "Item to delete: " << u;

    bool useTrash;
    bool preselectDeletePermanently = permanently;

    DeleteDialog dialog(this);

    QList<QUrl> urlList;
    urlList.append(u);

    if (!dialog.confirmDeleteList(urlList, DeleteDialogMode::Files, preselectDeletePermanently ?
                                  DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
    {
        return;
    }

    useTrash = !dialog.shouldDelete();

    DIO::del(info, useTrash);
}

void LightTableWindow::slotRemoveItem()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        slotRemoveItem(d->thumbView->currentInfo());
    }
}

void LightTableWindow::slotRemoveItem(const ItemInfo& info)
{
/*
    if (!d->previewView->leftItemInfo().isNull())
    {
        if (d->previewView->leftItemInfo() == info)
        {
            d->previewView->setLeftItemInfo();
            d->leftSideBar->slotNoCurrentItem();
        }
    }

    if (!d->previewView->rightItemInfo().isNull())
    {
        if (d->previewView->rightItemInfo() == info)
        {
            d->previewView->setRightItemInfo();
            d->rightSideBar->slotNoCurrentItem();
        }
    }

    d->thumbView->removeItemByInfo(info);
    d->thumbView->setSelected(d->thumbView->currentItem());
*/

    // When either the image from the left or right panel is removed,
    // there are various situations to account for.
    // To describe them, 4 images A B C D are used
    // and the subscript _L and _ R  mark the currently
    // active item on the left and right panel

    ItemInfo new_linfo;
    ItemInfo new_rinfo;
    bool leftPanelActive = false;
    ItemInfo curr_linfo = d->previewView->leftItemInfo();
    ItemInfo curr_rinfo = d->previewView->rightItemInfo();
    qint64 infoId        = info.id();

    // First determine the next images to the current left and right image:
    ItemInfo next_linfo;
    ItemInfo next_rinfo;

    if (!curr_linfo.isNull())
    {
        QModelIndex index = d->thumbView->findItemByInfo(curr_linfo);

        if (index.isValid())
        {
            QModelIndex next = d->thumbView->nextIndex(index);

            if (next.isValid())
            {
                next_linfo = d->thumbView->findItemByIndex(next);
            }
        }
    }

    if (!curr_rinfo.isNull())
    {
        QModelIndex index = d->thumbView->findItemByInfo(curr_rinfo);

        if (index.isValid())
        {
            QModelIndex next = d->thumbView->nextIndex(index);

            if (next.isValid())
            {
                next_rinfo = d->thumbView->findItemByIndex(next);
            }
        }
    }

    d->thumbView->removeItemByInfo(info);

    // Make sure that next_linfo and next_rinfo are still available:
    if (!d->thumbView->findItemByInfo(next_linfo).isValid())
    {
        next_linfo = ItemInfo();
    }

    if (!d->thumbView->findItemByInfo(next_rinfo).isValid())
    {
        next_rinfo = ItemInfo();
    }

    // removal of the left panel item?
    if (!curr_linfo.isNull())
    {
        if (curr_linfo.id() == infoId)
        {
            leftPanelActive = true;
            // Delete the item A_L of the left panel:
            // 1)  A_L  B_R  C    D   ->   B_L  C_R  D
            // 2)  A_L  B    C_R  D   ->   B    C_L  D_R
            // 3)  A_L  B    C    D_R ->   B_R  C    D_L
            // 4)  A_L  B_R           ->   A_L
            // some more corner cases:
            // 5)  A    B_L  C_R  D   ->   A    C_L  D_R
            // 6)  A    B_L  C_R      ->   A_R  C_L
            // 7)  A_LR B    C    D   ->   B_L    C_R  D  (does not yet work)
            // I.e. in 3) we wrap around circularly.

            // When removing the left panel image,
            // put the right panel image into the left panel.
            // Check if this one is not the same (i.e. also removed).
            if (!curr_rinfo.isNull())
            {
                if (curr_rinfo.id() != infoId)
                {
                    new_linfo = curr_rinfo;
                    // Set the right panel to the next image:
                    new_rinfo = next_rinfo;

                    // set the right panel active, but not in pair mode
                    if (!d->navigateByPairAction->isChecked())
                    {
                        leftPanelActive = false;
                    }
                }
            }
        }
    }

    // removal of the right panel item?
    if (!curr_rinfo.isNull())
    {
        if (curr_rinfo.id() == infoId)
        {
            // Leave the left panel as the current one
            new_linfo = curr_linfo;
            // Set the right panel to the next image
            new_rinfo = next_rinfo;
        }
    }

    // Now we deal with the corner cases, where no left or right item exists.
    // If the right panel would be set, but not the left-one, then swap
    if (new_linfo.isNull() && !new_rinfo.isNull())
    {
        new_linfo       = new_rinfo;
        new_rinfo       = ItemInfo();
        leftPanelActive = true;
    }

    if (new_linfo.isNull())
    {
        if (d->thumbView->countItems() > 0)
        {
            QModelIndex first = d->thumbView->firstIndex();
            new_linfo = d->thumbView->findItemByIndex(first);
        }
    }

    // Make sure that new_linfo and new_rinfo exist.
    // This addresses a crash occurring if the last image is removed
    // in the navigate by pairs mode.
    if (!d->thumbView->findItemByInfo(new_linfo).isValid())
    {
        new_linfo = ItemInfo();
    }

    if (!d->thumbView->findItemByInfo(new_rinfo).isValid())
    {
        new_rinfo = ItemInfo();
    }

    // no right item defined?
    if (new_rinfo.isNull())
    {
        // If there are at least two items, we can find reasonable right image.
        if (d->thumbView->countItems() > 1)
        {
            // See if there is an item next to the left one:
            QModelIndex index = d->thumbView->findItemByInfo(new_linfo);
            QModelIndex next;

            if (index.isValid())
            {
                next = d->thumbView->nextIndex(index);
            }

            if (next.isValid())
            {
                new_rinfo = d->thumbView->findItemByIndex(next);
            }
            else
            {
                // If there is no item to the right of new_linfo
                // then we can choose the first item for new_rinfo
                // (as we made sure that there are at least two items)
                QModelIndex first = d->thumbView->firstIndex();
                new_rinfo         = d->thumbView->findItemByIndex(first);
            }
        }
    }

    // Check if left and right are set to the same
    if (!new_linfo.isNull() && !new_rinfo.isNull())
    {
        if (new_linfo.id() == new_rinfo.id())
        {
            // Only keep the left one
            new_rinfo = ItemInfo();
        }
    }

    // If the right panel would be set, but not the left-one, then swap
    // (note that this has to be done here again!)
    if (new_linfo.isNull() && !new_rinfo.isNull())
    {
        new_linfo       = new_rinfo;
        new_rinfo       = ItemInfo();
        leftPanelActive = true;
    }

    // set the image for the left panel
    if (!new_linfo.isNull())
    {
        d->thumbView->setOnLeftPanel(new_linfo);
        slotSetItemOnLeftPanel(new_linfo);

        //  make this the selected item if the left was active before
        if (leftPanelActive)
        {
            d->thumbView->setCurrentInfo(new_linfo);
        }
    }
    else
    {
        d->previewView->setLeftItemInfo();
        d->leftSideBar->slotNoCurrentItem();
    }

    // set the image for the right panel
    if (!new_rinfo.isNull())
    {
        d->thumbView->setOnRightPanel(new_rinfo);
        slotSetItemOnRightPanel(new_rinfo);

        //  make this the selected item if the left was active before
        if (!leftPanelActive)
        {
            d->thumbView->setCurrentInfo(new_rinfo);
        }
    }
    else
    {
        d->previewView->setRightItemInfo();
        d->rightSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotLeftZoomFactorChanged(double zoom)
{
    double zmin = d->previewView->leftZoomMin();
    double zmax = d->previewView->leftZoomMax();
    d->leftZoomBar->setZoom(zoom, zmin, zmax);

    d->leftZoomPlusAction->setEnabled(!d->previewView->leftMaxZoom());
    d->leftZoomMinusAction->setEnabled(!d->previewView->leftMinZoom());
}

void LightTableWindow::slotRightZoomFactorChanged(double zoom)
{
    double zmin = d->previewView->rightZoomMin();
    double zmax = d->previewView->rightZoomMax();
    d->rightZoomBar->setZoom(zoom, zmin, zmax);

    d->rightZoomPlusAction->setEnabled(!d->previewView->rightMaxZoom());
    d->rightZoomMinusAction->setEnabled(!d->previewView->rightMinZoom());
}

void LightTableWindow::slotToggleSyncPreview()
{
    d->previewView->setSyncPreview(d->syncPreviewAction->isChecked());
}

void LightTableWindow::slotToggleOnSyncPreview(bool t)
{
    d->syncPreviewAction->setEnabled(t);

    if (!t)
    {
        d->syncPreviewAction->setChecked(false);
    }
    else
    {
        if (d->autoSyncPreview)
        {
            d->syncPreviewAction->setChecked(true);
        }
    }
}

void LightTableWindow::slotBackward()
{
    d->thumbView->toPreviousIndex();
}

void LightTableWindow::slotForward()
{
    d->thumbView->toNextIndex();
}

void LightTableWindow::slotFirst()
{
    d->thumbView->toFirstIndex();
}

void LightTableWindow::slotLast()
{
    d->thumbView->toLastIndex();
}

void LightTableWindow::slotToggleNavigateByPair()
{
    d->thumbView->setNavigateByPair(d->navigateByPairAction->isChecked());
    d->previewView->setNavigateByPair(d->navigateByPairAction->isChecked());
    slotItemSelected(d->thumbView->currentInfo());
}

void LightTableWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void LightTableWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void LightTableWindow::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

void LightTableWindow::toggleTag(int tagID)
{
    d->thumbView->toggleTag(tagID);
}

void LightTableWindow::slotAssignPickLabel(int pickId)
{
    d->thumbView->slotAssignPickLabel(pickId);
}

void LightTableWindow::slotAssignColorLabel(int colorId)
{
    d->thumbView->slotAssignColorLabel(colorId);
}

void LightTableWindow::slotAssignRating(int rating)
{
    d->thumbView->slotAssignRating(rating);
}

void LightTableWindow::showSideBars(bool visible)
{
    if (visible)
    {
        d->leftSideBar->restore();
        d->rightSideBar->restore();
    }
    else
    {
        d->leftSideBar->backup();
        d->rightSideBar->backup();
    }
}

void LightTableWindow::slotToggleLeftSideBar()
{
    d->leftSideBar->isExpanded() ? d->leftSideBar->shrink()
                                 : d->leftSideBar->expand();
}

void LightTableWindow::slotToggleRightSideBar()
{
    d->rightSideBar->isExpanded() ? d->rightSideBar->shrink()
                                  : d->rightSideBar->expand();
}

void LightTableWindow::slotPreviousLeftSideBarTab()
{
    d->leftSideBar->activePreviousTab();
}

void LightTableWindow::slotNextLeftSideBarTab()
{
    d->leftSideBar->activeNextTab();
}

void LightTableWindow::slotPreviousRightSideBarTab()
{
    d->rightSideBar->activePreviousTab();
}

void LightTableWindow::slotNextRightSideBarTab()
{
    d->rightSideBar->activeNextTab();
}

void LightTableWindow::customizedFullScreenMode(bool set)
{
    showStatusBarAction()->setEnabled(!set);
    toolBarMenuAction()->setEnabled(!set);
    showMenuBarAction()->setEnabled(!set);
    d->showBarAction->setEnabled(!set);

    d->previewView->toggleFullScreen(set);
}

void LightTableWindow::slotFileWithDefaultApplication()
{
    if (!d->thumbView->currentInfo().isNull())
    {
        DFileOperations::openFilesWithDefaultApplication(QList<QUrl>() << d->thumbView->currentInfo().fileUrl());
    }
}

void LightTableWindow::slotRightSideBarActivateTitles()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void LightTableWindow::slotRightSideBarActivateComments()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void LightTableWindow::slotRightSideBarActivateAssignedTags()
{
    d->rightSideBar->setActiveTab(d->rightSideBar->imageDescEditTab());
    d->rightSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void LightTableWindow::slotLeftSideBarActivateTitles()
{
    d->leftSideBar->setActiveTab(d->leftSideBar->imageDescEditTab());
    d->leftSideBar->imageDescEditTab()->setFocusToTitlesEdit();
}

void LightTableWindow::slotLeftSideBarActivateComments()
{
    d->leftSideBar->setActiveTab(d->leftSideBar->imageDescEditTab());
    d->leftSideBar->imageDescEditTab()->setFocusToCommentsEdit();
}

void LightTableWindow::slotLeftSideBarActivateAssignedTags()
{
    d->leftSideBar->setActiveTab(d->leftSideBar->imageDescEditTab());
    d->leftSideBar->imageDescEditTab()->activateAssignedTagsButton();
}

void LightTableWindow::slotToggleColorManagedView()
{
    if (!IccSettings::instance()->isEnabled())
    {
        return;
    }

    bool cmv = !IccSettings::instance()->settings().useManagedPreviews;
    IccSettings::instance()->setUseManagedPreviews(cmv);
}

} // namespace Digikam

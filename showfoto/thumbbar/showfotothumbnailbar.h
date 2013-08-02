/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 02-08-2013
 * Description : Thumbnail bar for Showfoto
 *
 * Copyright (C) 2013 by Mohamed Anwer <mohammed dot ahmed dot anwer at gmail dot com>
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

#ifndef SHOWFOTOTHUMBNAILBAR_H
#define SHOWFOTOTHUMBNAILBAR_H

//Local Includes
#include "dcategorizedview.h"
#include "showfotoimagemodel.h"
#include "showfotothumbnailmodel.h"
#include "showfotofiltermodel.h"
#include "showfotoiteminfo.h"
#include "showfotodelegate.h"
#include "imagedelegateoverlay.h"

using namespace Digikam;

namespace ShowFoto {

class ShowfotoThumbnailBar : public DCategorizedView
{
    Q_OBJECT

public:

    explicit ShowfotoThumbnailBar(QWidget* const parent = 0);
    ~ShowfotoThumbnailBar();

    void setModels(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel);

    /**
     * This installs a duplicate filter model, if the ShowfotoImageModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ShowfotoImageModel* model, ShowfotoSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex() const;
    QModelIndex lastIndex()  const;

    /// Sets the policy always for the one scroll bar which is relevant, depending on orientation
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    void installRatingOverlay();

    ShowfotoImageModel* showfotoImageModel() const;
    ShowfotoSortFilterModel* showfotoSortFilterModel() const;
    ShowfotoFilterModel* showfotoFilterModel() const;
    ShowfotoThumbnailModel* showfotoThumbnailModel() const;
    QSortFilterProxyModel* filterModel() const;
    ShowfotoDelegate* delegate() const;

    void setItemDelegate(ShowfotoDelegate* delegate);
    ThumbnailSize thumbnailSize() const;
    void scrollToStoredItem();

public Q_SLOTS:

    //TODO: make sure that you won't use ratings
    //void assignRating(const QList<QModelIndex>& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected Q_SLOTS:

    void slotShowfotoItemInfosAdded();

Q_SIGNALS:

    /// Emitted when a new model is set
    void modelChanged();

protected:

    virtual void slotSetupChanged();
    virtual bool event(QEvent*);

private:

    class Private;
    Private* const d;
    
};
}
#endif // SHOWFOTOTHUMBNAILBAR_H

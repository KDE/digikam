/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 02-08-2013
 * Description : Thumbnail bar for Showfoto
 *
 * Copyright (C) 2013      by Mohamed Anwer <m dot anwer at gmx dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Local Includes

#include "showfotocategorizedview.h"

namespace ShowFoto
{

class ShowfotoItemViewToolTip;

class ShowfotoThumbnailBar : public ShowfotoCategorizedView
{
    Q_OBJECT

public:

    explicit ShowfotoThumbnailBar(QWidget* const parent = 0);
    ~ShowfotoThumbnailBar();

    /**
     * This installs a duplicate filter model, if the ShwofotoImageModel may contain duplicates.
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

    ShowfotoItemInfo findItemByUrl(const QUrl url);

    void installOverlays();

public Q_SLOTS:

    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    virtual void slotSetupChanged();
    virtual bool event(QEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace ShowFoto

#endif // SHOWFOTOTHUMBNAILBAR_H

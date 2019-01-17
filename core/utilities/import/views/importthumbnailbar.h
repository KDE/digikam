/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2012-20-07
 * Description : Thumbnail bar for import tool
 *
 * Copyright (C) 2012      by Islam Wazery <wazery at ubuntu dot com>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_IMPORT_THUMBNAIL_BAR_H
#define DIGIKAM_IMPORT_THUMBNAIL_BAR_H

// Local includes

#include "importcategorizedview.h"

namespace Digikam
{

class ImportThumbnailBar : public ImportCategorizedView
{
    Q_OBJECT

public:

    explicit ImportThumbnailBar(QWidget* const parent = 0);
    ~ImportThumbnailBar();

    /**
     * This installs a duplicate filter model, if the ImportItemModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ImportItemModel* model, ImportSortFilterModel* filterModel);

    QModelIndex nextIndex(const QModelIndex& index)     const;
    QModelIndex previousIndex(const QModelIndex& index) const;
    QModelIndex firstIndex() const;
    QModelIndex lastIndex()  const;

    /// Sets the policy always for the one scroll bar which is relevant, depending on orientation
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    void installOverlays();

public Q_SLOTS:

    void assignRating(const QList<QModelIndex>& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    virtual void slotSetupChanged();
    virtual bool event(QEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_IMPORT_THUMBNAIL_BAR_H

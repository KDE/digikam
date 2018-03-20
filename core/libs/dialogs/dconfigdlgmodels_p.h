/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-03
 * Description : A dialog base class which can handle multiple pages.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2007      by Matthias Kretz <kretz at kde dot org>
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

#ifndef DBCONFIGDLG_MODELS_P_H
#define DBCONFIGDLG_MODELS_P_H

#include "dconfigdlgmodels.h"

namespace Digikam
{

class DConfigDlgModelPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgModel)

public:

    virtual ~DConfigDlgModelPrivate();

protected:

    DConfigDlgModel* q_ptr;
};

// ----------------------------------------------------------------------------

class PageItem
{
public:

    explicit PageItem(DConfigDlgWdgItem* pageItem, PageItem* parent = 0);
    ~PageItem();

    void appendChild(PageItem* child);
    void insertChild(int row, PageItem* child);
    void removeChild(int row);

    PageItem* child(int row);
    int childCount()  const;
    int columnCount() const;
    int row()         const;
    PageItem* parent();

    DConfigDlgWdgItem* pageWidgetItem() const;

    PageItem* findChild(const DConfigDlgWdgItem* item);

    void dump(int indent = 0);

private:

    DConfigDlgWdgItem* mPageWidgetItem;
    QList<PageItem*>   mChildItems;
    PageItem*          mParentItem;
};

// ----------------------------------------------------------------------------

class DConfigDlgWdgModelPrivate : public DConfigDlgModelPrivate
{
    Q_DECLARE_PUBLIC(DConfigDlgWdgModel)

protected:

    DConfigDlgWdgModelPrivate()
        : rootItem(new PageItem(0, 0))
    {
    }

    ~DConfigDlgWdgModelPrivate()
    {
        delete rootItem;
        rootItem = 0;
    }

    void _k_itemChanged()
    {
        Q_Q(DConfigDlgWdgModel);
        DConfigDlgWdgItem* const item = qobject_cast<DConfigDlgWdgItem*>(q->sender());

        if (!item)
        {
            return;
        }

        const QModelIndex index = q->index(item);

        if (!index.isValid())
        {
            return;
        }

        emit q->dataChanged(index, index);
    }

    void _k_itemToggled(bool checked)
    {
        Q_Q(DConfigDlgWdgModel);
        DConfigDlgWdgItem* const item = qobject_cast<DConfigDlgWdgItem*>(q->sender());

        if (!item)
        {
            return;
        }

        emit q->toggled(item, checked);
    }

protected:

    PageItem* rootItem = 0;
};

}  // namespace Digikam

#endif // DBCONFIGDLG_MODELS_P_H

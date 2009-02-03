/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-31
 * Description : implementation of item folder.
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FOLDERITEM_H
#define FOLDERITEM_H

// Qt includes.

#include <Q3ListView>
#include <QStyleOptionQ3ListView>

// Local includes.

#include "digikam_export.h"

namespace Digikam
{

class FolderView;

class FolderItem : public Q3ListViewItem
{
public:

    FolderItem(Q3ListView* parent, const QString& text, bool special=false);
    FolderItem(Q3ListViewItem* parent, const QString& text, bool special=false);

    virtual ~FolderItem();

    virtual int id() const;

    void  setHighlighted(bool val);
    bool  isHighlighted() const;

    virtual void takeItem(Q3ListViewItem *item);

protected:

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);
    void setup();

private:

    bool m_highlighted;
    bool m_special;
};

// ------------------------------------------------------------------------------------

class FolderCheckListItem : public Q3CheckListItem
{
public:

    FolderCheckListItem(Q3ListView* parent, const QString& text,
                        Q3CheckListItem::Type tt);
    FolderCheckListItem(Q3ListViewItem* parent, const QString& text,
                        Q3CheckListItem::Type tt);
    virtual ~FolderCheckListItem();

    void  setHighlighted(bool val);
    bool  isHighlighted() const;

    virtual void takeItem(Q3ListViewItem *item);

protected:

    void paintCell(QPainter* p, const QColorGroup & cg, int column, int width, int align);
    void setup();
    QStyleOptionQ3ListView getStyleOption(const FolderView *fv);

private:

    bool m_highlighted;
};

}  // namespace Digikam

#endif /* FOLDERITEM_H */

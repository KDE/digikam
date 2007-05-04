/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-25
 * Description : implementation to render album icons group item.
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef ALBUMICONGROUPITEM_H
#define ALBUMICONGROUPITEM_H

// Local includes.

#include "icongroupitem.h"

namespace Digikam
{

class AlbumIconView;

class AlbumIconGroupItem : public IconGroupItem
{
public:

    AlbumIconGroupItem(AlbumIconView* view, int albumID);
    ~AlbumIconGroupItem();

    int albumID() const { return m_albumID; }
    
    virtual int compare(IconGroupItem* group);

protected:

    void paintBanner();

private:

    int            m_albumID;
    AlbumIconView* m_view;
};

}  // namespace Digikam

#endif /* ALBUMICONGROUPITEM_H */

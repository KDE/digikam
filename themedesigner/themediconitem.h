/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon item 
 * 
 * Copyright (C) 2005 by Renchi Raju <renchi at pooh.tam.uiuc.edu>
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

#ifndef THEMEDICONITEM_H
#define THEMEDICONITEM_H

// Local includes.

#include "iconitem.h"

namespace Digikam
{

class ThemedIconItem : public IconItem
{
public:

    ThemedIconItem(IconGroupItem* parent);
    ~ThemedIconItem();

protected:

    void paintItem();
};

}  // NameSpace Digikam

#endif /* THEMEDICONITEM_H */

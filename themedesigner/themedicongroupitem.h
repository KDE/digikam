/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon group item 
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

#ifndef THEMEDICONGROUPITEM_H
#define THEMEDICONGROUPITEM_H

// Local includes.

#include "icongroupitem.h"

namespace Digikam
{

class ThemedIconView;

class ThemedIconGroupItem : public IconGroupItem
{
public:
    
    ThemedIconGroupItem(ThemedIconView* view);
    ~ThemedIconGroupItem();

protected:

    void paintBanner();

private:

    ThemedIconView* m_view;
};

}  // NameSpace Digikam

#endif /* THEMEDICONGROUPITEM_H */

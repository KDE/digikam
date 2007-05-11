/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-07-15
 * Description : themed icon view 
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

#ifndef THEMEDICONVIEW_H
#define THEMEDICONVIEW_H

// Qt includes.

#include <qpixmap.h>

// Local includes.

#include "iconview.h"

namespace Digikam
{

class ThemedIconViewPriv;

class ThemedIconView : public IconView
{
    Q_OBJECT
    
public:

    ThemedIconView(QWidget* parent);
    ~ThemedIconView();
    
    QRect    itemRect() const;
    QRect    itemDateRect() const;
    QRect    itemPixmapRect() const;
    QRect    itemNameRect() const;
    QRect    itemCommentsRect() const;
    QRect    itemResolutionRect() const;
    QRect    itemSizeRect() const;
    QRect    itemTagRect() const;
    QRect    bannerRect() const;

    QPixmap* itemBaseRegPixmap() const;
    QPixmap* itemBaseSelPixmap() const;
    QPixmap  bannerPixmap() const;

    QFont    itemFontReg() const;
    QFont    itemFontCom() const;
    QFont    itemFontXtra() const;

protected:

    void resizeEvent(QResizeEvent* e);
    
private:

    void updateBannerRectPixmap();
    void updateItemRectsPixmap();

    ThemedIconViewPriv* d;
    
private slots:

    void slotThemeChanged();
};

}  // NameSpace Digikam

#endif /* THEMEDICONVIEW_H */

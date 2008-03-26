/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-22
 * Description : a widget to display a panel to choose
 *               a rectangular image area.
 * 
 * Copyright (C) 2004-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPANICONWIDGET_H
#define IMAGEPANICONWIDGET_H

// Qt includes.

#include <qpointarray.h>

// Local includes.

#include "paniconwidget.h"

namespace Digikam
{

class ImagePanIconWidgetPriv;

class ImagePanIconWidget : public PanIconWidget
{
Q_OBJECT

public:

    ImagePanIconWidget(int width, int height, QWidget *parent=0, WFlags flags=Qt::WDestructiveClose);
    ~ImagePanIconWidget();

    void  setHighLightPoints(const QPointArray& pointsList);
       
public slots:

    void slotSeparateViewToggled(int t);
            
private:
    
    void updatePixmap();

private:

    ImagePanIconWidgetPriv *d;        
};

}  // NameSpace Digikam

#endif /* IMAGEPANICONWIDGET_H */

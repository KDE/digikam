/* ============================================================
 * File  : cameraiconview.h
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-07-13
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

#ifndef CAMERAICONVIEW_H
#define CAMERAICONVIEW_H

#include <kfileitem.h>
#include <kio/global.h>

#include "thumbview.h"

namespace KIO
{
class Job;
class Slave;
}

class CameraIconItem;
class CameraIconViewPriv;

class CameraIconView : public ThumbView
{
    Q_OBJECT
    
public:

    CameraIconView(QWidget* parent);
    ~CameraIconView();

    CameraIconItem* firstSelectedItem();
    
private:

    CameraIconViewPriv *d;

signals:

    void signalFileView(CameraIconItem* item);
    
public slots:

    void slotNewItems(const KFileItemList& itemList);
    void slotDeleteItem(KFileItem *item);
    void slotClear();

    void slotGotThumbnail(const KFileItem* item, const QPixmap& pix);
    void slotFailedThumbnail(const KFileItem* item);

private slots:

    void slotDoubleClicked(ThumbItem *item);
};


#endif /* CAMERAICONVIEW_H */

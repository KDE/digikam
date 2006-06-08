/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date   : 2004-09-18
 * Description : 
 * 
 * Copyright 2004-2005 by Renchi Raju
 * Copyright 2006 by Gilles Caulier
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

#ifndef CAMERAICONVIEW_H
#define CAMERAICONVIEW_H

// Qt includes.

#include <qdict.h>
#include <qrect.h>

// Local includes.

#include "iconview.h"
#include "renamecustomizer.h"

namespace Digikam
{

class GPItemInfo;
class RenameCustomizer;
class CameraUI;
class CameraIconViewItem;

class CameraIconView : public IconView
{
    Q_OBJECT
    
public:

    CameraIconView(CameraUI* ui, QWidget* parent);
    ~CameraIconView();

    void setRenameCustomizer(RenameCustomizer* renamer);
    
    void addItem(const GPItemInfo& itemInfo);
    void removeItem(const QString& folder, const QString& file);
    void setThumbnail(const QString& folder, const QString& filename, const QImage& image);

    CameraIconViewItem* findItem(const QString& folder, const QString& file);

    virtual QRect itemRect() const;
    
signals:

    void signalSelected(CameraIconViewItem*, bool);
    void signalFileView(CameraIconViewItem*);

    void signalDownload();
    void signalDelete();
    
public slots:

    void slotDownloadNameChanged();
    void slotSelectionChanged();
    void slotSelectAll();
    void slotSelectNone();
    void slotSelectInvert();
    void slotSelectNew();

private slots:

    void slotContextMenu(IconItem* item, const QPoint& pos);
    void slotDoubleClicked(IconItem* item);

protected:

    void startDrag();
    void updateItemRectsPixmap();
    
private:

    QString getTemplatedName(const QString& templ, const GPItemInfo* itemInfo, int position);
    QString getCasedName(const RenameCustomizer::Case ccase, const GPItemInfo* itemInfo);

private:

    QDict<CameraIconViewItem> m_itemDict;
    RenameCustomizer*         m_renamer;
    CameraUI*                 m_ui;
    IconGroupItem*            m_groupItem;
    QRect                     m_itemRect;
};

}  // namespace Digikam

#endif /* CAMERAICONVIEW_H */

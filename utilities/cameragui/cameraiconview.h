/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2004-09-18
 * Description : 
 * 
 * Copyright 2004 by Renchi Raju

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

#include <thumbview.h>
#include <qdict.h>

class GPItemInfo;
class RenameCustomizer;
class CameraUI;
class CameraIconViewItem;

class CameraIconView : public ThumbView
{
    Q_OBJECT
    
public:

    CameraIconView(CameraUI* ui, QWidget* parent);
    ~CameraIconView();

    void setRenameCustomizer(RenameCustomizer* renamer);
    
    void addItem(const GPItemInfo& itemInfo);
    void removeItem(const QString& folder, const QString& file);
    void setThumbnail(const QString& folder, const QString& filename,
                      const QPixmap& pixmap);

    CameraIconViewItem* findItem(const QString& folder, const QString& file);
    
private:

    QString getTemplatedName(const QString& templ, 
                             const GPItemInfo* itemInfo,
                             int position);
    
    QDict<CameraIconViewItem> m_itemDict;
    RenameCustomizer*         m_renamer;
    CameraUI*                 m_ui;

signals:

    void signalSelected(bool selected);
    void signalFileView(CameraIconViewItem*);
    void signalFileProperties(CameraIconViewItem*);
    void signalFileExif(CameraIconViewItem*);
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

    void slotContextMenu(ThumbItem* item, const QPoint& pos);
    void slotDoubleClicked(ThumbItem* item);
};    

#endif /* CAMERAICONVIEW_H */

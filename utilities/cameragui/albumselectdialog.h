/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-06-16
 * Description :
 * 
 * Copyright 2005 by Renchi Raju
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

#ifndef ALBUMSELECTDIALOG_H
#define ALBUMSELECTDIALOG_H

// Qt includes.

#include <qmap.h>

// KDE includes.

#include <kdialogbase.h>

class FolderView;
class FolderItem;
class PAlbum;

namespace Digikam
{

class AlbumSelectDialog : public KDialogBase
{
    Q_OBJECT

public:

    AlbumSelectDialog(QWidget* parent, PAlbum* albumToSelect,
                      const QString& header=QString::null,
                      const QString& newAlbumString=QString::null,
                      bool allowRootSelection=false);

    static PAlbum* selectAlbum(QWidget* parent,
                               PAlbum* albumToSelect,
                               const QString& header=QString::null,
                               const QString& newAlbumString=QString::null,
                               bool allowRootSelection=false);


private slots:

    void slotAlbumAdded(Album* a);
    void slotAlbumDeleted(Album* a);
    void slotAlbumsCleared();
    void slotSelectionChanged();
    void slotContextMenu(QListViewItem *item, const QPoint &, int);
    void slotUser1();
    
private:

    bool                       m_allowRootSelection;
    FolderView*                m_folderView;
    QString                    m_newAlbumString;
    QMap<FolderItem*, PAlbum*> m_albumMap;
};

}  // namespace Digikam

#endif /* ALBUMSELECTDIALOG_H */

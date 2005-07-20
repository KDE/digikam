/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
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
 
#ifndef IMAGEDESCEDIT_H
#define IMAGEDESCEDIT_H

// Qt includes.

#include <qguardedptr.h>
#include <qpixmap.h>

// KDE includes.

#include <kdialogbase.h>

class QLabel;
class QListView;
class QPixmap;
class QCheckListItem;
class QCheckBox;
class QPushButton;

class KTextEdit;

class AlbumIconView;
class AlbumIconItem;
class TAlbum;
class Album;
class ThumbnailJob;

class ImageDescEdit : public KDialogBase
{
    Q_OBJECT

public:

    ImageDescEdit(AlbumIconView* view, AlbumIconItem* currItem, 
                  QWidget *parent=0, bool singleMode=false);
    ~ImageDescEdit();

    static bool editDescription(AlbumIconView* view, AlbumIconItem* currItem);

protected:

    bool eventFilter(QObject *o, QEvent *e);

private:

    AlbumIconView *m_view;
    AlbumIconItem *m_currItem;
    QLabel        *m_thumbLabel;
    QLabel        *m_nameLabel;
    KTextEdit     *m_commentsEdit;
    QListView     *m_tagsView;
    QPushButton   *m_recentTagsBtn;
    bool           m_modified;

    void tagNew(TAlbum* parAlbum);
    void tagEdit(TAlbum* album);
    void tagDelete(TAlbum *album);
    
    QGuardedPtr<ThumbnailJob> m_thumbJob;

    void     populateTags();
    QPixmap  tagThumbnail(TAlbum* album) const;
    
private slots:

    void slotItemChanged();
    void slotModified();
    void slotUser1();
    void slotUser2();
    void slotApply();
    void slotOk();
    void slotGotThumbnail(const KURL&, const QPixmap& pix);
    void slotFailedThumbnail(const KURL&);
    void slotRightButtonClicked(QListViewItem *, const QPoint &, int);
    void slotRecentTags();

    void slotAlbumAdded(Album* a);
    void slotAlbumDeleted(Album* a);
    void slotAlbumIconChanged(Album* a);
    void slotAlbumRenamed(Album* a);

    void slotItemDeleted(AlbumIconItem* iconItem);
    void slotCleared();
};
 
#endif  // IMAGEDESCEDIT_H

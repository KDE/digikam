/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003-2005 by Renchi Raju & Gilles Caulier
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
 
#ifndef IMAGEDESCEDITTAB_H
#define IMAGEDESCEDITTAB_H

// Qt includes.

#include <qwidget.h>
#include <qpixmap.h>
#include <qlistview.h>

// Local includes.

#include "digikam_export.h"

class QListViewItem;
class QCheckListItem;
class QCheckBox;
class QPushButton;
class QToolButton;

class KTextEdit;
class KLineEdit;
class KDateTimeEdit;

namespace Digikam
{
class AlbumIconView;
class AlbumIconItem;
class TAlbum;
class Album;
class ThumbnailJob;
class RatingWidget;
class NavigateBarWidget;

class TAlbumListView : public QListView
{
    Q_OBJECT
    
public:

    TAlbumListView(QWidget* parent);

    void emitSignalItemStateChanged();
    
signals:

    void signalItemStateChanged();
};

// ------------------------------------------------------------------------

class DIGIKAM_EXPORT ImageDescEditTab : public QWidget
{
    Q_OBJECT

public:

    ImageDescEditTab(QWidget *parent, bool navBar=true);
    ~ImageDescEditTab();

    void setItem(AlbumIconView *view=0, AlbumIconItem* currItem=0, int itemType=0);
    void populateTags();
        
signals:
    
    void signalFirstItem(void);    
    void signalPrevItem(void);    
    void signalNextItem(void);    
    void signalLastItem(void); 
        
protected:

    bool eventFilter(QObject *o, QEvent *e);

private:

    AlbumIconView   *m_view;
    AlbumIconItem   *m_currItem;
    QLabel          *m_thumbLabel;
    QLabel          *m_nameLabel;
    KTextEdit       *m_commentsEdit;
    TAlbumListView  *m_tagsView;
    QPushButton     *m_recentTagsBtn;
    KLineEdit       *m_tagsSearchEdit;
    QToolButton     *m_tagsSearchClearBtn;
    KDateTimeEdit   *m_dateTimeEdit;
    RatingWidget    *m_ratingWidget;
    bool             m_modified;
    
    NavigateBarWidget *m_navigateBar;
    
private:    

    void applyChanges();

    void tagNew(TAlbum* parAlbum);
    void tagEdit(TAlbum* album);
    void tagDelete(TAlbum *album);

    QPixmap tagThumbnail(TAlbum* album) const;
    
private slots:

    void slotModified();
    void slotRightButtonClicked(QListViewItem *, const QPoint &, int);
    void slotRecentTags();
    void slotTagsSearchChanged();

    void slotAlbumAdded(Album* a);
    void slotAlbumDeleted(Album* a);
    void slotAlbumIconChanged(Album* a);
    void slotAlbumRenamed(Album* a);

    void slotItemDeleted(AlbumIconItem* iconItem);
    void slotCleared();
};

}  // NameSpace Digikam
 
#endif  // IMAGEDESCEDITTAB_H

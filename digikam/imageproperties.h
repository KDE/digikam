/* ============================================================
 * File  : imageproperties.h
 * Author: Caulier Gilles <caulier dot gilles at free.fr>
 * Date  : 2004-11-17
 * Description :
 *
 * Copyright 2003 by Gilles Caulier
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
 
#ifndef IMAGEPROPERTIES_H
#define IMAGEPROPERTIES_H

// Qt includes.

#include <qguardedptr.h>
#include <qpixmap.h>

// KDE includes.

#include <kdialogbase.h>

// Local includes.

#include "thumbnailjob.h"

class QLabel;
class QListView;
class QPixmap;
class QCheckListItem;
class QCheckBox;

class KFileMetaInfo;
class KTextEdit;

class AlbumIconView;
class AlbumIconItem;
class AlbumLister;
class TAlbum;

class ImageProperties : public KDialogBase
{
    Q_OBJECT

public:

    ImageProperties(AlbumIconView* view, AlbumIconItem* currItem);
    ~ImageProperties();

    static bool editDescription(AlbumIconView* view, AlbumIconItem* currItem);

protected:

    bool eventFilter(QObject *o, QEvent *e);

private:

    AlbumIconView *m_view;
    AlbumIconItem *m_currItem;
    AlbumLister   *m_lister;
    QLabel        *m_thumbLabel;
    QLabel        *m_nameLabel;
    KTextEdit     *m_commentsEdit;
    QListView     *m_tagsView;
    QCheckBox     *m_autoSaveBox;
    bool           m_modified;

    void tagNew(TAlbum* parAlbum, QCheckListItem *item);
    void tagEdit(TAlbum* album);
    void tagDelete(TAlbum *album, QCheckListItem *item);
    
    QGuardedPtr<ThumbnailJob> m_thumbJob;

    void populateTags();
    void populateTags(QCheckListItem* parItem, TAlbum* parAlbum);
    
private slots:

    void slotItemChanged();
    void slotModified();
    void slotUser1();
    void slotUser2();
    void slotApply();
    void slotOk();
    void slotGotThumbnail(const KURL&, const QPixmap& pix,
                          const KFileMetaInfo*);    
    void slotRightButtonClicked(QListViewItem *, const QPoint &, int);
};

#endif  // IMAGEPROPERTIES_H

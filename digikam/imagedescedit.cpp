/* ============================================================
 * File  : imagedescedit.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-03-09
 * Description :
 *
 * Copyright 2003 by Renchi Raju
 *
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

#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>

#include <qframe.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlistview.h>
#include <qguardedptr.h>
#include <qheader.h>

#include <libkexif/kexif.h>
#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "imagedescedit.h"

class TAlbumCheckListItem : public QCheckListItem
{
public:

    TAlbumCheckListItem(QListView* parent, TAlbum* album)
        : QCheckListItem(parent, album->getTitle()),
          m_album(album)
        {}
    
    TAlbumCheckListItem(QCheckListItem* parent, TAlbum* album)
        : QCheckListItem(parent, album->getTitle(), QCheckListItem::CheckBox),
          m_album(album)
        {}

    TAlbum* m_album;
};

ImageDescEdit::ImageDescEdit(AlbumIconView* view, AlbumIconItem* currItem)
    : KDialogBase(Plain, i18n("Image Comments/Tags"), User1|User2|Stretch|Ok|Apply|Cancel,
                  Ok, view, 0, true, true, i18n("&Next"), i18n("&Previous"))
{
    m_view     = view;
    m_currItem = currItem;
    m_lister   = view->albumLister();

    QGridLayout *topLayout = new QGridLayout(plainPage(), 3, 2, 5, spacingHint());

    QGroupBox  *thumbBox = new QGroupBox(plainPage());
    QVBoxLayout *thumbBoxLayout = new QVBoxLayout(thumbBox, marginHint(),
                                                  spacingHint());
    m_thumbLabel = new QLabel(thumbBox);
    m_thumbLabel->setFixedSize(256, 256);
    m_thumbLabel->setScaledContents(false);
    m_thumbLabel->setAlignment(Qt::AlignCenter);
    thumbBoxLayout->addWidget(m_thumbLabel, 0, Qt::AlignCenter);
    m_nameLabel = new QLabel(thumbBox);
    thumbBoxLayout->addWidget(m_nameLabel, 0, Qt::AlignCenter);
    topLayout->addWidget(thumbBox, 0, 0);

    QVGroupBox* commentsBox = new QVGroupBox(i18n("Comments"), plainPage());
    m_commentsEdit = new QTextEdit(commentsBox);
    topLayout->addWidget(commentsBox, 1, 0);

    QVGroupBox* tagsBox = new QVGroupBox(i18n("Tags"), plainPage());
    m_tagsView = new QListView(tagsBox);
    topLayout->addMultiCellWidget(tagsBox, 0, 1, 1, 1);

    m_tagsView->addColumn("Tags");
    m_tagsView->header()->hide();
    m_tagsView->setSelectionMode(QListView::Single);
    m_tagsView->setResizeMode(QListView::LastColumn);
    populateTags();

    connect(m_commentsEdit, SIGNAL(textChanged()),
            SLOT(slotModified()));
    
    slotItemChanged();

    resize(configDialogSize(*(kapp->config()),"Image Description Dialog"));
}

ImageDescEdit::~ImageDescEdit()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();
    
    saveDialogSize(*(kapp->config()), "Image Description Dialog");
}

void ImageDescEdit::populateTags()
{
    m_tagsView->clear();

    TAlbum* rootTag = AlbumManager::instance()->findTAlbum(0);
    if (!rootTag)
        return;

    TAlbumCheckListItem* item = new TAlbumCheckListItem(m_tagsView, rootTag);
    item->setPixmap(0, rootTag->getPixmap());
    item->setOpen(true);
    populateTags(item, rootTag);
}

void ImageDescEdit::populateTags(QCheckListItem* parItem, TAlbum* parAlbum)
{
    TAlbum* child = dynamic_cast<TAlbum*>(parAlbum->firstChild());
    if (!child)
        return;

    while (child)
    {
        TAlbumCheckListItem* item = new TAlbumCheckListItem(parItem, child);
        item->setPixmap(0, child->getPixmap());
        item->setOpen(true);
        populateTags(item, child);
        child = dynamic_cast<TAlbum*>(child->next());
    }
}

void ImageDescEdit::slotModified()
{
    m_modified = true;
}

void ImageDescEdit::slotUser1()
{
    if (!m_currItem)
        return;

    m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
    slotItemChanged();
}

void ImageDescEdit::slotUser2()
{
    if (!m_currItem)
        return;

    m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
    slotItemChanged();
}

void ImageDescEdit::slotApply()
{
    if (!m_currItem)
        return;

    KURL fileURL(m_currItem->fileItem()->url());    
    
    PAlbum *album = m_lister->findParentAlbum(m_currItem->fileItem());
    if (!album)
    {
        kdWarning() << "Failed to find parent album for"
                    << m_currItem->fileItem()->url().prettyURL()
                    << endl;
        return;
    }

    AlbumDB* db  = AlbumManager::instance()->albumDB();

    if (m_modified)
    {
        db->setItemCaption(album, fileURL.fileName(), m_commentsEdit->text());

        if (AlbumSettings::instance() &&
            AlbumSettings::instance()->getSaveExifComments())
        {
            // store as JPEG Exif comment
            KFileMetaInfo metaInfo(fileURL, "image/jpeg", KFileMetaInfo::Fastest);

            // set Jpeg comment
             if (metaInfo.isValid () && metaInfo.mimeType() == "image/jpeg"
                 && metaInfo.containsGroup("Jpeg EXIF Data"))
            {
                kdDebug() << k_funcinfo << "Contains JPEG Exif data, setting comment"
                          << endl;
                metaInfo["Jpeg EXIF Data"].item("Comment")
                    .setValue(m_commentsEdit->text());
                metaInfo.applyChanges();
            }

            // set EXIF UserComment
            KExifUtils::writeComment(fileURL.path(), m_commentsEdit->text());
        }
        
        m_modified = false;
    }

    db->removeItemTags(album, fileURL.fileName());
    QListViewItemIterator it(m_tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem =
            dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem && tItem->isOn())
        {
            db->setItemTag(album, fileURL.fileName(), tItem->m_album);
        }
        ++it;
    }
}

void ImageDescEdit::slotOk()
{
    slotApply();
    KDialogBase::slotOk();    
}

void ImageDescEdit::slotItemChanged()
{
    if (!m_currItem)
        return;

    m_modified = false;
    
    if (!m_thumbJob.isNull())
    {
        m_thumbJob->kill();
    }

    if (!m_thumbJob.isNull())
    {
        delete m_thumbJob;
    }

    m_thumbJob = new Digikam::ThumbnailJob(m_currItem->fileItem(), 256);
    connect(m_thumbJob,
            SIGNAL(signalThumbnailMetaInfo(const KFileItem*,
                                           const QPixmap&,
                                           const KFileMetaInfo*)),
            SLOT(slotGotThumbnail(const KFileItem*,
                                  const QPixmap&,
                                  const KFileMetaInfo*)));
    
    KURL fileURL(m_currItem->fileItem()->url());    
    
    PAlbum *album = m_lister->findParentAlbum(m_currItem->fileItem());
    if (!album)
    {
        kdWarning() << "Failed to find parent album for"
                    << m_currItem->fileItem()->url().prettyURL()
                    << endl;
        return;
    }
    
    AlbumDB* db  = AlbumManager::instance()->albumDB();

    m_nameLabel->setText(fileURL.fileName());
    m_thumbLabel->setPixmap(QPixmap());
    m_commentsEdit->setText(db->getItemCaption(album, fileURL.fileName()));

    IntList tagIDs = db->getItemTagIDs(album, fileURL.fileName());
    
    QListViewItemIterator it( m_tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem =
            dynamic_cast<TAlbumCheckListItem*>(it.current());
        
        if (tItem)
        {
            if (tagIDs.contains(tItem->m_album->getID()))
                tItem->setOn(true);
            else
                tItem->setOn(false);
        }
        ++it;
    }

    enableButton(User1, m_currItem->nextItem() != 0);
    enableButton(User2, m_currItem->prevItem() != 0);
}

void ImageDescEdit::slotGotThumbnail(const KFileItem*, const QPixmap& pix,
                                     const KFileMetaInfo*)
{
    // todo: exif autorotate the thumbnail
    m_thumbLabel->setPixmap(pix);
}


#include "imagedescedit.moc"


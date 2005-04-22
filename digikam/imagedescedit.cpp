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


#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kdeversion.h>

#include <qframe.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qcheckbox.h>
#include <qlayout.h>
#include <qgroupbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qlistview.h>
#include <qguardedptr.h>
#include <qheader.h>
#include <qpopupmenu.h>
#include <qcursor.h>

#include <libkexif/kexifwidget.h>
#include <libkexif/kexifutils.h>
#include <libkexif/kexifdata.h>

#include "albumfolderview.h"
#include "albumfolderitem.h"
#include "albumiconview.h"
#include "albumiconitem.h"
#include "albumlister.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "imagedescedit.h"
#include "tagcreatedlg.h"

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

ImageDescEdit::ImageDescEdit(AlbumIconView* view, AlbumIconItem* currItem,
                             QWidget *parent, bool singleMode)
             : KDialogBase(Plain, i18n("Image Comments/Tags"),
                           singleMode ? Help|Stretch|Ok|Apply|Cancel : Help|User1|User2|Stretch|Ok|Apply|Cancel,
                           Ok, parent, 0, true, true,
                           KStdGuiItem::guiItem(KStdGuiItem::Forward),
                           KStdGuiItem::guiItem(KStdGuiItem::Back))
{
    setHelp("tagscommentsedit.anchor", "digikam");
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
    m_commentsEdit = new KTextEdit(commentsBox);
    m_commentsEdit->setTextFormat(QTextEdit::PlainText);
#if KDE_IS_VERSION(3,2,0)
    m_commentsEdit->setCheckSpellingEnabled(true);
#endif
    topLayout->addWidget(commentsBox, 1, 0);

    QVGroupBox* tagsBox = new QVGroupBox(i18n("Tags"), plainPage());
    m_tagsView = new QListView(tagsBox);
    topLayout->addMultiCellWidget(tagsBox, 0, 1, 1, 1);

    m_autoSaveBox = new QCheckBox(i18n("Automatically save comments and tags "
                                       "when navigating between items"),
                                  plainPage());
    topLayout->addMultiCellWidget(m_autoSaveBox, 2, 2, 0, 1);

    if (singleMode)
       m_autoSaveBox->hide();

    kapp->config()->setGroup("Image Description Dialog");
    m_autoSaveBox->setChecked(kapp->config()->readBoolEntry("Auto Save", true));

    m_tagsView->addColumn(i18n( "Tags" ));
    m_tagsView->header()->hide();
    m_tagsView->setSelectionMode(QListView::Single);
    m_tagsView->setResizeMode(QListView::LastColumn);
    populateTags();

    connect(m_commentsEdit, SIGNAL(textChanged()),
            SLOT(slotModified()));

    connect(m_tagsView, SIGNAL(rightButtonClicked(QListViewItem*,
            const QPoint &, int)), this,
            SLOT(slotRightButtonClicked(QListViewItem*,
            const QPoint&, int)));

    slotItemChanged();

    resize(configDialogSize("Image Description Dialog"));

    m_commentsEdit->installEventFilter(this);
    m_tagsView->installEventFilter(this);

    m_commentsEdit->setFocus();
}

ImageDescEdit::~ImageDescEdit()
{
    if (!m_thumbJob.isNull())
        m_thumbJob->kill();

    kapp->config()->setGroup("Image Description Dialog");
    kapp->config()->writeEntry("Auto Save", m_autoSaveBox->isChecked());

    saveDialogSize("Image Description Dialog");
}

bool ImageDescEdit::eventFilter(QObject *, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *k = (QKeyEvent *)e;
        if (k->state() == Qt::ControlButton &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();
            AlbumIconItem* item =
                dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
            if (item)
            {
                slotUser1();
            }
            return true;
        }
        else if (k->state() == Qt::ShiftButton &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            slotApply();
            AlbumIconItem* item =
                dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
            if (item)
            {
                slotUser2();
            }
            return true;
        }

        return false;
    }
    return false;
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

    if (m_autoSaveBox->isChecked())
       slotApply();

    m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->nextItem());
    m_currItem->setSelected(true);
    m_view->ensureItemVisible(m_currItem);

    slotItemChanged();
}

void ImageDescEdit::slotUser2()
{
    if (!m_currItem)
        return;

    if (m_autoSaveBox->isChecked())
        slotApply();

    m_currItem = dynamic_cast<AlbumIconItem*>(m_currItem->prevItem());
    m_currItem->setSelected(true);
    m_view->ensureItemVisible(m_currItem);

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
            KFileMetaInfo*  metaInfo =
                new KFileMetaInfo(fileURL.path(), "image/jpeg", KFileMetaInfo::Fastest);

            // set Jpeg comment
             if (metaInfo->isValid () && metaInfo->mimeType() == "image/jpeg"
                 && metaInfo->containsGroup("Jpeg EXIF Data"))
             {
                 kdDebug() << k_funcinfo << "Contains JPEG Exif data, setting comment"
                           << endl;
                 (*metaInfo)["Jpeg EXIF Data"].item("Comment")
                     .setValue(m_commentsEdit->text());
                 metaInfo->applyChanges();
                 m_currItem->setMetaInfo(metaInfo);
             }
             else
             {
                 delete metaInfo;
                 metaInfo = 0;
             }

            // set EXIF UserComment
            //KExifUtils::writeComment(fileURL.path(), m_commentsEdit->text());
        }

        m_modified = false;
    }

    db->removeItemAllTags(album, fileURL.fileName());
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

    KURL fileURL(m_currItem->fileItem()->url());

    m_thumbJob = new ThumbnailJob(fileURL, 256);

    connect(m_thumbJob,
            SIGNAL(signalThumbnailMetaInfo(const KURL&,
                                           const QPixmap&,
                                           const KFileMetaInfo*)),
            SLOT(slotGotThumbnail(const KURL&,
                                  const QPixmap&,
                                  const KFileMetaInfo*)));

    connect(m_thumbJob,
            SIGNAL(signalFailed(const KURL&)),
            SLOT(slotFailedThumbnail(const KURL&)));

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

void ImageDescEdit::slotGotThumbnail(const KURL&, const QPixmap& pix,
                                     const KFileMetaInfo*)
{
    m_thumbLabel->setPixmap(pix);
}

void ImageDescEdit::slotFailedThumbnail(const KURL&)
{
    m_thumbLabel->clear();
    m_thumbLabel->setText(i18n("Thumbnail unavailable"));
}

void ImageDescEdit::slotRightButtonClicked(QListViewItem *item,
                                           const QPoint &, int )
{
    TAlbum              *album;
    TAlbumCheckListItem *albumItem = 0;

    if (!item)
    {
        album = AlbumManager::instance()->findTAlbum(0);
        albumItem = dynamic_cast<TAlbumCheckListItem*>(m_tagsView->firstChild());
    }
    else
    {
        albumItem = dynamic_cast<TAlbumCheckListItem*>(item);

        if(!albumItem)
            album = AlbumManager::instance()->findTAlbum(0);
        else
            album = albumItem->m_album;
    }

    if(!album)
        return;

    QPopupMenu popmenu(this);

    popmenu.insertItem(SmallIcon("tag"),
                       i18n("New Tag..."), 10);
    if (!album->isRoot())
    {
        popmenu.insertItem(SmallIcon("pencil"),
                           i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("edittrash"),
                           i18n("Delete Tag"), 12);
    }

    switch (popmenu.exec(QCursor::pos()))
    {
    case 10:
    {
        tagNew(album, albumItem);
        break;
    }
    case 11:
    {
        if (!album->isRoot())
            tagEdit(album);
        break;
    }
    case 12:
    {
        if (!album->isRoot())
            tagDelete(album, albumItem);
        break;
    }
    default:
        break;
    }
}

void ImageDescEdit::tagNew(TAlbum* parAlbum, QCheckListItem *item)
{
    if(!parAlbum || !item)
        return;

    QString title, icon;
    AlbumManager *albumMan_ = AlbumManager::instance();

    if (!TagCreateDlg::tagCreate(parAlbum, title, icon))
        return;

    QString errMsg;
    if (!albumMan_->createTAlbum(parAlbum, title, icon, errMsg))
    {
        KMessageBox::error(this, errMsg);
    }
    else
    {
        TAlbum* child = dynamic_cast<TAlbum*>(parAlbum->firstChild());
        while (child)
        {
            if (child->getTitle() == title)
            {
                new TAlbumCheckListItem(item, child);
                return;
            }
            child = dynamic_cast<TAlbum*>(child->next());
        }
    }
}

void ImageDescEdit::tagDelete(TAlbum *album, QCheckListItem *item)
{
    if (!album || album->isRoot())
        return;

    AlbumManager *albumMan_ = AlbumManager::instance();

    int result =
        KMessageBox::questionYesNo(this, i18n("Delete '%1' tag?")
                                   .arg(album->getTitle()));

    if (result == KMessageBox::Yes)
    {
        QString errMsg;
        if (!albumMan_->deleteTAlbum(album, errMsg))
            KMessageBox::error(0, errMsg);
        else
        {
            if(item)
                delete item;
        }
    }
}


void ImageDescEdit::tagEdit(TAlbum* album)
{
    if (!album || album->isRoot())
        return;

    AlbumFolderItem *folderItem =
        static_cast<AlbumFolderItem*>(album->getViewItem());

    AlbumFolderView* folderView =
        static_cast<AlbumFolderView*>(folderItem->listView());
    folderView->tagEdit(album);

    // Now update the icon/title of the corresponding checklistitem

    QListViewItemIterator it(m_tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem =
            dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem && tItem->m_album == album)
        {
            tItem->setText(0, album->getTitle());
            tItem->setPixmap(0, album->getPixmap());
        }
        ++it;
    }

}

#include "imagedescedit.moc"



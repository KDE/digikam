/* ============================================================
 * Authors: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 *          Caulier Gilles <caulier dot gilles at kdemail dot net>
 * Date  : 2003-03-09
 * Description : Comments, Tags, and Rating properties editor
 *
 * Copyright 2003-2005 by Renchi Raju & Gilles Caulier
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

// Qt includes.
 
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
#include <qtoolbutton.h>
#include <qpushbutton.h>

// KDE includes.

#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kdialogbase.h>
#include <kdatetimeedit.h>

// Local includes.

#include "dmetadata.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "tagcreatedlg.h"
#include "syncjob.h"
#include "navigatebarwidget.h"
#include "imageinfo.h"
#include "ratingwidget.h"
#include "imagedescedittab.h"

namespace Digikam
{

class TAlbumCheckListItem : public QCheckListItem
{
public:

    TAlbumCheckListItem(QListView* parent, TAlbum* album)
        : QCheckListItem(parent, album->title()),
          m_album(album)
    {
    }

    TAlbumCheckListItem(QCheckListItem* parent, TAlbum* album)
        : QCheckListItem(parent, album->title(), QCheckListItem::CheckBox),
          m_album(album)
    {
    }

    TAlbum* m_album;

protected:
    
    virtual void stateChange(bool val)
    {
        QCheckListItem::stateChange(val);
        TAlbumListView* view = dynamic_cast<TAlbumListView*>(listView());
        view->emitSignalItemStateChanged();
    }
};

// ------------------------------------------------------------------------

class ImageDescEditTabPriv
{
public:

    ImageDescEditTabPriv()
    {
        modified           = false;
        recentTagsBtn      = 0;
        tagsSearchClearBtn = 0 ;
        commentsEdit       = 0;
        tagsSearchEdit     = 0;
        dateTimeEdit       = 0;
        currItem           = 0;
        tagsView           = 0;
        ratingWidget       = 0;
        navigateBar        = 0;
    }

    bool               modified;

    QPushButton       *recentTagsBtn;

    QToolButton       *tagsSearchClearBtn;

    KTextEdit         *commentsEdit;

    KLineEdit         *tagsSearchEdit;

    KDateTimeEdit     *dateTimeEdit;

    AlbumIconItem     *currItem;

    TAlbumListView    *tagsView;

    RatingWidget      *ratingWidget;
    
    NavigateBarWidget *navigateBar;
};

ImageDescEditTab::ImageDescEditTab(QWidget *parent, bool navBar)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageDescEditTabPriv;
    
    QGridLayout *topLayout = new QGridLayout(this, 4, 2, KDialog::marginHint(), KDialog::spacingHint());

    d->navigateBar  = new NavigateBarWidget(this, navBar);
    topLayout->addMultiCellWidget(d->navigateBar, 0, 0, 0, 2);
    
    // Comments view ---------------------------------------------------
    
    QVGroupBox* commentsBox = new QVGroupBox(i18n("Comments"), this);
    d->commentsEdit = new KTextEdit(commentsBox);
    d->commentsEdit->setTextFormat(QTextEdit::PlainText);
    d->commentsEdit->setCheckSpellingEnabled(true);
    topLayout->addMultiCellWidget(commentsBox, 1, 1, 0, 2);

    // Date and Time view ---------------------------------------------------
    
    QHGroupBox* dateTimeBox = new QHGroupBox(i18n("Date && Time"), this);
    d->dateTimeEdit = new KDateTimeEdit( dateTimeBox, "datepicker");
    topLayout->addMultiCellWidget(dateTimeBox, 2, 2, 0, 2);

    // Rating view --------------------------------------------------

    QHGroupBox* ratingBox = new QHGroupBox(i18n("Rating"), this);
    ratingBox->layout()->setAlignment(Qt::AlignCenter);
    d->ratingWidget = new RatingWidget(ratingBox);
    topLayout->addMultiCellWidget(ratingBox, 3, 3, 0, 2);
        
    // Tags view ---------------------------------------------------

    QGroupBox* tagsBox = new QGroupBox(i18n("Tags"), this);
    QVBoxLayout* tagsBoxLayout = new QVBoxLayout(tagsBox, KDialog::marginHint(), KDialog::spacingHint());

    d->tagsSearchClearBtn = new QToolButton(tagsBox);
    d->tagsSearchClearBtn->setAutoRaise(true);
    d->tagsSearchClearBtn->setIconSet(kapp->iconLoader()->loadIcon("locationbar_erase",
                                                                  KIcon::Toolbar,
                                                                  KIcon::SizeSmall));

    QLabel* tagsSearchTextBtn = new QLabel(i18n("Search:"), tagsBox);
    d->tagsSearchEdit = new KLineEdit(tagsBox);

    QHBoxLayout* tagsSearchLayout = new QHBoxLayout(0, 5, 5);
    tagsSearchLayout->addWidget(d->tagsSearchClearBtn);
    tagsSearchLayout->addWidget(tagsSearchTextBtn);
    tagsSearchLayout->addWidget(d->tagsSearchEdit);
    tagsBoxLayout->addLayout(tagsSearchLayout);

    d->tagsView = new TAlbumListView(tagsBox);
    tagsBoxLayout->addWidget(d->tagsView);

    d->recentTagsBtn = new QPushButton(i18n("Recent Tags"), tagsBox);
    tagsBoxLayout->addWidget(d->recentTagsBtn);

    topLayout->addMultiCellWidget(tagsBox, 4, 4, 0, 2);

    d->tagsView->addColumn(i18n( "Tags" ));
    d->tagsView->header()->hide();
    d->tagsView->setSelectionMode(QListView::Single);
    d->tagsView->setResizeMode(QListView::LastColumn);

    // --------------------------------------------------
    
    connect(d->tagsView, SIGNAL(signalItemStateChanged()),
            this, SLOT(slotModified()));
    
    connect(d->navigateBar, SIGNAL(signalFirstItem()),
            this, SIGNAL(signalFirstItem()));
                    
    connect(d->navigateBar, SIGNAL(signalPrevItem()),
            this, SIGNAL(signalPrevItem()));
    
    connect(d->navigateBar, SIGNAL(signalNextItem()),
            this, SIGNAL(signalNextItem()));

    connect(d->navigateBar, SIGNAL(signalLastItem()),
            this, SIGNAL(signalLastItem()));

    connect(d->commentsEdit, SIGNAL(textChanged()),
            this, SLOT(slotModified()));
    
    connect(d->dateTimeEdit, SIGNAL(dateTimeChanged(const QDateTime& )),
            this, SLOT(slotModified()));

    connect(d->ratingWidget, SIGNAL(signalRatingChanged(int)),
            this, SLOT(slotModified()));
     
    connect(d->tagsView, SIGNAL(rightButtonClicked(QListViewItem*, const QPoint &, int)),
            this, SLOT(slotRightButtonClicked(QListViewItem*, const QPoint&, int)));
            
    connect(d->tagsSearchClearBtn, SIGNAL(clicked()),
            d->tagsSearchEdit, SLOT(clear()));
            
    connect(d->tagsSearchEdit, SIGNAL(textChanged(const QString&)),
            this, SLOT(slotTagsSearchChanged()));
            
    connect(d->recentTagsBtn, SIGNAL(clicked()),
            this, SLOT(slotRecentTags()));

    // Initalize ---------------------------------------------

    d->commentsEdit->installEventFilter(this);
    d->dateTimeEdit->installEventFilter(this);
    d->ratingWidget->installEventFilter(this);
    d->tagsView->installEventFilter(this);

    d->commentsEdit->setFocus();

    // Connect to album manager -----------------------------

    AlbumManager* man = AlbumManager::instance();
    
    connect(man, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));
    
    connect(man, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
    
    connect(man, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));
    
    connect(man, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));
}

ImageDescEditTab::~ImageDescEditTab()
{
    applyAllChanges();
    
    /*
    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        (*it)->removeExtraData(this);
    }
    */

    delete d;
}

bool ImageDescEditTab::eventFilter(QObject *, QEvent *e)
{
    if ( e->type() == QEvent::KeyPress )
    {
        QKeyEvent *k = (QKeyEvent *)e;
        if (k->state() == Qt::ControlButton &&
            (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            emit signalNextItem();
            return true;
        }
        else if (k->state() == Qt::ShiftButton &&
                 (k->key() == Qt::Key_Enter || k->key() == Qt::Key_Return))
        {
            emit signalPrevItem();
            return true;
        }

        return false;
    }
    
    return false;
}

void ImageDescEditTab::populateTags()
{
    d->tagsView->clear();

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);

        slotAlbumAdded(tag);
    }
}

void ImageDescEditTab::slotModified()
{
    d->modified = true;
}

void ImageDescEditTab::assignRating(int rating)
{
    d->ratingWidget->setRating(rating);
}

void ImageDescEditTab::applyAllChanges()
{
    if (!d->modified)
        return;
    
    if (!d->currItem)
        return;

    ImageInfo* info = d->currItem->imageInfo();

    info->setCaption(d->commentsEdit->text());
    info->setDateTime(d->dateTimeEdit->dateTime());
    info->setRating(d->ratingWidget->rating());
    info->removeAllTags();

    QStringList keywordsList;
    QListViewItemIterator it(d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem && tItem->isOn())
        {
            info->setTag(tItem->m_album->id());
            keywordsList.append(tItem->m_album->title());
        }
        ++it;
    }

    // Store data in image metadata.

    if (AlbumSettings::instance())
    {
        DMetadata metadata(info->filePath());

        if (AlbumSettings::instance()->getSaveComments())
        {
            // Store comments in image as JFIF comments, Exif comments, and Iptc Comments.
            metadata.setImageComment(d->commentsEdit->text());
        }

        if (AlbumSettings::instance()->getSaveDateTime())
        {
            // Store Image Date & Time as Exif and Iptc tags.
            metadata.setImageDateTime(d->dateTimeEdit->dateTime());
        }
        
        if (AlbumSettings::instance()->getSaveIptcRating())
        {
            // Store Image rating as Iptc tag.
            metadata.setImageRating(d->ratingWidget->rating());
        }

        if (AlbumSettings::instance()->getSaveIptcTags())
        {
            // Store Image Tags like Iptc keywords tag.
            metadata.setImageKeywords(keywordsList);
        }

        if (AlbumSettings::instance()->getSaveIptcPhotographerId())
        {
            // Store Photograph indentity into Iptc tags.
            metadata.setImagePhotographerId(AlbumSettings::instance()->getIptcAuthor(),
                                            AlbumSettings::instance()->getIptcAuthorTitle(),
                                            AlbumSettings::instance()->getIptcCity(),
                                            AlbumSettings::instance()->getIptcProvince(),
                                            AlbumSettings::instance()->getIptcCountry());
        }
        
        metadata.applyChanges();
    }

    d->modified = false;
}

void ImageDescEditTab::setItem(AlbumIconItem* currItem, int itemType)
{
    applyAllChanges();
    
    if (!currItem)
    {
       d->navigateBar->setFileName();
       d->commentsEdit->clear();
       d->currItem = 0;
       setEnabled(false);
       return;
    }

    setEnabled(true);
    d->currItem = currItem;
    d->modified = false;
    
    ImageInfo* info = currItem->imageInfo();
    KURL fileURL;
    fileURL.setPath(info->filePath());
    d->navigateBar->setFileName(info->name());
    d->navigateBar->setButtonsState(itemType);

    PAlbum *album = currItem->imageInfo()->album();
    if (!album)
    {
        kdWarning() << k_funcinfo << "Failed to find parent album for"
                    << fileURL << endl;
        return;
    }

    d->commentsEdit->blockSignals(true);
    d->dateTimeEdit->blockSignals(true);
    d->tagsView->blockSignals(true);

    d->commentsEdit->setText(info->caption());
    d->dateTimeEdit->setDateTime(info->dateTime());
    d->ratingWidget->setRating(info->rating());

    QValueList<int> tagIDs = info->tagIDs();

    QListViewItemIterator it( d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem =
            dynamic_cast<TAlbumCheckListItem*>(it.current());

        if (tItem)
        {
            if (tagIDs.contains(tItem->m_album->id()))
                tItem->setOn(true);
            else
                tItem->setOn(false);
        }
        ++it;
    }

    d->commentsEdit->blockSignals(false);
    d->dateTimeEdit->blockSignals(false);
    d->tagsView->blockSignals(false);
}

void ImageDescEditTab::slotRightButtonClicked(QListViewItem *item, const QPoint &, int )
{
    TAlbum *album;

    if (!item)
    {
        album = AlbumManager::instance()->findTAlbum(0);
    }
    else
    {
        TAlbumCheckListItem* viewItem = dynamic_cast<TAlbumCheckListItem*>(item);

        if(!viewItem)
            album = AlbumManager::instance()->findTAlbum(0);
        else
            album = viewItem->m_album;
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
        tagNew(album);
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
            tagDelete(album);
        break;
    }
    default:
        break;
    }
}

void ImageDescEditTab::tagNew(TAlbum* parAlbum)
{
    if (!parAlbum)
        return;

    QString title, icon;
    AlbumManager *albumMan_ = AlbumManager::instance();

    if (!TagCreateDlg::tagCreate(parAlbum, title, icon))
        return;

    QString errMsg;
    TAlbum* album = albumMan_->createTAlbum(parAlbum, title, icon, errMsg);

    if (!album)
    {
        KMessageBox::error(this, errMsg);
    }
    else
    {
        TAlbumCheckListItem* viewItem =
            (TAlbumCheckListItem*)album->extraData(this);
        if (viewItem)
        {
            viewItem->setOn(true);
            d->tagsView->setSelected(viewItem, true);
            d->tagsView->ensureItemVisible(viewItem);
        }
    }
}

void ImageDescEditTab::tagDelete(TAlbum *album)
{
    if (!album || album->isRoot())
        return;

    AlbumManager *albumMan = AlbumManager::instance();

    if (album == albumMan->currentAlbum() ||
        album->isAncestorOf(albumMan->currentAlbum()))
    {
        KMessageBox::error(this, i18n("You are currently viewing items in the "
                                      "tag '%1' that you are about to delete. "
                                      "You will need to close this dialog first "
                                      "if you want to delete the tag." )
                           .arg(album->title()));
        return;
    }

    int result = KMessageBox::warningContinueCancel(this, i18n("Delete '%1' tag?")
                                                    .arg(album->title()),i18n("Delete Tag"),
                                                    KGuiItem(i18n("Delete"), "editdelete"));

    if (result == KMessageBox::Continue)
    {
        QString errMsg;
        if (!albumMan->deleteTAlbum(album, errMsg))
            KMessageBox::error(this, errMsg);
    }
}

void ImageDescEditTab::tagEdit(TAlbum* album)
{
    if (!album || album->isRoot())
        return;

    QString title;
    QString icon;

    if (!TagEditDlg::tagEdit(album, title, icon))
        return;


    AlbumManager *albumMan = AlbumManager::instance();
    if (album->title() != title)
    {
        QString errMsg;
        if (!albumMan->renameTAlbum(album, title, errMsg))
        {
            KMessageBox::error(this, errMsg);
            return;
        }
    }

    if (album->icon() != icon)
    {
        QString errMsg;
        if (!albumMan->updateTAlbumIcon(album, icon, 0, errMsg));
        {
            KMessageBox::error(this, errMsg);
        }
    }
}

void ImageDescEditTab::slotAlbumAdded(Album* a)
{
    if (!a || a->type() != Album::TAG)
        return;

    TAlbumCheckListItem* viewItem = 0;

    TAlbum* tag = (TAlbum*)a;

    if (tag->isRoot())
    {
        viewItem = new TAlbumCheckListItem(d->tagsView, tag);
    }
    else
    {
        QCheckListItem* parentItem = (QCheckListItem*)(tag->parent()->extraData(this));

        if (!parentItem)
        {
            kdWarning() << k_funcinfo << "Failed to find parent for Tag " << tag->title()
                        << endl;
            return;
        }

        viewItem = new TAlbumCheckListItem(parentItem, tag);
    }

    if (viewItem)
    {
        viewItem->setOpen(true);
        viewItem->setPixmap(0, tagThumbnail(tag));
        tag->setExtraData(this, viewItem);
    }

}

void ImageDescEditTab::slotAlbumDeleted(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    delete viewItem;
    album->removeExtraData(this);
}

void ImageDescEditTab::slotAlbumIconChanged(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    if (!viewItem)
    {
        kdWarning() << "Failed to find view item for Tag "
                    << album->title() << endl;
        return;
    }

    viewItem->setPixmap(0, tagThumbnail(album));
}

void ImageDescEditTab::slotAlbumRenamed(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    if (!viewItem)
    {
        kdWarning() << "Failed to find view item for Tag "
                    << album->title() << endl;
        return;
    }

    viewItem->setText(0, album->title());
}

QPixmap ImageDescEditTab::tagThumbnail(TAlbum* album) const
{
    KIconLoader *iconLoader = KApplication::kApplication()->iconLoader();

    QPixmap pix;

    if (!album->isRoot())
        pix = SyncJob::getTagThumbnail(album->icon(), 20);
    else
        pix = iconLoader->loadIcon("tag-folder", KIcon::NoGroup, 20,
                                   KIcon::DefaultState, 0, true);

    return pix;
}

void ImageDescEditTab::slotRecentTags()
{
    QPopupMenu menu(this);

    AlbumManager* albumMan = AlbumManager::instance();
    IntList recentTags = albumMan->albumDB()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        menu.insertItem(i18n("No Recently Assigned Tags"), 0);
        menu.setItemEnabled(0, false);
    }
    else
    {
        for (IntList::const_iterator it = recentTags.begin();
             it != recentTags.end(); ++it)
        {
            TAlbum* album = albumMan->findTAlbum(*it);
            if (album)
            {
                QPixmap pix = SyncJob::getTagThumbnail(album->icon(),
                                                       KIcon::SizeSmall);
                QString text = album->title() + " (" +
                               ((TAlbum*)album->parent())->prettyURL() +
                               ")";
                menu.insertItem(pix, text, album->id());
            }
        }
    }

    int id = menu.exec(QCursor::pos());

    if (id > 0)
    {
        TAlbum* album = albumMan->findTAlbum(id);
        if (album)
        {
            TAlbumCheckListItem* viewItem =
                (TAlbumCheckListItem*)album->extraData(this);
            if (viewItem)
            {
                viewItem->setOn(true);
                d->tagsView->setSelected(viewItem, true);
                d->tagsView->ensureItemVisible(viewItem);
            }
        }
    }
}

void ImageDescEditTab::slotTagsSearchChanged()
{
    QString search(d->tagsSearchEdit->text());
    search = search.lower();

    bool atleastOneMatch = false;

    AlbumList tList = AlbumManager::instance()->allTAlbums();
    for (AlbumList::iterator it = tList.begin(); it != tList.end(); ++it)
    {
        TAlbum* tag  = (TAlbum*)(*it);

        // don't touch the root Tag
        if (tag->isRoot())
            continue;

        bool match = tag->title().lower().contains(search);
        if (!match)
        {
            // check if any of the parents match the search
            Album* parent = tag->parent();
            while (parent && !parent->isRoot())
            {
                if (parent->title().lower().contains(search))
                {
                    match = true;
                    break;
                }

                parent = parent->parent();
            }
        }

        if (!match)
        {
            // check if any of the children match the search
            AlbumIterator it(tag);
            while (it.current())
            {
                if ((*it)->title().lower().contains(search))
                {
                    match = true;
                    break;
                }
                ++it;
            }
        }

        TAlbumCheckListItem* viewItem
            = (TAlbumCheckListItem*)(tag->extraData(this));

        if (match)
        {
            atleastOneMatch = true;

            if (viewItem)
                viewItem->setVisible(true);
        }
        else
        {
            if (viewItem)
            {
                viewItem->setVisible(false);
            }
        }
    }

    if (search.isEmpty())
    {
        d->tagsSearchEdit->unsetPalette();
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem =
            (TAlbumCheckListItem*)(root->extraData(this));
        if (rootItem)
            rootItem->setText(0, root->title());
    }
    else
    {
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem =
            (TAlbumCheckListItem*)(root->extraData(this));
        if (rootItem)
            rootItem->setText(0, i18n("Found Tags"));

        QPalette pal = d->tagsSearchEdit->palette();
        pal.setColor(QPalette::Active, QColorGroup::Base,
                     atleastOneMatch ?  QColor(200,255,200) :
                     QColor(255,200,200));
        d->tagsSearchEdit->setPalette(pal);
    }
}

// ------------------------------------------------------------------------

TAlbumListView::TAlbumListView(QWidget* parent)
    : QListView(parent)
{
}

void TAlbumListView::emitSignalItemStateChanged()
{
    emit signalItemStateChanged();
}

}  // NameSpace Digikam

#include "imagedescedittab.moc"

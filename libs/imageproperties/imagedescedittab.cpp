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
#include <qcursor.h>
#include <qtoolbutton.h>
#include <qpushbutton.h>
#include <qiconset.h>

// KDE includes.

#include <kpopupmenu.h>
#include <klocale.h>
#include <kurl.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <ktextedit.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kdialogbase.h>

// Local includes.

#include "ddebug.h"
#include "dmetadata.h"
#include "kdatetimeedit.h"
#include "albumiconitem.h"
#include "albummanager.h"
#include "albumdb.h"
#include "album.h"
#include "albumsettings.h"
#include "tagcreatedlg.h"
#include "navigatebarwidget.h"
#include "imageinfo.h"
#include "ratingwidget.h"
#include "imagedescedittab.h"
#include "imageattributeswatch.h"
#include "albumthumbnailloader.h"

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
        modified                   = false;
        ignoreImageAttributesWatch = false;
        recentTagsBtn              = 0;
        tagsSearchClearBtn         = 0;
        commentsEdit               = 0;
        tagsSearchEdit             = 0;
        dateTimeEdit               = 0;
        currInfo                   = 0;
        tagsView                   = 0;
        ratingWidget               = 0;
        navigateBar                = 0;
    }

    bool               modified;
    bool               ignoreImageAttributesWatch;

    QToolButton       *recentTagsBtn;
    QToolButton       *tagsSearchClearBtn;

    KTextEdit         *commentsEdit;

    KLineEdit         *tagsSearchEdit;

    KDateTimeEdit     *dateTimeEdit;

    ImageInfo         *currInfo;

    TAlbumListView    *tagsView;

    RatingWidget      *ratingWidget;

    NavigateBarWidget *navigateBar;
};

ImageDescEditTab::ImageDescEditTab(QWidget *parent, bool navBar)
                : QWidget(parent, 0, Qt::WDestructiveClose)
{
    d = new ImageDescEditTabPriv;
    
    QVBoxLayout *vLayout        = new QVBoxLayout(this);
    d->navigateBar              = new NavigateBarWidget(this, navBar);
    QWidget *settingsArea       = new QWidget(this);
    QGridLayout *settingsLayout = new QGridLayout(settingsArea, 3, 1, KDialog::marginHint(), KDialog::spacingHint());
    
    // Comments view ---------------------------------------------------
    
    QVGroupBox* commentsBox = new QVGroupBox(i18n("&Comments"), settingsArea);
    d->commentsEdit         = new KTextEdit(commentsBox);
    d->commentsEdit->setTextFormat(QTextEdit::PlainText);
    d->commentsEdit->setCheckSpellingEnabled(true);

    // Date and Time view ---------------------------------------------------
    
    QHGroupBox* dateTimeBox = new QHGroupBox(i18n("&Date && Time"), settingsArea);
    d->dateTimeEdit         = new KDateTimeEdit( dateTimeBox, "datepicker");

    // Rating view --------------------------------------------------

    QHGroupBox* ratingBox = new QHGroupBox(i18n("Rating"), settingsArea);
    ratingBox->layout()->setAlignment(Qt::AlignCenter);
    d->ratingWidget = new RatingWidget(ratingBox);
        
    // Tags view ---------------------------------------------------

    QGroupBox* tagsBox         = new QGroupBox(i18n("&Tags"), settingsArea);
    QVBoxLayout* tagsBoxLayout = new QVBoxLayout(tagsBox, KDialog::marginHint(), KDialog::spacingHint());

    d->tagsSearchClearBtn = new QToolButton(tagsBox);
    d->tagsSearchClearBtn->setAutoRaise(true);
    d->tagsSearchClearBtn->setIconSet(kapp->iconLoader()->loadIcon("locationbar_erase",
                                      KIcon::Toolbar, KIcon::SizeSmall));

    QLabel* tagsSearchTextBtn = new QLabel(i18n("Search:"), tagsBox);
    d->tagsSearchEdit         = new KLineEdit(tagsBox);

    QHBoxLayout* tagsSearchLayout = new QHBoxLayout(0, 5, 5);
    tagsSearchLayout->addWidget(d->tagsSearchClearBtn);
    tagsSearchLayout->addWidget(tagsSearchTextBtn);
    tagsSearchLayout->addWidget(d->tagsSearchEdit);
    tagsBoxLayout->addLayout(tagsSearchLayout);

    d->tagsView = new TAlbumListView(tagsBox);
    tagsBoxLayout->addWidget(d->tagsView);

    d->recentTagsBtn      = new QToolButton(tagsBox);
    QPopupMenu *popupMenu = new QPopupMenu(d->recentTagsBtn);
    d->recentTagsBtn->setTextLabel(i18n("Recent Tags"));
    d->recentTagsBtn->setIconSet(kapp->iconLoader()->loadIcon("tag", KIcon::NoGroup,
                                 KIcon::SizeSmall, KIcon::DefaultState, 0, true));
    d->recentTagsBtn->setUsesTextLabel(true);
    d->recentTagsBtn->setUsesBigPixmap(false);
    d->recentTagsBtn->setTextPosition(QToolButton::BesideIcon);
    d->recentTagsBtn->setPopup(popupMenu);
    d->recentTagsBtn->setPopupDelay(1);
    tagsBoxLayout->addWidget(d->recentTagsBtn);

    d->tagsView->addColumn(i18n( "Tags" ));
    d->tagsView->header()->hide();
    d->tagsView->setSelectionMode(QListView::Single);
    d->tagsView->setResizeMode(QListView::LastColumn);

    // --------------------------------------------------
    
    settingsLayout->addMultiCellWidget(commentsBox, 0, 0, 0, 1);
    settingsLayout->addMultiCellWidget(dateTimeBox, 1, 1, 0, 1);
    settingsLayout->addMultiCellWidget(ratingBox, 2, 2, 0, 1);
    settingsLayout->addMultiCellWidget(tagsBox, 3, 3, 0, 1);
    settingsLayout->setRowStretch(3, 10);

    vLayout->addWidget(d->navigateBar);
    vLayout->addWidget(settingsArea);    

    // --------------------------------------------------

    connect(popupMenu, SIGNAL(activated(int)),
            this, SLOT(slotRecentTagsMenuActivated(int)));

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
            
    // Initalize ---------------------------------------------

    d->commentsEdit->installEventFilter(this);
    d->dateTimeEdit->installEventFilter(this);
    d->ratingWidget->installEventFilter(this);
    d->tagsView->installEventFilter(this);
    d->commentsEdit->setFocus();
    updateRecentTags();

    // Connect to album manager -----------------------------

    AlbumManager* man = AlbumManager::instance();
    
    connect(man, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));
    
    connect(man, SIGNAL(signalAlbumDeleted(Album*)),
            this, SLOT(slotAlbumDeleted(Album*)));
    
    connect(man, SIGNAL(signalAlbumRenamed(Album*)),
            this, SLOT(slotAlbumRenamed(Album*)));
    
    connect(man, SIGNAL(signalAlbumsCleared()),
            this, SLOT(slotAlbumsCleared()));
    
    connect(man, SIGNAL(signalAlbumIconChanged(Album*)),
            this, SLOT(slotAlbumIconChanged(Album*)));

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();

    connect(loader, SIGNAL(signalThumbnail(Album *, const QPixmap&)),
            this, SLOT(slotGotThumbnailFromIcon(Album *, const QPixmap&)));

    connect(loader, SIGNAL(signalFailed(Album *)),
            this, SLOT(slotThumbnailLost(Album *)));

    ImageAttributesWatch *watch = ImageAttributesWatch::instance();

    connect(watch, SIGNAL(signalImageTagsChanged(Q_LLONG)),
            this, SLOT(slotImageTagsChanged(Q_LLONG)));

    connect(watch, SIGNAL(signalImagesChanged(int)),
            this, SLOT(slotImagesChanged(int)));

    connect(watch, SIGNAL(signalImageRatingChanged(Q_LLONG)),
            this, SLOT(slotImageRatingChanged(Q_LLONG)));

    connect(watch, SIGNAL(signalImageDateChanged(Q_LLONG)),
            this, SLOT(slotImageDateChanged(Q_LLONG)));

    connect(watch, SIGNAL(signalImageCaptionChanged(Q_LLONG)),
            this, SLOT(slotImageCaptionChanged(Q_LLONG)));
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

    if (!d->currInfo)
        return;

    QStringList oldKeywords = d->currInfo->tagPaths();
    for (QStringList::iterator it = oldKeywords.begin(); it != oldKeywords.end(); ++it)
        (*it).remove(0, 1);

    // we are now changing attributes ourselves
    d->ignoreImageAttributesWatch = true;

    d->currInfo->setCaption(d->commentsEdit->text());
    d->currInfo->setDateTime(d->dateTimeEdit->dateTime());
    d->currInfo->setRating(d->ratingWidget->rating());
    d->currInfo->removeAllTags();

    QListViewItemIterator it(d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem = dynamic_cast<TAlbumCheckListItem*>(it.current());
        if (tItem && tItem->isOn())
        {
            d->currInfo->setTag(tItem->m_album->id());
        }
        ++it;
    }

    d->ignoreImageAttributesWatch = false;

    // Store data in image metadata.

    if (AlbumSettings::instance())
    {
        DMetadata metadata(d->currInfo->filePath());

        if (AlbumSettings::instance()->getSaveComments())
        {
            // Store comments in image as JFIF comments, Exif comments, and Iptc Comments.
            metadata.setImageComment(d->commentsEdit->text());
        }

        if (AlbumSettings::instance()->getSaveDateTime())
        {
            // Store Image Date & Time as Exif and Iptc tags.
            metadata.setImageDateTime(d->dateTimeEdit->dateTime(), false);
        }

        if (AlbumSettings::instance()->getSaveIptcRating())
        {
            // Store Image rating as Iptc tag.
            metadata.setImageRating(d->ratingWidget->rating());
        }

        if (AlbumSettings::instance()->getSaveIptcTags())
        {
            // Store Image Tag paths like Iptc keywords tag.
            QStringList tagPaths = d->currInfo->tagPaths();
            for (QStringList::iterator it = tagPaths.begin(); it != tagPaths.end(); ++it)
                (*it).remove(0, 1);
    
            metadata.setImageKeywords(oldKeywords, tagPaths);
        }

        if (AlbumSettings::instance()->getSaveIptcPhotographerId())
        {
            // Store Photograph indentity into Iptc tags.
            metadata.setImagePhotographerId(AlbumSettings::instance()->getIptcAuthor(),
                                            AlbumSettings::instance()->getIptcAuthorTitle());
        }

        if (AlbumSettings::instance()->getSaveIptcCredits())
        {
            // Store Photograph indentity into Iptc tags.
            metadata.setImageCredits(AlbumSettings::instance()->getIptcCredit(),
                                     AlbumSettings::instance()->getIptcSource(),
                                     AlbumSettings::instance()->getIptcCopyright());
        }

        metadata.applyChanges();
        ImageAttributesWatch::instance()->fileMetadataChanged(d->currInfo->kurl());
    }

    d->modified = false;

    updateRecentTags();
}

void ImageDescEditTab::setItem(ImageInfo *info, int itemType)
{
    applyAllChanges();

    if (!info)
    {
       d->navigateBar->setFileName();
       d->commentsEdit->clear();
       d->currInfo = 0;
       setEnabled(false);
       return;
    }

    setEnabled(true);
    d->currInfo = info;
    d->modified = false;

    KURL fileURL;
    fileURL.setPath(d->currInfo->filePath());
    d->navigateBar->setFileName(d->currInfo->name());
    d->navigateBar->setButtonsState(itemType);

    PAlbum *album = d->currInfo->album();
    if (!album)
    {
        DWarning() << k_funcinfo << "Failed to find parent album for"
                    << fileURL << endl;
        return;
    }

    updateComments();
    updateRating();
    updateDate();
    updateTagsView();
    update();
}

void ImageDescEditTab::updateTagsView()
{
    d->tagsView->blockSignals(true);

    QValueList<int> tagIDs = d->currInfo->tagIDs();

    QListViewItemIterator it( d->tagsView);
    while (it.current())
    {
        TAlbumCheckListItem* tItem = dynamic_cast<TAlbumCheckListItem*>(it.current());

        if (tItem)
        {
            if (tagIDs.contains(tItem->m_album->id()))
                tItem->setOn(true);
            else
                tItem->setOn(false);
        }
        ++it;
    }

    d->tagsView->blockSignals(false);
}

void ImageDescEditTab::updateComments()
{
    d->commentsEdit->blockSignals(true);
    d->commentsEdit->setText(d->currInfo->caption());
    d->commentsEdit->blockSignals(false);
}

void ImageDescEditTab::updateRating()
{
    d->ratingWidget->blockSignals(true);
    d->ratingWidget->setRating(d->currInfo->rating());
    d->ratingWidget->blockSignals(false);
}

void ImageDescEditTab::updateDate()
{
    d->dateTimeEdit->blockSignals(true);
    d->dateTimeEdit->setDateTime(d->currInfo->dateTime());
    d->dateTimeEdit->blockSignals(false);
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

    KPopupMenu popmenu(this);
    popmenu.insertTitle(SmallIcon("digikam"), i18n("Tags"));
    popmenu.insertItem(SmallIcon("tag"), i18n("New Tag..."), 10);

    if (!album->isRoot())
    {
        popmenu.insertItem(SmallIcon("pencil"),      i18n("Edit Tag Properties..."), 11);
        popmenu.insertItem(SmallIcon("reload_page"), i18n("Reset Tag Icon"),         13);        
        popmenu.insertSeparator(-1);
        popmenu.insertItem(SmallIcon("edittrash"),   i18n("Delete Tag"),             12);
    }

    popmenu.insertSeparator(-1);
    popmenu.insertItem(i18n("Select All"),       14);
    popmenu.insertItem(i18n("Deselect"),         15);
    popmenu.insertItem(i18n("Invert Selection"), 16);

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
        case 13:
        {
            QString errMsg;
            AlbumManager::instance()->updateTAlbumIcon(album, QString("tag"), 0, errMsg);
            break;
        }
        case 14:
        {
            QListViewItemIterator it(d->tagsView, QListViewItemIterator::NotChecked);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                item->setOn(true);
                ++it;
            }
            break;
        }
        case 15:
        {
            QListViewItemIterator it(d->tagsView, QListViewItemIterator::Checked);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                item->setOn(false);
                ++it;
            }
            break;
        }
        case 16:
        {
            QListViewItemIterator it(d->tagsView);
            while (it.current())
            {
                TAlbumCheckListItem* item = dynamic_cast<TAlbumCheckListItem*>(it.current());
                TAlbum *tag = item->m_album;
                if (tag)
                    if (!tag->isRoot())
                        item->setOn(!item->isOn());

                ++it;
            }
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

    if (!TagCreateDlg::tagCreate(kapp->activeWindow(), parAlbum, title, icon))
        return;

    QString errMsg;
    TAlbum* album = albumMan_->createTAlbum(parAlbum, title, icon, errMsg);

    if (!album)
    {
        KMessageBox::error(this, errMsg);
    }
    else
    {
        TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)album->extraData(this);
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

    if (!TagEditDlg::tagEdit(kapp->activeWindow(), album, title, icon))
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
        if (!albumMan->updateTAlbumIcon(album, icon, 0, errMsg))
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
            DWarning() << k_funcinfo << "Failed to find parent for Tag " << tag->title()
                        << endl;
            return;
        }

        viewItem = new TAlbumCheckListItem(parentItem, tag);
    }

    if (viewItem)
    {
        viewItem->setOpen(true);
        tag->setExtraData(this, viewItem);
        setTagThumbnail(tag);
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

void ImageDescEditTab::slotAlbumsCleared()
{
    d->tagsView->clear();
}

void ImageDescEditTab::slotAlbumIconChanged(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    setTagThumbnail((TAlbum *)a);
}

void ImageDescEditTab::slotAlbumRenamed(Album* a)
{
    if (!a || a->isRoot() || a->type() != Album::TAG)
        return;

    TAlbum* album = (TAlbum*)a;

    QCheckListItem* viewItem = (QCheckListItem*)(album->extraData(this));
    if (!viewItem)
    {
        DWarning() << "Failed to find view item for Tag "
                    << album->title() << endl;
        return;
    }

    viewItem->setText(0, album->title());
}

void ImageDescEditTab::setTagThumbnail(TAlbum *album)
{
    if(!album)
        return;

    QCheckListItem* item = (QCheckListItem*) album->extraData(this);

    if(!item)
        return;

    AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
    QPixmap icon;
    if (!loader->getTagThumbnail(album, icon))
    {
        if (icon.isNull())
        {
            item->setPixmap(0, loader->getStandardTagIcon(album, 20));
        }
        else
        {
            item->setPixmap(0, icon);
        }
    }
}

void ImageDescEditTab::slotGotThumbnailFromIcon(Album *album, const QPixmap& thumbnail)
{
    if(!album || album->type() != Album::TAG)
        return;

    // update item in tags tree
    QCheckListItem* item = (QCheckListItem*)album->extraData(this);

    if(!item)
        return;

    item->setPixmap(0, thumbnail);

    // update item in recent tags popup menu, if found therein
    QPopupMenu *menu = d->recentTagsBtn->popup();
    if (menu->indexOf(album->id()) != -1)
    {
        menu->changeItem(album->id(), thumbnail, menu->text(album->id()));
    }
}

void ImageDescEditTab::slotThumbnailLost(Album *)
{
    // we already set the standard icon before loading
}

void ImageDescEditTab::slotImageTagsChanged(Q_LLONG imageId)
{
    if (!d->ignoreImageAttributesWatch && d->currInfo && d->currInfo->id() == imageId)
        updateTagsView();
}

void ImageDescEditTab::slotImagesChanged(int albumId)
{
    Album *a = AlbumManager::instance()->findAlbum(albumId);
    if (!d->ignoreImageAttributesWatch && 
        !d->currInfo || !a || a->isRoot() || a->type() != Album::TAG)
        return;

    updateTagsView();
}

void ImageDescEditTab::slotImageRatingChanged(Q_LLONG imageId)
{
    if (!d->ignoreImageAttributesWatch && 
        d->currInfo && d->currInfo->id() == imageId)
        updateRating();
}

void ImageDescEditTab::slotImageCaptionChanged(Q_LLONG imageId)
{
    if (!d->ignoreImageAttributesWatch && 
        d->currInfo && d->currInfo->id() == imageId)
        updateComments();
}

void ImageDescEditTab::slotImageDateChanged(Q_LLONG imageId)
{
    if (!d->ignoreImageAttributesWatch && 
        d->currInfo && d->currInfo->id() == imageId)
        updateDate();
}

void ImageDescEditTab::updateRecentTags()
{
    QPopupMenu *menu = d->recentTagsBtn->popup();
    menu->clear();
    
    AlbumManager* albumMan = AlbumManager::instance();
    IntList recentTags     = albumMan->albumDB()->getRecentlyAssignedTags();

    if (recentTags.isEmpty())
    {
        menu->insertItem(i18n("No Recently Assigned Tags"), 0);
        menu->setItemEnabled(0, false);
    }
    else
    {
        for (IntList::const_iterator it = recentTags.begin();
             it != recentTags.end(); ++it)
        {
            TAlbum* album = albumMan->findTAlbum(*it);
            if (album)
            {
                AlbumThumbnailLoader *loader = AlbumThumbnailLoader::instance();
                QPixmap icon;
                if (!loader->getTagThumbnail(album, icon))
                {
                    if (icon.isNull())
                    {
                        icon = loader->getStandardTagIcon(album, 20);
                    }
                }
                QString text = album->title() + " (" + ((TAlbum*)album->parent())->prettyURL() + ")";
                menu->insertItem(icon, text, album->id());
            }
        }
    }
}

void ImageDescEditTab::slotRecentTagsMenuActivated(int id)
    
{
    AlbumManager* albumMan = AlbumManager::instance();
    
    if (id > 0)
    {
        TAlbum* album = albumMan->findTAlbum(id);
        if (album)
        {
            TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)album->extraData(this);
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

        TAlbumCheckListItem* viewItem = (TAlbumCheckListItem*)(tag->extraData(this));

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
        TAlbumCheckListItem* rootItem = (TAlbumCheckListItem*)(root->extraData(this));
        if (rootItem)
            rootItem->setText(0, root->title());
    }
    else
    {
        TAlbum* root = AlbumManager::instance()->findTAlbum(0);
        TAlbumCheckListItem* rootItem = (TAlbumCheckListItem*)(root->extraData(this));
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

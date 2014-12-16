/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : a dialog to delete item.
 *
 * Copyright (C) 2004      by Michael Pyne <michael.pyne@kdemail.net>
 * Copyright (C) 2006      by Ian Monroe <ian@monroe.nu>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008-2012 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "deletedialog.moc"

// Qt includes

#include <QStackedWidget>
#include <QPainter>
#include <QLayout>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>
#include <QKeyEvent>
#include <QHeaderView>

// KDE includes

#include <kiconloader.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kstdguiitem.h>
#include <kpushbutton.h>
#include <kdebug.h>

// Local includes

#include "applicationsettings.h"
#include "databaseurl.h"

namespace Digikam
{

class DeleteItem::Private
{

public:

    Private()
    {
        hasThumb = false;
    }

    bool hasThumb;

    KUrl url;
};

DeleteItem::DeleteItem(QTreeWidget* const parent, const KUrl& url)
    : QTreeWidgetItem(parent), d(new Private)
{
    d->url = url;

    if (d->url.protocol() == "digikamalbums")
    {
        if (DatabaseUrl(d->url).isAlbumUrl())
        {
            setThumb(SmallIcon("folder", parent->iconSize().width()));
        }
        else
        {
            setThumb(SmallIcon("tag", parent->iconSize().width()));
        }
    }
    else
    {
        setThumb(SmallIcon("image-x-generic", parent->iconSize().width(), KIconLoader::DisabledState), false);
    }

    setText(1, fileUrl());
}

DeleteItem::~DeleteItem()
{
    delete d;
}

bool DeleteItem::hasValidThumbnail() const
{
    return d->hasThumb;
}

KUrl DeleteItem::url() const
{
    return d->url;
}

QString DeleteItem::fileUrl() const
{
    if (d->url.isLocalFile())
    {
        return (d->url.toLocalFile());
    }
    else if (d->url.protocol() == "digikamalbums")
    {
        return (DatabaseUrl(d->url).fileUrl().toLocalFile());
    }

    return (d->url.prettyUrl());
}

void DeleteItem::setThumb(const QPixmap& pix, bool hasThumb)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize+2, iconSize+2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2)  - (pix.width()/2),
                 (pixmap.height()/2) - (pix.height()/2), pix);

    QIcon icon = QIcon(pixmap);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);

    d->hasThumb = hasThumb;
}

//----------------------------------------------------------------------------

class DeleteItemList::Private
{

public:

    Private()
        : iconSize(64)
    {
        thumbLoadThread = 0;
    }

    const int            iconSize;

    ThumbnailLoadThread* thumbLoadThread;
};

DeleteItemList::DeleteItemList(QWidget* const parent)
    : QTreeWidget(parent), d(new Private)
{
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    setRootIsDecorated(false);
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAllColumnsShowFocus(true);
    setIconSize(QSize(d->iconSize, d->iconSize));
    setColumnCount(2);
    setHeaderLabels(QStringList() << i18n("Thumb") << i18n("Path"));
    header()->setResizeMode(0, QHeaderView::ResizeToContents);
    header()->setResizeMode(1, QHeaderView::Stretch);
    setToolTip(i18n("List of items that are about to be deleted."));
    setWhatsThis(i18n("This is the list of items that are about to be deleted."));

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(LoadingDescription,QPixmap)),
            this, SLOT(slotThumbnailLoaded(LoadingDescription,QPixmap)));
}

DeleteItemList::~DeleteItemList()
{
    delete d;
}

void DeleteItemList::slotThumbnailLoaded(const LoadingDescription& desc, const QPixmap& pix)
{
    QTreeWidgetItemIterator it(this);

    while (*it)
    {
        DeleteItem* item = dynamic_cast<DeleteItem*>(*it);

        if (item && item->fileUrl() == desc.filePath)
        {
            if (!pix.isNull())
            {
                item->setThumb(pix.scaled(d->iconSize, d->iconSize, Qt::KeepAspectRatio));
            }
            return;
        }

        ++it;
    }
}

void DeleteItemList::drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const
{
    DeleteItem* item = dynamic_cast<DeleteItem*>(itemFromIndex(index));

    if (item && !item->hasValidThumbnail())
    {
        d->thumbLoadThread->find(ThumbnailIdentifier(item->fileUrl()));
    }

    QTreeWidget::drawRow(p, opt, index);
}

//----------------------------------------------------------------------------

class DeleteWidget::Private
{
public:

    Private()
    {
        checkBoxStack   = 0;
        warningIcon     = 0;
        deleteText      = 0;
        numFiles        = 0;
        shouldDelete    = 0;
        doNotShowAgain  = 0;
        fileList        = 0;
        listMode        = DeleteDialogMode::Files;
        deleteMode      = DeleteDialogMode::UseTrash;
    }

    QStackedWidget*              checkBoxStack;

    QLabel*                      warningIcon;
    QLabel*                      deleteText;
    QLabel*                      numFiles;

    QCheckBox*                   shouldDelete;
    QCheckBox*                   doNotShowAgain;

    QTreeWidget*                 fileList;

    DeleteDialogMode::ListMode   listMode;
    DeleteDialogMode::DeleteMode deleteMode;
};

DeleteWidget::DeleteWidget(QWidget* const parent)
    : QWidget(parent), d(new Private)
{
    setObjectName("DeleteDialogBase");

    resize(540, 370);
    setMinimumSize(QSize(420, 320));

    d->checkBoxStack = new QStackedWidget(this);
    QLabel* logo     = new QLabel(this);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    d->warningIcon   = new QLabel(this);
    d->warningIcon->setWordWrap(false);

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHeightForWidth(d->warningIcon->sizePolicy().hasHeightForWidth());
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    d->warningIcon->setSizePolicy(sizePolicy);

    d->deleteText     = new QLabel(this);
    d->deleteText->setAlignment(Qt::AlignCenter);
    d->deleteText->setWordWrap(true);

    QHBoxLayout* hbox = new QHBoxLayout();
    hbox->setSpacing(KDialog::spacingHint());
    hbox->setMargin(0);
    hbox->addWidget(logo);
    hbox->addWidget(d->deleteText, 10);
    hbox->addWidget(d->warningIcon);

    d->fileList       = new DeleteItemList(this);
    d->numFiles       = new QLabel(this);
    d->numFiles->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    d->numFiles->setWordWrap(false);

    d->shouldDelete   = new QCheckBox(d->checkBoxStack);
    d->shouldDelete->setGeometry(QRect(0, 0, 542, 32));
    d->shouldDelete->setToolTip(i18n("If checked, files will be permanently removed instead of being placed "
                                    "in the Trash."));
    d->shouldDelete->setWhatsThis(i18n("<p>If this box is checked, items will be "
                                      "<b>permanently removed</b> instead of "
                                      "being placed in the Trash.</p>"
                                      "<p><em>Use this option with caution</em>: most filesystems "
                                      "are unable to "
                                      "undelete deleted items reliably.</p>"));
    d->shouldDelete->setText(i18n("&Delete items instead of moving them to the trash"));

    connect(d->shouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));

    d->doNotShowAgain = new QCheckBox(d->checkBoxStack);
    d->doNotShowAgain->setGeometry(QRect(0, 0, 100, 30));
    d->doNotShowAgain->setText(i18n("Do not &ask again"));

    QVBoxLayout* vbox = new QVBoxLayout(this);
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(KDialog::spacingHint());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addLayout(hbox);
    vbox->addWidget(d->fileList, 10);
    vbox->addWidget(d->numFiles);
    vbox->addWidget(d->checkBoxStack);

    d->checkBoxStack->addWidget(d->shouldDelete);
    d->checkBoxStack->addWidget(d->doNotShowAgain);
    d->checkBoxStack->setCurrentWidget(d->shouldDelete);

    bool deleteInstead = !ApplicationSettings::instance()->getUseTrash();
    slotShouldDelete(deleteInstead);
    d->shouldDelete->setChecked(deleteInstead);
}

DeleteWidget::~DeleteWidget()
{
    delete d;
}

void DeleteWidget::setUrls(const KUrl::List& urls)
{
    d->fileList->clear();

    foreach(const KUrl& url, urls)
    {
        new DeleteItem(d->fileList, url);
    }

    updateText();
}

void DeleteWidget::slotShouldDelete(bool shouldDelete)
{
    setDeleteMode(shouldDelete ? DeleteDialogMode::DeletePermanently : DeleteDialogMode::UseTrash);
}

void DeleteWidget::setDeleteMode(DeleteDialogMode::DeleteMode deleteMode)
{
    d->deleteMode = deleteMode;
    updateText();
}

void DeleteWidget::setListMode(DeleteDialogMode::ListMode listMode)
{
    d->listMode = listMode;
    updateText();
}

void DeleteWidget::updateText()
{
    // set "do not ask again checkbox text
    if (d->deleteMode == DeleteDialogMode::DeletePermanently)
    {
        d->doNotShowAgain->setToolTip(i18n("If checked, this dialog will no longer be shown, and items will "
                                           "be directly and permanently deleted."));
        d->doNotShowAgain->setWhatsThis(i18n("If this box is checked, this dialog will no longer be shown, "
                                             "and items will be directly and permanently deleted."));
    }
    else
    {
        d->doNotShowAgain->setToolTip(i18n("If checked, this dialog will no longer be shown, and items will "
                                           "be directly moved to the Trash."));
        d->doNotShowAgain->setWhatsThis(i18n("If this box is checked, this dialog will no longer be shown, "
                                             "and items will be directly moved to the Trash."));
    }

    switch (d->listMode)
    {
        case DeleteDialogMode::Files:
        {
            // Delete files

            if (d->deleteMode == DeleteDialogMode::DeletePermanently)
            {
                d->deleteText->setText(i18n("These items will be <b>permanently "
                                            "deleted</b> from your hard disk."));
                d->warningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                          KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                d->deleteText->setText(i18n("These items will be moved to Trash."));
                d->warningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                          KIconLoader::Desktop, KIconLoader::SizeLarge));
                d->numFiles->setText(i18np("<b>1</b> item selected.", "<b>%1</b> items selected.",
                                           d->fileList->topLevelItemCount()));
            }

            break;
        }
        case DeleteDialogMode::Albums:
        {
            // Delete albums = folders

            if (d->deleteMode == DeleteDialogMode::DeletePermanently)
            {
                d->deleteText->setText(i18n("These albums will be <b>permanently "
                                            "deleted</b> from your hard disk."));
                d->warningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                          KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                d->deleteText->setText(i18n("These albums will be moved to Trash."));
                d->warningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                          KIconLoader::Desktop, KIconLoader::SizeLarge));
            }

            d->numFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                       d->fileList->topLevelItemCount()));
            break;
        }
        case DeleteDialogMode::Subalbums:
        {
            // As above, but display additional warning

            if (d->deleteMode == DeleteDialogMode::DeletePermanently)
            {
                d->deleteText->setText(i18n("<p>These albums will be <b>permanently "
                                            "deleted</b> from your hard disk.</p>"
                                            "<p>Note that <b>all subalbums</b> "
                                            "are included in this list and will "
                                            "be deleted permanently as well.</p>"));
                d->warningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                          KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                d->deleteText->setText(i18n("<p>These albums will be moved to Trash.</p>"
                                            "<p>Note that <b>all subalbums</b> "
                                            "are included in this list and will "
                                            "be moved to Trash as well.</p>"));
                d->warningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                          KIconLoader::Desktop, KIconLoader::SizeLarge));
            }

            d->numFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                       d->fileList->topLevelItemCount()));
            break;
        }
    }
}

//----------------------------------------------------------------------------

class DeleteDialog::Private
{
public:

    Private()
    {
        saveShouldDeleteUserPreference = true;
        saveDoNotShowAgainTrash        = false;
        saveDoNotShowAgainPermanent    = false;
        trashGuiItem                   = KGuiItem(i18n("&Move to Trash"), "user-trash-full");
        widget                         = 0;
    }

    bool          saveShouldDeleteUserPreference;
    bool          saveDoNotShowAgainTrash;
    bool          saveDoNotShowAgainPermanent;

    KGuiItem      trashGuiItem;

    DeleteWidget* widget;
};

DeleteDialog::DeleteDialog(QWidget* const parent)
    : KDialog(parent), d(new Private)
{
    setButtons(User1 | Cancel);
    setButtonFocus(User1);
    setModal(true);
    d->widget = new DeleteWidget(this);
    setMainWidget(d->widget);

    d->widget->setMinimumSize(400, 300);
    setMinimumSize(410, 326);
    adjustSize();

    slotShouldDelete(shouldDelete());

    connect(d->widget->d->shouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));

    connect(this, SIGNAL(user1Clicked()),
            this, SLOT(slotUser1Clicked()));
}

DeleteDialog::~DeleteDialog()
{
    delete d;
}

bool DeleteDialog::confirmDeleteList(const KUrl::List& condemnedFiles,
                                     DeleteDialogMode::ListMode listMode,
                                     DeleteDialogMode::DeleteMode deleteMode)
{
    setUrls(condemnedFiles);
    presetDeleteMode(deleteMode);
    setListMode(listMode);

    if (deleteMode == DeleteDialogMode::NoChoiceTrash)
    {
        if (!ApplicationSettings::instance()->getShowTrashDeleteDialog())
        {
            return true;
        }
    }
    else if (deleteMode == DeleteDialogMode::NoChoiceDeletePermanently)
    {
        if (!ApplicationSettings::instance()->getShowPermanentDeleteDialog())
        {
            return true;
        }
    }

    return exec() == QDialog::Accepted;
}

void DeleteDialog::setUrls(const KUrl::List& urls)
{
    d->widget->setUrls(urls);
}

void DeleteDialog::slotUser1Clicked()
{
    // Save user's preference
    ApplicationSettings* settings = ApplicationSettings::instance();

    if (d->saveShouldDeleteUserPreference)
    {
        settings->setUseTrash(!shouldDelete());
    }

    if (d->saveDoNotShowAgainTrash)
    {
        kDebug() << "setShowTrashDeleteDialog " << !d->widget->d->doNotShowAgain->isChecked();
        settings->setShowTrashDeleteDialog(!d->widget->d->doNotShowAgain->isChecked());
    }

    if (d->saveDoNotShowAgainPermanent)
    {
        kDebug() << "setShowPermanentDeleteDialog " << !d->widget->d->doNotShowAgain->isChecked();
        settings->setShowPermanentDeleteDialog(!d->widget->d->doNotShowAgain->isChecked());
    }

    settings->saveSettings();

    KDialog::accept();
}

bool DeleteDialog::shouldDelete() const
{
    return d->widget->d->shouldDelete->isChecked();
}

void DeleteDialog::slotShouldDelete(bool shouldDelete)
{
    // This is called once from constructor, and then when the user changed the checkbox state.
    // In that case, save the user's preference.
    d->saveShouldDeleteUserPreference = true;
    setButtonGuiItem(User1, shouldDelete ? KStandardGuiItem::del() : d->trashGuiItem);
}

void DeleteDialog::presetDeleteMode(DeleteDialogMode::DeleteMode mode)
{
    switch (mode)
    {
        case DeleteDialogMode::NoChoiceTrash:
        {
            // access the widget directly, signals will be fired to DeleteDialog and DeleteWidget
            d->widget->d->shouldDelete->setChecked(false);
            d->widget->d->checkBoxStack->setCurrentWidget(d->widget->d->doNotShowAgain);
            d->saveDoNotShowAgainTrash = true;
            break;
        }
        case DeleteDialogMode::NoChoiceDeletePermanently:
        {
            d->widget->d->shouldDelete->setChecked(true);
            d->widget->d->checkBoxStack->setCurrentWidget(d->widget->d->doNotShowAgain);
            d->saveDoNotShowAgainPermanent = true;
            //d->widget->d->checkBoxStack->hide();
            break;
        }
        case DeleteDialogMode::UserPreference:
        {
            break;
        }
        case DeleteDialogMode::UseTrash:
        case DeleteDialogMode::DeletePermanently:
        {
            // toggles signals which do the rest
            d->widget->d->shouldDelete->setChecked(mode == DeleteDialogMode::DeletePermanently);

            // the preference set by this preset method will be ignored
            // for the next DeleteDialog instance and not stored as user preference.
            // Only if the user once changes this value, it will be taken as user preference.
            d->saveShouldDeleteUserPreference = false;
            break;
        }
    }
}

void DeleteDialog::setListMode(DeleteDialogMode::ListMode mode)
{
    d->widget->setListMode(mode);

    switch (mode)
    {
        case DeleteDialogMode::Files:
            setCaption(i18n("About to delete selected items"));
            break;

        case DeleteDialogMode::Albums:
        case DeleteDialogMode::Subalbums:
            setCaption(i18n("About to delete selected albums"));
            break;
    }
}

void DeleteDialog::keyPressEvent(QKeyEvent* e)
{
    if ( e->modifiers() == 0 )
    {
        if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
        {
            if (button(User1)->hasFocus())
            {
                e->accept();
                button(User1)->animateClick();
                return;
            }
            else if (button(Cancel)->hasFocus())
            {
                e->accept();
                button(Cancel)->animateClick();
                return;
            }
        }
    }

    KDialog::keyPressEvent(e);
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-07
 * Description : a dialog to delete item.
 *
 * Copyright (C) 2004 by Michael Pyne <michael.pyne@kdemail.net>
 * Copyright (C) 2006 by Ian Monroe <ian@monroe.nu>
 * Copyright (C) 2006-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "deletedialog.h"
#include "deletedialog.moc"

// Qt includes

#include <QStackedWidget>
#include <QLayout>
#include <QLabel>
#include <QTimer>
#include <QCheckBox>

// KDE includes

#include <kdebug.h>
#include <klistwidget.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <klocale.h>
#include <kstdguiitem.h>

// Local includes

#include "albumsettings.h"

namespace Digikam
{

DeleteWidget::DeleteWidget(QWidget *parent)
            : QWidget(parent),
              m_listMode(DeleteDialogMode::Files),
              m_deleteMode(DeleteDialogMode::UseTrash)
{
    setObjectName("DeleteDialogBase");

    resize(540, 370);
    setMinimumSize(QSize(420, 320));

    m_checkBoxStack = new QStackedWidget(this);

    QLabel *logo = new QLabel(this);
    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                            .scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    m_warningIcon = new QLabel(this);
    m_warningIcon->setWordWrap(false);

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHeightForWidth(m_warningIcon->sizePolicy().hasHeightForWidth());
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    m_warningIcon->setSizePolicy(sizePolicy);

    m_deleteText = new QLabel(this);
    m_deleteText->setAlignment(Qt::AlignCenter);
    m_deleteText->setWordWrap(true);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(KDialog::spacingHint());
    hbox->setMargin(0);
    hbox->addWidget(logo);
    hbox->addWidget(m_deleteText, 10);
    hbox->addWidget(m_warningIcon);

    m_fileList = new KListWidget(this);
    m_fileList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_fileList->setToolTip(i18n("List of files that are about to be deleted."));
    m_fileList->setWhatsThis(i18n("This is the list of items that are about to be deleted."));

    m_numFiles = new QLabel(this);
    m_numFiles->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    m_numFiles->setWordWrap(false);

    m_shouldDelete = new QCheckBox(m_checkBoxStack);
    m_shouldDelete->setGeometry(QRect(0, 0, 542, 32));
    m_shouldDelete->setToolTip(i18n("If checked, files will be permanently removed instead of being placed "
                                    "in the Trash."));
    m_shouldDelete->setWhatsThis(i18n("<p>If this box is checked, files will be "
                                      "<b>permanently removed</b> instead of "
                                      "being placed in the Trash.</p>"
                                      "<p><em>Use this option with caution</em>: most filesystems "
                                      "are unable to "
                                      "undelete deleted files reliably.</p>"));
    m_shouldDelete->setText(i18n("&Delete files instead of moving them to the trash"));

    connect(m_shouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));

    m_doNotShowAgain = new QCheckBox(m_checkBoxStack);
    m_doNotShowAgain->setGeometry(QRect(0, 0, 100, 30));
    m_doNotShowAgain->setToolTip(i18n("If checked, this dialog will no longer be shown, and files will "
                                      "be directly moved to the Trash."));
    m_doNotShowAgain->setWhatsThis(i18n("If this box is checked, this dialog will no longer be shown, "
                                        "and files will be directly moved to the Trash."));
    m_doNotShowAgain->setText(i18n("Do not &ask again"));

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(KDialog::spacingHint());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addLayout(hbox);
    vbox->addWidget(m_fileList, 10);
    vbox->addWidget(m_numFiles);
    vbox->addWidget(m_checkBoxStack);

    m_checkBoxStack->addWidget(m_shouldDelete);
    m_checkBoxStack->addWidget(m_doNotShowAgain);
    m_checkBoxStack->setCurrentWidget(m_shouldDelete);

    bool deleteInstead = !AlbumSettings::instance()->getUseTrash();
    slotShouldDelete(deleteInstead);
    m_shouldDelete->setChecked(deleteInstead);
}

void DeleteWidget::setFiles(const KUrl::List& files)
{
    m_fileList->clear();
    for( KUrl::List::ConstIterator it = files.begin(); it != files.end(); ++it)
    {
        if( (*it).isLocalFile() ) //path is null for non-local
            m_fileList->addItem( (*it).toLocalFile() );
        else if ( (*it).protocol() == "digikamalbums")
            m_fileList->addItem( (*it).path() );
        else
            m_fileList->addItem( (*it).prettyUrl() );
    }
    updateText();
}

void DeleteWidget::slotShouldDelete(bool shouldDelete)
{
    setDeleteMode(shouldDelete ? DeleteDialogMode::DeletePermanently : DeleteDialogMode::UseTrash);
}

void DeleteWidget::setDeleteMode(DeleteDialogMode::DeleteMode deleteMode)
{
    m_deleteMode = deleteMode;
    updateText();
}

void DeleteWidget::setListMode(DeleteDialogMode::ListMode listMode)
{
    m_listMode = listMode;
    updateText();
}

void DeleteWidget::updateText()
{
    switch (m_listMode)
    {
        case DeleteDialogMode::Files:
        {
            // Delete files

            if (m_deleteMode == DeleteDialogMode::DeletePermanently)
            {
                m_deleteText->setText(i18n("These items will be <b>permanently "
                                           "deleted</b> from your hard disk."));
                m_warningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                m_deleteText->setText(i18n("These items will be moved to Trash."));
                m_warningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            m_numFiles->setText(i18np("<b>1</b> file selected.", "<b>%1</b> files selected.",
                                      m_fileList->count()));
            break;
        }
        case DeleteDialogMode::Albums:
        {
            // Delete albums = folders

            if (m_deleteMode == DeleteDialogMode::DeletePermanently)
            {
                m_deleteText->setText(i18n("These albums will be <b>permanently "
                                           "deleted</b> from your hard disk."));
                m_warningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                m_deleteText->setText(i18n("These albums will be moved to Trash."));
                m_warningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            m_numFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                m_fileList->count()));
            break;
        }
        case DeleteDialogMode::Subalbums:
        {
            // As above, but display additional warning

            if (m_deleteMode == DeleteDialogMode::DeletePermanently)
            {
                m_deleteText->setText(i18n("<p>These albums will be <b>permanently "
                                           "deleted</b> from your hard disk.</p>"
                                           "<p>Note that <b>all subalbums</b> "
                                           "are included in this list and will "
                                           "be deleted permanently as well.</p>"));
                m_warningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                m_deleteText->setText(i18n("<p>These albums will be moved to Trash.</p>"
                                           "<p>Note that <b>all subalbums</b> "
                                           "are included in this list and will "
                                           "be moved to Trash as well.</p>"));
                m_warningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            m_numFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                m_fileList->count()));
            break;
        }
    }
}

//----------------------------------------------------------------------------

DeleteDialog::DeleteDialog(QWidget *parent)
            : KDialog(parent),
              m_saveShouldDeleteUserPreference(true),
              m_saveDoNotShowAgain(false),
              m_trashGuiItem(i18n("&Move to Trash"), "user-trash-full")
{
    setButtons(Ok | Cancel);
    setDefaultButton(Cancel);
    setModal(true);
    m_widget = new DeleteWidget(this);
    setMainWidget(m_widget);

    m_widget->setMinimumSize(400, 300);
    setMinimumSize(410, 326);
    adjustSize();

    slotShouldDelete(shouldDelete());

    connect(m_widget->m_shouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));
}

bool DeleteDialog::confirmDeleteList(const KUrl::List& condemnedFiles,
                                     DeleteDialogMode::ListMode listMode,
                                     DeleteDialogMode::DeleteMode deleteMode)
{
    setURLs(condemnedFiles);
    presetDeleteMode(deleteMode);
    setListMode(listMode);

    if (deleteMode == DeleteDialogMode::NoChoiceTrash)
    {
        if (!AlbumSettings::instance()->getShowTrashDeleteDialog())
            return true;
    }
    return exec() == QDialog::Accepted;
}

void DeleteDialog::setURLs(const KUrl::List& files)
{
    m_widget->setFiles(files);
}

void DeleteDialog::accept()
{
    // Save user's preference
    AlbumSettings *settings = AlbumSettings::instance();

    if (m_saveShouldDeleteUserPreference)
    {
        settings->setUseTrash(!shouldDelete());
    }
    if (m_saveDoNotShowAgain)
    {
        kDebug(50003) << "setShowTrashDeleteDialog " << !m_widget->m_doNotShowAgain->isChecked();
        settings->setShowTrashDeleteDialog(!m_widget->m_doNotShowAgain->isChecked());
    }

    settings->saveSettings();

    KDialog::accept();
}

bool DeleteDialog::shouldDelete() const
{
    return m_widget->m_shouldDelete->isChecked();
}

void DeleteDialog::slotShouldDelete(bool shouldDelete)
{
    // This is called once from constructor, and then when the user changed the checkbox state.
    // In that case, save the user's preference.
    m_saveShouldDeleteUserPreference = true;
    setButtonGuiItem(Ok, shouldDelete ? KStandardGuiItem::del() : m_trashGuiItem);
}

void DeleteDialog::presetDeleteMode(DeleteDialogMode::DeleteMode mode)
{
    switch (mode)
    {
        case DeleteDialogMode::NoChoiceTrash:
        {
            // access the widget directly, signals will be fired to DeleteDialog and DeleteWidget
            m_widget->m_shouldDelete->setChecked(false);
            m_widget->m_checkBoxStack->setCurrentWidget(m_widget->m_doNotShowAgain);
            m_saveDoNotShowAgain = true;
            break;
        }
        case DeleteDialogMode::NoChoiceDeletePermanently:
        {
            m_widget->m_shouldDelete->setChecked(true);
            m_widget->m_checkBoxStack->hide();
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
            m_widget->m_shouldDelete->setChecked(mode == DeleteDialogMode::DeletePermanently);

            // the preference set by this preset method will be ignored
            // for the next DeleteDialog instance and not stored as user preference.
            // Only if the user once changes this value, it will be taken as user preference.
            m_saveShouldDeleteUserPreference = false;
            break;
        }
    }
}

void DeleteDialog::setListMode(DeleteDialogMode::ListMode mode)
{
    m_widget->setListMode(mode);
    switch (mode)
    {
        case DeleteDialogMode::Files:
            setCaption(i18n("About to delete selected files"));
            break;

        case DeleteDialogMode::Albums:
        case DeleteDialogMode::Subalbums:
            setCaption(i18n("About to delete selected albums"));
            break;
    }
}

} // namespace Digikam

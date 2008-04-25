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
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include <QStackedWidget>
#include <QStringList>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>

// KDE includes.

#include "k3listbox.h"
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstdguiitem.h>

// Local includes.

#include "ddebug.h"
#include "albumsettings.h"
#include "deletedialog.h"
#include "deletedialog.moc"

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

    ddCheckBoxStack = new QStackedWidget(this);

    ddWarningIcon = new QLabel(this);
    ddWarningIcon->setWordWrap(false);

    QSizePolicy sizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    sizePolicy.setHeightForWidth(ddWarningIcon->sizePolicy().hasHeightForWidth());
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    ddWarningIcon->setSizePolicy(sizePolicy);

    ddDeleteText = new QLabel(this);
    ddDeleteText->setAlignment(Qt::AlignCenter);
    ddDeleteText->setWordWrap(true);

    QVBoxLayout *vbox2 = new QVBoxLayout();
    vbox2->setSpacing(KDialog::spacingHint());
    vbox2->addWidget(ddDeleteText);

    QHBoxLayout *hbox = new QHBoxLayout();
    hbox->setSpacing(KDialog::spacingHint());
    hbox->addWidget(ddWarningIcon);
    hbox->addLayout(vbox2);

    ddFileList = new K3ListBox(this);
    ddFileList->setSelectionMode(Q3ListBox::Single);
    ddFileList->setToolTip(i18n("List of files that are about to be deleted."));
    ddFileList->setWhatsThis(i18n("This is the list of items that are about to be deleted."));

    ddNumFiles = new QLabel(this);
    ddNumFiles->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
    ddNumFiles->setWordWrap(false);

    ddShouldDelete = new QCheckBox(ddCheckBoxStack);
    ddShouldDelete->setGeometry(QRect(0, 0, 542, 32));
    ddShouldDelete->setToolTip(i18n("If checked, files will be permanently removed instead of being placed "
                                    "in the Trash Bin"));
    ddShouldDelete->setWhatsThis(i18n("<qt><p>If this box is checked, files will be "
                                      "<b>permanently removed</b> instead of "
                                      "being placed in the Trash Bin.</p>\n"
                                      "    \n"
                                      "    <p><em>Use this option with caution</em>: most filesystems "
                                      "are unable to "
                                      "undelete deleted files reliably.</p></qt>"));
    ddShouldDelete->setText(i18n("&Delete files instead of moving them to the trash"));

    connect(ddShouldDelete, SIGNAL(toggled(bool)), 
            this, SLOT(slotShouldDelete(bool)));

    ddDoNotShowAgain = new QCheckBox(ddCheckBoxStack);
    ddDoNotShowAgain->setGeometry(QRect(0, 0, 100, 30));
    ddDoNotShowAgain->setToolTip(i18n("If checked, this dialog will no longer be shown, and files will "
                                      "be directly moved to the Trash Bin"));
    ddDoNotShowAgain->setWhatsThis(i18n("<qt><p>If this box is checked, this dialog will no longer be shown, "
                                        "and files will be directly moved to the Trash Bin</p>"));
    ddDoNotShowAgain->setText(i18n("Do not &ask again"));

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->setSpacing(KDialog::spacingHint());
    vbox->setMargin(KDialog::spacingHint());
    vbox->setContentsMargins(0, 0, 0, 0);
    vbox->addLayout(hbox);
    vbox->addWidget(ddFileList, 10);
    vbox->addWidget(ddNumFiles);
    vbox->addWidget(ddCheckBoxStack);

    ddCheckBoxStack->addWidget(ddShouldDelete);
    ddCheckBoxStack->addWidget(ddDoNotShowAgain);
    ddCheckBoxStack->setCurrentWidget(ddShouldDelete);

    bool deleteInstead = !AlbumSettings::instance()->getUseTrash();
    slotShouldDelete(deleteInstead);
    ddShouldDelete->setChecked(deleteInstead);
}

void DeleteWidget::setFiles(const KUrl::List &files)
{
    ddFileList->clear();
    for( KUrl::List::ConstIterator it = files.begin(); it != files.end(); it++)
    {
        if( (*it).isLocalFile() ) //path is nil for non-local
            ddFileList->insertItem( (*it).path() );
        else if ( (*it).protocol() == "digikamalbums")
            ddFileList->insertItem( (*it).path() );
        else
            ddFileList->insertItem( (*it).prettyUrl() );
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
                ddDeleteText->setText(i18n("<qt>These items will be <b>permanently "
                                           "deleted</b> from your hard disk.</qt>"));
                ddWarningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                ddDeleteText->setText(i18n("<qt>These items will be moved to Trash.</qt>"));
                ddWarningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            ddNumFiles->setText(i18np("<b>1</b> file selected.", "<b>%1</b> files selected.",
                                      ddFileList->count()));
            break;
        }
        case DeleteDialogMode::Albums:
        {
            // Delete albums = folders

            if (m_deleteMode == DeleteDialogMode::DeletePermanently)
            {
                ddDeleteText->setText(i18n("<qt>These albums will be <b>permanently "
                                           "deleted</b> from your hard disk.</qt>"));
                ddWarningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                ddDeleteText->setText(i18n("<qt>These albums will be moved to Trash.</qt>"));
                ddWarningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            ddNumFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                ddFileList->count()));
            break;
        }
        case DeleteDialogMode::Subalbums:
        {
            // As above, but display additional warning

            if (m_deleteMode == DeleteDialogMode::DeletePermanently)
            {
                ddDeleteText->setText(i18n("<qt>These albums will be <b>permanently "
                                           "deleted</b> from your hard disk.<br>"
                                           "Note that <b>all subalbums</b> "
                                           "are included in this list and will "
                                           "be deleted permanently as well.</qt>"));
                ddWarningIcon->setPixmap(KIconLoader::global()->loadIcon("dialog-warning",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            else
            {
                ddDeleteText->setText(i18n("<qt>These albums will be moved to Trash.<br>"
                                           "Note that <b>all subalbums</b> "
                                           "are included in this list and will "
                                           "be moved to Trash as well.</qt>"));
                ddWarningIcon->setPixmap(KIconLoader::global()->loadIcon("user-trash-full",
                                         KIconLoader::Desktop, KIconLoader::SizeLarge));
            }
            ddNumFiles->setText(i18np("<b>1</b> album selected.", "<b>%1</b> albums selected.",
                                ddFileList->count()));
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
    setDefaultButton(Ok);
    setModal(true);
    m_widget = new DeleteWidget(this);
    setMainWidget(m_widget);

    m_widget->setMinimumSize(400, 300);
    setMinimumSize(410, 326);
    adjustSize();

    slotShouldDelete(shouldDelete());

    connect(m_widget->ddShouldDelete, SIGNAL(toggled(bool)), 
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

void DeleteDialog::setURLs(const KUrl::List &files)
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
        DDebug() << "setShowTrashDeleteDialog " << !m_widget->ddDoNotShowAgain->isChecked() << endl;
        settings->setShowTrashDeleteDialog(!m_widget->ddDoNotShowAgain->isChecked());
    }

    settings->saveSettings();

    KDialog::accept();
}

bool DeleteDialog::shouldDelete() const 
{ 
    return m_widget->ddShouldDelete->isChecked(); 
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
            m_widget->ddShouldDelete->setChecked(false);
            m_widget->ddCheckBoxStack->setCurrentWidget(m_widget->ddDoNotShowAgain);
            m_saveDoNotShowAgain = true;
            break;
        }
        case DeleteDialogMode::NoChoiceDeletePermanently:
        {
            m_widget->ddShouldDelete->setChecked(true);
            m_widget->ddCheckBoxStack->hide();
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
            m_widget->ddShouldDelete->setChecked(mode == DeleteDialogMode::DeletePermanently);

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

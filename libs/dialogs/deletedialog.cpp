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
 * Copyright (C) 2006-2007 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#include <qstringlist.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qvbox.h>
#include <qhbox.h>
#include <qwidgetstack.h>

// KDE includes.

#include <kconfig.h>
#include <kdeversion.h>
#include <kdialogbase.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kstdguiitem.h>

// Local includes.

#include "deletedialog.h"
#include "albumsettings.h"
#include "deletedialog.moc"

namespace Digikam
{

//////////////////////////////////////////////////////////////////////////////
// DeleteWidget implementation
//////////////////////////////////////////////////////////////////////////////

DeleteWidget::DeleteWidget(QWidget *parent, const char *name)
            : DeleteDialogBase(parent, name),
              m_listMode(DeleteDialogMode::Files),
              m_deleteMode(DeleteDialogMode::UseTrash)
{
    ddCheckBoxStack->raiseWidget(ddShouldDelete);

    bool deleteInstead = !AlbumSettings::instance()->getUseTrash();
    slotShouldDelete(deleteInstead);
    ddShouldDelete->setChecked(deleteInstead);
}

void DeleteWidget::setFiles(const KURL::List &files)
{
    ddFileList->clear();
    for( KURL::List::ConstIterator it = files.begin(); it != files.end(); it++)
    {
        if( (*it).isLocalFile() ) //path is nil for non-local
            ddFileList->insertItem( (*it).path() );
        else if ( (*it).protocol() == "digikamalbums")
            ddFileList->insertItem( (*it).path() );
        else
            ddFileList->insertItem( (*it).prettyURL() );
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

        // Delete files

        if (m_deleteMode == DeleteDialogMode::DeletePermanently)
        {
            ddDeleteText->setText(i18n("<qt>These items will be <b>permanently "
                                       "deleted</b> from your hard disk.</qt>"));
            ddWarningIcon->setPixmap(KGlobal::iconLoader()->loadIcon("messagebox_warning",
                KIcon::Desktop, KIcon::SizeLarge));
        }
        else
        {
            ddDeleteText->setText(i18n("<qt>These items will be moved to Trash.</qt>"));
            ddWarningIcon->setPixmap(KGlobal::iconLoader()->loadIcon("trashcan_full",
                KIcon::Desktop, KIcon::SizeLarge));
        }
        ddNumFiles->setText(i18n("<b>1</b> file selected.", "<b>%n</b> files selected.", ddFileList->count()));
        break;

        case DeleteDialogMode::Albums:

        // Delete albums = folders

        if (m_deleteMode == DeleteDialogMode::DeletePermanently)
        {
            ddDeleteText->setText(i18n("<qt>These albums will be <b>permanently "
                                       "deleted</b> from your hard disk.</qt>"));
            ddWarningIcon->setPixmap(KGlobal::iconLoader()->loadIcon("messagebox_warning",
                                     KIcon::Desktop, KIcon::SizeLarge));
        }
        else
        {
            ddDeleteText->setText(i18n("<qt>These albums will be moved to Trash.</qt>"));
            ddWarningIcon->setPixmap(KGlobal::iconLoader()->loadIcon("trashcan_full",
                                     KIcon::Desktop, KIcon::SizeLarge));
        }
        ddNumFiles->setText(i18n("<b>1</b> album selected.", "<b>%n</b> albums selected.", ddFileList->count()));
        break;

        case DeleteDialogMode::Subalbums:

        // As above, but display additional warning

        if (m_deleteMode == DeleteDialogMode::DeletePermanently)
        {
            ddDeleteText->setText(i18n("<qt>These albums will be <b>permanently "
                                       "deleted</b> from your hard disk.<br>"
                                       "Note that <b>all subalbums</b> "
                                       "are included in this list and will "
                                       "be deleted permanently as well.</qt>"));
            ddWarningIcon->setPixmap(KGlobal::iconLoader()->loadIcon("messagebox_warning",
                                     KIcon::Desktop, KIcon::SizeLarge));
        }
        else
        {
            ddDeleteText->setText(i18n("<qt>These albums will be moved to Trash.<br>"
                                       "Note that <b>all subalbums</b> "
                                       "are included in this list and will "
                                       "be moved to Trash as well.</qt>"));
            ddWarningIcon->setPixmap(KGlobal::iconLoader()->loadIcon("trashcan_full",
                                     KIcon::Desktop, KIcon::SizeLarge));
        }
        ddNumFiles->setText(i18n("<b>1</b> album selected.", "<b>%n</b> albums selected.", ddFileList->count()));
        break;

    }
}

//////////////////////////////////////////////////////////////////////////////
// DeleteDialog implementation
//////////////////////////////////////////////////////////////////////////////

DeleteDialog::DeleteDialog(QWidget *parent, const char *name) 
            : KDialogBase(Swallow, WStyle_DialogBorder, parent, name,
                          true, // modal
                          i18n("About to delete selected files"), // caption
                          Ok | Cancel, // available buttons
                          Ok,  // default button
                          true // use separator between buttons and the main widget
                         ),
              m_saveShouldDeleteUserPreference(true),
              m_saveDoNotShowAgain(false),
              m_trashGuiItem(i18n("&Move to Trash"), "trashcan_full")
{
    m_widget = new DeleteWidget(this, "delete_dialog_widget");
    setMainWidget(m_widget);

    m_widget->setMinimumSize(400, 300);
    setMinimumSize(410, 326);
    adjustSize();

    slotShouldDelete(shouldDelete());
    connect(m_widget->ddShouldDelete, SIGNAL(toggled(bool)),
            this, SLOT(slotShouldDelete(bool)));

    actionButton(Ok)->setFocus();
}

bool DeleteDialog::confirmDeleteList(const KURL::List& condemnedFiles,
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

void DeleteDialog::setURLs(const KURL::List &files)
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
        settings->setShowTrashDeleteDialog(!m_widget->ddDoNotShowAgain->isChecked());
    }

    settings->saveSettings();

    KDialogBase::accept();
}

void DeleteDialog::slotShouldDelete(bool shouldDelete)
{
    // This is called once from constructor, and then when the user changed the checkbox state.
    // In that case, save the user's preference.
    m_saveShouldDeleteUserPreference = true;
    setButtonGuiItem(Ok, shouldDelete ? KStdGuiItem::del() : m_trashGuiItem);
}

void DeleteDialog::presetDeleteMode(DeleteDialogMode::DeleteMode mode)
{
    switch (mode)
    {
        case DeleteDialogMode::NoChoiceTrash:
        {
            // access the widget directly, signals will be fired to DeleteDialog and DeleteWidget
            m_widget->ddShouldDelete->setChecked(false);
            m_widget->ddCheckBoxStack->raiseWidget(m_widget->ddDoNotShowAgain);
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

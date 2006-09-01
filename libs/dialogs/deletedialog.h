/***************************************************************************
    begin                : Tue Aug 31 21:54:20 EST 2004
    copyright            : (C) 2004 by Michael Pyne <michael.pyne@kdemail.net>
                           (C) 2006 by Ian Monroe <ian@monroe.nu>
                           (C) 2006 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _DELETEDIALOG_H
#define _DELETEDIALOG_H

// Qt includes.

#include <qcheckbox.h>

// KDE includes.

#include <kdialogbase.h>
#include <kurl.h>

// Local includes.

#include "digikam_export.h"
#include "deletedialogbase.h"

class QStringList;
class KListBox;
class KGuiItem;
class QLabel;
class QWidgetStack;

namespace Digikam
{

namespace DeleteDialogMode
{
    enum ListMode
    {
        Files,
        Albums,
        Subalbums
    };

    enum DeleteMode
    {
        NoChoiceTrash, // "Do not show again" checkbox, does not show if config entry is set
        NoChoiceDeletePermanently, // No checkbox
        UserPreference, // Checkbox to toggle trash/permanent, preset to user's last preference
        UseTrash, // same beckbox as above, preset to trash
        DeletePermanently // same checkbox as above, preset to permanent
    };
}

class DeleteWidget : public DeleteDialogBase
{
    Q_OBJECT

public:

    DeleteWidget(QWidget *parent = 0, const char *name = 0);

    void setFiles(const KURL::List &files);
    void setListMode(DeleteDialogMode::ListMode mode);
    void setDeleteMode(DeleteDialogMode::DeleteMode deleteMode);

protected slots:

    void slotShouldDelete(bool shouldDelete);

protected:

    void updateText();
    DeleteDialogMode::ListMode m_listMode;
    DeleteDialogMode::DeleteMode m_deleteMode;
};

class DIGIKAM_EXPORT DeleteDialog : public KDialogBase
{
    Q_OBJECT

public:
    enum Mode
    {
        ModeFiles,
        ModeAlbums,
        ModeSubalbums
    };

    DeleteDialog(QWidget *parent, const char *name = "delete_dialog");

    bool confirmDeleteList(const KURL::List &condemnedURLs,
                           DeleteDialogMode::ListMode listMode,
                           DeleteDialogMode::DeleteMode deleteMode);
    bool shouldDelete() const { return m_widget->ddShouldDelete->isChecked(); }

    void setURLs(const KURL::List &files);
    void presetDeleteMode(DeleteDialogMode::DeleteMode mode);
    void setListMode(DeleteDialogMode::ListMode mode);

protected slots:
    virtual void accept();
    void slotShouldDelete(bool shouldDelete);

private:
    DeleteWidget *m_widget;
    KGuiItem m_trashGuiItem;
    bool m_saveShouldDeleteUserPreference;
};

}

#endif

// vim: set et ts=4 sw=4:

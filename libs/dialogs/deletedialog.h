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
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DELETEDIALOG_H
#define DELETEDIALOG_H

// Qt includes

#include <QWidget>

// KDE includes

#include <kdialog.h>
#include <kurl.h>

// Local includes

#include "digikam_export.h"

class QStackedWidget;
class QLabel;
class QCheckBox;

class KListWidget;

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
    NoChoiceTrash,             // "Do not show again" checkbox, does not show if config entry is set
    NoChoiceDeletePermanently, // same as above
    UserPreference,            // Checkbox to toggle trash/permanent, preset to user's last preference
    UseTrash,                  // same checkbox as above, preset to trash
    DeletePermanently          // same checkbox as above, preset to permanent
};
}

// -----------------------------------------------------------

class DeleteWidget : public QWidget
{
    Q_OBJECT

public:

    DeleteWidget(QWidget* parent = 0);
    virtual ~DeleteWidget() {};

    void setFiles(const KUrl::List& files);
    void setListMode(DeleteDialogMode::ListMode mode);
    void setDeleteMode(DeleteDialogMode::DeleteMode deleteMode);

protected Q_SLOTS:

    void slotShouldDelete(bool);

protected:

    void updateText();

protected:

    QStackedWidget*               m_checkBoxStack;

    QLabel*                       m_warningIcon;
    QLabel*                       m_deleteText;
    QLabel*                       m_numFiles;

    QCheckBox*                    m_shouldDelete;
    QCheckBox*                    m_doNotShowAgain;

    KListWidget*                  m_fileList;

    DeleteDialogMode::ListMode    m_listMode;
    DeleteDialogMode::DeleteMode  m_deleteMode;

    friend class DeleteDialog;
};

// -----------------------------------------------------------

class DeleteDialog : public KDialog
{
    Q_OBJECT

public:

    enum Mode
    {
        ModeFiles,
        ModeAlbums,
        ModeSubalbums
    };

public:

    DeleteDialog(QWidget* parent);
    virtual ~DeleteDialog();

    bool confirmDeleteList(const KUrl::List& condemnedURLs,
                           DeleteDialogMode::ListMode listMode,
                           DeleteDialogMode::DeleteMode deleteMode);
    bool shouldDelete() const;

    void setURLs(const KUrl::List& files);
    void presetDeleteMode(DeleteDialogMode::DeleteMode mode);
    void setListMode(DeleteDialogMode::ListMode mode);

protected Q_SLOTS:

    void slotUser1Clicked();
    void slotShouldDelete(bool);

private:

    void keyPressEvent(QKeyEvent*);

private:

    class DeleteDialogPriv;
    DeleteDialogPriv* const d;
};

} // namespace Digikam

#endif // DELETEDIALOG_H

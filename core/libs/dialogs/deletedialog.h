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
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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
#include <QTreeWidget>
#include <QUrl>
#include <QDialog>

// Local includes

#include "digikam_export.h"
#include "thumbnailloadthread.h"

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

} // namespace DeleteDialogMode

// -----------------------------------------------------------

class DeleteItem : public QTreeWidgetItem
{

public:

    DeleteItem(QTreeWidget* const parent, const QUrl& url);
    virtual ~DeleteItem();

    bool    hasValidThumbnail() const;
    QUrl    url()               const;
    QString fileUrl()           const;

    void setThumb(const QPixmap& pix, bool hasThumb=true);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------

class DeleteItemList : public QTreeWidget
{
    Q_OBJECT

public:

    explicit DeleteItemList(QWidget* const parent=0);
    virtual ~DeleteItemList();

private :

    void drawRow(QPainter* p, const QStyleOptionViewItem& opt, const QModelIndex& index) const;

private Q_SLOTS:

    void slotThumbnailLoaded(const LoadingDescription&, const QPixmap&);

private:

    class Private;
    Private* const d;
};

// -----------------------------------------------------------

class DeleteWidget : public QWidget
{
    Q_OBJECT

public:

    explicit DeleteWidget(QWidget* const parent = 0);
    virtual ~DeleteWidget();

    void setUrls(const QList<QUrl>& urls);
    void setListMode(DeleteDialogMode::ListMode mode);
    void setDeleteMode(DeleteDialogMode::DeleteMode deleteMode);

private Q_SLOTS:

    void slotShouldDelete(bool);

private:

    void updateText();

private:

    class Private;
    Private* const d;

    friend class DeleteDialog;
};

// -----------------------------------------------------------

class DeleteDialog : public QDialog
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

    explicit DeleteDialog(QWidget* const parent);
    virtual ~DeleteDialog();

    bool confirmDeleteList(const QList<QUrl>& condemnedURLs,
                           DeleteDialogMode::ListMode listMode,
                           DeleteDialogMode::DeleteMode deleteMode);
    bool shouldDelete() const;

    void setUrls(const QList<QUrl>& urls);
    void presetDeleteMode(DeleteDialogMode::DeleteMode mode);
    void setListMode(DeleteDialogMode::ListMode mode);

protected Q_SLOTS:

    void slotUser1Clicked();
    void slotShouldDelete(bool);

private:

    void keyPressEvent(QKeyEvent*);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DELETEDIALOG_H

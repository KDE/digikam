/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2013-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADVANCEDRENAMEDIALOG_H
#define ADVANCEDRENAMEDIALOG_H

// Qt includes

#include <QList>
#include <QPair>
#include <QTreeWidgetItem>
#include <QUrl>
#include <QDialog>

// Local includes

#include "imageinfo.h"
#include "digikam_export.h"

class QEvent;
class QMoveEvent;
class QString;
class QTreeWidget;

namespace Digikam
{

class Parser;

class DIGIKAM_EXPORT AdvancedRenameListItem : public QTreeWidgetItem
{
public:

    enum Column
    {
        OldName = 0,
        NewName
    };

public:

    explicit AdvancedRenameListItem(QTreeWidget* const view);
    AdvancedRenameListItem(QTreeWidget* const view, const QUrl& info);
    virtual ~AdvancedRenameListItem();

    void setImageUrl(const QUrl& url);
    QUrl imageUrl() const;

    void setName(const QString& name);
    QString name() const;

    void setNewName(const QString& name);
    QString newName() const;

    void markInvalid(bool invalid);
    bool isNameEqual() const;

private:

    AdvancedRenameListItem(const AdvancedRenameListItem&);
    AdvancedRenameListItem& operator=(const AdvancedRenameListItem&);

private:

    class Private;
    Private* const d;
};

// --------------------------------------------------------

typedef QPair<QUrl, QString> NewNameInfo;
typedef QList<NewNameInfo>   NewNamesList;

class DIGIKAM_EXPORT AdvancedRenameDialog : public QDialog
{
    Q_OBJECT

public:

    explicit AdvancedRenameDialog(QWidget* const parent = 0);
    ~AdvancedRenameDialog();

    NewNamesList newNames() const;

public Q_SLOTS:

    void slotAddImages(const QList<QUrl>& urls);

private Q_SLOTS:

    void slotParseStringChanged(const QString&);
    void slotReturnPressed();

    void slotSortActionTriggered(QAction*);
    void slotSortDirectionTriggered(QAction*);

    void slotShowContextMenu(const QPoint&);
    void slotHelp();

private:

    AdvancedRenameDialog(const AdvancedRenameDialog&);
    AdvancedRenameDialog& operator=(const AdvancedRenameDialog&);

    void setupWidgets();
    void setupConnections();

    void initDialog();
    void readSettings();
    void writeSettings();
    bool checkNewNames() const;

    NewNamesList filterNewNames() const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEDIALOG_H */

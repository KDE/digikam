/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : a rename dialog for the AdvancedRename utility
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// KDE includes

#include <kdialog.h>
#include <kurl.h>

// Local includes

#include "imageinfo.h"

class QEvent;
class QMoveEvent;
class QString;
class QTreeWidget;

namespace Digikam
{

class AdvancedRenameListItemPriv;

class AdvancedRenameListItem : public QTreeWidgetItem
{
public:

    AdvancedRenameListItem(QTreeWidget* view);
    AdvancedRenameListItem(QTreeWidget* view, const KUrl& info);
    virtual ~AdvancedRenameListItem();

    void setImageUrl(const KUrl& url);
    KUrl imageUrl() const;

    void setName(const QString& name);
    QString name() const;

    void setNewName(const QString& name);
    QString newName() const;

private:

    AdvancedRenameListItemPriv* const d;
};

// --------------------------------------------------------

typedef QPair<KUrl, QString>      NewNameInfo;
typedef QList<NewNameInfo>        NewNamesList;

class AdvancedRenameDialogPriv;

class AdvancedRenameDialog : public KDialog
{
    Q_OBJECT

public:

    AdvancedRenameDialog(QWidget* parent = 0);
    ~AdvancedRenameDialog();

    NewNamesList newNames();

Q_SIGNALS:

    void signalWindowHasMoved();
    void signalWindowLostFocus();

public Q_SLOTS:

    void slotAddImages(const KUrl::List& urls);

protected:

    void moveEvent(QMoveEvent* e);
    bool event(QEvent* e);

private Q_SLOTS:

    void slotParseStringChanged(const QString&);

private:

    void initDialog(int count = 1);
    void readSettings();
    void writeSettings();
    bool newNamesAreValid();

private:

    AdvancedRenameDialogPriv* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEDIALOG_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-22
 * Description : a control widget for the ManualRenameParser
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

#ifndef MANUALRENAMEDIALOG_H
#define MANUALRENAMEDIALOG_H

// Qt includes

#include <QList>
#include <QPair>
#include <QTreeWidgetItem>

// KDE includes

#include <kdialog.h>
#include <kurl.h>

// Local includes

#include "imageinfo.h"
#include "digikam_export.h"

class QEvent;
class QMoveEvent;
class QString;
class QTreeWidget;

namespace Digikam
{

class ManualRenameListItemPriv;

class DIGIKAM_EXPORT ManualRenameListItem : public QTreeWidgetItem
{
public:

    ManualRenameListItem(QTreeWidget* view);
    ManualRenameListItem(QTreeWidget* view, const ImageInfo& info);
    virtual ~ManualRenameListItem();

    void setImageInfo(const ImageInfo& info);
    ImageInfo imageInfo() const;

    void setName(const QString& name);
    QString name() const;

    void setNewName(const QString& name);
    QString newName() const;

private:

    ManualRenameListItemPriv* const d;
};

// --------------------------------------------------------

class ManualRenameDialogPriv;

class DIGIKAM_EXPORT ManualRenameDialog : public KDialog
{
    Q_OBJECT

public:

    typedef QPair<ImageInfo, QString> NewNameInfo;
    typedef QList<NewNameInfo>        NewNamesList;

public:

    ManualRenameDialog(QWidget* parent = 0);
    ~ManualRenameDialog();

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

    void slotParseStringChanged();

private:

    void initDialog(int count = 1);

private:

    ManualRenameDialogPriv* const d;
};

}  // namespace Digikam

#endif /* MANUALRENAMEDIALOG_H */

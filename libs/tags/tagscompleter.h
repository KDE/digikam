/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-06-12
 * Description : Completion Box for tags
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef ADDTAGSCOMPLETIONBOX_H
#define ADDTAGSCOMPLETIONBOX_H

// Qt includes

#include <QCompleter>

// Local includes

#include "modelcompletion.h"
#include "taggingaction.h"

class QLineEdit;

namespace Digikam
{

class AlbumFilterModel;
class TAlbum;
class TagModel;

class TagCompleter : public QCompleter
{
    Q_OBJECT

public:

    /** A completion object operating on a TagModel
     */
    TagCompleter(QObject* parent = 0);
    ~TagCompleter();

    // Update the completer for the given fragment
    void update(const QString& fragment);
    // Set a parent tag which may by the user be considered as a parent for a new tag during completion
    void setContextParentTag(int parentTagId);
    // Set a supporting model from which the completer may get data for its display. Optional.
    void setSupportingTagModel(TagModel* supportingModel);
    void setTagFilterModel(AlbumFilterModel* supportingModel);

    // You must call this for the completion to work properly.
    // This also calls lineEdit->setCompleter, you do not need to call this.
    void setWidget(QLineEdit* lineEdit);

Q_SIGNALS:

    void activated(const TaggingAction& action);
    void highlighted(const TaggingAction& action);

private Q_SLOTS:

    void slotActivated(const QModelIndex& index);
    void slotHighlighted(const QModelIndex& index);
    void textChanged(const QString& text);
    void textEdited(const QString& text);
    /*void slotInsertRows(QModelIndex index, int start, int end);
    void slotDeleteRows(QModelIndex index, int start, int end);*/

private:

    void setModel(QAbstractItemModel*);

    class Private;
    Private *d;
};

} // namespace Digikam

#endif // ADDTAGSCOMPLETIONBOX_H

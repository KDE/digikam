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

namespace Digikam
{

class AlbumFilterModel;
class TAlbum;
class TagModel;

class TagModelCompletion : public QCompleter
{
    Q_OBJECT

public:

    /** A completion object operating on a TagModel
     */
    TagModelCompletion();

    void setModel(QAbstractItemModel *model, int idRole);
    void setModel(AlbumFilterModel* model);
    void setModel(TagModel* model);
    TagModel* model() const;

    void update(QString word);

public Q_SLOTS:

    void complete(const QRect &rect);

private Q_SLOTS:

    void slotInsertRows(QModelIndex index, int start, int end);
    void slotDeleteRows(QModelIndex index, int start, int end);

private:

    class Private;
    Private *d;
};

} // namespace Digikam

#endif // ADDTAGSCOMPLETIONBOX_H

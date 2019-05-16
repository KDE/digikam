/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2010-04-25
 * Description : A class to hold undo/redo commands.
 *
 * Copyright (C) 2010-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Michael G. Hansen <mike at mghansen dot de>
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

#ifndef DIGIKAM_GPS_UNDO_COMMAND_H
#define DIGIKAM_GPS_UNDO_COMMAND_H

// Qt includes

#include <QUndoCommand>

// Local includes

#include "gpsitemcontainer.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT GPSUndoCommand : public QUndoCommand
{
public:

    class UndoInfo
    {
    public:

        explicit UndoInfo(QPersistentModelIndex pModelIndex)
            : modelIndex(pModelIndex)
        {
        }

        void readOldDataFromItem(const GPSItemContainer* const imageItem);
        void readNewDataFromItem(const GPSItemContainer* const imageItem);

        QPersistentModelIndex modelIndex;
        GPSDataContainer dataBefore;
        GPSDataContainer dataAfter;

        QList<QList<TagData> > oldTagList;
        QList<QList<TagData> > newTagList;

        typedef QList<UndoInfo> List;
    };

    explicit GPSUndoCommand(QUndoCommand* const parent = nullptr);

    void addUndoInfo(const UndoInfo& info);
    void changeItemData(const bool redoIt);

    inline int affectedItemCount() const
    {
        return undoList.count();
    }

    virtual void redo() override;
    virtual void undo() override;

private:

    UndoInfo::List undoList;
};

} // namespace Digikam

#endif // DIGIKAM_GPS_UNDO_COMMAND_H

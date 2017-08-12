/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef UNDOMOVEROWSCOMMAND_H
#define UNDOMOVEROWSCOMMAND_H

#include <QModelIndex>
#include <QUndoCommand>

namespace PhotoLayoutsEditor
{
    class LayersModel;
    class LayersModelItem;

    class UndoMoveRowsCommand : public QUndoCommand
    {
            LayersModel * m_model;
            LayersModelItem * m_src_parent_row;
            LayersModelItem * m_dest_parent_row;
            int m_starting_row;
            int m_rows_count;
            int m_destination_row;

        public:

            UndoMoveRowsCommand(int startingRow, int rowsCount, const QModelIndex & sourceParent, int destinationRow, const QModelIndex & destinationParent, LayersModel * model, QUndoCommand * parent = 0);
            virtual void redo();
            virtual void undo();

        private:

            void reverse();
    };
}

#endif // UNDOMOVEROWSCOMMAND_H

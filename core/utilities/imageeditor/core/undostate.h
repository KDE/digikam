/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : Undo state container
 *
 * Copyright (C) 2004-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_UNDO_STATE_H
#define DIGIKAM_UNDO_STATE_H

namespace Digikam
{

class UndoState
{
public:

    explicit UndoState()
      : hasUndo(false),
        hasRedo(false),
        hasChanges(false),
        hasUndoableChanges(false)
    {
    }

    ~UndoState()
    {
    }

public:

    bool hasUndo;
    bool hasRedo;
    bool hasChanges;
    bool hasUndoableChanges;
};

} // namespace Digikam

#endif // DIGIKAM_UNDO_STATE_H

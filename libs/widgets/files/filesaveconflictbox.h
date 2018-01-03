/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-09-13
 * Description : a widget to provide conflict rules to save image.
 *
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef FILE_SAVE_CONFLICT_BOX_H
#define FILE_SAVE_CONFLICT_BOX_H

// Qt includes

#include <QWidget>

// KDE includes

#include <kconfig.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT FileSaveConflictBox : public QWidget
{
    Q_OBJECT

public:

    enum ConflictRule
    {
        OVERWRITE = 0,
        DIFFNAME
    };

public:

    FileSaveConflictBox(QWidget* const parent);
    ~FileSaveConflictBox();

    ConflictRule conflictRule() const;
    void setConflictRule(ConflictRule r);

    void resetToDefault();

    void readSettings(KConfigGroup& group);
    void writeSettings(KConfigGroup& group);

Q_SIGNALS:

    void signalConflictButtonChanged(int);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // FILE_SAVE_CONFLICT_BOX_H

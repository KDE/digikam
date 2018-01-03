/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : setup Image Editor page
 *
 * Copyright (C) 2007-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPEDITOR_H
#define SETUPEDITOR_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupEditor : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupEditor(QWidget* const parent = 0);
    ~SetupEditor();

    void applySettings();

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUPEDITOR_H

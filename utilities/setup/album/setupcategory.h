/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-01-02
 * Description : album category setup tab.
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

#ifndef SETUPCATEGORY_H
#define SETUPCATEGORY_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupCategory : public QScrollArea
{
    Q_OBJECT

public:

    explicit SetupCategory(QWidget* const parent = 0);
    virtual ~SetupCategory();

    void applySettings();
    void readSettings();

private Q_SLOTS:

    void slotCategorySelectionChanged();
    void slotAddCategory();
    void slotDelCategory();
    void slotRepCategory();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // SETUPCATEGORY_H

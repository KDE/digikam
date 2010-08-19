/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-10
 * Description : setup tab for face tags
 *
 * Copyright (C) 2010 by Aditya Bhatt <adityabhatt1991 at gmail dot com>
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

#ifndef SETUPFACETAGS_H
#define SETUPFACETAGS_H

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupFaceTags : public QScrollArea
{
    Q_OBJECT

public:

    SetupFaceTags(QWidget* parent = 0);
    ~SetupFaceTags();

    void applySettings();

public Q_SLOTS:

    void updateDetection(int value);
    void updateSuggestion(int value);

private:

    void readSettings();

private:

    class SetupFaceTagsPriv;
    SetupFaceTagsPriv* const d;
};

}   // namespace Digikam

#endif /* SETUPFACETAGS_H */
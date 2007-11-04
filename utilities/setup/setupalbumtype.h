/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 * 
 * Date        : 2004-01-02
 * Description : album type setup tab.
 * 
 * Copyright (C) 2004-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPALBUMTYPE_H
#define SETUPALBUMTYPE_H

// Qt includes.

#include <QWidget>

namespace Digikam
{

class SetupAlbumTypePriv;

class SetupAlbumType : public QWidget
{
    Q_OBJECT

public:

    SetupAlbumType(QWidget* parent = 0);
    ~SetupAlbumType();

    void applySettings();

private:

    void readSettings();

private slots:

    void slotCollectionSelectionChanged();
    void slotAddCollection();
    void slotDelCollection();
    void slotRepCollection();

private:

    SetupAlbumTypePriv* d;
};

}  // namespace Digikam

#endif // SETUPALBUMTYPE_H 

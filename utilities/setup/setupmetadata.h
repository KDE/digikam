/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 * 
 * Copyright (C) 2003-2004 by Ralf Holzer  <ralf at well.com>
 * Copyright (C) 2003-2007 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPMETADATA_H
#define SETUPMETADATA_H

// Qt includes.

#include <qwidget.h>

namespace Digikam
{

class SetupMetadataPriv;

class SetupMetadata : public QWidget
{
    Q_OBJECT
    
public:

    SetupMetadata(QWidget* parent = 0);
    ~SetupMetadata();

    void applySettings();

    bool exifAutoRotateAsChanged();

private:

    void readSettings();

private slots:

    void processExiv2URL(const QString&);
    void slotExifAutoRotateToggled(bool);

private:

    SetupMetadataPriv* d;   
};

}  // namespace Digikam

#endif // SETUPMETADATA_H 

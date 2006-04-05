/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Ralf Holzer <ralf at well.com>
 * Date   : 2003-08-03
 * Description : setup Metadata tab.
 * 
 * Copyright 2003-2004 by Ralf Holzer and Gilles Caulier
 * Copyright 2005-2006 by Gilles Caulier
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

private:

    void readSettings();

private slots:

    void processExiv2URL(const QString& url);

private:

    SetupMetadataPriv* d;
   
};

}  // namespace Digikam

#endif // SETUPMETADATA_H 

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-18
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SHOWFOTOSETUPMETADATA_H
#define SHOWFOTOSETUPMETADATA_H

// Qt includes

#include <QScrollArea>

namespace ShowFoto
{

class SetupMetadata : public QScrollArea
{
    Q_OBJECT

public:

    enum MetadataTab
    {
        Behavior = 0,
        ExifViewer,
        MakernotesViewer,
        IptcViewer,
        XmpViewer
    };

public:

    explicit SetupMetadata(QWidget* const parent = 0);
    ~SetupMetadata();

    void applySettings();
    void setActiveTab(MetadataTab tab);

private:

    void readSettings();

private:

    class Private;
    Private* const d;
};

}  // namespace ShowFoto

#endif // SHOWFOTOSETUPMETADATA_H

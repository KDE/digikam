/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-08-03
 * Description : setup Metadata tab.
 *
 * Copyright (C) 2003-2004 by Ralf Holzer  <ralf at well.com>
 * Copyright (C) 2003-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

// Qt includes

#include <QScrollArea>

namespace Digikam
{

class SetupMetadata : public QScrollArea
{
    Q_OBJECT

public:

    enum MetadataTab
    {
        Behavior = 0,
        Sidecars,
        Rotation,
        Display,
        AdvancedConfig,
        Baloo
    };

public:

    explicit SetupMetadata(QWidget* const parent = 0);
    ~SetupMetadata();

    void applySettings();

    bool exifAutoRotateHasChanged() const;

    void setActiveMainTab(MetadataTab tab);
    void setActiveSubTab(int tab);

private:

    void readSettings();

private Q_SLOTS:

    void slotExifAutoRotateToggled(bool);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // SETUPMETADATA_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : DMetadata Settings Container.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef DMETADATASETTINGSCONTAINER_H
#define DMETADATASETTINGSCONTAINER_H

// Qt includes

#include <QFlags>
#include <QString>


// Local includes

#include "digikam_export.h"

class KConfigGroup;


namespace Digikam
{

/**
    The class DMetadataSettingsContainer is designed to dynamically add namespaces.
*/
class DIGIKAM_EXPORT DMetadataSettingsContainer
{
public:

    DMetadataSettingsContainer();
    ~DMetadataSettingsContainer()
    {
    };

public:

    void readFromConfig(KConfigGroup& group);
    void writeToConfig(KConfigGroup& group) const;

};

}  // namespace Digikam


#endif  // METADATASETTINGSCONTAINER_H

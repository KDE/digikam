/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : a parser for the AdvancedRename utility used for importing images,
 *               excluding the database options
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "importrenameparser.h"

// Local includes

#include "databaseoption.h"
#include "cameranameoption.h"
#include "metadataoption.h"

namespace Digikam
{

ImportRenameParser::ImportRenameParser()
    : Parser()
{
    // unregister options that are not suitable while import
    RulesList oplist = options();

    foreach(Rule* const option, oplist)
    {
        if (dynamic_cast<DatabaseOption*>(option) ||
            dynamic_cast<MetadataOption*>(option) ||
            dynamic_cast<CameraNameOption*>(option))
        {
            unregisterOption(option);
        }
    }
}

}  // namespace Digikam

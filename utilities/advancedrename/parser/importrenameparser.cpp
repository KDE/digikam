/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : a parser for the AdvancedRename utility used for importing images,
 *               excluding the database options
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

// LibKExiv2 includes

#include <libkexiv2/version.h>

// Local includes


#include "cameranameoption.h"
#include "dateoption.h"
#include "directorynameoption.h"
#include "filepropertiesoption.h"
#include "metadataoption.h"
#include "sequencenumberoption.h"

namespace Digikam
{

ImportRenameParser::ImportRenameParser()
                  : Parser()
{
    /*
     * RENAMING OPTIONS
     */
    registerOption(new FilePropertiesOption());
    registerOption(new DirectoryNameOption());
    registerOption(new CameraNameOption());
    registerOption(new SequenceNumberOption());
    registerOption(new DateOption());

#if KEXIV2_VERSION >= 0x010000
    registerOption(new MetadataOption());
#endif
}

}  // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the main parser object for manual rename
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

#include "manualrenameparser.h"

// Qt includes

#include <QFileInfo>

// Libkexiv2 includes

#include <libkexiv2/version.h>

// Local includes

#include "cameranameparser.h"
#include "dateparser.h"
#include "directorynameparser.h"
#include "filenameparser.h"
#include "metadataparser.h"
#include "sequencenumberparser.h"

namespace Digikam
{
namespace ManualRename
{

ManualRenameParser::ManualRenameParser()
                  : MainParser()
{
    registerParser(new FilenameParser());
    registerParser(new DirectoryNameParser());
    registerParser(new SequenceNumberParser());
    registerParser(new CameraNameParser());
    registerParser(new DateParser());

#if KEXIV2_VERSION >= 0x010000
    registerParser(new MetadataParser());
#endif
}

ManualRenameParser::~ManualRenameParser()
{
}

}  // namespace ManualRename
}  // namespace Digikam

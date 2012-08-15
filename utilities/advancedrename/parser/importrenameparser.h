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

#ifndef IMPORTRENAMEPARSER_H
#define IMPORTRENAMEPARSER_H

// Local includes

#include "parser.h"

namespace Digikam
{

class ImportRenameParser : public Parser
{

public:

    ImportRenameParser();

private:

    ImportRenameParser(const ImportRenameParser&);
    ImportRenameParser& operator=(const ImportRenameParser&);
};

}  // namespace Digikam


#endif /* IMPORTRENAMEPARSER_H */

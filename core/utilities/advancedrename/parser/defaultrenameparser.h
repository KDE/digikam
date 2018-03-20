/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-11
 * Description : the default parser for the AdvancedRename utility,
 *               includes all renaming options
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

#ifndef DEFAULTRENAMEPARSER_H
#define DEFAULTRENAMEPARSER_H

// Local includes

#include "parser.h"

namespace Digikam
{

class DefaultRenameParser : public Parser
{

public:

    DefaultRenameParser();

private:

    DefaultRenameParser(const DefaultRenameParser&);
    DefaultRenameParser& operator=(const DefaultRenameParser&);
};

}  // namespace Digikam


#endif /* DEFAULTRENAMEPARSER_H */

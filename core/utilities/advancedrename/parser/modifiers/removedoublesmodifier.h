/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a modifier for deleting duplicate words
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

#ifndef REMOVEDOUBLESMODIFIER_H
#define REMOVEDOUBLESMODIFIER_H

// Local includes

#include "modifier.h"
#include "ruledialog.h"

namespace Digikam
{

class RemoveDoublesModifier : public Modifier
{
    Q_OBJECT

public:

    RemoveDoublesModifier();
    virtual QString parseOperation(ParseSettings& settings);

private:

    RemoveDoublesModifier(const RemoveDoublesModifier&);
    RemoveDoublesModifier& operator=(const RemoveDoublesModifier&);
};

} // namespace Digikam


#endif /* REMOVEDOUBLESMODIFIER_H */

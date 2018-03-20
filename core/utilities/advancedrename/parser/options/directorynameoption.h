/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-02
 * Description : an option to provide directory information to the parser
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

#ifndef DIRECTORYNAMEOPTION_H
#define DIRECTORYNAMEOPTION_H

// Qt includes

#include <QObject>

// Local includes

#include "option.h"

class QString;

namespace Digikam
{

class DirectoryNameOption : public Option
{
    Q_OBJECT

public:

    DirectoryNameOption();
    ~DirectoryNameOption() {};

protected:

    virtual QString parseOperation(ParseSettings& settings);

private:

    DirectoryNameOption(const DirectoryNameOption&);
    DirectoryNameOption& operator=(const DirectoryNameOption&);
};

} // namespace Digikam

#endif /* DIRECTORYNAMEOPTION_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an option to provide file information to the parser
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

#ifndef FILEPROPERTIESOPTION_H
#define FILEPROPERTIESOPTION_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "option.h"

namespace Digikam
{

class FilePropertiesOption : public Option
{
    Q_OBJECT

public:

    FilePropertiesOption();
    ~FilePropertiesOption() {};

protected:

    virtual QString parseOperation(ParseSettings& settings);

private:

    FilePropertiesOption(const FilePropertiesOption&);
    FilePropertiesOption& operator=(const FilePropertiesOption&);
};

} // namespace Digikam

#endif /* FILEPROPERTIESOPTION_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract option class
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

#ifndef OPTION_H
#define OPTION_H

// Local includes

#include "rule.h"

namespace Digikam
{

class Option : public Rule
{
    Q_OBJECT

public:

    Option(const QString& name, const QString& description);
    Option(const QString& name, const QString& description, const QString& icon);
    virtual ~Option();

protected:

    virtual QString parseOperation(ParseSettings& settings) = 0;

private:

    Option(const Option&);
    Option& operator=(const Option&);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* OPTION_H */

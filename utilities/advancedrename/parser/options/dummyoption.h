/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 200X-XX-XX
 * Description : an option to provide <FILL IN PURPOSE> information to the parser
 *
 * Copyright (C) 2009 by YOUR NAME <YOUR MAIL ADDRESS>
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

#ifndef DUMMYOPTION_H
#define DUMMYOPTION_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "option.h"

namespace Digikam
{

class DummyOption : public Option
{
    Q_OBJECT

public:

    DummyOption();
    ~DummyOption() {};

protected:

    virtual void parseOperation(const QString& parseString, ParseSettings& info, ParseResults& results);
};

} // namespace Digikam

#endif /* DUMMYOPTION_H */

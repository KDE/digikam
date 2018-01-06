/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef INT_THEME_PARAMETER_H
#define INT_THEME_PARAMETER_H

// Local includes

#include "abstractthemeparameter.h"

namespace Digikam
{

/**
 * A theme parameter to select an integer value.
 */
class IntThemeParameter : public AbstractThemeParameter
{
public:

    explicit IntThemeParameter();
    virtual ~IntThemeParameter();

    virtual void init(const QByteArray& internalName, const KConfigGroup* configGroup);
    virtual QWidget* createWidget(QWidget* parent, const QString& value) const;
    virtual QString valueFromWidget(QWidget*) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // INT_THEME_PARAMETER_H

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

#include "intthemeparameter.h"

// Qt includes

#include <QSpinBox>

// KDE includes

#include <kconfiggroup.h>

static const char* MIN_VALUE_KEY = "Min";
static const char* MAX_VALUE_KEY = "Max";

namespace Digikam
{

class IntThemeParameter::Private
{
public:

    explicit Private()
      : minValue(0),
        maxValue(99999)
    {
    }

    int minValue;
    int maxValue;
};

IntThemeParameter::IntThemeParameter()
    : d(new Private)
{
}

IntThemeParameter::~IntThemeParameter()
{
    delete d;
}

void IntThemeParameter::init(const QByteArray& internalName, const KConfigGroup* configGroup)
{
    AbstractThemeParameter::init(internalName, configGroup);

    d->minValue = configGroup->readEntry(MIN_VALUE_KEY, 0);
    d->maxValue = configGroup->readEntry(MAX_VALUE_KEY, 99999);
}

QWidget* IntThemeParameter::createWidget(QWidget* parent, const QString& value) const
{
    QSpinBox* const spinBox = new QSpinBox(parent);
    spinBox->setValue(value.toInt());
    spinBox->setMinimum(d->minValue);
    spinBox->setMaximum(d->maxValue);

    return spinBox;
}

QString IntThemeParameter::valueFromWidget(QWidget* widget) const
{
    QSpinBox* const spinBox = static_cast<QSpinBox*>(widget);

    return QString::number(spinBox->value());
}

} // namespace Digikam

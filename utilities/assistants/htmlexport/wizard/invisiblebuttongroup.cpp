/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "invisiblebuttongroup.h"

// Qt includes

#include <QAbstractButton>
#include <QButtonGroup>

// KDE includes

#include <kconfigdialogmanager.h>

namespace Digikam
{

class InvisibleButtonGroupPrivate
{
public:

    QButtonGroup* mGroup;
};

InvisibleButtonGroup::InvisibleButtonGroup(QWidget* parent)
    : QWidget(parent),
      d(new InvisibleButtonGroupPrivate)
{
    hide();
    d->mGroup = new QButtonGroup(this);
    d->mGroup->setExclusive(true);

    connect(d->mGroup, SIGNAL(buttonClicked(int)),
            SIGNAL(selectionChanged(int)) );

    const QString name = QString::fromLatin1(metaObject()->className());

    if (!KConfigDialogManager::propertyMap()->contains(name))
    {
        KConfigDialogManager::propertyMap()->insert(name, "current");
        KConfigDialogManager::changedMap()->insert(name, SIGNAL(selectionChanged(int)));
    }
}

InvisibleButtonGroup::~InvisibleButtonGroup()
{
    delete d;
}

int InvisibleButtonGroup::selected() const
{
    return d->mGroup->checkedId();
}

void InvisibleButtonGroup::addButton(QAbstractButton* button, int id)
{
    d->mGroup->addButton(button, id);
}

void InvisibleButtonGroup::setSelected(int id)
{
    QAbstractButton* const button = d->mGroup->button(id);

    if (button)
    {
        button->setChecked(true);
    }
}

} // namespace Digikam

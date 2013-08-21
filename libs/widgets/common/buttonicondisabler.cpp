/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-09
 * Description : A helper class to disable the icon of a togglebutton, if it is not checked.
 *               This will simulate a disabled button, that is still clickable.
 *
 * Copyright (C) 2006-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmail dot com>
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

#include "buttonicondisabler.moc"

// Qt includes

#include <QAbstractButton>

namespace Digikam
{

class ButtonIconDisabler::Private
{

public:

    Private()
    {
        button = 0;
    }

    QAbstractButton* button;
    QIcon            icon;
    QIcon            iconDisabled;
};

ButtonIconDisabler::ButtonIconDisabler(QAbstractButton* const button)
    : QObject(button), d(new Private)
{
    d->button       = button;
    d->icon         = d->button->icon();
    int minSize     = qMin(d->button->size().width(), d->button->size().height());
    QSize size(minSize, minSize);
    QPixmap pix     = d->icon.pixmap(size, QIcon::Disabled);
    d->iconDisabled = QIcon(pix);

    d->button->setEnabled(d->button->isEnabled());
    showIcon(d->button->isChecked());

    connect(d->button, SIGNAL(toggled(bool)),
            this, SLOT(showIcon(bool)));
}

ButtonIconDisabler::~ButtonIconDisabler()
{
    delete d;
}

void ButtonIconDisabler::showIcon(bool show)
{
    d->button->setIcon(show ? d->icon : d->iconDisabled);
}

} // namespace Digikam

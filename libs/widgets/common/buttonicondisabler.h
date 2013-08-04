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

#ifndef BUTTONICONDISABLER_H
#define BUTTONICONDISABLER_H

// Qt includes

#include <QIcon>
#include <QObject>

// Local includes

#include "digikam_export.h"

class QAbstractButton;

namespace Digikam
{

class DIGIKAM_EXPORT ButtonIconDisabler : public QObject
{
    Q_OBJECT

public:

    explicit ButtonIconDisabler(QAbstractButton* const button);
    ~ButtonIconDisabler();

private Q_SLOTS:

    void showIcon(bool);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* BUTTONICONDISABLER_H */

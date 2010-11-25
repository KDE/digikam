/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : a widget to host settings as expander box in exclusive mode
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010      by Manuel Viet    <contact at 13zenrv dot fr>
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

#ifndef REXPANDERBOXEXCLUSIVE_H
#define REXPANDERBOXEXCLUSIVE_H

// Qt includes

#include <QtCore/QObject>

// Libkdcraw includes

#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT RExpanderBoxExclusive : public KDcrawIface::RExpanderBox
{
    Q_OBJECT

public:

    RExpanderBoxExclusive(QWidget* parent = 0);
    ~RExpanderBoxExclusive();

    /** Show one expander open at most */
    void setIsToolBox(bool b);
    bool isToolBox() const;

private Q_SLOTS:

    void slotItemExpanded(bool b);

private:

    bool m_toolbox;
};

} // namespace Digikam

#endif // REXPANDERBOXEXCLUSIVE_H

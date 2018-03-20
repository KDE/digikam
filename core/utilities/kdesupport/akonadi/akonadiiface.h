/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : Akonadi Address Book contacts interface
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef AKONADI_IFACE_H
#define AKONADI_IFACE_H

// Qt includes

#include <QObject>
#include <QMenu>
#include <QAction>

// Local includes

#include "digikam_export.h"
#include "digikam_config.h"

class KJob;

namespace Digikam
{

class DIGIKAM_EXPORT AkonadiIface : public QObject
{
    Q_OBJECT

public:

    explicit AkonadiIface(QMenu* const parent);
    virtual ~AkonadiIface();

Q_SIGNALS:

    void signalContactTriggered(const QString&);

private Q_SLOTS:

#ifdef HAVE_AKONADICONTACT

    void slotABCSearchResult(KJob*);
    void slotABCMenuTriggered(QAction*);

#endif // HAVE_AKONADICONTACT

private:

    QMenu* m_parent;
    QMenu* m_ABCmenu;
};

} // namespace Digikam

#endif /* AKONADI_IFACE_H */

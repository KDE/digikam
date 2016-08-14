/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-08
 * Description : database server starter
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
 * Copyright (C) 2016 by Swati Lodha <swatilodha27 at gmail dot com>
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

#ifndef DATABASE_SERVER_STARTER_H_
#define DATABASE_SERVER_STARTER_H_

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"
#include "databaseservererror.h"
#include "dbengineparameters.h"

namespace Digikam
{

class DIGIKAM_EXPORT DatabaseServerStarter : public QObject
{
    Q_OBJECT

public:

    /**
     * Global instance of internal server starter. All accessor methods are thread-safe.
     */
    static DatabaseServerStarter* instance();

    DatabaseServerError startServerManagerProcess(const DbEngineParameters& parameters) const;
    void                stopServerManagerProcess();

private:

    DatabaseServerStarter();
    ~DatabaseServerStarter();

private:

    class Private;
    Private* const d;

    friend class DatabaseServerStarterCreator;
};

} // namespace Digikam

#endif // DATABASE_SERVER_STARTER_H_

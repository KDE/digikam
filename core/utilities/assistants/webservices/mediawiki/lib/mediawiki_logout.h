/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Alexandre Mendes <alex dot mendes1988 at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef MEDIAWIKI_LOGOUT_H
#define MEDIAWIKI_LOGOUT_H

// Local includes

#include "mediawiki_job.h"
#include "digikam_export.h"

namespace mediawiki
{

class Iface;
class LogoutPrivate;

/**
 * @brief Logout job.
 *
 * Uses for log out a user.
 */
class DIGIKAM_EXPORT Logout : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Logout)

public:

    /**
     * @brief Constructs a Logout job.
     * @param parent the QObject parent
     */
    explicit Logout(Iface& mediawiki, QObject* const parent = 0);

    /**
     * @brief Destroys the Logout job.
     */
    virtual ~Logout();

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace mediawiki

#endif // MEDIAWIKI_LOGOUT_H

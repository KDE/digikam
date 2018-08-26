/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a MediaWiki C++ interface
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Ludovic Delfau <ludovicdelfau at gmail dot com>
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

#ifndef MEDIAWIKI_QUERYSITEINFOUSERGROUPS_H
#define MEDIAWIKI_QUERYSITEINFOUSERGROUPS_H

// Local includes

#include "digikam_export.h"
#include "mediawiki_job.h"
#include "mediawiki_usergroup.h"

namespace mediawiki
{

class MediaWiki;
class QuerySiteinfoUsergroupsPrivate;

/**
 * @brief UserGroups job.
 *
 * Uses for fetch a list of all user groups and their permissions.
 */
class DIGIKAM_EXPORT QuerySiteinfoUsergroups : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuerySiteinfoUsergroups)

public:

    /**
     * @brief Constructs a UserGroups job.
     * @param mediawiki the mediawiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QuerySiteinfoUsergroups(MediaWiki& mediawiki, QObject* const parent = 0);

    /**
     * @brief Destroys the UserGroups job.
     */
    virtual ~QuerySiteinfoUsergroups();

    /**
     * @brief If true number of users of each user group is included
     * @param includeNumber if true number of users of each user group is included
     */
    void setIncludeNumber(bool includeNumber);

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:

    /**
     * @brief Provides a list of all user groups.
     * @param usergroups list of all user groups
     * @see QuerySiteinfoUsergroups::Result
     */
    void usergroups(const QList<UserGroup>& usergroups);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace mediawiki

#endif // MEDIAWIKI_QUERYSITEINFOUSERGROUPS_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Hormiere Guillaume <hormiere dot guillaume at gmail dot com>
 * Copyright (C) 2011      by Manuel Campomanes <campomanes dot manuel at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_QUERYREVISION_H
#define DIGIKAM_MEDIAWIKI_QUERYREVISION_H

// Qt includes

#include <QDateTime>
#include <QList>
#include <QString>

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_revision.h"
#include "digikam_export.h"

namespace MediaWiki
{

class Iface;
class QueryRevisionPrivate;

/**
 * @brief QueryRevision job.
 *
 * Uses for fetch a revision information about one pages of the wiki.
 */
class DIGIKAM_EXPORT QueryRevision : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryRevision)

public:

    /**
     * @brief Direction to list revisions.
     */
    enum Direction
    {
        /**
         * @brief List newest revisions first.
         */
        Older,

        /**
         * @brief List oldest revisions first.
         */
        Newer
    };

    /**
     * @brief Tokens can get for each revision.
     */
    enum Token
    {
        /**
         * @brief Rollback token.
         */
        Rollback
    };

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
        /**
         * @brief The revids= parameter may not be used with the list options (limit, startid, endid, dirNewer, start, end).
         */
        WrongRevisionId = Job::UserDefinedError + 1,

        /**
         * @brief titles, pageids or a generator was used to supply multiple pages, but the limit, startid, endid, dirNewer, user, excludeuser, start and end parameters may only be used on a single page.
         */
        MultiPagesNotAllowed,

        /**
         * @brief The current user is not allowed to read title.
         */
        TitleAccessDenied,

        /**
         * @brief start and startid or end and endid or user and excludeuser cannot be used together
         */
        TooManyParams,

        /**
         * @brief There is no section section in rrevid
         */
        SectionNotFound
    };

    /**
     * @brief Property.
     */
    enum Property
    {
        Ids         = 0x01,
        Flags       = 0x02,
        Timestamp   = 0x04,
        User        = 0x08,
        Comment     = 0x10,
        Size        = 0x20,
        Content     = 0x40
    };
    Q_DECLARE_FLAGS(Properties, Property)

public:

    /**
     * @brief Constructs a Revision job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QueryRevision(Iface& MediaWiki, QObject* const parent = 0);

    /**
     * @brief Destroys the QueryRevision job.
     */
    virtual ~QueryRevision();

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * @brief Set the page id.
     * @param pageId the page id
     */
    void setPageId(unsigned int pageId);

    /**
     * @param Set the revision id.
     * @param revisionId the revision id
     */
    void setRevisionId(unsigned int revisionId);

    /**
     * @brief Set the page name.
     * @param pageName the page name
     */
    void setPageName(const QString& pageName);

    /**
     * @brief Which properties to get for each revision.
     * @param properties properties to get for each revision
     */
    void setProperties(Properties properties);

    /**
     * @brief Set the maximum number of revisions to return.
     * @param limit the maximum number of revisions to return
     */
    void setLimit(int limit);

    /**
     * @brief Set the revision ID to start listing from.
     * @param startId the revision ID to start listing from
     */
    void setStartId(int startId);

    /**
     * @brief Set the revision ID to stop listing at.
     * @param endId the revision ID to stop listing at
     */
    void setEndId(int endId);

    /**
     * @brief Set the timestamp to start listing from.
     * @param start the timestamp to start listing from
     */
    void setStartTimestamp(const QDateTime& start);

    /**
     * @brief Set the timestamp to end listing at.
     * @param end the timestamp to end listing at
     */
    void setEndTimestamp(const QDateTime& end);

    /**
     * @brief Set the user.
     *
     * Do list revisions made by this user.
     *
     * @param user the user
     */
    void setUser(const QString& user);

    /**
     * @brief Set the user to exclude.
     *
     * Do not list revisions made by this user
     *
     * @param excludeUser the user to exclude
     */
    void setExcludeUser(const QString& excludeUser);

    /**
     * @brief Set the direction to list revisions.
     * @param direction the direction to list revisions
     */
    void setDirection(QueryRevision::Direction direction);

    /**
     * @brief Set XML generation to parse tree for revision content.
     * @param generateXML if true set XML generation to parse tree for revision content
     */
    void setGenerateXML(bool generateXML);

    /**
     * @brief Set the section.
     *
     * If the property content is set, only retrieve the contents of this section.
     *
     * @param section the section
     */
    void setSection(int section);

    /**
     * @brief Set the token to get for each revision.
     * @param token the token to get for each revision
     */
    void setToken(QueryRevision::Token token);

    /**
     * @brief Set expand templates.
     *
     * Only if the property content is set.
     *
     * @param expandTemplates if true set expand templates
     */
    void setExpandTemplates(bool expandTemplates);

Q_SIGNALS:

    /**
     * @brief Provides a list of all user groups.
     * @param revision list of all user groups
     */
    void revision(const QList<Revision>& revision);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QueryRevision::Properties)

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_QUERYREVISION_H

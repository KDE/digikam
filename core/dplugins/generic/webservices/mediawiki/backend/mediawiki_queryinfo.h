/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_QUERYINFO_H
#define DIGIKAM_MEDIAWIKI_QUERYINFO_H

// Qt includes

#include <QList>
#include <QString>
#include <QUrl>
#include <QDateTime>

// Local includes

#include "mediawiki_page.h"
#include "mediawiki_protection.h"
#include "mediawiki_job.h"
#include "digikam_export.h"

namespace MediaWiki
{

class Iface;
class QueryInfoPrivate;

/**
 * @brief QueryInfo job.
 *
 * Uses to send a request to get basic page information.
 */
class DIGIKAM_EXPORT QueryInfo : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryInfo)

public:

    /**
     * @brief Constructs a QueryInfo job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QueryInfo(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys the QuerySiteInfoGeneral job.
     */
    virtual ~QueryInfo();

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * @brief Set the page name.
     * @param title the page name
     */
    void setPageName(const QString& title);

    /**
     * @brief Set the token to perform a data-modifying action on a page
     * @param token the token
     */
    void setToken(const QString& token);

    /**
     * @brief Set the page id.
     * @param id the page id
     */
    void setPageId(unsigned int id);

    /**
     * @brief Set the page revision
     * @param id the page revision
     */
    void setRevisionId(unsigned int id);

Q_SIGNALS:

    /**
     * @brief Provides a page
     * @param
     */
    void page(const Page& p);
    void protection(const QVector<Protection>& protect);

private Q_SLOTS:

    /**
     * @brief Send a request.
     */
    void doWorkSendRequest();

    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_QUERYINFO_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
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

#ifndef DIGIKAM_MEDIAWIKI_QUERYSITEINFOGENERAL_H
#define DIGIKAM_MEDIAWIKI_QUERYSITEINFOGENERAL_H

// Qt includes

#include <QList>
#include <QString>

// Local includes

#include "digikam_export.h"
#include "mediawiki_job.h"
#include "mediawiki_generalinfo.h"

namespace MediaWiki
{

class Iface;
class QuerySiteInfoGeneralPrivate;

/**
 * @brief QuerySiteInfoGeneral job.
 *
 * Uses for fetch a generals information about the wiki.
 */
class DIGIKAM_EXPORT QuerySiteInfoGeneral : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QuerySiteInfoGeneral)

public:

    enum
    {
        IncludeAllDenied = Job::UserDefinedError + 1
    };

public:

    /**
     * @brief Constructs a QuerySiteInfoGeneral job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QuerySiteInfoGeneral(Iface& MediaWiki, QObject* const parent = nullptr);

    /**
     * @brief Destroys the QuerySiteInfoGeneral job.
     */
    virtual ~QuerySiteInfoGeneral();

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:

    /**
     * @brief Provide general info.
     * @param generalinfo the general info
     */
    void result(const Generalinfo& generalinfo);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_QUERYSITEINFOGENERAL_H

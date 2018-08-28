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
 * Copyright (C) 2011      by Vincent Garcia <xavier dot vincent dot garcia at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_PARSE_H
#define DIGIKAM_MEDIAWIKI_PARSE_H

// Qt includes

#include <QString>
#include <QLocale>

// Local includes

#include "mediawiki_job.h"
#include "digikam_export.h"

namespace MediaWiki
{

class Iface;
class ParsePrivate;

class DIGIKAM_EXPORT Parse : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Parse)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
         /**
         * @brief An internal error occurred.
         */
        InternalError= Job::UserDefinedError+1,

        /**
         * @brief The page parameter cannot be used together with the text and title parameters
         */
        TooManyParams,

        /**
         * @brief The page you specified doesn't exist
         */
        MissingPage
    };

    explicit Parse(Iface& MediaWiki, QObject* const parent = 0);
    virtual ~Parse();

    void setText(const QString& param);

    void setTitle(const QString& param);

    void setPageName(const QString& param);

    void setUseLang(const QString& param);

    void start() Q_DECL_OVERRIDE;

    Q_SIGNALS:

    void result(const QString& text);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_PARSE_H

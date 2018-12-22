/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2011      by Paolo de Vathaire <paolo dot devathaire at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_JOB_H
#define DIGIKAM_MEDIAWIKI_JOB_H

// KDE includes

#include <kjob.h>

// Local includes

#include "digikam_export.h"

namespace MediaWiki
{

class Iface;
class JobPrivate;

/**
 * @brief The base class for all Iface jobs.
 */
class DIGIKAM_EXPORT Job : public KJob
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Job)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
        NetworkError            = KJob::UserDefinedError + 1,
        XmlError,
        UserRequestDefinedError = KJob::UserDefinedError + 100,
        MissingMandatoryParameter
    };

public:

    /**
     * @brief Destructs the Job.
     */
    virtual ~Job();

    /**
     * @brief Aborts this job quietly.
     */
    bool doKill() Q_DECL_OVERRIDE;

protected:

    /**
     * @brief Constructs a Job by a private class.
     * @param dd a private class
     * @param parent the QObject parent
     */
    Job(JobPrivate& dd, QObject* const parent = 0);

    /**
     * @brief Connects signals of the reply object (in the private object) to
     * slots of this base class.
     */
    void connectReply();

    /**
     * @brief The private d pointer.
     */
    JobPrivate* const d_ptr;

private Q_SLOTS:

    void processUploadProgress(qint64 bytesReceived, qint64 bytesTotal);
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_JOB_H

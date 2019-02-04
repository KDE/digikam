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

#ifndef DIGIKAM_MEDIAWIKI_UPLOAD_H
#define DIGIKAM_MEDIAWIKI_UPLOAD_H

// Qt includes

#include <QIODevice>
#include <QString>
#include <QDateTime>
#include <QNetworkCookieJar>
#include <QUrl>

// Local includes

#include "digikam_export.h"
#include "mediawiki_job.h"
#include "mediawiki_queryinfo.h"

namespace MediaWiki
{

class Iface;
class UploadPrivate;

/**
 * @brief Upload job.
 *
 * Uses for upload files.
 */
class DIGIKAM_EXPORT Upload : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Upload)

public:

    enum
    {
        /**
         * @brief An internal error occurred.
         */
        InternalError= Job::UserDefinedError+1,
        /**
         * @brief The module is disabled.
         */
        UploadDisabled,

        /**
         * @brief The session key is invalid.
         */
        InvalidSessionKey,

        /**
         * @brief The current user can't upload.
         */
        BadAccess,

        /**
         * @brief A param is missing.
         */
        ParamMissing,

        /**
         * @brief No upload without logged in.
         */
        MustBeLoggedIn,

        /**
         * @brief
         */
        FetchFileError,

        /**
         * @brief No upload module set.
         */
        NoModule,

        /**
         * @brief The file submitted was empty.
         */
        EmptyFile,

        /**
         * @brief The file is missing an extension.
         */
        ExtensionMissing,

        /**
         * @brief The filename is too short.
         */
        TooShortFilename,

        /**
         * @brief Overwriting an existing file is not allowed.
         */
        OverWriting,

        /**
         * @brief Stashing temporary file failed.
         */
        StashFailed
    };

public:

    /**
     * @brief Constructs an Upload job.
     * @param parent the QObject parent
     */
    Upload(Iface& MediaWiki, QObject* const parent = 0);

    /**
     * @brief Destroys the Upload job.
     */
    virtual ~Upload();

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * @brief Set the target filename.
     * @param filename the filename
     */
    void setFilename(const QString&);

    /**
     * @brief Set the file.
     * @param file the file
     */
    void setFile(QIODevice* const);

    /**
     * @brief Set the upload comment. Also used as the initial page text for new files if text parameter not provided.
     * @param comment the comment
     */
    void setComment(const QString&);

    /**
     * @brief Set the information of the image. Use this template {{Information|Description=|Source=|Date=|Author=|Permission=|other_versions=}}.
     * @param text the text
     */
    void setText(const QString&);

private Q_SLOTS:

    void doWorkSendRequest(Page page);
    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // UPLOAD_H

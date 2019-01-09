/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_QUERYIMAGES_H
#define DIGIKAM_MEDIAWIKI_QUERYIMAGES_H

// KDE includes

#include <kjob.h>

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_image.h"
#include "digikam_export.h"

namespace MediaWiki
{

class Iface;
class QueryImagesPrivate;

/**
 * @brief Query images job.
 *
 * Gets a list of all images used on pages.
 */
class DIGIKAM_EXPORT QueryImages : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryImages)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the job.
     */
    enum
    {
        /**
         * @brief A network error has occurred.
         */
        NetworkError = KJob::UserDefinedError + 1,

        /**
         * @brief A XML error has occurred.
         */
        XmlError
    };

public:

    /**
     * @brief Constructs a query images job.
     * @param MediaWiki the MediaWiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QueryImages(Iface& MediaWiki, QObject* const parent = 0);

    /**
     * @brief Destroys a query images job.
     */
    virtual ~QueryImages();

    /**
     * @brief Set the title.
     * @param title the title of the page
     */
    void setTitle(const QString& title);

    /**
     * @brief Set the limit.
     * @param limit number of images returned by one signal #pages()
     */
    void setLimit(unsigned int limit);

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:

    /**
     * @brief Provides a list of all images used on pages
     *
     * This signal can be emitted several times.
     *
     * @param pages list of all images used on pages
     */
    void images(const QList<Image> & images);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_QUERYIMAGES_H

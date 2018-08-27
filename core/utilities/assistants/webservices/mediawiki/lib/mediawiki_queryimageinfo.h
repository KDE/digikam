/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
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

#ifndef MEDIAWIKI_QUERYIMAGEINFO_H
#define MEDIAWIKI_QUERYIMAGEINFO_H

// Qt includes

#include <QList>

// Local includes

#include "mediawiki_imageinfo.h"
#include "mediawiki_job.h"
#include "digikam_export.h"

namespace mediawiki
{

class Iface;
class QueryImageinfoPrivate;

/**
 * @brief Query image info job.
 *
 * Gets image information for an image.
 */
class DIGIKAM_EXPORT QueryImageinfo : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QueryImageinfo)

public:

    /**
    * @brief Property.
    */
    enum Property
    {
        Timestamp = 0x01,
        User      = 0x02,
        Comment   = 0x04,
        Url       = 0x08,
        Size      = 0x10,
        Sha1      = 0x20,
        Mime      = 0x40,
        Metadata  = 0x80
    };
    Q_DECLARE_FLAGS(Properties, Property)

public:

    /**
     * @brief Constructs a query image info job.
     *
     * You can set parameters of the query after.
     *
     * By default, a single image info is returned.
     *
     * @param mediawiki the mediawiki concerned by the job
     * @param parent the QObject parent
     */
    explicit QueryImageinfo(Iface& mediawiki, QObject* const parent = 0);

    /**
     * @brief Destructs a query image info job.
     */
    virtual ~QueryImageinfo();

    /**
     * @brief Set the title of the image requested.
     * @param title the title of the image requested
     */
    void setTitle(const QString& title);

    /**
     * @brief Set which properties to get.
     * @param properties properties to get
     */
    void setProperties(Properties properties);

    /**
     * @brief Set how many image info to return per signal.
     * @param limit how many image info to return per signal
     */
    void setLimit(unsigned int limit);

    /**
     * @brief If true stop the request to the first signal.
     * @param onlyOneSignal if true stop the request to the first signal
     */
    void setOnlyOneSignal(bool onlyOneSignal);

    /**
     * @brief Set timestamp to start listing from.
     * @param begin timestamp to start listing from
     */
    void setBeginTimestamp(const QDateTime& begin);

    /**
     * @brief Set timestamp to stop listing at.
     * @param end timestamp to stop listing at
     */
    void setEndTimestamp(const QDateTime& end);

    /**
     * @brief Set width scale.
     *
     * Only for the first image info. The property URL must be set.
     *
     * @param width width scale
     */
    void setWidthScale(unsigned int width);

    /**
     * @brief Set height scale.
     *
     * Only for the first image info. The property URL must be set.
     *
     * @param height height scale
     */
    void setHeightScale(unsigned int height);

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

Q_SIGNALS:

    /**
    * @brief Provides a list of imageinfos.
    * @param imageinfos a list of imageinfos
    */
    void result(const QList<Imageinfo>& imageinfos);

private Q_SLOTS:

    void doWorkSendRequest();
    void doWorkProcessReply();
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QueryImageinfo::Properties)

} // namespace mediawiki

#endif // MEDIAWIKI_QUERYIMAGEINFO_H

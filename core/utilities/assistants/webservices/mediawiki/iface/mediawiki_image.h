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

#ifndef DIGIKAM_MEDIAWIKI_IMAGE_H
#define DIGIKAM_MEDIAWIKI_IMAGE_H

// Qt includes

#include <QString>

// Local includes

#include "digikam_export.h"

namespace MediaWiki
{

/**
 * @brief A image.
 */
class DIGIKAM_EXPORT Image
{
public:

    /**
     * @brief Constructs a image.
     */
    Image();

    /**
     * @brief Constructs an image from an other image.
     * @param other an other image
     */
    Image(const Image& other);

    /**
     * @brief Destructs an image.
     */
    ~Image();

    /**
     * @brief Assigning an image from an other image.
     * @param other an other image
     */
    Image& operator=(Image other);

    /**
     * @brief Returns true if this instance and other are equal, else false.
     * @param other instance to compare
     * @return true if there are equal, else false
     */
    bool operator==(const Image& other) const;

    /**
     * @brief Returns the namespace id of the image.
     * @return the namespace id of the image
     */
    qint64 namespaceId() const;

    /**
     * @brief Set the namespace id.
     * @param namespaceId the namespace id of the image
     */
    void setNamespaceId(qint64 namespaceId);

    /**
     * @brief Returns the title of the image.
     * @return the title of the image
     */
    QString title() const;

    /**
     * @brief Set the title.
     * @param title the title of the image
     */
    void setTitle(const QString& title);

private:

    class Private;
    Private* const d;
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_IMAGE_H

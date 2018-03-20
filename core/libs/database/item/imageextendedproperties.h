/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-04
 * Description : Access to extended properties of an image in the database
 *
 * Copyright (C) 2009-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef IMAGEEXTENDEDPROPERTIES_H
#define IMAGEEXTENDEDPROPERTIES_H

// Qt includes

#include <QString>
#include <QStringList>
#include <QList>

// Local includes

#include "template.h"
#include "metadatainfo.h"
#include "digikam_export.h"

namespace Digikam
{


class DIGIKAM_DATABASE_EXPORT ImageExtendedProperties
{

public:

    explicit ImageExtendedProperties(qlonglong imageid);

    /** Create a null ImageExtendedProperties object */
    ImageExtendedProperties();

    /**
     * Return the Intellectual Genre.
     * This is Photoshop Object Attribute Reference.
     * &ldquo; Describes the nature, intellectual or journalistic characteristic of a news object,
     *   not specifically its content.
     *   Note / Examples:
     *   Journalistic genres: actuality, interview, background, feature, summary, wrapup
     *   News category related genres: daybook, obituary, press release, transcript
     *   It is advised to use terms from a controlled vocabulary.&rdquo;
     */
    QString intellectualGenre();
    void setIntellectualGenre(const QString& intellectualGenre);
    void removeIntellectualGenre();

    /**
     * Returns the Job ID.
     * This is Photoshop Transmission Reference.
     * This is IPTC Original Transmission Reference
     * &ldquo; Number or identifier for the purpose of improved workflow handling. This ID should be
     *   added by the creator or provider for transmission and routing purposes only and should
     *   have no significance for archiving.&rdquo;
     */
    QString jobId();
    void setJobId(const QString& jobId);
    void removeJobId();

    /**
     * Returns the Scene.
     * &ldquo; Describes the scene of a photo content. Specifies one ore more terms from the
     *   IPTC &lsquo;Scene-NewsCodes&rsquo;. Each Scene is represented as a string of 6 digits in an unordered list.&rdquo;
     */
    QStringList scene();
    void setScene(const QStringList& scene);
    void removeScene();

    /**
     * Returns the Subject Code.
     * This is IPTC Subject Reference.
     * &ldquo; Specifies one or more Subjects from the IPTC &lsquo;Subject-NewsCodes&rsquo; taxonomy to categorize
     *   the content. Each Subject is represented as a string of 8 digits in an unordered list.
     *   Note: Only Subjects from a controlled vocabulary should be used in this metadata
     *   element, free text has to be put into the Keyword element. More about IPTC
     *   Subject-NewsCodes at www.newscodes.org.&rdquo;
     */
    QStringList subjectCode();
    void setSubjectCode(const QStringList& subjectCode);
    void removeSubjectCode();

    /**
     * Returns the similarity. of the image to the given image.
     */
    double similarityTo(const qlonglong imageId);
    void setSimilarityTo(const qlonglong imageId, const double value);
    void removeSimilarityTo(const qlonglong imageId);

    /**
     * Return the IPTC Core Location.
     * This includes Country, Country Code, City, Location and ProvinceState.
     * This includes IPTC Country Name, Country Code, City, SubLocation and ProvinceState.
     */
    IptcCoreLocationInfo location();
    void setLocation(const IptcCoreLocationInfo& location);
    void removeLocation();

protected:

    QString     readProperty(const QString& property);
    void        setProperty(const QString& property, const QString& value);
    QStringList readFakeListProperty(const QString& property);
    void        setFakeListProperty(const QString& property, const QStringList& value);
    void        removeProperty(const QString& property);

protected:

    qlonglong m_id;
};

} // namespace Digikam

#endif // IMAGEEXTENDEDPROPERTIES_H

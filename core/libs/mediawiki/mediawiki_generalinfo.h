/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-03-22
 * Description : a Iface C++ interface
 *
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_MEDIAWIKI_GENERALINFO_H
#define DIGIKAM_MEDIAWIKI_GENERALINFO_H

// Qt includes

#include <QDateTime>
#include <QString>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace MediaWiki
{

/**
 * @brief A general info.
 */
class DIGIKAM_EXPORT Generalinfo
{

public:

    /**
     * @brief Constructs a general info.
     */
    Generalinfo();

    /**
     * @brief Constructs a generalinfo from an other generalinfo.
     * @param other an other generalinfo
     */
    Generalinfo(const Generalinfo& other);

    /**
     * @brief Destructs a general info.
     */
    ~Generalinfo();

    /**
     * @brief Assigning an image from an other image.
     * @param other an other image
     */
    Generalinfo& operator=(const Generalinfo& other);

    /**
     * @brief Returns true if this instance and other are equal, else false.
     * @param other instance to compare
     * @return true if there are equal, else false
     */
    bool operator==(const Generalinfo& other) const;

    /**
     * @brief Get the name of the main page.
     * @return the name of the main page
     */
    QString mainPage() const;

    /**
     * @brief Set the name of the main page.
     * @param mainPage the name of the main page
     */
    void setMainPage(const QString& mainPage);

    /**
     * @brief Get the url of the page.
     * @return the url of the page
     */
    QUrl url() const;

    /**
     * @brief Set the url of the page.
     * @param url the url of the page
     */
    void setUrl(const QUrl& url);

    /**
     * @brief Get the name of the web site.
     * @return the name of the web site
     */
    QString siteName() const;

    /**
     * @brief Set the name of the web site.
     * @param siteName the name of the web site
     */
    void setSiteName(const QString& siteName);

    /**
     * @brief Get the generator.
     * @return the generator
     */
    QString generator() const;

    /**
     * @brief Set the generator.
     * @param generator
     */
    void setGenerator(const QString& generator);

    /**
     * @brief Get the PHP version.
     * @return the PHP version
     */
    QString phpVersion() const;

    /**
     * @brief Set the PHP version.
     * @param phpVersion the PHP version
     */
    void setPhpVersion(const QString& phpVersion);

    /**
     * @brief Get the PHP API name.
     * @return the PHP API name
     */
    QString phpApi() const;

    /**
     * @brief Set the PHP API name.
     * @param phpApi the PHP API name
     */
    void setPhpApi(const QString& phpApi);

    /**
     * @brief Get the type of the database.
     * @return the type of the database
     */
    QString dataBaseType() const;

    /**
     * @brief Set the type of the database.
     * @param dataBaseType the type of the database
     */
    void setDataBaseType(const QString& dataBaseType);

    /**
     * @brief Get the version of the database.
     * @return the version of the database
     */
    QString dataBaseVersion() const;

    /**
     * @brief Set the version of the database.
     * @param dataBaseVersion the version of the database
     */
    void setDataBaseVersion(const QString& dataBaseVersion);

    /**
     * @brief Get the rev number.
     * @return the rev number
     */
    QString rev() const;

    /**
     * @brief Set the rev number.
     * @param rev the rev number
     */
    void setRev(const QString& rev);

    /**
     * @brief Get the case.
     * @return the case
     */
    QString cas() const;

    /**
     * @brief Set the case.
     * @param cas the case
     */
    void setCas(const QString& cas);

    /**
     * @brief Get the license.
     * @return the license
     */
    QString license() const;

    /**
     * @brief Set the license.
     * @param license the license
     */
    void setLicense(const QString& license);

    /**
     * @brief Get the language.
     * @return the language
     */
    QString language() const;

    /**
     * @brief Set the language.
     * @param language
     */
    void setLanguage(const QString& language);

    /**
     * @brief Get the fallBack8bitEncoding.
     * @return the fallBack8bitEncoding
     */
    QString fallBack8bitEncoding() const;

    /**
     * @brief Set the fallBack8bitEncoding.
     * @param fallBack8bitEncoding
     */
    void setFallBack8bitEncoding(const QString& fallBack8bitEncoding);

    /**
     * @brief Get the writeApi.
     * @return the writeApi
     */
    QString writeApi() const;

    /**
     * @brief Set the writeApi.
     * @param writeApi
     */
    void setWriteApi(const QString& writeApi);

    /**
     * @brief Get the timeZone.
     * @return the timeZone
     */
    QString timeZone() const;

    /**
     * @brief Set the timeZone.
     * @param timeZone
     */
    void setTimeZone(const QString& timeZone);

    /**
     * @brief Get the timeOffset.
     * @return the timeOffset
     */
    QString timeOffset() const;

    /**
     * @brief Set the timeOffset.
     * @param timeOffset
     */
    void setTimeOffset(const QString& timeOffset);

    /**
     * @brief Get the path of the article.
     * @return the path of the article
     */
    QString articlePath() const;

    /**
     * @brief Set the path of the article.
     * @param articlePath the path of the article
     */
    void setArticlePath(const QString& articlePath);

    /**
     * @brief Get the path of the script.
     * @return the path of the script
     */
    QString scriptPath() const;

    /**
     * @brief Set the path of the script.
     * @param scriptPath the path of the script
     */
    void setScriptPath(const QString& scriptPath);

    /**
     * @brief Get the path of the script file.
     * @return the path of the script file
     */
    QString script() const;

    /**
     * @brief Set the path of the script file.
     * @param script the path of the script file
     */
    void setScript(const QString& script);

    /**
     * @brief Get the path of the variant article.
     * @return the path of the variant article
     */
    QString variantArticlePath() const;

    /**
     * @brief Set the path of the variant article.
     * @param variantArticlePath the path of the variant article
     */
    void setVariantArticlePath(const QString& variantArticlePath);

    /**
     * @brief Get the url of the server.
     * @return the url of the server
     */
    QUrl serverUrl() const;

    /**
     * @brief Set the url of the server.
     * @param serverUrl the url of the server
     */
    void setServerUrl(const QUrl& serverUrl);

    /**
     * @brief Get the id of the wiki.
     * @return the id of the wiki
     */
    QString wikiId() const;

    /**
     * @brief Set the id of the wiki.
     * @param wikiId the id of the wiki
     */
    void setWikiId(const QString& wikiId);

    /**
     * @brief Get the time.
     * @return the time
     */
    QDateTime time() const;

    /**
     * @brief Set the time.
     * @param time
     */
    void setTime(const QDateTime& time);

private:

    class Private;
    Private* const d;
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_GENERALINFO_H

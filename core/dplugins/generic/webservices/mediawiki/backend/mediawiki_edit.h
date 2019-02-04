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

#ifndef DIGIKAM_MEDIAWIKI_EDIT_H
#define DIGIKAM_MEDIAWIKI_EDIT_H

// Qt includes

#include <QDateTime>
#include <QString>
#include <QUrl>
#include <QVariant>
#include <QNetworkCookieJar>

// Local includes

#include "mediawiki_job.h"
#include "mediawiki_queryinfo.h"
#include "digikam_export.h"

namespace MediaWiki
{

class Iface;
class EditPrivate;

/**
 * @brief Edit job.
 *
 * Uses for create or edit a wiki.
 */
class DIGIKAM_EXPORT Edit : public Job
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(Edit)

public:

    /**
     * @brief Indicates all possible error conditions found during the processing of the edit.
     */
    enum
    {
        /**
         * @brief Text is missing.
         */
        TextMissing = Job::UserDefinedError+1,

        /**
         * @brief The section is invalid.
         */
        InvalidSection,

        /**
         * @brief The page name is protected.
         */
        TitleProtected,

        /**
         * @brief The permission for create page is missing.
         */
        CreatePagePermissionMissing,

        /**
         * @brief The permission for create page is missing for anonymous.
         */
        AnonymousCreatePagePermissionMissing,

        /**
         * @brief The article already exist.
         */
        ArticleDuplication,

        /**
         * @brief The permission for create image is missing for anonymous.
         */
        AnonymousCreateImagePermissionMissing,

        /**
         * @brief The permission for create image is missing.
         */
        CreateImagePermissionMissing,

        /**
         * @brief The MediaWiki considers you are spamming.
         */
        SpamDetected,

        /**
         * @brief The MediaWiki refuses your edit.
         */
        Filtered,

        /**
         * @brief The size of the article exceed.
         */
        ArticleSizeExceed,

        /**
         * @brief The permission for edit page is missing for anonymous.
         */
        AnonymousEditPagePermissionMissing,

        /**
         * @brief The permission for edit page is missing.
         */
        EditPagePermissionMissing,

        /**
         * @brief The page is deleted.
         */
        PageDeleted,

        /**
         * @brief The page is empty.
         */
        EmptyPage,

        /**
         * @brief The section is empty.
         */
        EmptySection,

        /**
         * @brief Mediwiki detect an edit conflict.
         */
        EditConflict,

        /**
         * @brief The revision isn't a valid revision.
         */
        RevWrongPage,

        /**
         * @brief The undo failed.
         */
        UndoFailed
    };

    /**
     * @brief Specify how the watchlist is affected by this edit.
     */
    enum Watchlist
    {
        /**
         * @brief Add the page to the watchlist
         */
        watch,

        /**
         * @brief Remove the page from the watchlist
         */
        unwatch,

        /**
         * @brief Use the preference settings
         */
        preferences,

        /**
         * @brief Don't change the watchlist
         */
        nochange
    };

public:

    /**
     * @brief Constructs an Edit job.
     * @param parent the QObject parent
     */
    explicit Edit(Iface& media, QObject* const parent = 0);
    /**
     * @brief Destroys the Edit job.
     */
    virtual ~Edit();

    /**
     * @brief Starts the job asynchronously.
     */
    void start() Q_DECL_OVERRIDE;

    /**
     * @brief Specify how the watchlist is affected by this edit.
     * @param watchlist Specify how the watchlist is affected by this edit
     */
    void setWatchList(Edit::Watchlist watchlist);

    /**
     * @brief If set, suppress errors about the page having been deleted in the meantime and recreate it.
     * @param recreate if set, suppress errors about the page having been deleted in the meantime and recreate it
     */
    void setRecreate(bool recreate);

    /**
     * @brief If set, throw an error if the page already exists.
     * @param if set, throw an error if the page already exists
     */
    void setCreateonly(bool createonly);

    /**
     * @brief If set, throw a missingtitle error if the page doesn't exist.
     * @param norecreate if set, throw a missingtitle error if the page doesn't exist
     */
    void setNocreate(bool norecreate);

    /**
     * @brief If set to true, mark the edit as minor
     * @param minor if set to true, mark the edit as minor
     */
    void setMinor(bool minor);

    /**
     * @brief Set the section.
     * @param section new section or integer
     */
    void setSection(const QString& section);

    /**
     * @brief Set the summary.
     * @param summary the summary
     */
    void setSummary(const QString& summary);

    /**
     * @brief Undo all revisions from undo to this one. If not set, just undo one revision.
     * @param undoafter if true set the undo after property
     */
    void setUndoAfter(int undoafter);

    /**
     * @brief Undo this revision. Overrides text, prependtext and appendtext.
     * @param undo if true set the undo
     */
    void setUndo(int undo);

    /**
     * @brief Set the text added to the beginning of the page. Overrides text.
     * @param prependText the text added to the beginning of the page
     */
    void setPrependText(const QString& prependText);

    /**
     * @brief Set the text added to the end of the page. Overrides text.
     * @param appendText the text added to the end of the page
     */
    void setAppendText(const QString& appendText);

    /**
     * @brief Set the page title.
     * @param pageName the page title
     */
    void setPageName(const QString& pageName);

    /**
     * @brief Set the edit token. Retrieve from QueryInfo.
     * @param token the edit token
     */
    void setToken(const QString& token);

    /**
     * @brief Set the timestamp of the base revision. Leave unset to ignore conflit.
     * @param baseTimestamp the timestamp of the base revision
     */
    void setBaseTimestamp(const QDateTime& baseTimestamp);

    /**
     * @brief Set the timestamp when you obtained the edit token.
     * @param startTimestamp the timestamp when you obtained the edit token
     */
    void setStartTimestamp(const QDateTime& startTimestamp);

    /**
     * @brief Set the page content.
     * @param text the page content.
     */
    void setText(const QString& text);

Q_SIGNALS:

    /**
     * @brief Emit the captcha question.
     * @param captcha the captcha question
     */
    void resultCaptcha(const QVariant& captcha);

private Q_SLOTS:

    void doWorkSendRequest(Page page);
    void finishedEdit();

public Q_SLOTS:

    /**
     * @brief Put the captcha answer.
     * @param captcha the captcha answer
     */
    void finishedCaptcha(const QString& captcha);
};

} // namespace MediaWiki

#endif // DIGIKAM_MEDIAWIKI_EDIT_H

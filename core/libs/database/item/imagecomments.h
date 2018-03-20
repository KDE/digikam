/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Access to comments of an image in the database
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008      by Patrick Spendrin <ps_ml at gmx dot de>
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

#ifndef IMAGECOMMENTS_H
#define IMAGECOMMENTS_H

// Qt includes

#include <QString>
#include <QDateTime>
#include <QSharedDataPointer>
#include <QSharedData>
#include <QSet>

// Local includes

#include "digikam_export.h"
#include "coredbalbuminfo.h"
#include "captionvalues.h"
#include "coredbaccess.h"

namespace Digikam
{

class DIGIKAM_DATABASE_EXPORT ImageComments
{
public:

    /**
     * The ImageComments class shall provide short-lived objects that provide read/write access
     * to the comments stored in the database. It is a mere wrapper around the less
     * convenient access methods in CoreDB.
     * Database results are cached, but the object will not listen to database changes from other places.
     *
     * Changes are applied to the database only after calling apply(), which you can call any time
     * and which will in any case be called from the destructor.
     */

    enum LanguageChoiceBehavior
    {
        /**
         *  Return only a comment if the language code
         *  (at least the language code, the country part may differ)
         *  is identical. Else returns a null QString.
         */
        ReturnMatchingLanguageOnly,
        /// If no matching language as above is found, return the default language.
        ReturnMatchingOrDefaultLanguage,
        /** If no matching or default language is found, return the first comment.
         *  Returns a null string only if no comment is available.
         */
        ReturnMatchingDefaultOrFirstLanguage
    };

    enum UniqueBehavior
    {
        /** Allow only one comment per language. Default setting. */
        UniquePerLanguage,
        /** Allow multiple comments per language, each with a different author */
        UniquePerLanguageAndAuthor
    };

public:

    /** Create a null ImageComments object */
    ImageComments();

    /**
     * Create a ImageComments object for the image with the specified id.
     */
    explicit ImageComments(qlonglong imageid);

    /**
     * Create a ImageComments object for the image with the specified id.
     * The existing CoreDbAccess object will be used to access the database.
     */
    ImageComments(CoreDbAccess& access, qlonglong imageid);

    ImageComments(const ImageComments& other);
    ~ImageComments();

    ImageComments& operator=(const ImageComments& other);

    bool isNull() const;

    /**
     * Changes the behavior to unique comments per language, see the enum above for possible
     * values.
     * Default value is UniquePerLanguage.
     * Note: This is _not_ a property of the database, but only of this single ImageComments object,
     */
    void setUniqueBehavior(UniqueBehavior behavior);

    /**
     * This methods presents one of the comment strings of the available comment
     * as the default value, when you just want to have one string.
     * Optionally also returns the index with which you can access further information about the comment.
     */
    QString defaultComment(Digikam::DatabaseComment::Type type = DatabaseComment::Comment) const
    {
        return defaultComment(0, type);
    }

    QString defaultComment(int* const index, Digikam::DatabaseComment::Type type = DatabaseComment::Comment) const;

    /**
     * Returns a comment for the specified language.
     * Matching behavior can be specified.
     * Optionally also returns the index with which you can access further information about the comment.
     */
    QString commentForLanguage(const QString& languageCode, int* const index = 0,
                               LanguageChoiceBehavior behavior = ReturnMatchingDefaultOrFirstLanguage) const;

    /** Returns the number of comments available. */
    int numberOfComments()                const;

    /// Access individual properties. Please ensure that the specified index is a valid index
    DatabaseComment::Type type(int index) const;
    QString language(int index)           const; /// RFC 3066 notation, or "x-default"
    QString author(int index)             const;
    QDateTime date(int index)             const;
    QString comment(int index)            const;

    /**
     * Add a new comment to the list of normal image comments, specified with language and author.
     * Checking for unique comments is done as set by setUniqueBehavior.
     * If you pass a null string as language, it will be translated to the language code designating
     * the default language ("x-default").
     * If you just want to change the one comment of the image, call addComment(myComment);
     */
    void addComment(const QString& comment,
                    const QString& language = QString(),
                    const QString& author = QString(),
                    const QDateTime& date = QDateTime(),
                    DatabaseComment::Type type = DatabaseComment::Comment);

    /** Convenience method to add a comment of type Headline. Calls addComment, see above for more info. */
    void addHeadline(const QString& headline,
                     const QString& language = QString(),
                     const QString& author = QString(),
                     const QDateTime& date = QDateTime());

    /** Convenience method to add a comment of type Headline. Calls addComment, see above for more info. */
    void addTitle(const QString& title,
                  const QString& language = QString(),
                  const QString& author = QString(),
                  const QDateTime& date = QDateTime());

    /**
     * Replaces all existing comments with the given set of comments and associated language.
     * Optionally date and author can be specified in CaptionsMap container.
     */
    void replaceComments(const CaptionsMap& comments,
                         DatabaseComment::Type type = DatabaseComment::Comment);

    /**
     * Remove the entry referred to by index.
     */
    void remove(int index);

    /**
     * Remove all entries of the given type
     */
    void removeAll(DatabaseComment::Type type);

    /**
     * Convenience method: remove all entries of type Comment
     */
    void removeAllComments();

    /**
     * Remove all entries of all types: Comments, Headlines, Titles
     */
    void removeAll();

    /**
     * Access individual properties.
     * Please ensure that the specified index is a valid index
     */
    void changeComment(int index, const QString& comment);
    void changeLanguage(int index, const QString& language);
    void changeAuthor(int index, const QString& author);
    void changeDate(int index, const QDateTime& date);
    void changeType(int index, DatabaseComment::Type type);

    /**
     * Apply all changes.
     * Also called in destructor, so you typically do not need to call this.
     */
    void apply();
    void apply(CoreDbAccess& access);

    /**
     * Returns all entries of the given type in a CaptionsMap container.
     */
    CaptionsMap toCaptionsMap(DatabaseComment::Type = DatabaseComment::Comment) const;

    /**
     * Replaces all entries in this object with all entries from source.
     */
    void replaceFrom(const ImageComments& source);

    // If you need more methods, add your methods here!

protected:

    void addCommentDirectly(const QString& comment,
                            const QString& language,
                            const QString& author,
                            DatabaseComment::Type type,
                            const QDateTime& date);
public:

    class Private;

protected:

    QSharedDataPointer<Private> d;
};

} // namespace Digikam

#endif // IMAGECOMMENTS_H

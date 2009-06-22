/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Access to comments of an image in the database
 *
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imagecomments.h"

// KDE includes

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>

// Local includes

#include "albumdb.h"

using namespace KExiv2Iface;

namespace Digikam
{

class ImageCommentsPriv : public QSharedData
{
public:

    ImageCommentsPriv()
    {
        id     = -1;
        unique = ImageComments::UniquePerLanguage;
    }

    qlonglong                     id;
    QList<CommentInfo>            infos;
    QSet<int>                     dirtyIndices;
    QSet<int>                     newIndices;
    ImageComments::UniqueBehavior unique;

    void languageMatch(const QString& fullCode, const QString& langCode,
                       int& fullCodeMatch, int& langCodeMatch, int& defaultCodeMatch, int& firstMatch) const
    {
        // if you change the algorithm, please take a look at ImageCopyright as well
        fullCodeMatch    = -1;
        langCodeMatch    = -1;
        defaultCodeMatch = -1;
        firstMatch       = -1;

        if (infos.isEmpty())
        {
            return;
        }
        else
        {
            firstMatch = 0; // index of first entry - at least we have one
        }

        // First we search for a full match
        // Second for a match of the language code
        // Third for the default code
        // Fourth we return the first comment

        QLatin1String defaultCode("x-default");

        for (int i=0; i<infos.size(); ++i)
        {
            const CommentInfo& info = infos[i];
            if (info.type == DatabaseComment::Comment)
            {
                if (info.language == fullCode)
                {
                    fullCodeMatch = i;
                    break;
                }
                else if (info.language.startsWith(langCode) && langCodeMatch == -1)
                {
                    langCodeMatch = i;
                }
                else if (info.language == defaultCode)
                {
                    defaultCodeMatch = i;
                }
            }
        }
    }
};

ImageComments::ImageComments()
{
}

ImageComments::ImageComments(qlonglong imageid)
{
    d = new ImageCommentsPriv;
    d->id    = imageid;
    DatabaseAccess access;
    d->infos = access.db()->getImageComments(imageid);
}

ImageComments::ImageComments(DatabaseAccess& access, qlonglong imageid)
{
    d = new ImageCommentsPriv;
    d->id    = imageid;
    d->infos = access.db()->getImageComments(imageid);
}

ImageComments::ImageComments(const ImageComments& other)
{
    d = other.d;
}

ImageComments::~ImageComments()
{
    apply();
}

ImageComments& ImageComments::operator=(const ImageComments& other)
{
    d = other.d;
    return *this;
}

bool ImageComments::isNull() const
{
    return !d;
}

QString ImageComments::defaultComment(int *index) const
{
    if (!d)
        return QString();

    KLocale *locale  = KGlobal::locale();
    QString langCode = locale->language().toLower() + '-';
    QString fullCode = langCode + locale->country().toLower();

    int fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch;

    d->languageMatch(fullCode, langCode, fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch);

    int chosen = fullCodeMatch;
    if (chosen == -1)
        chosen = langCodeMatch;
    if (chosen == -1)
        chosen = defaultCodeMatch;
    if (chosen == -1)
        chosen = firstMatch;

    if (index)
        *index = chosen;

    if (chosen == -1)
        return QString();
    else
        return d->infos[chosen].comment;
}

QString ImageComments::commentForLanguage(const QString& languageCode, int *index, 
                                          LanguageChoiceBehavior behavior) const
{
    if (!d)
        return QString();

    int fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch;

    // en-us => en-
    QString firstPart;
    if (languageCode == "x-default")
        firstPart = languageCode;
    else
        firstPart = languageCode.section('-', 0, 0, QString::SectionIncludeTrailingSep);

    d->languageMatch(languageCode, firstPart, fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch);

    int chosen = fullCodeMatch;
    if (chosen == -1)
        chosen = langCodeMatch;
    if (chosen == -1 && behavior > ReturnMatchingLanguageOnly)
    {
        chosen = defaultCodeMatch;
        if (chosen == -1 && behavior == ReturnMatchingDefaultOrFirstLanguage)
            chosen = firstMatch;
    }

    if (index)
        *index = chosen;

    if (chosen == -1)
        return QString();
    else
        return d->infos[chosen].comment;
}

int ImageComments::numberOfComments() const
{
    if (!d)
        return 0;

    return d->infos.size();
}

DatabaseComment::Type ImageComments::type(int index) const
{
    if (!d)
        return DatabaseComment::UndefinedType;

    return d->infos[index].type;
}

QString ImageComments::language(int index) const
{
    if (!d)
        return QString();

    return d->infos[index].language;
}

QString ImageComments::author(int index) const
{
    if (!d)
        return QString();

    return d->infos[index].author;
}

QDateTime ImageComments::date(int index) const
{
    if (!d)
        return QDateTime();

    return d->infos[index].date;
}

QString ImageComments::comment(int index) const
{
    if (!d)
        return QString();

    return d->infos[index].comment;
}

void ImageComments::setUniqueBehavior(UniqueBehavior behavior)
{
    if (!d)
        return;

    d->unique = behavior;
}

void ImageComments::addComment(const QString& comment, const QString& lang, const QString& author,
                               const QDateTime& date, DatabaseComment::Type type)
{
    if (!d)
        return;

    bool multipleCommentsPerLanguage = (d->unique == UniquePerLanguageAndAuthor);
    QString language = lang;
    if (language.isNull())
        language = "x-default";

    for (int i=0; i<d->infos.size(); ++i)
    {
        CommentInfo& info = d->infos[i];

        // some extra considerations on replacing
        if (info.type == DatabaseComment::Comment && info.language == language)
        {
            if ( !multipleCommentsPerLanguage
                 || (multipleCommentsPerLanguage && info.author == author) )
            {
                info.comment = comment;
                info.date    = date;
                info.author  = author;
                d->dirtyIndices << i;
                return;
            }
        }

        // simulate unique restrictions of db
        if (info.type == type && info.language == language && info.author == author)
        {
            info.comment = comment;
            info.date    = date;
            d->dirtyIndices << i;
            return;
        }
    }

    return addCommentDirect(comment, language, author, type, date);
}

void ImageComments::addHeadline(const QString& comment, const QString& lang,
                                const QString& author, const QDateTime& date)
{
    return addComment(comment, lang, author, date, DatabaseComment::Headline);
}

void ImageComments::addTitle(const QString& comment, const QString& lang,
                             const QString& author, const QDateTime& date)
{
    return addComment(comment, lang, author, date, DatabaseComment::Title);
}

void ImageComments::addCommentDirect(const QString& comment, const QString& language, const QString& author,
                                     DatabaseComment::Type type, const QDateTime& date)
{
    CommentInfo info;
    info.comment  = comment;
    info.language = language;
    info.author   = author;
    info.type     = type;
    info.date     = date;

    d->newIndices << d->infos.size();
    d->infos      << info;
}

void ImageComments::changeComment(int index, const QString& comment)
{
    if (!d)
        return;

    d->infos[index].comment = comment;
    d->dirtyIndices << index;
}

void ImageComments::changeLanguage(int index, const QString& language)
{
     if (!d)
        return;

    d->infos[index].language = language;
    d->dirtyIndices << index;
}

void ImageComments::changeAuthor(int index, const QString& author)
{
    if (!d)
        return;

    d->infos[index].author = author;
    d->dirtyIndices << index;
}

void ImageComments::changeDate(int index, const QDateTime& date)
{
    if (!d)
        return;

    d->infos[index].date = date;
    d->dirtyIndices << index;
}

void ImageComments::changeType(int index, DatabaseComment::Type type)
{
    if (!d)
        return;

    d->infos[index].type = type;
    d->dirtyIndices << index;
}

void ImageComments::apply()
{
    if (!d)
        return;

    DatabaseAccess access;
    apply(access);
}

void ImageComments::apply(DatabaseAccess& access)
{
    if (!d)
        return;

    foreach(int index, d->newIndices)
    {
        CommentInfo& info = d->infos[index];
        DatabaseAccess access;
        info.id = access.db()->setImageComment(d->id, info.comment, info.type, info.language, info.author, info.date);
    }
    d->dirtyIndices.subtract(d->newIndices);
    d->newIndices.clear();

    foreach(int index, d->dirtyIndices)
    {
        QVariantList values;
        CommentInfo& info = d->infos[index];
        values << (int)info.type << info.language << info.author << info.date << info.comment;
        access.db()->changeImageComment(info.id, d->id, values);
    }
    d->dirtyIndices.clear();
}

void ImageComments::setAltComments(const KExiv2::AltLangMap& comments)
{
    for (int i = 0 ; i < numberOfComments() ; ++i)
    {
        CommentInfo& info = d->infos[i];
        kDebug(50003) << "Remove Comment from DB: " << info.comment;
        DatabaseAccess access;
        access.db()->removeImageComment(info.id, d->id);
    }

    d->infos.clear();

    for (KExiv2::AltLangMap::const_iterator it = comments.constBegin(); it != comments.constEnd(); ++it)
        addComment(it.value(), it.key());

    apply();
}

KExiv2::AltLangMap ImageComments::altComments() const
{
    KExiv2::AltLangMap comments;

    for (int i = 0 ; i < numberOfComments() ; ++i)
        comments.insert(language(i), comment(i));

    return comments;
}

} // namespace Digikam

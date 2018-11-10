/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Access to comments of an item in the database
 *
 * Copyright (C) 2007-2013 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "itemcomments.h"

// Qt includes

#include <QLocale>

// Local includes

#include "coredb.h"

namespace Digikam
{

class Q_DECL_HIDDEN ItemComments::Private : public QSharedData
{
public:

    explicit Private()
      : id(-1),
        unique(ItemComments::UniquePerLanguage)
    {
    }

    void init(CoreDbAccess& access, qlonglong imageId)
    {
        id    = imageId;
        infos = access.db()->getItemComments(id);

        for (int i = 0 ; i < infos.size() ; ++i)
        {
            CommentInfo& info = infos[i];

            if (info.language.isNull())
            {
                info.language = QLatin1String("x-default");
            }
        }
    }

    void languageMatch(const QString& fullCode,
                       const QString& langCode,
                       int& fullCodeMatch,
                       int& langCodeMatch,
                       int& defaultCodeMatch,
                       int& firstMatch,
                       DatabaseComment::Type type = DatabaseComment::Comment) const
    {
        // if you change the algorithm, please take a look at ItemCopyright as well
        fullCodeMatch    = -1;
        langCodeMatch    = -1;
        defaultCodeMatch = -1;
        firstMatch       = -1;

        if (infos.isEmpty())
        {
            return;
        }

        // First we search for a full match
        // Second for a match of the language code
        // Third for the default code
        // Fourth we return the first comment

        QLatin1String defaultCode("x-default");

        for (int i = 0 ; i < infos.size() ; ++i)
        {
            const CommentInfo& info = infos.at(i);

            if (info.type == type)
            {
                if (firstMatch == -1)
                {
                    firstMatch = i;
                }
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

    void adjustStoredIndexes(QSet<int> &set, int removedIndex)
    {
        QSet<int> newSet;

        foreach(int index, set)
        {
            if (index > removedIndex)
            {
                newSet << index - 1;
            }
            else if (index < removedIndex)
            {
                newSet << index;
            }

            // drop index == removedIndex
        }

        set = newSet;
    }

    void adjustStoredIndexes(int removedIndex)
    {
        adjustStoredIndexes(dirtyIndices, removedIndex);
        adjustStoredIndexes(newIndices, removedIndex);
    }

public:

    qlonglong                     id;
    QList<CommentInfo>            infos;
    QSet<int>                     dirtyIndices;
    QSet<int>                     newIndices;
    QSet<int>                     idsToRemove;
    ItemComments::UniqueBehavior unique;
};

ItemComments::ItemComments()
    : d(0)
{
}

ItemComments::ItemComments(qlonglong imageid)
    : d(new Private)
{
    CoreDbAccess access;
    d->init(access, imageid);
}

ItemComments::ItemComments(CoreDbAccess& access, qlonglong imageid)
    : d(new Private)
{
    d->init(access, imageid);
}

ItemComments::ItemComments(const ItemComments& other)
{
    d = other.d;
}

ItemComments::~ItemComments()
{
    apply();
}

ItemComments& ItemComments::operator=(const ItemComments& other)
{
    d = other.d;
    return *this;
}

bool ItemComments::isNull() const
{
    return !d;
}

QString ItemComments::defaultComment(int* const index, DatabaseComment::Type type) const
{
    if (!d)
    {
        return QString();
    }

    QString spec     = QLocale().name().toLower();
    QString langCode = spec.left(spec.indexOf(QLatin1Char('_'))) + QLatin1Char('-');
    QString fullCode = spec.replace(QLatin1Char('_'), QLatin1Char('-'));

    int fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch;

    d->languageMatch(fullCode, langCode, fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch, type);

    int chosen = fullCodeMatch;

    if (chosen == -1)
    {
        chosen = langCodeMatch;
    }

    if (chosen == -1)
    {
        chosen = defaultCodeMatch;
    }

    if (chosen == -1)
    {
        chosen = firstMatch;
    }

    if (index)
    {
        *index = chosen;
    }

    if (chosen == -1)
    {
        return QString();
    }
    else
    {
        return d->infos.at(chosen).comment;
    }
}

QString ItemComments::commentForLanguage(const QString& languageCode,
                                          int* const index,
                                          LanguageChoiceBehavior behavior) const
{
    if (!d)
    {
        return QString();
    }

    int fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch;

    // en-us => en-
    QString firstPart;

    if (languageCode == QLatin1String("x-default"))
    {
        firstPart = languageCode;
    }
    else
    {
        firstPart = languageCode.section(QLatin1Char('-'), 0, 0, QString::SectionIncludeTrailingSep);
    }

    d->languageMatch(languageCode, firstPart, fullCodeMatch, langCodeMatch, defaultCodeMatch, firstMatch);

    int chosen = fullCodeMatch;

    if (chosen == -1)
    {
        chosen = langCodeMatch;
    }

    if (chosen == -1 && behavior > ReturnMatchingLanguageOnly)
    {
        chosen = defaultCodeMatch;

        if (chosen == -1 && behavior == ReturnMatchingDefaultOrFirstLanguage)
        {
            chosen = firstMatch;
        }
    }

    if (index)
    {
        *index = chosen;
    }

    if (chosen == -1)
    {
        return QString();
    }
    else
    {
        return d->infos.at(chosen).comment;
    }
}

int ItemComments::numberOfComments() const
{
    if (!d)
    {
        return 0;
    }

    return d->infos.size();
}

DatabaseComment::Type ItemComments::type(int index) const
{
    if (!d)
    {
        return DatabaseComment::UndefinedType;
    }

    return d->infos.at(index).type;
}

QString ItemComments::language(int index) const
{
    if (!d)
    {
        return QString();
    }

    return d->infos.at(index).language;
}

QString ItemComments::author(int index) const
{
    if (!d)
    {
        return QString();
    }

    return d->infos.at(index).author;
}

QDateTime ItemComments::date(int index) const
{
    if (!d)
    {
        return QDateTime();
    }

    return d->infos.at(index).date;
}

QString ItemComments::comment(int index) const
{
    if (!d)
    {
        return QString();
    }

    return d->infos.at(index).comment;
}

void ItemComments::setUniqueBehavior(UniqueBehavior behavior)
{
    if (!d)
    {
        return;
    }

    d->unique = behavior;
}

void ItemComments::addComment(const QString& comment,
                               const QString& lang,
                               const QString& author_,
                               const QDateTime& date,
                               DatabaseComment::Type type)
{
    if (!d)
    {
        return;
    }

    bool multipleCommentsPerLanguage = (d->unique == UniquePerLanguageAndAuthor);
    QString language                 = lang;

    if (language.isEmpty())
    {
        language = QLatin1String("x-default");
    }

    QString author = author_;

    /// @todo This makes no sense - is another variable supposed to be used instead? - Michael Hansen
    if (author.isEmpty())
    {
        author = QString();
    }

    for (int i = 0 ; i < d->infos.size() ; ++i)
    {
        CommentInfo& info = d->infos[i];

        // some extra considerations on replacing
        if (info.type == type && info.type == DatabaseComment::Comment && info.language == language)
        {
            if (!multipleCommentsPerLanguage || (info.author == author))
            {
                info.comment = comment;
                info.date    = date;
                info.author  = author;
                d->dirtyIndices << i;
                return;
            }
        }

        // simulate unique restrictions of db.
        // There is a problem that a NULL value is never unique, see #189080
        if ((info.type == type) &&
            (info.language == language) &&
            ((info.author == author) || (info.author.isEmpty() && author.isEmpty())))
        {
            info.comment = comment;
            info.date    = date;
            d->dirtyIndices << i;
            return;
        }
    }

    return addCommentDirectly(comment, language, author, type, date);
}

void ItemComments::addHeadline(const QString& headline, const QString& lang,
                                const QString& author, const QDateTime& date)
{
    return addComment(headline, lang, author, date, DatabaseComment::Headline);
}

void ItemComments::addTitle(const QString& title, const QString& lang,
                             const QString& author, const QDateTime& date)
{
    return addComment(title, lang, author, date, DatabaseComment::Title);
}

void ItemComments::replaceComments(const CaptionsMap& map, DatabaseComment::Type type)
{
    if (!d)
    {
        return;
    }

    d->dirtyIndices.clear();

    for (CaptionsMap::const_iterator it = map.constBegin(); it != map.constEnd(); ++it)
    {
        CaptionValues val = it.value();
        addComment(val.caption, it.key(), val.author, val.date, type);
    }

    // remove all comments of this type that have not been touched above
    for (int i = 0 ; i < d->infos.size() /* changing! */ ; )
    {
        if (!d->dirtyIndices.contains(i) && !d->newIndices.contains(i) && d->infos[i].type == type)
        {
            remove(i);
        }
        else
        {
            ++i;
        }
    }
}

void ItemComments::replaceFrom(const ItemComments& source)
{
    if (!d)
    {
        return;
    }

    if (!source.d)
    {
        removeAll();
        return;
    }

    foreach(const CommentInfo& info, source.d->infos)
    {
        addComment(info.comment, info.language, info.author, info.date, info.type);
    }

    // remove all that have not been touched above
    for (int i = 0 ; i < d->infos.size() /* changing! */ ; )
    {
        if (!d->dirtyIndices.contains(i) && !d->newIndices.contains(i))
        {
            remove(i);
        }
        else
        {
            ++i;
        }
    }
}

void ItemComments::addCommentDirectly(const QString& comment,
                                       const QString& language,
                                       const QString& author,
                                       DatabaseComment::Type type,
                                       const QDateTime& date)
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

void ItemComments::remove(int index)
{
    if (!d)
    {
        return;
    }

    d->idsToRemove << d->infos.at(index).id;
    d->infos.removeAt(index);
    d->adjustStoredIndexes(index);
}

void ItemComments::removeAll(DatabaseComment::Type type)
{
    if (!d)
    {
        return;
    }

    for (int i = 0 ; i < d->infos.size() /* changing! */ ; )
    {
        if (d->infos.at(i).type == type)
        {
            remove(i);
        }
        else
        {
            ++i;
        }
    }
}

void ItemComments::removeAllComments()
{
    removeAll(DatabaseComment::Comment);
}

void ItemComments::removeAll()
{
    if (!d)
    {
        return;
    }

    foreach(const CommentInfo& info, d->infos)
    {
        d->idsToRemove << info.id;
    }

    d->infos.clear();
    d->dirtyIndices.clear();
    d->newIndices.clear();
}

void ItemComments::changeComment(int index, const QString& comment)
{
    if (!d)
    {
        return;
    }

    d->infos[index].comment = comment;
    d->dirtyIndices << index;
}

void ItemComments::changeLanguage(int index, const QString& language)
{
    if (!d)
    {
        return;
    }

    d->infos[index].language = language;
    d->dirtyIndices << index;
}

void ItemComments::changeAuthor(int index, const QString& author)
{
    if (!d)
    {
        return;
    }

    d->infos[index].author = author;
    d->dirtyIndices << index;
}

void ItemComments::changeDate(int index, const QDateTime& date)
{
    if (!d)
    {
        return;
    }

    d->infos[index].date = date;
    d->dirtyIndices << index;
}

void ItemComments::changeType(int index, DatabaseComment::Type type)
{
    if (!d)
    {
        return;
    }

    d->infos[index].type = type;
    d->dirtyIndices << index;
}

void ItemComments::apply()
{
    if (!d)
    {
        return;
    }

    CoreDbAccess access;
    apply(access);
}

void ItemComments::apply(CoreDbAccess& access)
{
    if (!d)
    {
        return;
    }

    foreach(int commentId, d->idsToRemove)
    {
        access.db()->removeImageComment(commentId, d->id);
    }

    d->idsToRemove.clear();

    foreach(int index, d->newIndices)
    {
        CommentInfo& info = d->infos[index];
        info.id           = access.db()->setImageComment(d->id, info.comment, info.type, info.language, info.author, info.date);
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

CaptionsMap ItemComments::toCaptionsMap(DatabaseComment::Type type) const
{
    CaptionsMap map;

    if (d)
    {
        foreach(const CommentInfo& info, d->infos)
        {
            if (info.type == type)
            {
                CaptionValues val;
                val.caption        = info.comment;
                val.author         = info.author;
                val.date           = info.date;
                map[info.language] = val;
            }
        }
    }

    return map;
}

} // namespace Digikam

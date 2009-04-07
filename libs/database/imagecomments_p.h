/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-10-26
 * Description : Access to comments of an image in the database
 *
 * Copyright (C) 2007-2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2008 by Patrick Spendrin <ps_ml at gmx dot de>
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

// Qt includes

#include <QString>
#include <QSharedData>
#include <QSet>

namespace Digikam
{

class ImageCommentsPriv : public QSharedData
{
public:

    ImageCommentsPriv()
    {
        id = -1;
        unique = ImageComments::UniquePerLanguage;
    }

    qlonglong                     id;
    QList<CommentInfo>            infos;
    QSet<int>                     dirtyIndices;
    QSet<int>                     newIndices;
    ImageComments::UniqueBehavior unique;

    void languageMatch(const QString &fullCode, const QString &langCode,
                       int &fullCodeMatch, int &langCodeMatch, int &defaultCodeMatch, int &firstMatch) const
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
            firstMatch = 0; // index of first entry - at least we have one

        // First we search for a full match
        // Second for a match of the language code
        // Third for the default code
        // Fourth we return the first comment

        QLatin1String defaultCode("x-default");

        for (int i=0; i<infos.size(); ++i)
        {
            const CommentInfo &info = infos[i];
            if (info.type == DatabaseComment::Comment)
            {
                if (info.language == fullCode)
                {
                    fullCodeMatch = i;
                    break;
                }
                else if (info.language.startsWith(langCode) && langCodeMatch == -1)
                    langCodeMatch = i;
                else if (info.language == defaultCode)
                    defaultCodeMatch = i;
            }
        }
    }
};

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2009-09-12
 * Description : parse settings class
 *
 * Copyright (C) 2009-2012 by Andi Clemens <andi dot clemens at gmail dot com>
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

#ifndef DIGIKAM_PARSE_SETTINGS_H
#define DIGIKAM_PARSE_SETTINGS_H

// Qt includes

#include <QDateTime>
#include <QFileInfo>
#include <QString>

// Local includes

#include "iteminfo.h"
#include "parseresults.h"
#include "advancedrenamemanager.h"

namespace Digikam
{

class ParseSettings
{
public:

    // default ctors
    ParseSettings()
    {
        init();
    };

    explicit ParseSettings(const QString& _parseString)
        : parseString(_parseString)
    {
        init();
    };

    // ItemInfo ctors
    explicit ParseSettings(const ItemInfo& info)
    {
        init(info);
    };

    ParseSettings(const QString& _parseString, const ItemInfo& info)
        : parseString(_parseString)
    {
        init(info);
    };

    // --------------------------------------------------------

    bool isValid() const
    {
        QFileInfo fi(fileUrl.toLocalFile());
        return fi.isReadable();
    };

public:

    QUrl                     fileUrl;
    QString                  parseString;
    QString                  str2Modify;
    QDateTime                creationTime;
    ParseResults             results;
    ParseResults             invalidModifiers;
    ParseResults::ResultsKey currentResultsKey;

    int                      startIndex;
    bool                     useOriginalFileExtension;
    AdvancedRenameManager*   manager;

private:

    void init()
    {
        startIndex               = 1;
        useOriginalFileExtension = true;
        manager                  = nullptr;
        str2Modify.clear();
    }

    void init(const ItemInfo& info)
    {
        init();
        fileUrl = info.fileUrl();
    }
};

} // namespace Digikam

#endif // DIGIKAM_PARSE_SETTINGS_H

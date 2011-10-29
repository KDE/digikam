/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a class that manages the files to be renamed
 *
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at googlemail dot com>
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

#ifndef ADVANCEDRENAMEMANAGER_H
#define ADVANCEDRENAMEMANAGER_H

// KDE includes

#include <kurl.h>

// Qt includes

#include <QString>
#include <QStringList>

namespace Digikam
{

class AdvancedRenameWidget;
class Parser;
class ParseSettings;

class AdvancedRenameManager : public QObject
{
    Q_OBJECT

public:

    enum ParserType
    {
        DefaultParser = 0,
        ImportParser
    };

    enum SortType
    {
        SortNameAscending = 0,
        SortNameDescending,
        SortDateAscending,
        SortDateDescending,
        SortSizeAscending,
        SortSizeDescending,
        SortCustom
    };

public:

    AdvancedRenameManager();
    explicit AdvancedRenameManager(const QList<ParseSettings>& files, SortType sort = SortCustom);
    virtual ~AdvancedRenameManager();

    void addFiles(const QList<ParseSettings>& files, SortType sort = SortCustom);
    void reset();

    void parseFiles();
    void parseFiles(ParseSettings& settings);
    void parseFiles(const QString& parseString);
    void parseFiles(const QString& parseString, ParseSettings& settings);

    void setParserType(ParserType type);
    Parser* getParser();

    void setSortType(SortType type);
    SortType sortType() const;

    void setStartIndex(int index) const;

    void setWidget(AdvancedRenameWidget* widget);

    int indexOfFile(const QString& filename);
    int indexOfFolder(const QString& filename);
    int indexOfFileGroup(const QString& filename);
    QString newName(const QString& filename);

    QStringList            fileList();
    QMap<QString, QString> newFileList() const;

Q_SIGNALS:

    void signalSortingChanged(KUrl::List);

private:

    AdvancedRenameManager(const AdvancedRenameManager&);
    AdvancedRenameManager& operator=(const AdvancedRenameManager&);

    void addFile(const QString& filename) const;
    void addFile(const QString& filename, const QDateTime& datetime) const;
    bool initialize();
    void resetState();

    QString fileGroupKey(const QString& filename);

    void clearMappings();
    void clearAll();

private:

    class ParseManagerPriv;
    ParseManagerPriv* const d;
};

} // namespace Digikam

#endif /* ADVANCEDRENAMEMANAGER_H */

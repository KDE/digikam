/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a class that manages the files to be renamed
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

#ifndef ADVANCEDRENAMEMANAGER_H
#define ADVANCEDRENAMEMANAGER_H

// Qt includes

#include <QUrl>
#include <QString>
#include <QStringList>
#include <QObject>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class AdvancedRenameWidget;
class Parser;
class ParseSettings;

class DIGIKAM_EXPORT AdvancedRenameManager : public QObject
{
    Q_OBJECT

public:

    enum ParserType
    {
        DefaultParser = 0,
        ImportParser
    };

    enum SortAction
    {
        SortName = 0,
        SortDate,
        SortSize,
        SortCustom
    };

    enum SortDirection
    {
        SortAscending = 0,
        SortDescending
    };

public:

    AdvancedRenameManager();
    explicit AdvancedRenameManager(const QList<ParseSettings>& files);
    virtual ~AdvancedRenameManager();

    void addFiles(const QList<ParseSettings>& files);
    void reset();

    void parseFiles();
    void parseFiles(const ParseSettings& settings);
    void parseFiles(const QString& parseString);
    void parseFiles(const QString& parseString, const ParseSettings& settings);

    void setParserType(ParserType type);
    Parser* getParser() const;

    void setSortAction(SortAction action);
    SortAction sortAction() const;

    void setSortDirection(SortDirection direction);
    SortDirection sortDirection() const;

    void setStartIndex(int index);

    void setWidget(AdvancedRenameWidget* widget);

    int indexOfFile(const QString& filename);
    int indexOfFolder(const QString& filename);
    int indexOfFileGroup(const QString& filename);
    QString newName(const QString& filename) const;

    QStringList            fileList() const;
    QMap<QString, QString> newFileList() const;

Q_SIGNALS:

    void signalSortingChanged(QList<QUrl>);

private:

    AdvancedRenameManager(const AdvancedRenameManager&);
    AdvancedRenameManager& operator=(const AdvancedRenameManager&);

    void addFile(const QString& filename) const;
    void addFile(const QString& filename, const QDateTime& datetime) const;
    bool initialize();
    void initializeFileList();
    void resetState();

    QString fileGroupKey(const QString& filename) const;

    void clearMappings();
    void clearAll();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif /* ADVANCEDRENAMEMANAGER_H */

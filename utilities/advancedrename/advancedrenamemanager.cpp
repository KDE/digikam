/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a class that manages the files to be renamed
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "advancedrenamemanager.h"

// Qt includes

#include <QList>
#include <QMap>
#include <QFileInfo>

#include <kdebug.h>

// Local includes

#include "advancedrenamewidget.h"
#include "parsesettings.h"
#include "parser.h"
#include "defaultrenameparser.h"
#include "importrenameparser.h"
#include "imageinfo.h"

namespace Digikam
{

class AdvancedRenameManager::ParseManagerPriv
{
public:

    ParseManagerPriv() :
        parser(0),
        widget(0),
        parserType(AdvancedRenameManager::DefaultParser),
        sortType(AdvancedRenameManager::SortAscending),
        startIndex(1)
    {
    }

    QStringList                       files;
    QMap<QString, int>                fileIndexMap;
    QMap<QString, int>                folderIndexMap;
    QMap<QString, int>                fileGroupIndexMap;
    QMap<QString, QDateTime>          fileDatesMap;
    QMap<QString, QString>            renamedFiles;

    Parser*                           parser;
    AdvancedRenameWidget*             widget;
    AdvancedRenameManager::ParserType parserType;
    AdvancedRenameManager::SortType   sortType;

    int                               startIndex;
};

AdvancedRenameManager::AdvancedRenameManager()
    : d(new ParseManagerPriv)
{
    setParserType(DefaultParser);
    setSortType(SortAscending);
}

AdvancedRenameManager::AdvancedRenameManager(const QList<ParseSettings>& files, SortType type)
    : d(new ParseManagerPriv)
{
    setParserType(DefaultParser);
    setSortType(type);
    addFiles(files);
}

AdvancedRenameManager::~AdvancedRenameManager()
{
    clearAll();

    delete d->parser;
    delete d;
}

void AdvancedRenameManager::setSortType(SortType type)
{
    d->sortType = type;
}

void AdvancedRenameManager::setStartIndex(int index)
{
    d->startIndex = index;
}

QStringList AdvancedRenameManager::sortListCaseInsensitive(const QStringList& list)
{
    QMap<QString, QString> tmpMap;
    foreach (const QString& item, list)
    {
        tmpMap[item.toLower()] = item;
    }

    QStringList tmpList;
    foreach (const QString& item, tmpMap)
    {
        tmpList << item;
    }

    return tmpList;
}

void AdvancedRenameManager::setWidget(AdvancedRenameWidget* widget)
{
    if (!widget)
    {
        d->widget = 0;
        return;
    }

    d->widget = widget;
    setParserType(d->parserType);
}

void AdvancedRenameManager::setParserType(ParserType type)
{
    delete d->parser;

    if (type == ImportParser)
    {
        d->parser = new ImportRenameParser();
    }
    else
    {
        d->parser = new DefaultRenameParser();
    }

    d->parserType = type;

    if (d->widget)
    {
        d->widget->setParser(d->parser);

        if (type == ImportParser)
        {
            d->widget->setLayoutStyle(AdvancedRenameWidget::LayoutCompact);
        }
        else
        {
            d->widget->setLayoutStyle(AdvancedRenameWidget::LayoutNormal);
        }
    }
}

Parser* AdvancedRenameManager::getParser()
{
    if (!d->parser)
    {
        return 0;
    }

    return d->parser;
}

void AdvancedRenameManager::parseFiles()
{
    if (!d->widget)
    {
        return;
    }

    parseFiles(d->widget->parseString());
}

void AdvancedRenameManager::parseFiles(ParseSettings& _settings)
{
    if (!d->widget)
    {
        if (!(_settings.parseString.isEmpty()))
        {
            parseFiles(_settings.parseString, _settings);
        }
    }
    else
    {
        parseFiles(d->widget->parseString(), _settings);
    }
}

void AdvancedRenameManager::parseFiles(const QString& parseString)
{
    if (!d->parser)
    {
        return;
    }

    d->parser->reset();

    foreach (const QString& file, fileList())
    {
        KUrl url(file);
        ParseSettings settings;
        settings.fileUrl      = url;
        settings.parseString  = parseString;
        settings.startIndex   = d->startIndex;
        settings.creationTime = d->fileDatesMap[file];
        settings.manager      = this;
        d->renamedFiles[file] = d->parser->parse(settings);
    }
}

void AdvancedRenameManager::parseFiles(const QString& parseString, ParseSettings& _settings)
{
    if (!d->parser)
    {
        return;
    }

    d->parser->reset();

    foreach (const QString& file, fileList())
    {
        KUrl url(file);
        ParseSettings settings = _settings;
        settings.fileUrl       = url;
        settings.parseString   = parseString;
        settings.startIndex    = d->startIndex;
        settings.manager       = this;

        d->renamedFiles[file] = d->parser->parse(settings);
    }
}

void AdvancedRenameManager::addFiles(const QList<ParseSettings>& files, SortType type)
{
    foreach (const ParseSettings& ps, files)
    {
        addFile(ps.fileUrl.toLocalFile(), ps.creationTime);
    }
    setSortType(type);
    initialize();
}

void AdvancedRenameManager::clearMappings()
{
    d->fileIndexMap.clear();
    d->folderIndexMap.clear();
    d->fileGroupIndexMap.clear();
    d->renamedFiles.clear();
    d->fileGroupIndexMap.clear();
}

void AdvancedRenameManager::clearAll()
{
    d->files.clear();
    clearMappings();
}

void AdvancedRenameManager::reset()
{
    clearAll();
    resetState();
}

void AdvancedRenameManager::resetState()
{
    d->parser->reset();
    d->startIndex = 1;
}

QStringList AdvancedRenameManager::fileList()
{
    QStringList tmpFiles;

    switch (d->sortType)
    {
        case SortAscending:
        {
            tmpFiles = sortListCaseInsensitive(d->files);
            break;
        }
        case SortDescending:
        {
            QStringList sortedList = sortListCaseInsensitive(d->files);

            for (int i = sortedList.size() - 1; i >= 0; --i)
            {
                tmpFiles << sortedList.at(i);
            }

            break;
        }
        case SortCustom:
        default:
            tmpFiles = d->files;
            break;
    }

    return tmpFiles;
}

QMap<QString, QString> AdvancedRenameManager::newFileList()
{
    return d->renamedFiles;
}

bool AdvancedRenameManager::initialize()
{
    if (d->files.isEmpty())
    {
        return false;
    }

    // clear mappings
    clearMappings();

    // fill normal index map
    {
        int counter = 1;
        foreach (const QString& file, fileList())
        {
            d->fileIndexMap[file] = counter++;
        }
    }

    // fill file group index map
    {
        int counter = 1;
        foreach (const QString& file, fileList())
        if (!d->fileGroupIndexMap.contains(fileGroupKey(file)))
        {
            d->fileGroupIndexMap[fileGroupKey(file)] = counter++;
        }
    }

    // fill folder group index map
    {
        QMap<QString, QList<QString> > dirMap;
        foreach (const QString& file, fileList())
        {
            QFileInfo fi(file);
            QString path = fi.absolutePath();

            if (!path.isEmpty())
            {
                if (!dirMap.contains(path))
                {
                    dirMap[path] = QList<QString>();
                }

                dirMap[path].push_back(file);
            }
        }

        foreach (const QString& dir, dirMap.keys())
        {
            int index = 0;
            foreach (const QString& f, dirMap[dir])
            {
                if (!d->folderIndexMap.contains(f))
                {
                    d->folderIndexMap[f] = ++index;
                }
            }
        }
    }

    return true;
}

QString AdvancedRenameManager::fileGroupKey(const QString& filename)
{
    QFileInfo fi(filename);
    QString tmp = fi.absoluteFilePath().left(fi.absoluteFilePath().lastIndexOf(fi.suffix()));
    return tmp;
}

int AdvancedRenameManager::indexOfFile(const QString& filename)
{
    int index = -1;

    if (d->fileIndexMap.contains(filename))
    {
        index = d->fileIndexMap.value(filename);
    }

    return index;
}

int AdvancedRenameManager::indexOfFolder(const QString& filename)
{
    int index = -1;

    if (d->folderIndexMap.contains(filename))
    {
        index = d->folderIndexMap.value(filename);
    }

    return index;
}

int AdvancedRenameManager::indexOfFileGroup(const QString& filename)
{
    int index = -1;

    if (d->fileGroupIndexMap.contains(fileGroupKey(filename)))
    {
        index = d->fileGroupIndexMap.value(fileGroupKey(filename));
    }

    return index;
}

QString AdvancedRenameManager::newName(const QString& filename)
{
    if (!d->renamedFiles.contains(filename))
    {
        return filename;
    }

    return d->renamedFiles.value(filename);
}

void AdvancedRenameManager::addFile(const QString& filename)
{
    d->files << filename;
}

void AdvancedRenameManager::addFile(const QString& filename, const QDateTime& datetime)
{
    d->files << filename;
    d->fileDatesMap[filename] = datetime;
}

} // namespace Digikam

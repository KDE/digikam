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

#include "advancedrenamemanager.h"

// C++ includes

#include <algorithm>

// Qt includes

#include <QList>
#include <QMap>
#include <QFileInfo>

// Local includes

#include "digikam_debug.h"
#include "digikam_config.h"
#include "advancedrenamewidget.h"
#include "parsesettings.h"
#include "parser.h"
#include "defaultrenameparser.h"
#include "importrenameparser.h"
#include "imageinfo.h"

namespace Digikam
{

struct SortByNameCaseInsensitive
{
    bool operator() (const QString& s1, const QString& s2) const
    {
        return s1.toLower() < s2.toLower();
    }
};

struct SortByDate
{
    bool operator() (const QString& s1, const QString& s2) const
    {
        QFileInfo fi1(s1);
        QFileInfo fi2(s2);
        return fi1.created() < fi2.created();
    }
};

struct SortBySize
{
    bool operator() (const QString& s1, const QString& s2) const
    {
        QFileInfo fi1(s1);
        QFileInfo fi2(s2);
        return fi1.size() < fi2.size();
    }
};

class AdvancedRenameManager::Private
{
public:

    Private() :
        parser(0),
        widget(0),
        parserType(AdvancedRenameManager::DefaultParser),
        sortAction(AdvancedRenameManager::SortCustom),
        sortDirection(AdvancedRenameManager::SortAscending),
        startIndex(1)
    {
    }

    QStringList                          files;
    QMap<QString, int>                   fileIndexMap;
    QMap<QString, int>                   folderIndexMap;
    QMap<QString, int>                   fileGroupIndexMap;
    QMap<QString, QDateTime>             fileDatesMap;
    QMap<QString, QString>               renamedFiles;

    Parser*                              parser;
    AdvancedRenameWidget*                widget;
    AdvancedRenameManager::ParserType    parserType;
    AdvancedRenameManager::SortAction    sortAction;
    AdvancedRenameManager::SortDirection sortDirection;

    int                                  startIndex;
};

AdvancedRenameManager::AdvancedRenameManager()
    : d(new Private)
{
    setParserType(DefaultParser);
}

AdvancedRenameManager::AdvancedRenameManager(const QList<ParseSettings>& files)
    : d(new Private)
{
    setParserType(DefaultParser);
    addFiles(files);
}

AdvancedRenameManager::~AdvancedRenameManager()
{
    clearAll();

    delete d->parser;
    delete d;
}

void AdvancedRenameManager::setSortAction(SortAction action)
{
    d->sortAction = action;
    initialize();

    QList<QUrl> list;

    foreach(const QString& file, d->files)
    {
        list << QUrl::fromLocalFile(file);
    }

    emit signalSortingChanged(list);
}

AdvancedRenameManager::SortAction AdvancedRenameManager::sortAction() const
{
    return d->sortAction;
}

void AdvancedRenameManager::setSortDirection(SortDirection direction)
{
    d->sortDirection = direction;
    initialize();

    QList<QUrl> list;

    foreach(const QString& file, d->files)
    {
        list << QUrl::fromLocalFile(file);
    }

    emit signalSortingChanged(list);
}

AdvancedRenameManager::SortDirection AdvancedRenameManager::sortDirection() const
{
    return d->sortDirection;
}

void AdvancedRenameManager::setStartIndex(int index)
{
    d->startIndex = index < 1 ? 1 : index;
    initialize();
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

Parser* AdvancedRenameManager::getParser() const
{
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

void AdvancedRenameManager::parseFiles(const ParseSettings& _settings)
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

    foreach(const QString& file, d->files)
    {
        QUrl url = QUrl::fromLocalFile(file);
        ParseSettings settings;
        settings.fileUrl      = url;
        settings.parseString  = parseString;
        settings.startIndex   = d->startIndex;
        settings.creationTime = d->fileDatesMap[file];
        settings.manager      = this;
        d->renamedFiles[file] = d->parser->parse(settings);
    }
}

void AdvancedRenameManager::parseFiles(const QString& parseString, const ParseSettings& _settings)
{
    if (!d->parser)
    {
        return;
    }

    d->parser->reset();

    foreach(const QString& file, d->files)
    {
        QUrl url = QUrl::fromLocalFile(file);
        ParseSettings settings = _settings;
        settings.fileUrl       = url;
        settings.parseString   = parseString;
        settings.startIndex    = d->startIndex;
        settings.manager       = this;

        d->renamedFiles[file] = d->parser->parse(settings);
    }
}

void AdvancedRenameManager::addFiles(const QList<ParseSettings>& files)
{
    foreach(const ParseSettings& ps, files)
    {
        addFile(ps.fileUrl.toLocalFile(), ps.creationTime);
    }

    initialize();
}

void AdvancedRenameManager::clearMappings()
{
    d->fileIndexMap.clear();
    d->folderIndexMap.clear();
    d->fileGroupIndexMap.clear();
    d->renamedFiles.clear();
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

void AdvancedRenameManager::initializeFileList()
{
    QStringList tmpFiles = d->files;

    switch (d->sortAction)
    {
        case SortName:
        {
            std::sort(tmpFiles.begin(), tmpFiles.end(), SortByNameCaseInsensitive());
            break;
        }

        case SortDate:
        {
            std::sort(tmpFiles.begin(), tmpFiles.end(), SortByDate());
            break;
        }

        case SortSize:
        {
            std::sort(tmpFiles.begin(), tmpFiles.end(), SortBySize());
            break;
        }

        case SortCustom:
        default:
            break;
    }

    if (d->sortAction != SortCustom && d->sortDirection == SortDescending)
    {
        std::reverse(tmpFiles.begin(), tmpFiles.end());
    }

    d->files = tmpFiles;
}

QStringList AdvancedRenameManager::fileList() const
{
    return d->files;
}

QMap<QString, QString> AdvancedRenameManager::newFileList() const
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

    // initialize the file list according to the sort action and direction
    initializeFileList();

    // fill normal index map
    {
        int counter = 1;

        foreach(const QString& file, d->files)
        {
            d->fileIndexMap[file] = counter++;
        }
    }

    // fill file group index map
    {
        int counter = 1;

        foreach(const QString& file, d->files)
        {
            if (!d->fileGroupIndexMap.contains(fileGroupKey(file)))
            {
                d->fileGroupIndexMap[fileGroupKey(file)] = counter++;
            }
        }
    }

    // fill folder group index map
    {
        QMap<QString, QList<QString> > dirMap;

        foreach(const QString& file, d->files)
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

        foreach(const QString& dir, dirMap.keys())
        {
            int index = 0;

            foreach(const QString& f, dirMap[dir])
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

QString AdvancedRenameManager::fileGroupKey(const QString& filename) const
{
    QFileInfo fi(filename);
    QString tmp = fi.absoluteFilePath().left(fi.absoluteFilePath().lastIndexOf(fi.suffix()));
    return tmp;
}

int AdvancedRenameManager::indexOfFile(const QString& filename)
{
    return d->fileIndexMap.value(filename, -1);
}

int AdvancedRenameManager::indexOfFolder(const QString& filename)
{
    return d->folderIndexMap.value(filename, -1);
}

int AdvancedRenameManager::indexOfFileGroup(const QString& filename)
{
    return d->fileGroupIndexMap.value(fileGroupKey(filename), -1);
}

QString AdvancedRenameManager::newName(const QString& filename) const
{
    QString newName = d->renamedFiles.value(filename, filename);

#ifdef Q_OS_WIN
    // For the Windows file system, we need to replace unsupported characters.
    newName.replace(QLatin1Char(':'), QLatin1Char('-'));
#endif

    return newName;
}

void AdvancedRenameManager::addFile(const QString& filename) const
{
    d->files << filename;
}

void AdvancedRenameManager::addFile(const QString& filename, const QDateTime& datetime) const
{
    d->files << filename;
    d->fileDatesMap[filename] = datetime;
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2013-05-13
 * Description : Table view column helpers: Audio/video properties
 *
 * Copyright (C) 2013 by Michael G. Hansen <mike at mghansen dot de>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "tableview_column_audiovideo.moc"

// Qt includes

#include <QModelIndex>

// KDE includes

#include <kdebug.h>
#include <klocale.h>

// local includes

#include "databaseinfocontainers.h"
#include "dmetadata.h"
#include "imageinfo.h"

namespace Digikam
{

namespace TableViewColumns
{

ColumnAudioVideoProperties::ColumnAudioVideoProperties(
        TableViewShared* const tableViewShared,
        const TableViewColumnConfiguration& pConfiguration,
        const SubColumn pSubColumn,
        QObject* const parent)
  : TableViewColumn(tableViewShared, pConfiguration, parent),
    subColumn(pSubColumn)
{

}

ColumnAudioVideoProperties::~ColumnAudioVideoProperties()
{

}

QStringList ColumnAudioVideoProperties::getSubColumns()
{
    QStringList columns;
    columns << QLatin1String("audiobitrate") << QLatin1String("audiochanneltype")
            << QLatin1String("audiocompressor") << QLatin1String("duration")
            << QLatin1String("framerate") << QLatin1String("videocodec");

    return columns;
}

TableViewColumnDescription ColumnAudioVideoProperties::getDescription()
{
    TableViewColumnDescription description(QLatin1String("audiovideo-properties"), i18n("Audio/video properties"));
    description.setIcon("video-x-generic");

    description.addSubColumn(
        TableViewColumnDescription("audiobitrate", i18n("Audio bitrate"))
    );

    description.addSubColumn(
        TableViewColumnDescription("audiochanneltype", i18n("Audio channel type"))
    );

    description.addSubColumn(
        TableViewColumnDescription("audiocompressor", i18n("Audio compressor"))
    );

    description.addSubColumn(
        TableViewColumnDescription("duration", i18n("Duration"))
    );

    description.addSubColumn(
        TableViewColumnDescription("framerate", i18n("Frame rate"))
    );

    description.addSubColumn(
        TableViewColumnDescription("videocodec", i18n("Video codec"))
    );

    return description;
}

QString ColumnAudioVideoProperties::getTitle() const
{
    switch (subColumn)
    {
    case SubColumnAudioBitRate:
        return i18n("Audio bitrate");
    case SubColumnAudioChannelType:
        return i18n("Audio channel type");
    case SubColumnAudioCompressor:
        return i18n("Audio compressor");
    case SubColumnDuration:
        return i18n("Duration");
    case SubColumnFrameRate:
        return i18n("Frame rate");
    case SubColumnVideoCodec:
        return i18n("Video codec");
    }

    return QString();
}

TableViewColumn::ColumnFlags ColumnAudioVideoProperties::getColumnFlags() const
{
    ColumnFlags flags(ColumnNoFlags);

    /// @todo AudioChannelType contains "Mono" or "2", have to check for custom sorting
    if (   (subColumn==SubColumnAudioBitRate)
        || (subColumn==SubColumnDuration)
        || (subColumn==SubColumnFrameRate) )
    {
        flags|=ColumnCustomSorting;
    }

    return flags;
}

QVariant ColumnAudioVideoProperties::data(TableViewModel::Item* const item, const int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    switch (subColumn)
    {
    case SubColumnAudioBitRate:
        {
            bool ok;
            const int audioBitRate = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::AudioBitRate).toInt(&ok);
            if (!ok)
            {
                return QString();
            }

            const QString audioBitRateString = KGlobal::locale()->formatNumber(audioBitRate, 0);
            return audioBitRateString;
        }
    case SubColumnAudioChannelType:
        {
            const QString audioChannelType = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::AudioChannelType).toString();
            return audioChannelType;
        }
    case SubColumnAudioCompressor:
        {
            const QString audioCompressor = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::AudioCompressor).toString();
            return audioCompressor;
        }
    case SubColumnDuration:
        {
            bool ok;
            // duration is in milliseconds
            const double duration = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::Duration).toDouble(&ok);
            if (!ok)
            {
                return QString();
            }

            const QTime durationTime = QTime().addMSecs(duration);
            const QString durationString = KGlobal::locale()->formatTime(durationTime, true, true);
            return durationString;
        }
    case SubColumnFrameRate:
        {
            bool ok;
            const double frameRate = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::FrameRate).toDouble(&ok);
            if (!ok)
            {
                return QString();
            }

            const QString frameRateString = KGlobal::locale()->formatNumber(frameRate);
            return frameRateString;
        }
    case SubColumnVideoCodec:
        {
            const QString videoCodec = s->tableViewModel->itemDatabaseFieldRaw(item, DatabaseFields::VideoCodec).toString();
            return videoCodec;
        }
    }

    return QVariant();
}

TableViewColumn::ColumnCompareResult ColumnAudioVideoProperties::compare(TableViewModel::Item* const itemA, TableViewModel::Item* const itemB) const
{
    /// @todo All the values used here are actually returned as strings in the QVariants, but should be stored as int/double
    switch (subColumn)
    {

    case SubColumnAudioBitRate:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::AudioBitRate);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::AudioBitRate);
            bool okA;
            const int audioBitRateA = variantA.toInt(&okA);
            bool okB;
            const int audioBitRateB = variantB.toDouble(&okB);

            ColumnCompareResult result;
            if (!compareHelperBoolFailCheck(okA, okB, &result))
            {
                return result;
            }

            return compareHelper<int>(audioBitRateA, audioBitRateB);
        }

    case SubColumnDuration:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::Duration);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::Duration);
            bool okA;
            const double durationA = variantA.toDouble(&okA);
            bool okB;
            const double durationB = variantB.toDouble(&okB);

            ColumnCompareResult result;
            if (!compareHelperBoolFailCheck(okA, okB, &result))
            {
                return result;
            }

            return compareHelper<double>(durationA, durationB);
        }

    case SubColumnFrameRate:
        {
            const QVariant variantA = s->tableViewModel->itemDatabaseFieldRaw(itemA, DatabaseFields::FrameRate);
            const QVariant variantB = s->tableViewModel->itemDatabaseFieldRaw(itemB, DatabaseFields::FrameRate);
            bool okA;
            const double frameRateA = variantA.toDouble(&okA);
            bool okB;
            const double frameRateB = variantB.toDouble(&okB);

            ColumnCompareResult result;
            if (!compareHelperBoolFailCheck(okA, okB, &result))
            {
                return result;
            }

            return compareHelper<double>(frameRateA, frameRateB);
        }

    default:
        kWarning() << "item: unimplemented comparison, subColumn=" << subColumn;
        return CmpEqual;
    }
}

void ColumnAudioVideoProperties::setConfiguration(const TableViewColumnConfiguration& newConfiguration)
{
    configuration = newConfiguration;

    emit(signalAllDataChanged());
}

} /* namespace TableViewColumns */

} /* namespace Digikam */


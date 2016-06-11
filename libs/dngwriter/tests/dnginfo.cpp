/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-01-07
 * Description : a command line tool to extract embedded originals
 *
 * Copyright (C) 2011 by Jens Mueller <tschenser at gmx dot de>
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

// Qt includes

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDataStream>

// DNG SDK includes

#include "dng_file_stream.h"
#include "dng_host.h"
#include "dng_image.h"
#include "dng_info.h"
#include "dng_memory_stream.h"
#include "dng_xmp_sdk.h"

// Local includes

#define CHUNK 65536

int main(int argc, char** argv)
{
    try
    {
        bool extractOriginal = false;

        if(argc == 1)
        {
            qDebug() <<
                    "\n"
                    "dnginfo - DNG information tool\n"
                    "Usage: %s [options] dngfile\n"
                    "Valid options:\n"
                    "  -extractraw            extract embedded original\n" <<
                    argv[0];

            return -1;
        }

        qint32 index;

        for (index = 1; index < argc && argv[index][0] == '-'; ++index)
        {
            QString option = QString::fromUtf8(&argv[index][1]);

            if (option == QLatin1String("extractraw"))
            {
                extractOriginal = true;
            }
        }

        if (index == argc)
        {
            qCritical() << "*** No file specified\n";
            return 1;
        }

        QFileInfo dngFileInfo(QString::fromUtf8(argv[index]));

        dng_xmp_sdk::InitializeSDK();

        dng_file_stream stream(QFile::encodeName(dngFileInfo.absoluteFilePath()).constData());
        dng_host        host;
        host.SetKeepOriginalFile(true);

        AutoPtr<dng_negative> negative;
        {
            dng_info info;
            info.Parse(host, stream);
            info.PostParse(host);

            if (!info.IsValidDNG())
            {
                return dng_error_bad_format;
            }

            negative.Reset(host.Make_dng_negative());
            negative->Parse(host, stream, info);
            negative->PostParse(host, stream, info);

            QString originalFileName(QString::fromUtf8(negative->OriginalRawFileName().Get()));
    //      dng_fingerprint originalDigest = negative->OriginalRawFileDigest();
            quint32 originalDataLength     = negative->OriginalRawFileDataLength();
            const void* originalData       = negative->OriginalRawFileData();

            if (extractOriginal)
            {
                if (originalDataLength > 0)
                {
                    dng_memory_allocator memalloc(gDefaultDNGMemoryAllocator);
                    dng_memory_stream compressedDataStream(memalloc);
                    compressedDataStream.Put(originalData, originalDataLength);
                    compressedDataStream.SetReadPosition(0);
                    compressedDataStream.SetBigEndian(true);
                    quint32 forkLength = compressedDataStream.Get_uint32();
                    quint32 forkBlocks = (uint32)floor((forkLength + 65535.0) / 65536.0);
                    QVector<quint32> offsets;

                    for(quint32 block = 0; block <= forkBlocks; ++block)
                    {
                        quint32 offset = compressedDataStream.Get_uint32();
                        offsets.push_back(offset);
                    }

                    QFile originalFile(dngFileInfo.absolutePath() + QLatin1Char('/') + originalFileName);
                    qDebug() << "extracting embedded original to " << dngFileInfo.fileName();

                    if (!originalFile.open(QIODevice::WriteOnly))
                    {
                        qDebug() << "Cannot open file. Aborted...";
                        return 1;
                    }

                    QDataStream originalDataStream(&originalFile);

                    for (quint32 block = 0; block < forkBlocks; ++block)
                    {
                        QByteArray compressedDataBlock;
                        compressedDataBlock.resize(offsets[block + 1] - offsets[block]);
                        compressedDataStream.Get(compressedDataBlock.data(), compressedDataBlock.size());
                        quint32 uncompressedDataSize = qMin((quint32)CHUNK, forkLength);

                        compressedDataBlock.prepend(uncompressedDataSize         & 0xFF);
                        compressedDataBlock.prepend((uncompressedDataSize >>  8) & 0xFF);
                        compressedDataBlock.prepend((uncompressedDataSize >> 16) & 0xFF);
                        compressedDataBlock.prepend((uncompressedDataSize >> 24) & 0xFF);
                        forkLength -= uncompressedDataSize;

                        QByteArray originalDataBlock = qUncompress((const uchar*)compressedDataBlock.data(), compressedDataBlock.size());
                        //qDebug() << "compressed data block " << compressedDataBlock.size() << " -> " << originalDataBlock.size();
                        originalDataStream.writeRawData(originalDataBlock.data(), originalDataBlock.size());
                    }

                    originalFile.close();
                }
                else
                {
                    qCritical() << "no embedded originals found\n";
                }
            }
        }

        dng_xmp_sdk::TerminateSDK();

        return 0;
    }

    catch (const dng_exception& exception)
    {
        int ret = exception.ErrorCode();
        qDebug() << "DNGWriter: DNG SDK exception code (" << ret << ")" ;
        return -1;
    }

    catch (...)
    {
        qDebug() << "DNGWriter: DNG SDK exception code unknow" ;
        return -1;
    }
}

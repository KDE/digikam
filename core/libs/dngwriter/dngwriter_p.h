/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * Copyright (C) 2008-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DNGWRITER_P_H
#define DNGWRITER_P_H

// Qt includes

#include <QString>
#include <QDateTime>

// Local includes

#include "drawdecoder.h"

// DNG SDK includes

#include "dng_camera_profile.h"
#include "dng_color_space.h"
#include "dng_exceptions.h"
#include "dng_file_stream.h"
#include "dng_globals.h"
#include "dng_host.h"
#include "dng_ifd.h"
#include "dng_image_writer.h"
#include "dng_info.h"
#include "dng_linearization_info.h"
#include "dng_memory_stream.h"
#include "dng_mosaic_info.h"
#include "dng_negative.h"
#include "dng_preview.h"
#include "dng_read_image.h"
#include "dng_render.h"
#include "dng_simple_image.h"
#include "dng_tag_codes.h"
#include "dng_tag_types.h"
#include "dng_tag_values.h"
#include "dng_xmp.h"
#include "dng_xmp_sdk.h"

// Local includes

#include "dngwriter.h"

namespace Digikam
{

class DNGWriter::Private
{

public:

    enum DNGBayerPattern
    {
        Unknown = 1,
        LinearRaw,
        Standard,
        Fuji,
        FourColor
    };

public:

    Private();
    ~Private();

public:

    void          reset();
    void          cleanup();
    dng_date_time dngDateTime(const QDateTime& qDT) const;

    bool fujiRotate(QByteArray& rawData, RawInfo& identify) const;

public:

    bool    cancel;
    bool    jpegLossLessCompression;
    bool    updateFileDate;
    bool    backupOriginalRawFile;

    int     previewMode;

    QString inputFile;
    QString outputFile;
};

} // namespace Digikam

#endif // DNGWRITER_P_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Jens Mueller <tschenser at gmx dot de>
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

#ifndef DIGIKAM_DNG_WRITER_H
#define DIGIKAM_DNG_WRITER_H

// Qt includes

#include <QString>

// Local includes

#include "drawdecoder.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNGWriter
{

public:

    enum JPEGPreview
    {
        NONE = 0,
        MEDIUM,
        FULLSIZE
    };

    enum ConvertError
    {
        PROCESSCOMPLETE     =  0,
        PROCESSFAILED       = -1,
        PROCESSCANCELED     = -2,
        FILENOTSUPPORTED    = -3,
        DNGSDKINTERNALERROR = -4
    };

public:

    explicit DNGWriter();
    ~DNGWriter();

    void setInputFile(const QString& filePath);
    void setOutputFile(const QString& filePath);

    QString inputFile()  const;
    QString outputFile() const;

    void setCompressLossLess(bool b);
    bool compressLossLess() const;

    void setUpdateFileDate(bool b);
    bool updateFileDate() const;

    void setBackupOriginalRawFile(bool b);
    bool backupOriginalRawFile() const;

    void setPreviewMode(int mode);
    int  previewMode() const;

    int  convert();
    void cancel();
    void reset();

public:

    // Declared public because of DNGWriterHost class.
    class Private;

private:

    DNGWriter(const DNGWriter&); // Disable

    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_DNG_WRITER_H

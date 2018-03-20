/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2016-04-21
 * Description : video thumbnails extraction based on ffmpeg
 *
 * Copyright (C) 2010      by Dirk Vanden Boer <dirk dot vdb at gmail dot com>
 * Copyright (C) 2016-2018 by Maik Qualmann <metzpinguin at gmail dot com>
 * Copyright (C) 2016-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef MOVIE_DECODER_H
#define MOVIE_DECODER_H

// Qt includes

#include <QString>

// Local includes

#include "imagewriter.h"

namespace Digikam
{

class MovieDecoder
{
public:

    explicit MovieDecoder(const QString& filename);
    ~MovieDecoder();

public:

    QString getCodec()       const;
    int     getWidth()       const;
    int     getHeight()      const;
    int     getDuration()    const;
    bool    getInitialized() const;

    void seek(int timeInSeconds);
    void decodeVideoFrame();
    void getScaledVideoFrame(int scaledSize,
                             bool maintainAspectRatio,
                             VideoFrame& videoFrame);

    void initialize(const QString& filename);
    void destroy();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // MOVIE_DECODER_H

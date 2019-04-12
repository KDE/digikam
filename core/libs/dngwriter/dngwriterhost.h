/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-09-25
 * Description : a tool to convert RAW file to DNG
 *
 * Copyright (C) 2008-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DNG_WRITER_HOST_H
#define DIGIKAM_DNG_WRITER_HOST_H

// Local includes

#include "dngwriter_p.h"
#include "digikam_export.h"

namespace Digikam
{

class DIGIKAM_EXPORT DNGWriterHost : public dng_host
{

public:

    explicit DNGWriterHost(DNGWriter::Private* const priv, dng_memory_allocator* const allocator=nullptr);
    ~DNGWriterHost();

private:

    void SniffForAbort();

private:

    DNGWriter::Private* const m_priv;
};

} // namespace Digikam

#endif // DIGIKAM_DNG_WRITER_HOST_H

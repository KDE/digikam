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

#ifndef DNGWRITERHOST_H
#define DNGWRITERHOST_H

// Local includes

#include "dngwriter_p.h"

namespace Digikam
{

class DNGWriterHost : public dng_host
{

public:

    DNGWriterHost(DNGWriter::Private* const priv, dng_memory_allocator* const allocator=0);
    ~DNGWriterHost();

private:

    void SniffForAbort();

private:

    DNGWriter::Private* const m_priv;
};

} // namespace Digikam

#endif // DNGWRITERHOST_H

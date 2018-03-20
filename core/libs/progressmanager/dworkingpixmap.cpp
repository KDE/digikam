/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : A working pixmap manager.
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dworkingpixmap.h"

// Qt includes

#include <QApplication>
#include <QStandardPaths>

// Local includes

#include "digikam_debug.h"

namespace Digikam
{

DWorkingPixmap::DWorkingPixmap()
{
    QPixmap pix(QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                       QLatin1String("digikam/data/process-working.png")));
    QSize   size(22, 22);

    if (pix.isNull())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Invalid pixmap specified.";
        return;
    }

    if (!size.isValid())
    {
        size = QSize(pix.width(), pix.width());
    }

    if (pix.width() % size.width() || pix.height() % size.height())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "Invalid framesize.";
        return;
    }

    const int rowCount = pix.height() / size.height();
    const int colCount = pix.width()  / size.width();
    m_frames.resize(rowCount * colCount);

    int pos = 0;

    for (int row = 0; row < rowCount; ++row)
    {
        for (int col = 0; col < colCount; ++col)
        {
            QPixmap frm     = pix.copy(col * size.width(), row * size.height(), size.width(), size.height());
            m_frames[pos++] = frm;
        }
    }
}

DWorkingPixmap::~DWorkingPixmap()
{
}

bool DWorkingPixmap::isEmpty() const
{
    return m_frames.isEmpty();
}

QSize DWorkingPixmap::frameSize() const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "No frame loaded.";
        return QSize();
    }

    return m_frames[0].size();
}

int DWorkingPixmap::frameCount() const
{
    return m_frames.size();
}

QPixmap DWorkingPixmap::frameAt(int index) const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_WIDGETS_LOG) << "No frame loaded.";
        return QPixmap();
    }

    return m_frames.at(index);
}

} // namespace Digikam

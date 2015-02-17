/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-04-02
 * Description : Pixmap to host a throbber ("working" animation)
 *
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "workingpixmap.h"

// Qt includes

#include <QPainter>
#include <QStandardPaths>
#include <QVector>

// Local includes

#include "digikam_debug.h"

WorkingPixmap::WorkingPixmap()
{
    QPixmap pix(QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("digikam/data/process-working.png")));
    QSize   size(22, 22);
    
    if (pix.isNull())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid pixmap specified.";
        return;
    }
    if (!size.isValid())
    {
        size = QSize(pix.width(), pix.width());
    }

    if (pix.width() % size.width() || pix.height() % size.height())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "Invalid framesize.";
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

WorkingPixmap::~WorkingPixmap()
{
}

bool WorkingPixmap::isEmpty() const
{
    return m_frames.isEmpty();
}

QSize WorkingPixmap::frameSize() const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No frame loaded.";
        return QSize();
    }

    return m_frames[0].size();
}

int WorkingPixmap::frameCount() const
{
    return m_frames.size();
}

QPixmap WorkingPixmap::frameAt(int index) const
{
    if (isEmpty())
    {
        qCWarning(DIGIKAM_GENERAL_LOG) << "No frame loaded.";
        return QPixmap();
    }

    return m_frames.at(index);
}

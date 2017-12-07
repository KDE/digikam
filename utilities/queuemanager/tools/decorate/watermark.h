/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-28
 * Description : batch tool to add visible watermark.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2010      by Mikkel Baekhoej Christensen <mbc at baekhoej dot dk>
 * Copyright (C) 2017      by Ahmed Fathi <ahmed dot fathi dot abdelmageed at gmail dot com>
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

#ifndef WATERMARK_H
#define WATERMARK_H

// Qt includes

#include <QFont>

// Local includes

#include "batchtool.h"

namespace Digikam
{

class WaterMark : public BatchTool
{
    Q_OBJECT

public:

    explicit WaterMark(QObject* const parent = 0);
    virtual ~WaterMark();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new WaterMark(parent); };

    void registerSettingsWidget();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotAssignSettings2Widget();

private:

    bool toolOperations();
    int  queryFontSize(const QString& text, const QFont& font, int length) const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // WATERMARK_H

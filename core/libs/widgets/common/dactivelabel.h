/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-12
 * Description : A label with an active url
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

#ifndef DACTIVE_LABEL_H
#define DACTIVE_LABEL_H

// Qt includes

#include <QLabel>
#include <QUrl>
#include <QString>
#include <QImage>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

/** A widget to host an image into a label with an active url which can be
 *  open to default web browser using simple mouse click.
 */
class DIGIKAM_EXPORT DActiveLabel : public QLabel
{
    Q_OBJECT

public:

    explicit DActiveLabel(const QUrl& url=QUrl(), const QString& imgPath=QString(), QWidget* const parent=0);
    virtual ~DActiveLabel();

    void updateData(const QUrl& url, const QImage& img);
};

} // namespace Digikam

#endif // DACTIVE_LABEL_H

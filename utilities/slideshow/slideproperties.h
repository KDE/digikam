/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-09-19
 * Description : slide properties widget
 *
 * Copyright (C) 2014 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDEPROPERTIES_H
#define SLIDEPROPERTIES_H

#include <QWidget>
#include <QPainter>
#include <QString>
#include <QColor>

// KDE includes

#include <kurl.h>

// Local includes

#include "slideshowsettings.h"

namespace Digikam
{

class SlideProperties : public QWidget
{
public:

    explicit SlideProperties(const SlideShowSettings& settings, QWidget* const parent);
    ~SlideProperties();

    void setCurrentInfo(const SlidePictureInfo& info, const KUrl& url);

private:

    void printInfoText(QPainter& p, int& offset, const QString& str, const QColor& pcol=Qt::white);
    void printComments(QPainter& p, int& offset, const QString& comments);
    void printTags(QPainter& p, int& offset, QStringList& tags);

    void paintEvent(QPaintEvent*);

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif /* SLIDEPROPERTIES_H */

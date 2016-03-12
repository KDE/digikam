/* ============================================================
 *
 * This file is a part of kipi-plugins project
 * http://www.digikam.org
 *
 * Date        : 2003-01-31
 * Description : a presentation tool.
 *
 * Copyright (C) 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2012-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef PRESENTATION_H
#define PRESENTATION_H

// Qt includes

#include <QUrl>
#include <QObject>
#include <QList>

namespace Digikam
{

class PresentationContainer;

class Presentation : public QObject
{
    Q_OBJECT

public:

    Presentation(const QList<QUrl>& urls, QObject* const parent);
    ~Presentation();

private Q_SLOTS:

    void slotSlideShow();

private:

    QList<QUrl>            m_urlList;
    PresentationContainer* m_sharedData;
};

}  // namespace Digikam

#endif  // PRESENTATION_H

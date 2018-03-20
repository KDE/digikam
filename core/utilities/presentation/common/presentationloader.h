/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-11-11
 * Description : a presentation tool.
 *
 * Copyright (C) 2007-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2012-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PRESENTATIONLOADER_H
#define PRESENTATIONLOADER_H

// Qt includes

#include <QString>
#include <QImage>
#include <QUrl>

namespace Digikam
{

class PresentationContainer;

class PresentationLoader
{
public:

    PresentationLoader(PresentationContainer* const sharedData, int width, int height,
                       int beginAtIndex = 0);

    ~PresentationLoader();

    void    next();
    void    prev();

    QImage  getCurrent()   const;
    QString currFileName() const;
    QUrl    currPath()     const;

private:

    void checkIsIn(int index) const;

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // PRESENTATIONLOADER_H

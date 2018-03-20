/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-31
 * Description : a presentation tool.
 *
 * Copyright (C) 2006-2009 by Valerio Fuoglio <valerio dot fuoglio at gmail dot com>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at googlemail dot com>
 * Copyright (C) 2003-2005 by Renchi Raju <renchi dot raju at gmail dot com>
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

#ifndef PRESENTATION_MNGR_H
#define PRESENTATION_MNGR_H

// Qt includes

#include <QObject>
#include <QList>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class PresentationContainer;

class DIGIKAM_EXPORT PresentationMngr : public QObject
{
    Q_OBJECT

public:

    PresentationMngr(QObject* const parent);
    ~PresentationMngr();

    void addFile(const QUrl& url, const QString& comment);
    void showConfigDialog();

private Q_SLOTS:

    void slotSlideShow();

private:

    PresentationContainer* m_sharedData;
};

}  // namespace Digikam

#endif  // PRESENTATION_MNGR_H

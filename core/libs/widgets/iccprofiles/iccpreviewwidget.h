/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
 *
 * Copyright (C) 2006-2007 by Francisco J. Cruz <fj dot cruz at supercable dot es>
 * Copyright (C) 2006-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ICCPREVIEWWIDGET_H
#define ICCPREVIEWWIDGET_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "digikam_export.h"

class QUrl;

namespace Digikam
{

class ICCProfileWidget;

class DIGIKAM_EXPORT ICCPreviewWidget : public QScrollArea
{
    Q_OBJECT

public:

    explicit ICCPreviewWidget(QWidget* const parent = 0);
    ~ICCPreviewWidget();

public Q_SLOTS:

    void slotShowPreview(const QUrl& url);
    void slotClearPreview();

private :

    ICCProfileWidget* m_iccProfileWidget;
};

} // namespace Digikam

#endif // ICCPREVIEWWIDGET_H

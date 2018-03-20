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

#include "iccpreviewwidget.h"

// Qt includes

#include <QFileInfo>
#include <QScrollArea>
#include <QUrl>

// Local includes

#include "digikam_debug.h"
#include "iccprofilewidget.h"

namespace Digikam
{

ICCPreviewWidget::ICCPreviewWidget(QWidget* const parent)
    : QScrollArea(parent)
{
    m_iccProfileWidget = new ICCProfileWidget(this);
    setWidget(m_iccProfileWidget);
    setWidgetResizable(true);
}

ICCPreviewWidget::~ICCPreviewWidget()
{
}

void ICCPreviewWidget::slotShowPreview(const QUrl& url)
{
    slotClearPreview();
    QFileInfo fInfo(url.toLocalFile());

    if ( url.isLocalFile() && fInfo.isFile() && fInfo.isReadable() )
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << url << " is a readable local file";
        m_iccProfileWidget->loadFromURL(url);
    }
    else
    {
        qCDebug(DIGIKAM_WIDGETS_LOG) << url << " is not a readable local file";
    }
}

void ICCPreviewWidget::slotClearPreview()
{
    m_iccProfileWidget->loadFromURL(QUrl());
}

} // namespace Digikam

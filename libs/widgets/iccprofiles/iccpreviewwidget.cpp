/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
 *
 * Copyright (C) 2006-2007 by Francisco J. Cruz <fj.cruz@supercable.es>
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
#include "iccpreviewwidget.moc"

// Qt includes

#include <QFileInfo>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QFrame>
#include <QScrollArea>

// KDE includes

#include <kdebug.h>
#include <kurl.h>

// Local includes

#include "iccprofilewidget.h"

namespace Digikam
{

ICCPreviewWidget::ICCPreviewWidget(QWidget *parent)
                : KPreviewWidgetBase( parent )
{
    QVBoxLayout *mainLayout = new QVBoxLayout;
    QScrollArea *scrollArea = new QScrollArea;
    m_iccProfileWidget      = new ICCProfileWidget(this);

    scrollArea->setWidget(m_iccProfileWidget);
    mainLayout->addWidget(scrollArea);
    setLayout(mainLayout);
}

ICCPreviewWidget::~ICCPreviewWidget()
{
}

void ICCPreviewWidget::showPreview( const KUrl& url)
{
    clearPreview();
    QFileInfo fInfo(url.path());

    if ( url.isLocalFile() && fInfo.isFile() && fInfo.isReadable() )
    {
        kDebug(50003) << url << " is a readable local file";
        m_iccProfileWidget->loadFromURL(url);
    }
    else
    {
        kDebug(50003) << url << " is not a readable local file";
    }
}

void ICCPreviewWidget::clearPreview()
{
    m_iccProfileWidget->loadFromURL(KUrl());
}

} // namespace Digikam

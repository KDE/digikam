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

// Qt includes. 

#include <qfileinfo.h>
#include <qlayout.h>
#include <q3vgroupbox.h>
//Added by qt3to4:
#include <Q3VBoxLayout>
#include <Q3Frame>

// KDE includes

#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "iccprofilewidget.h"
#include "iccpreviewwidget.h"
#include "iccpreviewwidget.moc"

namespace Digikam 
{

ICCPreviewWidget::ICCPreviewWidget(QWidget *parent)
                : KPreviewWidgetBase( parent )
{
    Q3VBoxLayout *layout = new Q3VBoxLayout( this );
    Q3VGroupBox *box     = new Q3VGroupBox( this );
    box->setInsideMargin(0);
    box->setFrameStyle(Q3Frame::NoFrame|Q3Frame::Plain);
    m_iccProfileWidget = new ICCProfileWidget(box);
    layout->addWidget( box );
}

ICCPreviewWidget::~ICCPreviewWidget()
{
}

void ICCPreviewWidget::showPreview( const KURL &url)
{
    clearPreview();
    QFileInfo fInfo(url.path());
    
    if ( url.isLocalFile() && fInfo.isFile() && fInfo.isReadable() )
    {
        DDebug() << url << " is a readble local file" << endl;
        m_iccProfileWidget->loadFromURL(url);
    }
    else
    {
        DDebug() << url << " is not a readable local file" << endl;
    }
}

void ICCPreviewWidget::clearPreview()
{
    m_iccProfileWidget->loadFromURL(KURL());
}

} // namespace Digikam



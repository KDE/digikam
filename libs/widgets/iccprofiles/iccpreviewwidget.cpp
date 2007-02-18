/* ============================================================
 * Author: Francisco J. Cruz <fj.cruz@supercable.es>
 * Date  : 2006-01-12
 * Description : a widget to display ICC profiles descriptions
 *               in file dialog preview.
 * 
 * Copyright 2006 by Francisco J. Cruz <fj.cruz@supercable.es>
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
#include <qvgroupbox.h>

// KDE includes

#include <kurl.h>

// Local includes.

#include "ddebug.h"
#include "iccprofilewidget.h"
#include "iccpreviewwidget.h"

namespace Digikam 
{

ICCPreviewWidget::ICCPreviewWidget(QWidget *parent)
                : KPreviewWidgetBase( parent )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    QVGroupBox *box     = new QVGroupBox( this );
    box->setInsideMargin(0);
    box->setFrameStyle(QFrame::NoFrame|QFrame::Plain);
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

#include "iccpreviewwidget.moc"

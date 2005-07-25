/* ============================================================
 * File  : imagedialog.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog without threadable 
 *               filter interface.
 *
 * Copyright 2005 by Gilles Caulier
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

#include <qlayout.h>
#include <qcolor.h>
#include <qgroupbox.h>
#include <qhgroupbox.h>
#include <qvgroupbox.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>
#include <qcheckbox.h>
#include <qfile.h>

// KDE includes.

#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>
#include <knuminput.h>
#include <kmessagebox.h>
#include <kselect.h>
#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kaboutdata.h>
#include <khelpmenu.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

// Digikam includes.

#include <digikamheaders.h>

// Local includes.

#include "version.h"
#include "bannerwidget.h"
#include "imagetabwidget.h"
#include "imagedialog.h"

namespace DigikamImagePlugins
{

ImageDialog::ImageDialog(QWidget* parent, QString title, QString name, bool loadFileSettings,
                         bool orgGuideVisible, bool targGuideVisible)
           : KDialogBase(Plain, title, Help|Default|User2|User3|Ok|Cancel, Ok,
                         parent, 0, true, true,
                         QString::null,
                         i18n("&Load..."),
                         i18n("&Save As...")),
             m_parent(parent), m_name(name)
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User2, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User3, i18n("<p>Save all filter parameters to settings text file.") );  
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);
    
    resize(configDialogSize(name + QString::QString(" Tool Dialog")));  
 
    // -------------------------------------------------------------

    m_mainLayout = new QGridLayout( plainPage(), 2, 1 , marginHint(), spacingHint());

    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(plainPage(), title); 
    m_mainLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);

    // -------------------------------------------------------------

    m_imageTabPreviewWidget = new DigikamImagePlugins::ImageTabWidget(plainPage(), orgGuideVisible, targGuideVisible);
    m_mainLayout->addMultiCellWidget(m_imageTabPreviewWidget, 1, 2, 0, 0);
    m_mainLayout->setColStretch(0, 10);
    m_mainLayout->setRowStretch(2, 10);
    
    kapp->restoreOverrideCursor();
    
    // -------------------------------------------------------------
}

ImageDialog::~ImageDialog()
{
    saveDialogSize(m_name + QString::QString(" Tool Dialog"));   
}

void ImageDialog::slotHelp()
{
    KApplication::kApplication()->invokeHelp(m_name, "digikamimageplugins");
}

void ImageDialog::setAboutData(KAboutData *about)
{
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

Digikam::ImageGuideWidget *ImageDialog::previewOriginalWidget(void)
{
    return m_imageTabPreviewWidget->previewOriginal(); 
}

Digikam::ImageGuideWidget *ImageDialog::previewTargetWidget(void) 
{ 
    return m_imageTabPreviewWidget->previewTarget();   
}

void ImageDialog::setUserAreaWidget(QWidget *w)
{
    QVBoxLayout *vLayout = new QVBoxLayout( spacingHint() ); 
    vLayout->addWidget(w);
    m_mainLayout->addMultiCellLayout(vLayout, 1, 1, 1, 1);    
}

}  // NameSpace DigikamImagePlugins

#include "imagedialog.moc"

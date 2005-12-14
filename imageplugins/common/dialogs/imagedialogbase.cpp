/* ============================================================
 * File  : imagedialogbase.cpp
 * Author: Gilles Caulier <caulier dot gilles at free.fr>
 * Date  : 2005-07-23
 * Description : simple plugins dialog without threadable
 *               filter interface. The dialog laytou is
 *               designed to accept custom widgets in
 *               preview and settings area.
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

#include <qgroupbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qframe.h>
#include <qtimer.h>

// KDE includes.

#include <kcursor.h>
#include <kdebug.h>
#include <klocale.h>
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
#include "imagedialogbase.h"

namespace DigikamImagePlugins
{

ImageDialogBase::ImageDialogBase(QWidget* parent, QString title, QString name, bool loadFileSettings)
               : KDialogBase(Plain, title, Help|Default|User2|User3|Ok|Cancel, Ok,
                             parent, 0, true, true,
                             QString::null,
                             i18n("&Save As..."),
                             i18n("&Load...")),
                 m_parent(parent), m_name(name)
{
    kapp->setOverrideCursor( KCursor::waitCursor() );

    m_about = 0L;

    setButtonWhatsThis ( Default, i18n("<p>Reset all filter parameters to their default values.") );
    setButtonWhatsThis ( User3, i18n("<p>Load all filter parameters from settings text file.") );
    setButtonWhatsThis ( User2, i18n("<p>Save all filter parameters to settings text file.") );
    showButton(User2, loadFileSettings);
    showButton(User3, loadFileSettings);

    resize(configDialogSize(name + QString::QString(" Tool Dialog")));

    // -------------------------------------------------------------

    m_mainLayout = new QGridLayout( plainPage(), 2, 1 , marginHint(), spacingHint());

    QFrame *headerFrame = new DigikamImagePlugins::BannerWidget(plainPage(), title);
    m_mainLayout->addMultiCellWidget(headerFrame, 0, 0, 0, 1);

    // -------------------------------------------------------------

    m_mainLayout->setColStretch(0, 10);
    m_mainLayout->setRowStretch(2, 10);

    kapp->restoreOverrideCursor();
}

ImageDialogBase::~ImageDialogBase()
{
    saveDialogSize(m_name + QString::QString(" Tool Dialog"));
    
    if (m_about)
       delete m_about;           
}

void ImageDialogBase::slotHelp()
{
    KApplication::kApplication()->invokeHelp(m_name, "digikamimageplugins");
}

void ImageDialogBase::setAboutData(KAboutData *about)
{
    m_about = about;
    QPushButton *helpButton = actionButton( Help );
    KHelpMenu* helpMenu = new KHelpMenu(this, m_about, false);
    helpMenu->menu()->removeItemAt(0);
    helpMenu->menu()->insertItem(i18n("Plugin Handbook"), this, SLOT(slotHelp()), 0, -1, 0);
    helpButton->setPopup( helpMenu->menu() );
}

void ImageDialogBase::setPreviewAreaWidget(QWidget *w)
{
    m_mainLayout->addMultiCellWidget(w, 1, 2, 0, 0);
}

void ImageDialogBase::setUserAreaWidget(QWidget *w)
{
    m_mainLayout->addMultiCellWidget(w, 1, 2, 1, 1);
}

}  // NameSpace DigikamImagePlugins

#include "imagedialogbase.moc"

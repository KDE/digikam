//////////////////////////////////////////////////////////////////////////////
//
//    SETUP.CPP
//
//    Copyright (C) 2003-2004 Renchi Raju <renchi at pooh.tam.uiuc.edu>
//                            Gilles CAULIER <caulier dot gilles at free.fr>
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation; either version 2 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//////////////////////////////////////////////////////////////////////////////

// Qt includes.

#include <qtabwidget.h>
#include <qapplication.h>
#include <qframe.h>

// KDE includes.

#include <klocale.h>
#include <kiconloader.h>

// Local includes.

#include "setupgeneral.h"
#include "setupmime.h"
#include "setupcamera.h"
#include "setup.h"


Setup::Setup(QWidget* parent, const char* name)
    : KDialogBase(IconList, i18n("Configure"), Help|Ok|Cancel, Ok, parent,
                  name, true, true )
{
    setWFlags(Qt::WDestructiveClose);
    setHelp("setupwindow.anchor", "digikam");
    
    page_general = addPage(i18n("Albums"), i18n("Albums settings"),
                           BarIcon("folder_image", KIcon::SizeMedium));
                                  
    generalPage_ = new SetupGeneral(page_general);

    page_mime = addPage(i18n("Mime types"), i18n("Mime types settings"),
                           BarIcon("mime", KIcon::SizeMedium));
                                  
    mimePage_ = new SetupMime(page_mime);
    
    page_camera = addPage(i18n("Cameras"), i18n("Cameras settings"),
                          BarIcon("digitalcam", KIcon::SizeMedium));
    cameraPage_ = new SetupCamera(page_camera);
    
    connect(this, SIGNAL(okClicked()),
            this, SLOT(slotOkClicked()) );

    show();
}

Setup::~Setup()
{
}

void Setup::slotOkClicked()
{
    generalPage_->applySettings();
    mimePage_->applySettings();
    cameraPage_->applySettings();
    close();
}


#include "setup.moc"

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "startscanpage.h"

// Qt includes

#include <QLabel>

// KDE includes

#include <kvbox.h>
#include <klocale.h>
#include <kstandarddirs.h>

namespace Digikam
{

StartScanPage::StartScanPage(KAssistantDialog* dlg)
             : AssistantDlgPage(dlg, i18n("Scan Your Collection"))
{
    KVBox *vbox  = new KVBox(this);
    QLabel *text = new QLabel(vbox);
    text->setWordWrap(true);
    text->setOpenExternalLinks(true);
    text->setText(i18n("<qt>"
                       "<p>Congratulation. Your minimal setup is done.</p>"
                       "<p>Don't forget that you can customize more settings using the digiKam configuration panel. "
                       "To learn more about the digiKam world, we recommend to read the <b>digiKam handbook</b> "
                       "using the Help/Handbook menu entry (you need to install the separate digiKam documentation package). "
                       "You can also read the manual online from "
                       "<a href='http://www.digikam.org/docs'>digikam.org website</a>.</p>"
                       "<p>Press <b>Finish</b> to close this assistant. digiKam will scan your "
                       "collection to register all items in the database.</p>"
                       "<p><i>Note:</i> depending of your collection size, this operation can take a while. "
		       "If you cancel scanning operation, it will start again at next digiKam session.</p>"
                       "<p>When scanning will be completed, at next start-up, digiKam will check your collection "
                       "to identify only new items. It will be more faster.</p>"
                       "</qt>"));

    setPageWidget(vbox);

    QPixmap leftPix = KStandardDirs::locate("data","digikam/data/assistant-scancollection.png");
    setLeftBottomPix(leftPix.scaledToWidth(128, Qt::SmoothTransformation));
}

StartScanPage::~StartScanPage()
{
}

}   // namespace Digikam

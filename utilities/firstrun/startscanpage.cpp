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
                       "<p>You can customize more settings using digiKam configuration panel. " 
                       "To learn more about digiKam world, we recommend to read <b>digiKam handbook</b> "
                       "using Help/Handbook menu entry (you need to install separate digiKam documentation package). "
                       "You can also read manual online from "
                       "<a href='http://www.digikam.org/docs'>digikam.org website</a>.</p>"
                       "<p>Press <b>Finish</b> to close this assistant. digiKam will scan your "
                       "collection to register all items in database.</p>"
                       "<p><i>Note:</i> depending of your collection size, this operation can take a while. "
                       "At next start-up digiKam will only scan your collection to identify new items. "
                       "It will be more faster.</p>"
                       "</qt>"));

    setPageWidget(vbox);
}

StartScanPage::~StartScanPage()
{
}

}   // namespace Digikam

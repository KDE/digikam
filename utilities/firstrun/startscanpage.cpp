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
    KVBox *vbox   = new KVBox(this);
    QLabel *title = new QLabel(vbox);
    title->setWordWrap(true);
    title->setText(i18n("<qt>"
                        "<p>Your minimal setup is done.</p>"
                        "<p>Press \"Finish\" to lets digiKam scan your collection to register items in database.</p>"
                        "<p>Note: this operation can take a while.</p>"
                        "</qt>"));

    setContentsWidget(vbox);
}

StartScanPage::~StartScanPage()
{
}

}   // namespace Digikam

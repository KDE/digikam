/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-04-13
 * Description : Autodetect dcraw binary
 *
 * Copyright 2006 by Marcel Wiesweg
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

// Qt includes

#include <qprocess.h>

// KDE includes

#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kaboutdata.h>

// Local includes

#include "dcrawbinary.h"

namespace Digikam
{

DcrawBinary *DcrawBinary::m_instance = 0;

DcrawBinary::DcrawBinary()
{
    m_available = false;
}

DcrawBinary::~DcrawBinary()
{
    m_instance = 0;
}

DcrawBinary *DcrawBinary::instance()
{
    if (!m_instance)
        m_instance = new DcrawBinary;
    return m_instance;
}

void DcrawBinary::cleanUp()
{
    delete m_instance;
}

bool DcrawBinary::checkSystem()
{
    QProcess process;

    process.clearArguments();
    process.addArgument("dcraw");

    QString appName = KGlobal::instance()->aboutData()->programName();

    m_available = process.start();

    if (!m_available) {
        int ret = KMessageBox::warningContinueCancel(
                           kapp->activeWindow(),
                           i18n("<qt><p>Unable to find the dcraw executable:<br> "
                                "This program is required by %1 to support raw file formats. "
                                "You can run %2 without this, but you will not be able "
                                "to view or edit any images in raw file formats. "
                                "Please install dcraw as a package from your distributor "
                                "or <a href=\"%3\">download the source</a>.</p>"
                                "<p>Do you want to continue starting %4?</p></qt>")
                                .arg(appName)
				.arg(appName)
                                .arg("http://www.cybercom.net/~dcoffin/dcraw/")
				.arg(appName),
                           QString::null,
                           KStdGuiItem::cont(),
                           QString::fromLatin1("dcrawdetection"),
                           KMessageBox::Notify | KMessageBox::AllowLink
                          );

        if (ret == KMessageBox::Cancel)
            return false;
    }

    // Veturn true even if m_available is false,
    // return value indicates whether the user wants to abort or continue
    return true;
}

const char *DcrawBinary::path()
{
    return "dcraw";
}

bool DcrawBinary::isAvailable()
{
    return m_available;
}

}  // namespace Digikam


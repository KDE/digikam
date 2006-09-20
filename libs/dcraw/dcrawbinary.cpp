/* ============================================================
 * Author: Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Date  : 2006-04-13
 * Description : Autodetect dcraw binary program and version
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

// KDE includes

#include <kapplication.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kdebug.h>
#include <kaboutdata.h>

// Local includes

#include "dcrawbinary.h"
#include "dcrawbinary.moc"

namespace Digikam
{

class DcrawBinaryPriv
{
public:

    DcrawBinaryPriv()
    {
        available = false;
        version   = QString::null;
    }

    bool    available;

    QString version;
};

DcrawBinary *DcrawBinary::m_instance = 0;

DcrawBinary::DcrawBinary()
           : QObject()
{
    d = new DcrawBinaryPriv;
}

DcrawBinary::~DcrawBinary()
{
    m_instance = 0;
    delete d;
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

void DcrawBinary::checkSystem()
{
    KProcess process;
    process.clearArguments();
    process << path();    

    connect(&process, SIGNAL(receivedStderr(KProcess *, char*, int)),
            this, SLOT(slotReadStderrFromDcraw(KProcess*, char*, int)));

    d->available = process.start(KProcess::Block, KProcess::Stderr);
}

void DcrawBinary::slotReadStderrFromDcraw(KProcess*, char* buffer, int buflen)
{
    // The dcraw ouput look like this : Raw Photo Decoder "dcraw" v8.30...
    QString dcrawHeader("Raw Photo Decoder \"dcraw\" v");

    QString stdErr    = QString::fromLocal8Bit(buffer, buflen);
    QString firstLine = stdErr.section('\n', 1, 1);    

    if (firstLine.startsWith(dcrawHeader))
    {
        d->version = firstLine.remove(0, dcrawHeader.length());    
        kdDebug() << "Found dcraw version: " << version() << endl;    
    }
}

const char *DcrawBinary::path()
{
    return "dcraw";
}

bool DcrawBinary::isAvailable() const
{
    return d->available;
}

QString DcrawBinary::version() const
{
    return d->version;
}

bool DcrawBinary::versionIsRight() const
{
    if (d->version.isNull() || !isAvailable())
        return false;

    if (d->version.toFloat() >= minimalVersion().toFloat())
        return true;

    return false;
}

QString DcrawBinary::minimalVersion() const
{
    return QString("8.16");
}

void DcrawBinary::checkReport()
{
    QString appName = KGlobal::instance()->aboutData()->programName();

    if (!isAvailable()) 
    {
        KMessageBox::information(
                     kapp->activeWindow(),
                     i18n("<qt><p>Unable to find the dcraw executable:<br> "
                          "This program is required by %1 to support raw file formats. "
                          "You can use %2 without this, but you will not be able "
                          "to view or edit any images in raw file formats. "
                          "Please install dcraw as a package from your distributor "
                          "or <a href=\"%3\">download the source</a>.</p>"
                          "<p>Note: at least, dcraw version %4 is required by %5</p></qt>")
                          .arg(appName)
                          .arg(appName)
                          .arg("http://www.cybercom.net/~dcoffin/dcraw")
                          .arg(minimalVersion())
                          .arg(appName),
                     QString::null,
                     i18n("Do not show this message again"),
                     KMessageBox::Notify | KMessageBox::AllowLink);
        return;
    }

    if (!versionIsRight()) 
    {
        KMessageBox::information(
                     kapp->activeWindow(),
                     i18n("<qt><p>dcraw executable isn't up to date:<br> "
                          "The version %1 of dcraw have been found on your computer. "
                          "This version is too old to run properly with %2. "
                          "You can run %3 like this, but you will not be able "
                          "to view or edit any images in raw file formats. "
                          "Please update dcraw as a package from your distributor "
                          "or <a href=\"%4\">download the source</a>.</p>"
                          "<p>Note: at least, dcraw version %5 is required by %6</p></qt>")
                          .arg(version())
                          .arg(appName)
                          .arg(appName)
                          .arg("http://www.cybercom.net/~dcoffin/dcraw")
                          .arg(minimalVersion())
                          .arg(appName),
                     QString::null,
                     i18n("Do not show this message again"),
                     KMessageBox::Notify | KMessageBox::AllowLink);
    }
}

}  // namespace Digikam

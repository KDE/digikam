/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at kdemail dot net>
 *          Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Date   : 2006-04-13
 * Description : detection of dcraw binary program
 *
 * Copyright 2006-2007 by Gilles Caulier and Marcel Wiesweg
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

#ifndef DCRAWBINARY_H
#define DCRAWBINARY_H

// Qt includes.

#include <qstring.h>
#include <qobject.h>

// Digikam Includes.

#include "digikam_export.h"

class KProcess;

namespace Digikam
{

class DcrawBinaryPriv;

class DIGIKAM_EXPORT DcrawBinary : public QObject
{
    Q_OBJECT

public:

    static DcrawBinary *instance();
    static void cleanUp();

    const char *path();
    bool isAvailable() const;
    QString version() const;
    bool versionIsRight() const;
    QString minimalVersion() const;

    void checkSystem();
    void checkReport();

private slots:

    void slotReadStderrFromDcraw(KProcess*, char*, int);

private:

    DcrawBinary();
    ~DcrawBinary();

private:

    DcrawBinaryPriv    *d;

    static DcrawBinary *m_instance;
};

} // namespace Digikam

#endif  // DCRAWBINARY_H

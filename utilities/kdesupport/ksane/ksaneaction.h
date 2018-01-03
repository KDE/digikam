/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-15
 * Description : KScan interface
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef KSCAN_IFACE_H
#define KSCAN_IFACE_H

// Qt includes

#include <QAction>
#include <QUrl>

// Local includes

#include "digikam_export.h"

namespace KSaneIface
{
    class KSaneWidget;
}

using namespace KSaneIface;

namespace Digikam
{

class DIGIKAM_EXPORT KSaneAction : public QAction
{
    Q_OBJECT

public:

    explicit KSaneAction(QObject* const parent);
    virtual ~KSaneAction();

    /** Use this method to trigger action with current directory to use to store scanned image.
     *  config is the application config name to store scan dialog qettings between sessions.
     */
    void activate(const QString& targetDir, const QString& config);

Q_SIGNALS:

    void signalImportedImage(const QUrl&);

private:

    KSaneIface::KSaneWidget* m_saneWidget;
};

} // namespace Digikam

#endif /* KSCAN_IFACE_H */

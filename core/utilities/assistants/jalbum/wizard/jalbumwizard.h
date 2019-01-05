/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-04-04
 * Description : a tool to generate HTML image galleries
 *
 * Copyright (C) 2006-2010 by Aurelien Gateau <aurelien dot gateau at free dot fr>
 * Copyright (C) 2012-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_JALBUM_WIZARD_H
#define DIGIKAM_JALBUM_WIZARD_H

// Qt includes

#include <QList>
#include <QUrl>

// Local includes

#include "dwizarddlg.h"
#include "digikam_export.h"

namespace Digikam
{

class JalbumInfo;
class DInfoInterface;

/**
 * The wizard used by the user to select the various settings.
 */
class DIGIKAM_EXPORT JALBUMWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit JALBUMWizard(QWidget* const parent, DInfoInterface* const iface = 0);
    ~JALBUMWizard();

    JalbumInfo*       jalbumInfo()  const;

    bool validateCurrentPage() override;
    int  nextId()                    const override;

    void setItemsList(const QList<QUrl>& urls);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_JALBUM_WIZARD_H

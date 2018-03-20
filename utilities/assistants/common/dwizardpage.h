/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-13
 * Description : a template to create wizard page.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DWIZARD_PAGE_H
#define DWIZARD_PAGE_H

// Qt includes

#include <QWizardPage>
#include <QString>
#include <QPixmap>

// Local includes

#include "digikam_export.h"

class QWizard;

namespace Digikam
{

class DIGIKAM_EXPORT DWizardPage : public QWizardPage
{

public:

    explicit DWizardPage(QWizard* const dlg, const QString& title);
    ~DWizardPage();

public:

    void setPageWidget(QWidget* const w);
    void removePageWidget(QWidget* const w);
    void setLeftBottomPix(const QPixmap& pix);
    void setLeftBottomPix(const QIcon& icon);
    void setShowLeftView(bool v);

    void setComplete(bool b);
    bool isComplete() const;

    int  id() const;

    QWizard* assistant() const;

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DWIZARD_PAGE_H

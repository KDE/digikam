/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2008-01-11
 * Description : a tool to print images
 *
 * Copyright (C) 2008-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef DIGIKAM_ADV_PRINT_WIZARD_H
#define DIGIKAM_ADV_PRINT_WIZARD_H

// Qt includes

#include <QImage>

// Local includes

#include "advprintsettings.h"
#include "ditemslist.h"
#include "dinfointerface.h"
#include "dwizarddlg.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamGenericPrintCreatorPlugin
{

class DIGIKAM_EXPORT AdvPrintWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit AdvPrintWizard(QWidget* const, DInfoInterface* const iface = nullptr);
    ~AdvPrintWizard();

    void setItemsList(const QList<QUrl>& fileList = QList<QUrl>());
    QList<QUrl> itemsList() const;

    DInfoInterface*   iface()    const;
    AdvPrintSettings* settings() const;

    /** Update the pages to be printed and preview first/last pages.
     */
    void previewPhotos();

    void updateCropFrame(AdvPrintPhoto* const, int);

    int  nextId() const override;

    static int normalizedInt(double n);

private:

    bool eventFilter(QObject*, QEvent*) Q_DECL_OVERRIDE;

private Q_SLOTS:

    void slotPreview(const QImage&);

private:

    class Private;
    Private* const d;
};

} // namespace DigikamGenericPrintCreatorPlugin

#endif // DIGIKAM_ADV_PRINT_WIZARD_H

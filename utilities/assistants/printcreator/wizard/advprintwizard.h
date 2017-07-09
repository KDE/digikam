/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-01-11
 * Description : a tool to print images
 *
 * Copyright (C) 2008-2012 by Angelo Naselli <anaselli at linux dot it>
 * Copyright (C) 2006-2017 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ADV_PRINT_WIZARD_H
#define ADV_PRINT_WIZARD_H

// QT incudes

#include <QPainter>
#include <QIcon>
#include <QPrinter>
#include <QDialog>

// Local includes

#include "advprintsettings.h"
#include "dimageslist.h"
#include "dinfointerface.h"
#include "dwizarddlg.h"
#include "digikam_export.h"

namespace Digikam
{

class TemplateIcon;

class DIGIKAM_EXPORT AdvPrintWizard : public DWizardDlg
{
    Q_OBJECT

public:

    explicit AdvPrintWizard(QWidget* const, DInfoInterface* const iface = 0);
    ~AdvPrintWizard();

    void setItemsList(const QList<QUrl>& fileList = QList<QUrl>());
    QList<QUrl> itemsList() const;

    DInfoInterface*   iface()    const;
    AdvPrintSettings* settings() const;

    /// update the pages to be printed and preview first/last pages
    void previewPhotos();

    /// Initialize page layout to the given pageSize in mm
    void initPhotoSizes(const QSizeF& pageSize);

    void updateCropFrame(AdvPrintPhoto* const, int);

    int  nextId() const;

private Q_SLOTS:

    void slotPageChanged(int);

private:

    /// To parse template file with 'fn' as filename, and 'pageSize' in mm.
    void parseTemplateFile(const QString& fn,
                           const QSizeF& pageSize);

    QRect* getLayout(int photoIndex) const;

    void saveSettings(const QString& pageName);
    void readSettings(const QString& pageName);
    void startPrinting();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ADV_PRINT_WIZARD_H

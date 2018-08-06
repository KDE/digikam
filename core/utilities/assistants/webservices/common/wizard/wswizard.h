/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-06-27
 * Description : a tool to export items to web services.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2018 by Thanh Trung Dinh <dinhthanhtrung1996 at gmail dot com>
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

#ifndef DIGIKAM_WS_WIZARD_H
#define DIGIKAM_WS_WIZARD_H

// Qt includes

#include <QList>
#include <QUrl>
#include <QSettings>

// Local includes

#include "dwizarddlg.h"
#include "dinfointerface.h"
#include "digikam_export.h"
#include "wssettings.h"
#include "wsauthentication.h"

namespace Digikam
{

class DIGIKAM_EXPORT WSWizard : public DWizardDlg
{
    Q_OBJECT
    
public:

    explicit WSWizard(QWidget* const parent, DInfoInterface* const iface = 0);
    ~WSWizard();

    bool validateCurrentPage() override;
    int  nextId() const override;

    DInfoInterface*     iface()  const;
    WSSettings*         settings() const;

    /*
     * Instance of WSAuthentication (which wraps instance of WSTalker) and correspondant QSettings
     * are initialized only once in WSWizard.
     *
     * These 2 methods below are getters, used in other pages of wizard so as to facilitate
     * access to WSAuthentication instance and its settings.
     */
    WSAuthentication*   wsAuth() const;
    QSettings*          oauthSettings() const;

    void setItemsList(const QList<QUrl>& urls);

public Q_SLOTS:
    
    void slotBusy(bool val);
    
private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // DIGIKAM_WS_WIZARD_H

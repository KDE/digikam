/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2011-04-12
 * Description : A tool to export items to Rajce web service
 *
 * Copyright (C) 2011      by Lukas Krejci <krejci.l at centrum dot cz>
 * Copyright (C) 2011-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_RAJCE_WINDOW_H
#define DIGIKAM_RAJCE_WINDOW_H

// Local includes

#include "wstooldialog.h"
#include "dinfointerface.h"
#include "digikam_export.h"

using namespace Digikam;

namespace DigikamGenericRajcePlugin
{

class RajceWidget;

class DIGIKAM_EXPORT RajceWindow : public WSToolDialog
{
    Q_OBJECT

public:

    explicit RajceWindow(DInfoInterface* const iface, QWidget* const parent = nullptr);
    ~RajceWindow();

    void reactivate();

protected:

    void closeEvent(QCloseEvent* e) override;

private Q_SLOTS:

    void slotSetUploadButtonEnabled(bool);
    void slotFinished();

private:

    RajceWidget* m_widget;
};

} // namespace DigikamGenericRajcePlugin

#endif // DIGIKAM_RAJCE_WINDOW_H

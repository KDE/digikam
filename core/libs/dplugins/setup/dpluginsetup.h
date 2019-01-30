/* ============================================================
 *
 * This file is a part of digiKam project
 * https://www.digikam.org
 *
 * Date        : 2007-02-06
 * Description : Config panel for generic dplugins.
 *
 * Copyright (C) 2018-2019 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIGIKAM_DPLUGIN_SETUP_H
#define DIGIKAM_DPLUGIN_SETUP_H

// Qt includes

#include <QWidget>

// Local includes

#include "searchtextbar.h"
#include "digikam_export.h"
#include "dpluginaction.h"

class QTreeWidgetItem;

namespace Digikam
{

class DPluginConfView;
    
class DIGIKAM_EXPORT DPluginSetup : public QWidget
{
    Q_OBJECT

public:

    explicit DPluginSetup(QWidget* const parent = 0);
    ~DPluginSetup();

    void setPluginConfView(DPluginConfView* const view);
    void applySettings();

private Q_SLOTS:

    void slotSearchTextChanged(const SearchTextSettings& settings);
    void slotSetFilter(const QString& filter, Qt::CaseSensitivity cs);
    void slotCheckAll();
    void slotClearList();
    void slotItemClicked();
    void slotSearchResult(int found);
    void slotAboutPlugin(QTreeWidgetItem*);

private:

    void updateInfo();

private:

    class Private;
    Private* const d;
};

}  // namespace Digikam

#endif // DIGIKAM_DPLUGIN_SETUP_H

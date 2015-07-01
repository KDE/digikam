/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-06-16
 * Description : Advanced Configuration tab for metadata.
 *
 * Copyright (C) 2015 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail.com>
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
#include <QWidget>

namespace Digikam
{


class AdvancedMetadataTab : public QWidget
{
    Q_OBJECT
public:
    AdvancedMetadataTab(QWidget *parent = 0);
    virtual ~AdvancedMetadataTab();

public Q_SLOTS:

    void slotResetView();

private:

    void connectButtons();
    void setModelData();
    class Private;
    Private* d;
};

}

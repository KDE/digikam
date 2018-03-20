/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-17
 * Description : Metadata tags selector config panel.
 *
 * Copyright (C) 2009-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATAPANEL_H
#define METADATAPANEL_H

// Qt includes

#include <QObject>
#include <QString>

// Local includes

#include "digikam_export.h"

class QTabWidget;

namespace Digikam
{

class MetadataSelectorView;

class DIGIKAM_EXPORT MetadataPanel : public QObject
{
    Q_OBJECT

public:

    explicit MetadataPanel(QTabWidget* const tab);
    ~MetadataPanel();

    void                         applySettings();

    QStringList                  getAllCheckedTags();
    QList<MetadataSelectorView*> viewers();

    static QStringList defaultExifFilter();
    static QStringList defaultMknoteFilter();
    static QStringList defaultIptcFilter();
    static QStringList defaultXmpFilter();

private Q_SLOTS:

    void slotTabChanged(int);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // METADATAPANEL_H

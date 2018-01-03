/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-19
 * Description : Restoration batch tool.
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

#ifndef RESTORATION_H
#define RESTORATION_H

// Local includes

#include "batchtool.h"
#include "greycstorationfilter.h"

class QComboBox;

namespace Digikam
{

class Restoration : public BatchTool
{
    Q_OBJECT

public:

    explicit Restoration(QObject* const parent = 0);
    ~Restoration();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new Restoration(parent); };

    void registerSettingsWidget();

    void cancel();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    enum RestorationPreset
    {
        ReduceUniformNoise = 0,
        ReduceJPEGArtefacts,
        ReduceTexturing
    };

    QComboBox*            m_comboBox;

    GreycstorationFilter* m_cimgIface;
};

}  // namespace Digikam

#endif /* RESTORATION_H */

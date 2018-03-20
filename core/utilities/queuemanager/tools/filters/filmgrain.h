/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : a batch tool to addd film grain to images.
 *
 * Copyright (C) 2010-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FILM_GRAIN_H_
#define FILM_GRAIN_H_

#include "batchtool.h"

namespace Digikam
{

class FilmGrainSettings;

class FilmGrain : public BatchTool
{
    Q_OBJECT

public:

    explicit FilmGrain(QObject* const parent = 0);
    ~FilmGrain();

    BatchToolSettings defaultSettings();

    BatchTool* clone(QObject* const parent=0) const { return new FilmGrain(parent); };

    void registerSettingsWidget();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    FilmGrainSettings* m_settingsView;
};

} // namespace Digikam

#endif // FILM_GRAIN_H_

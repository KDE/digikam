/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-09
 * Description : A widget to select Physical or virtual albums with combo-box
 *
 * Copyright (C) 2010-2012 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2012-2013 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMSELECTORS_H
#define ALBUMSELECTORS_H

// Qt includes

#include <QWidget>

namespace Digikam
{

class Album;

class AlbumSelectors : public QWidget
{
    Q_OBJECT

public:

    explicit AlbumSelectors(const QString& label, const QString& configName, QWidget* const parent = 0);
    ~AlbumSelectors();

    QList<Album*> selectedAlbums() const;

    void loadState();
    void saveState();

private Q_SLOTS:

    void slotUpdateClearButtons();

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // ALBUMSELECTORS_H

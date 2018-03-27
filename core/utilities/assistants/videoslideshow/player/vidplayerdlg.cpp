/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2017-05-25
 * Description : a tool to generate video slideshow from images.
 *
 * Copyright (C) 2017-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "vidplayerdlg.h"

// Qt includes

#include <QGridLayout>
#include <QApplication>
#include <QStyle>

// Local includes

#include "mediaplayerview.h"

namespace Digikam
{

class VidPlayerDlg::Private
{
public:

    Private()
    {
        player   = 0;
    }

public:

    MediaPlayerView* player;
};

VidPlayerDlg::VidPlayerDlg(const QString& file, QWidget* const parent)
    : QDialog(parent),
      d(new Private)
{
    setModal(false);
    setWindowTitle(file);

    d->player = new MediaPlayerView(this);
    d->player->setCurrentItem(QUrl::fromLocalFile(file));

    // ----------------------

    QGridLayout* const grid = new QGridLayout(this);
    grid->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));
    grid->addWidget(d->player, 0, 0, 1, 1);
    grid->setColumnStretch(0, 10);
    setLayout(grid);

    // ----------------------

    connect(d->player, SIGNAL(signalEscapePreview()),
            this, SLOT(accept()));
}

VidPlayerDlg::~VidPlayerDlg()
{
    delete d;
}

} // namespace Digikam

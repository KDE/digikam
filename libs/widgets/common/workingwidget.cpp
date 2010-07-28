/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-27
 * Description : Widget showing a throbber ("working" animation)
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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
#include "workingwidget.moc"

// Qt includes

#include <QImage>

// KDE includes

#include <KIconLoader>

// Local includes

#include "workingwidget.h"

namespace Digikam
{

WorkingWidget::WorkingWidget(QWidget* parent)
              : QLabel(parent), currentPixmap(0)
{
    KIconLoader iconLoader;
    QString iconName("process-working.png");
    QImage img;
    img.load(iconLoader.iconPath(iconName, KIconLoader::Dialog));

    // frames in the image, the "process-working.png" has 8
    int imageCount = 8;
    int subImageHeight = img.height() / imageCount;

    for (int i = 0; i < imageCount; i++)
    {
        QImage subImage = img.copy(0, i * subImageHeight, img.width(), subImageHeight);
        pixmaps.push_back(QPixmap::fromImage(subImage));
    }

    connect(&timer, SIGNAL(timeout()), SLOT(changeImage()));
    timer.start(100);
    changeImage();
}

WorkingWidget::~WorkingWidget()
{
}

void WorkingWidget::changeImage()
{
    if (currentPixmap >= pixmaps.length())
        currentPixmap = 0;

    setPixmap(pixmaps.at(currentPixmap));

    currentPixmap++;
}

} // namespace Digikam
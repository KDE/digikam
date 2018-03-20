/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-09
 * Description : color label filter
 *
 * Copyright (C) 2011-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorlabelfilter.h"

// Local includes

#include "digikam_debug.h"
#include "albummanager.h"
#include "tagscache.h"

namespace Digikam
{

ColorLabelFilter::ColorLabelFilter(QWidget* const parent)
    : ColorLabelWidget(parent)
{
    setDescriptionBoxVisible(false);
    setButtonsExclusive(false);
    reset();

    connect(this, SIGNAL(signalColorLabelChanged(int)),
            this, SLOT(slotColorLabelSelectionChanged()));
}

ColorLabelFilter::~ColorLabelFilter()
{
}

void ColorLabelFilter::reset()
{
    setColorLabels(QList<ColorLabel>());
    slotColorLabelSelectionChanged();
}

QList<TAlbum*> ColorLabelFilter::getCheckedColorLabelTags()
{
    QList<TAlbum*> list;
    int tagId   = 0;
    TAlbum* tag = 0;

    foreach(const ColorLabel& cl, colorLabels())
    {
        tagId = TagsCache::instance()->tagForColorLabel(cl);
        tag   = AlbumManager::instance()->findTAlbum(tagId);

        if (tagId)
        {
            list.append(tag);
        }
    }

    return list;
}

void ColorLabelFilter::slotColorLabelSelectionChanged()
{
    emit signalColorLabelSelectionChanged(colorLabels());
}

}  // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 20013-07-03
 * Description : Tag Properties widget to display tag properties
 *               when a tag or multiple tags are selected
 *
 * Copyright (C) 2013 by Veaceslav Munteanu <veaceslav dot munteanu90 at gmail dot com>
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

#ifndef TAGPROPWIDGET_H
#define TAGPROPWIDGET_H

#include <QWidget>

namespace Digikam
{

class TAlbum;

class TagPropWidget : public QWidget
{
    Q_OBJECT

public:
    TagPropWidget(QWidget* const parent);

public Q_SLOTS:
    void slotSelectionChanged(TAlbum* album);
private Q_SLOTS:
    void slotIconResetClicked();
    void slotIconChanged();

private:
    class PrivateTagProp;
    PrivateTagProp* d;
};

}

#endif //TAGPROPWIDGET

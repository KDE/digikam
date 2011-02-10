/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-02-09
 * Description : color label filter
 *
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef COLORLABELFILTER_H
#define COLORLABELFILTER_H

// Qt includes

#include <QList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>

// Local includes

#include "colorlabelwidget.h"

namespace Digikam
{

class TAlbum;

class ColorLabelCheckBox : public QTreeWidgetItem
{

public:

    ColorLabelCheckBox(QTreeWidget* parent=0);
    ~ColorLabelCheckBox();

    void setColorLabel(ColorLabel label);
    ColorLabel colorLabel() const;

private:

    ColorLabel m_label;
};

// ------------------------------------------------------------------------------

class ColorLabelFilter : public QTreeWidget
{
    Q_OBJECT

public:

    ColorLabelFilter(QWidget* parent=0);
    ~ColorLabelFilter();

    void setColorLabelSelection(const QList<ColorLabel>& sel);
    QList<ColorLabel> colorLabelSelection();

    QList<TAlbum*> getCheckedColorLabelTags();

    void reset();

Q_SIGNALS:

    void signalColorLabelSelectionChanged(const QList<ColorLabel>&);

private Q_SLOTS:

    void slotColorLabelSelectionChanged();
};

}  // namespace Digikam

Q_DECLARE_METATYPE(QList<Digikam::ColorLabel>)

#endif // COLORLABELFILTER_H

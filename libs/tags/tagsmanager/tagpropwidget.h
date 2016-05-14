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

// Qt includes

#include <QWidget>

// Local includes

#include "digikam_config.h"

namespace Digikam
{

class Album;

class TagPropWidget : public QWidget
{
    Q_OBJECT

public:

    enum ItemsEnable
    {
        DisabledAll,
        EnabledAll,
        IconOnly
    };

public:

    TagPropWidget(QWidget* const parent);
    ~TagPropWidget();

Q_SIGNALS:

    void signalTitleEditReady();

public Q_SLOTS:

    void slotSelectionChanged(QList<Album*> albums);
    void slotFocusTitleEdit();

private Q_SLOTS:

    void slotIconResetClicked();
    void slotIconChanged();
    void slotDataChanged();
    void slotSaveChanges();
    void slotDiscardChanges();
    void slotReturnPressed();

private:

    /**
     * @brief enableItems - enable items based on selection.
     *                      If no item is selected, disable all,
     *                      if one item selected, enable all,
     *                      if multiple selected, enable icon & icon button
     */
    void enableItems(ItemsEnable value);

private:

    class Private;
    Private* const d;
};

} // namespace Digikam

#endif // TAGPROPWIDGET

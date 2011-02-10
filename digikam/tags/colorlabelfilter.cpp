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

#include "colorlabelfilter.moc"

// Qt includes

#include <QBrush>
#include <QWidgetAction>

// KDE includes

#include <kmenu.h>
#include <klocale.h>
#include <kdebug.h>

namespace Digikam
{

ColorLabelCheckBox::ColorLabelCheckBox(QListWidget* parent)
    : QListWidgetItem(parent)
{
    setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
    setCheckState(Qt::Unchecked);
}

ColorLabelCheckBox::~ColorLabelCheckBox()
{
}

void ColorLabelCheckBox::setColorLabel(ColorLabel label)
{
    m_label = label;

    if (m_label != NoneLabel)
        setBackground(QBrush(ColorLabelWidget::labelColor(label)));

    if (m_label == BlackLabel)
        setForeground(QBrush(Qt::white));

    setText(ColorLabelWidget::labelColorName(label));
}

ColorLabel ColorLabelCheckBox::colorLabel() const
{
    return m_label;
}

// -----------------------------------------------------------------------------

ColorLabelFilterView::ColorLabelFilterView(QWidget* parent)
    : QListWidget(parent)
{
    for (int i = NoneLabel ; i <= WhiteLabel ; ++i)
    {
        ColorLabelCheckBox* item = new ColorLabelCheckBox(this);
        item->setColorLabel((ColorLabel)i);
    }
    connect(this, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SIGNAL(signalColorLabelSelectionChanged()));
}

ColorLabelFilterView::~ColorLabelFilterView()
{
}

void ColorLabelFilterView::setColorLabelSelection(const QList<ColorLabel>& sel)
{
    int it = 0;

    while (it <= count())
    {
        ColorLabelCheckBox* cb = dynamic_cast<ColorLabelCheckBox*>(item(it));
        if (cb && sel.contains(cb->colorLabel()))
        {
            cb->setCheckState(Qt::Checked);
        }

        ++it;
    }
}

QList<ColorLabel> ColorLabelFilterView::colorLabelSelection() const
{
    QList<ColorLabel> sel;
    int it = 0;

    while (it <= count())
    {
        ColorLabelCheckBox* cb = dynamic_cast<ColorLabelCheckBox*>(item(it));
        if (cb && cb->checkState() == Qt::Checked)
        {
            sel.append(cb->colorLabel());
        }

        ++it;
    }

    return sel;
}

// -----------------------------------------------------------------------------

ColorLabelFilter::ColorLabelFilter(QWidget* parent)
    : QPushButton(parent)
{
    KMenu* popup = new KMenu(this);
    setMenu(popup);
    setToolTip(i18n("Color Label Filter"));

    QWidgetAction* action = new QWidgetAction(this);
    m_view                = new ColorLabelFilterView(this);
    action->setDefaultWidget(m_view);
    popup->addAction(action);
    slotColorLabelSelectionChanged();

    connect(m_view, SIGNAL(signalColorLabelSelectionChanged()),
            this, SLOT(slotColorLabelSelectionChanged()));
}

ColorLabelFilter::~ColorLabelFilter()
{
}

void ColorLabelFilter::setColorLabelSelection(const QList<ColorLabel>& sel)
{
    m_view->setColorLabelSelection(sel);
}

QList<ColorLabel> ColorLabelFilter::colorLabelSelection() const
{
    return m_view->colorLabelSelection();
}

void ColorLabelFilter::slotColorLabelSelectionChanged()
{
    setText(i18nc("Indicate how many Color Labels are selected in Icon View filters", "(%1)",
                  colorLabelSelection().count()));

    emit signalColorLabelSelectionChanged(colorLabelSelection());
}

}  // namespace Digikam

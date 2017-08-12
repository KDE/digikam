/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-09-01
 * Description : a plugin to create photo layouts by fusion of several images.
 * 
 *
 * Copyright (C) 2011      by Lukasz Spas <lukasz dot spas at gmail dot com>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef LAYERSTREETITLEWIDGET_H
#define LAYERSTREETITLEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>

#include <QPushButton>
#include <QIcon>
#include <klocalizedstring.h>

namespace PhotoLayoutsEditor
{
    class LayersTreeTitleWidget : public QWidget
    {
            QHBoxLayout * m_layout;
            QLabel * m_label;
            QPushButton * m_up_btn;
            QPushButton * m_dwn_btn;

        public:

            LayersTreeTitleWidget (QWidget * parent = 0) :
                QWidget(parent),
                m_layout(new QHBoxLayout(this)),
                m_label(new QLabel(i18n("Layers"),this)),
                m_up_btn(new QPushButton(QIcon(QLatin1String(":/arrow_top.png")), QString(), this)),
                m_dwn_btn(new QPushButton(QIcon(QLatin1String(":/arrow_down.png")), QString(), this))
            {
                m_layout->addWidget(m_label,1);
                m_layout->addWidget(m_up_btn);
                m_layout->addWidget(m_dwn_btn);
                this->setLayout(m_layout);
                m_layout->setContentsMargins(QMargins());
                m_layout->setSpacing(0);
                m_layout->update();

                m_up_btn->setFixedSize(24,24);
                m_dwn_btn->setFixedSize(24,24);
            }

            QAbstractButton * moveUpButton() const
            {
                return m_up_btn;
            }

            QAbstractButton * moveDownButton() const
            {
                return m_dwn_btn;
            }
    };
}

#endif // LAYERSTREETITLEWIDGET_H

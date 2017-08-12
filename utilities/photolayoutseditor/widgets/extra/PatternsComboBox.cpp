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

#include "PatternsComboBox.h"

#include <QStyledItemDelegate>
#include <QStylePainter>
#include <QDebug>
#include <QListView>
#include <QPaintEngine>
#include <QPaintEvent>

using namespace PhotoLayoutsEditor;

class PatternDelegate : public QStyledItemDelegate
{
    public:
        PatternDelegate(QObject * parent = 0) :
            QStyledItemDelegate(parent)
        {}
        virtual ~PatternDelegate()
        {}
        virtual QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const
        {
            QSize result = option.rect.size();
            if (index.isValid())
                result.setHeight(24);
            return result;
        }
        virtual void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
        {
            if (index.isValid())
            {
                Qt::BrushStyle style = (Qt::BrushStyle) index.data(Qt::UserRole).toInt();
                QBrush b(Qt::black, style);
                QRectF r = option.rect;
                r.setHeight(24);
                painter->fillRect(r, b);
            }
        }
};

PatternsComboBox::PatternsComboBox(QWidget * parent) :
    QComboBox(parent)
{
    addItem(QLatin1String(""), QVariant((int)Qt::Dense1Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::Dense2Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::Dense3Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::Dense4Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::Dense5Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::Dense6Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::Dense7Pattern));
    addItem(QLatin1String(""), QVariant((int)Qt::HorPattern));
    addItem(QLatin1String(""), QVariant((int)Qt::VerPattern));
    addItem(QLatin1String(""), QVariant((int)Qt::CrossPattern));
    addItem(QLatin1String(""), QVariant((int)Qt::BDiagPattern));
    addItem(QLatin1String(""), QVariant((int)Qt::FDiagPattern));
    addItem(QLatin1String(""), QVariant((int)Qt::DiagCrossPattern));
    setItemDelegate(new PatternDelegate(this));
    setMinimumWidth(100);
    connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(emitPatternChanged(int)));
}

Qt::BrushStyle PatternsComboBox::pattern() const
{
    return (Qt::BrushStyle) this->itemData( this->currentIndex() ).toInt();
}

void PatternsComboBox::setPattern(Qt::BrushStyle pattern)
{
    for (int i = this->count()-1; i >= 0; --i)
    {
        if (static_cast<Qt::BrushStyle>(itemData(i).toInt()) == pattern)
        {
            this->setCurrentIndex(i);
            return;
        }
    }
    this->setCurrentIndex(-1);
    return;
}

void PatternsComboBox::paintEvent(QPaintEvent * e)
{
    QComboBox::paintEvent(e);
    QStylePainter p(this);

    QStyleOptionComboBox op;
    initStyleOption(&op);

    QRect r = style()->subElementRect( QStyle::SE_ComboBoxFocusRect, &op, this );
    r.setHeight(r.height()-3);
    r.setWidth(r.width()-3);
    r.setX(r.x()+1);
    r.setY(r.y()+1);
    QBrush b(Qt::black, (Qt::BrushStyle)this->itemData(this->currentIndex()).toInt());
    p.fillRect(r,b);
}

void PatternsComboBox::emitPatternChanged(int index)
{
    emit currentPatternChanged( (Qt::BrushStyle) this->itemData(index).toInt() );
}

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "clicklabel.h"
#include "clicklabel.moc"

// Qt includes

#include <QMouseEvent>
#include <QPainter>
#include <QPen>
#include <QStyle>
#include <QStyleOption>

// KDE includes

#include <kdebug.h>
#include <kglobalsettings.h>
#include <klocale.h>

// Local includes

namespace Digikam
{

ClickLabel::ClickLabel(QWidget *parent)
                : QLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

ClickLabel::ClickLabel(const QString &text, QWidget *parent)
                : QLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void ClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    QLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        emit activated();
        event->accept();
    }
}

void ClickLabel::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Down:
        case Qt::Key_Right:
        case Qt::Key_Space:
            emit activated();
            return;
        default:
    }

    QLabel::keyPressEvent(e);
}

// ------------------------------------------------------------------------

SqueezedClickLabel::SqueezedClickLabel(QWidget *parent)
                        : KSqueezedTextLabel(parent)
{
    setCursor(Qt::PointingHandCursor);
}

SqueezedClickLabel::SqueezedClickLabel(const QString &text, QWidget *parent)
                        : KSqueezedTextLabel(text, parent)
{
    setCursor(Qt::PointingHandCursor);
}

void SqueezedClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    KSqueezedTextLabel::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
        emit activated();
        event->accept();
    }
}

void SqueezedClickLabel::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
        case Qt::Key_Down:
        case Qt::Key_Right:
        case Qt::Key_Space:
            emit activated();
            return;
        default:
    }

    QLabel::keyPressEvent(e);
}

// ------------------------------------------------------------------------

ArrowClickLabel::ArrowClickLabel(QWidget *parent)
               : QWidget(parent), m_arrowType(Qt::RightArrow)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_size   = 8;
    m_margin = 2;
}

void ArrowClickLabel::setArrowType(Qt::ArrowType type)
{
    m_arrowType = type;
    update();
}

void ArrowClickLabel::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit leftClicked();
    }
}

void ArrowClickLabel::paintEvent(QPaintEvent*)
{
    // Inspired by karrowbutton.cpp,
    //  Copyright (C) 2001 Frerich Raabe <raabe@kde.org>

    QPainter p(this);

    QStyleOptionFrame opt;
    opt.init(this);
    opt.lineWidth    = 2;
    opt.midLineWidth = 0;

    /*
    p.fillRect( rect(), palette().brush( QPalette::Background ) );
    style()->drawPrimitive( QStyle::PE_Frame, &opt, &p, this);
    */

    if (m_arrowType == Qt::NoArrow)
        return;

    if (width() < m_size + m_margin ||
        height() < m_size + m_margin)
        return; // don't draw arrows if we are too small

    unsigned int x = 0, y = 0;
    if (m_arrowType == Qt::DownArrow) 
    {
        x = (width() - m_size) / 2;
        y = height() - (m_size + m_margin);
    } 
    else if (m_arrowType == Qt::UpArrow) 
    {
        x = (width() - m_size) / 2;
        y = m_margin;
    }
    else if (m_arrowType == Qt::RightArrow) 
    {
        x = width() - (m_size + m_margin);
        y = (height() - m_size) / 2;
    } 
    else // arrowType == LeftArrow
    {
        x = m_margin;
        y = (height() - m_size) / 2;
    }

    /*
    if (isDown()) 
    {
        x++;
        y++;
    }
    */

    QStyle::PrimitiveElement e = QStyle::PE_IndicatorArrowLeft;
    switch (m_arrowType)
    {
        case Qt::LeftArrow:
            e = QStyle::PE_IndicatorArrowLeft;
            break;
        case Qt::RightArrow:
            e = QStyle::PE_IndicatorArrowRight;
            break;
        case Qt::UpArrow:
            e = QStyle::PE_IndicatorArrowUp;
            break;
        case Qt::DownArrow:
            e = QStyle::PE_IndicatorArrowDown;
            break;
        case Qt::NoArrow:
            break;
    }

    opt.state |= QStyle::State_Enabled;
    opt.rect   = QRect( x, y, m_size, m_size);

    style()->drawPrimitive( e, &opt, &p, this );
}

QSize ArrowClickLabel::sizeHint() const
{
    return QSize(m_size + 2*m_margin, m_size + 2*m_margin);
}


} // namespace Digikam

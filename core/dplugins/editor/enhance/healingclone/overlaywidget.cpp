#include "overlaywidget.h"
#include <QPen>
#include <QBrush>
#include <QDebug>
#include<QPainter>

OverlayWidget::OverlayWidget(QWidget *parent) : QWidget(parent,Qt::Window)
{

    m_nPTargetPixmap = QPixmap(parent->size());
    m_nPTargetPixmap.fill();
    m_nbMousePressed = false;
}

OverlayWidget::~OverlayWidget()
{

}

void OverlayWidget::mousePressEvent(QMouseEvent* event)
{
    m_nbMousePressed = true;
    m_line.setP1(event->pos());
    m_line.setP2(event->pos());
}

void OverlayWidget::mouseReleaseEvent(QMouseEvent *event)
{
    m_nbMousePressed = false;
    update();
}

void OverlayWidget::paintEvent(QPaintEvent *e)
{
    static bool wasPressed = false;
    QPainter painter(this);

    if(m_nbMousePressed)
    {
        painter.drawPixmap(0, 0, m_nPTargetPixmap);
        painter.drawLine(m_line);
        wasPressed = true;
    }
    else if(wasPressed)
    {
        QPainter PixmapPainter(&m_nPTargetPixmap);
        QPen pen(Qt::green);
        PixmapPainter.setPen(pen);
        PixmapPainter.drawLine(m_line);

        painter.drawPixmap(0, 0, m_nPTargetPixmap);
        wasPressed = false;
    }
}

void OverlayWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->type() == QEvent::MouseMove)
    {
        m_line.setP2(event->pos());
    }
    update();
}

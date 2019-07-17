#ifndef OVERLAYWIDGET_H
#define OVERLAYWIDGET_H

#include <QObject>
#include <QWidget>
#include<QLine>
#include<QPixmap>
#include<QMouseEvent>
#include<QEvent>

class OverlayWidget : public QWidget
{
    Q_OBJECT
public:
    explicit OverlayWidget(QWidget *parent = nullptr);
    ~OverlayWidget();

Q_SIGNALS:

public Q_SLOTS:

protected:
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void paintEvent(QPaintEvent *);
private:
    QLine m_line;
    QPixmap m_nPTargetPixmap;
    bool m_nbMousePressed;
};

#endif // OVERLAYWIDGET_H

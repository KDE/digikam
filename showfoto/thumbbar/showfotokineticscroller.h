#ifndef SHOWFOTOKINETICSCROLLER_H
#define SHOWFOTOKINETICSCROLLER_H

#include <QObject>
#include <QScopedPointer>
#include <QAbstractScrollArea>
#include <QListView>

namespace ShowFoto
{

//! Vertical kinetic scroller implementation without overshoot and bouncing.
//! A temporary solution to get kinetic-like scrolling on Symbian.

class ShowfotoKineticScroller: public QObject
{
   Q_OBJECT

public:
   ShowfotoKineticScroller(QObject* parent = 0);
   ~ShowfotoKineticScroller();
   //! enabled for one widget only, new calls remove previous association
   void enableKineticScrollFor(QAbstractScrollArea* scrollArea);
   void setScrollFlow(QListView::Flow flow);

protected:
   bool eventFilter(QObject* object, QEvent* event);

private slots:
   void onKineticTimerElapsed();

private:
    class Private;
    Private* const d;
};

}
#endif // SHOWFOTOKINETICSCROLLER_H

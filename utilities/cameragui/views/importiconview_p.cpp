#include "importiconview_p.h"

namespace Digikam
{

ImportIconView::ImportIconViewPriv::ImportIconViewPriv(ImportIconView* qq)
    : overlaysActive(false), q_ptr(qq)
{
    //TODO: utilities          = 0;
    //TODO: rotateLeftOverlay  = 0;
    //TODO: rotateRightOverlay = 0;
    normalDelegate     = 0;
}

ImportIconView::ImportIconViewPriv::~ImportIconViewPriv()
{
}

void ImportIconView::ImportIconViewPriv::updateOverlays()
{
    Q_Q(ImportIconView);
    ImportSettings* settings = ImportSettings::instance();

//TODO: Implement overlays.
//    if (overlaysActive)
//    {
//        if (!settings->getIconShowOverlays())
//        {
//            disconnect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                       q, SLOT(slotRotateLeft(QList<QModelIndex>)));

//            disconnect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                       q, SLOT(slotRotateRight(QList<QModelIndex>)));

//            q->removeOverlay(rotateLeftOverlay);
//            q->removeOverlay(rotateRightOverlay);

//            overlaysActive = false;
//        }
//    }
//    else
//    {
//        if (settings->getIconShowOverlays())
//        {
//            q->addOverlay(rotateLeftOverlay, normalDelegate);
//            q->addOverlay(rotateRightOverlay, normalDelegate);

//            connect(rotateLeftOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                    q, SLOT(slotRotateLeft(QList<QModelIndex>)));

//            connect(rotateRightOverlay, SIGNAL(signalRotate(QList<QModelIndex>)),
//                    q, SLOT(slotRotateRight(QList<QModelIndex>)));

//            overlaysActive = true;
//        }
//    }
}

} // namespace Digikam

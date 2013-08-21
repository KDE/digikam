#include "showfotodragdrop.h"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>

// Local includes

#include "showfoto.h"
#include "ddragobjects.h"
#include "showfotocategorizedview.h"
#include "showfotoiteminfo.h"
#include "albummanager.h"
#include "digikamapp.h"
#include "digikamview.h"

using namespace Digikam;

namespace ShowFoto
{

ShowfotoDragDropHandler::ShowfotoDragDropHandler(ShowfotoImageModel* const model)
    : AbstractItemDragDropHandler(model)
{
}

QAction* ShowfotoDragDropHandler::addGroupAction(KMenu* const menu)
{
    return menu->addAction(SmallIcon("arrow-down-double"),
                           i18nc("@action:inmenu Group images with this image", "Group here"));
}

QAction* ShowfotoDragDropHandler::addCancelAction(KMenu* const menu)
{
    return menu->addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
}

ShowfotoDragDropHandler::DropAction ShowfotoDragDropHandler::copyOrMove(const QDropEvent* e, QWidget* const view,
                                                                    bool allowMove, bool askForGrouping)
{
    if (e->keyboardModifiers() & Qt::ControlModifier)
    {
        return CopyAction;
    }
    else if (e->keyboardModifiers() & Qt::ShiftModifier)
    {
        return MoveAction;
    }

    if (!allowMove && !askForGrouping)
    {
        switch (e->proposedAction())
        {
            case Qt::CopyAction:
                return CopyAction;
            case Qt::MoveAction:
                return MoveAction;
            default:
                return NoAction;
        }
    }

    KMenu popMenu(view);

    QAction* moveAction = 0;

    if (allowMove)
    {
        moveAction = popMenu.addAction( SmallIcon("go-jump"), i18n("&Move Here"));
    }

    QAction* const copyAction = popMenu.addAction( SmallIcon("edit-copy"), i18n("&Copy Here"));
    popMenu.addSeparator();

    QAction* groupAction = 0;

    if (askForGrouping)
    {
        groupAction = addGroupAction(&popMenu);
        popMenu.addSeparator();
    }

    addCancelAction(&popMenu);

    popMenu.setMouseTracking(true);
    QAction* const choice = popMenu.exec(QCursor::pos());

    if (moveAction && choice == moveAction)
    {
        return MoveAction;
    }
    else if (choice == copyAction)
    {
        return CopyAction;
    }
    else if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }

    return NoAction;
}

/*
static DropAction tagAction(const QDropEvent*, QWidget* view, bool askForGrouping)
{
}

static DropAction groupAction(const QDropEvent*, QWidget* view)
{
}
*/

bool ShowfotoDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
//    ShowfotoCategorizedView* const view = static_cast<ShowfotoCategorizedView*>(abstractview);

//    if (accepts(e, droppedOn) == Qt::IgnoreAction)
//    {
//        return false;
//    }

//    if (DItemDrag::canDecode(e->mimeData()))
//    {
//        //TODO::
//        //KUrl::List lst = DigikamApp::instance()->view()->selectedUrls();

//        KMenu popMenu(view);
//        popMenu.addTitle(SmallIcon("digikam"), i18n("Exporting"));
//        QAction* const upAction = popMenu.addAction(SmallIcon("media-flash-smart-media"),
//                                                    i18n("Upload to Camera"));
//        popMenu.addSeparator();
//        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
//        popMenu.setMouseTracking(true);
//        QAction* const choice = popMenu.exec(view->mapToGlobal(e->pos()));

//        if (choice)
//        {
//            if (choice == upAction)
//            {
//                //TODO: ADD THIS SLOT
//               // ShowFoto::instance()->slotUploadItems(lst);
//            }
//        }

//        return true;
//    }
/*
    TODO: Implement tag dropping in Showfoto tool.
    else if (DTagDrag::canDecode(e->mimeData()))
    {
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
    }
*/
    return false;
}

Qt::DropAction ShowfotoDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
//    if (Digikam::DItemDrag::canDecode(e->mimeData()) || KUrl::List::canDecode(e->mimeData()))
//    {
//        if (e->keyboardModifiers() & Qt::ControlModifier)
//        {
//            return Qt::CopyAction;
//        }
//        else if (e->keyboardModifiers() & Qt::ShiftModifier)
//        {
//            return Qt::MoveAction;
//        }

//        return Qt::MoveAction;
//    }

//    if (Digikam::DTagDrag::canDecode(e->mimeData())            ||
//        Digikam::DTagListDrag::canDecode(e->mimeData())        ||
//        Digikam::DCameraItemListDrag::canDecode(e->mimeData()) ||
//        Digikam::DCameraDragObject::canDecode(e->mimeData()))
//    {
//        return Qt::MoveAction;
//    }

    return Qt::IgnoreAction;
}

QStringList ShowfotoDragDropHandler::mimeTypes() const
{
//    QStringList mimeTypes;
//    mimeTypes << Digikam::DItemDrag::mimeTypes()
//              << Digikam::DTagDrag::mimeTypes()
//              << Digikam::DTagListDrag::mimeTypes()
//              << Digikam::DCameraItemListDrag::mimeTypes()
//              << Digikam::DCameraDragObject::mimeTypes()
//              << KUrl::List::mimeDataTypes();

//    return mimeTypes;//TODO
    return QStringList();
}

QMimeData* ShowfotoDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ShowfotoItemInfo> infos = model()->showfotoItemInfos(indexes);

    QStringList lst;

    foreach(ShowfotoItemInfo info, infos)
    {
        lst.append(info.folder + info.name);
    }

    if (lst.isEmpty())
    {
        return 0;
    }

//    return (new Digikam::DCameraItemListDrag(lst));
    return 0;
}

ShowfotoImageModel* ShowfotoDragDropHandler::model() const
{
    return static_cast<ShowfotoImageModel*>(m_model);
}

} // namespace ShowFoto

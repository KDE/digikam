#ifndef SHOWFOTODRAGDROP_H
#define SHOWFOTODRAGDROP_H

// KDE includes

#include <kmenu.h>

// Local includes

#include "abstractitemdragdrophandler.h"
#include "showfotoimagemodel.h"
#include "album.h"

class KJob;

namespace ShowFoto
{

class ShowfotoDragDropHandler : public AbstractItemDragDropHandler
{
    Q_OBJECT

public:

    explicit ShowfotoDragDropHandler(ShowfotoImageModel* const model);

    ShowfotoImageModel* model() const;

    virtual bool           dropEvent(QAbstractItemView* view, const QDropEvent* e, const QModelIndex& droppedOn);
    virtual Qt::DropAction accepts(const QDropEvent* e, const QModelIndex& dropIndex);
    virtual QStringList    mimeTypes() const;
    virtual QMimeData*     createMimeData(const QList<QModelIndex> &);

Q_SIGNALS:

    void dioResult(KJob*);

private:

    enum DropAction
    {
        NoAction,
        CopyAction,
        MoveAction,
        GroupAction,
        AssignTagAction
    };

private:

    QAction*   addGroupAction(KMenu* const menu);
    QAction*   addCancelAction(KMenu* const menu);
    DropAction copyOrMove(const QDropEvent* e, QWidget* const view, bool allowMove = true, bool askForGrouping = false);
};

} // namespace ShowFoto

#endif // SHOWFOTODRAGDROP_H

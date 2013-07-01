#ifndef TAGPROPWIDGET_H
#define TAGPROPWIDGET_H

#include "sidebarwidget.h"
#include <QWidget>

namespace Digikam
{

class TagPropWidget : public SidebarWidget
{
    Q_OBJECT

public:
    TagPropWidget(QWidget* const parent);
    void doLoadState();
    void doSaveState();
    void setActive(bool active);
    void applySettings();
    void changeAlbumFromHistory(Album *album);
    QString getCaption();
    QPixmap getIcon();
};

}

#endif //TAGPROPWIDGET

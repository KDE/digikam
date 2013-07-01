#include "tagpropwidget.h"
#include <QLabel>
#include <QVBoxLayout>

namespace Digikam
{


TagPropWidget::TagPropWidget(QWidget* const parent)
    : SidebarWidget(parent)
{
    QVBoxLayout* layout = new QVBoxLayout(this);

    layout->addWidget(new QLabel("Tag Properties"));
    layout->addWidget(new QLabel("Tag Properties"));
}

void TagPropWidget::doLoadState()
{
}

void TagPropWidget::doSaveState()
{
}

void TagPropWidget::setActive(bool active)
{
}

void TagPropWidget::applySettings()
{
}

void TagPropWidget::changeAlbumFromHistory(Album *album)
{
}

QString TagPropWidget::getCaption()
{
    return QString();
}

QPixmap TagPropWidget::getIcon()
{
    return QPixmap();
}

}

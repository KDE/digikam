#ifndef TAGPROPWIDGET_H
#define TAGPROPWIDGET_H

#include <QWidget>

namespace Digikam
{

class TagPropWidget : public QWidget
{
    Q_OBJECT

public:
    TagPropWidget(QWidget* const parent);

private:
    class PrivateTagProp;
    PrivateTagProp* d;
};

}

#endif //TAGPROPWIDGET

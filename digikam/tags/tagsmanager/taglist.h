#include <QWidget>

namespace Digikam
{

class TagModel;
class TagList : public QWidget
{
    Q_OBJECT

public:
    TagList(TagModel* model);
    ~TagList();

};

}
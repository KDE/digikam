#include <kdialog.h>

#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <kactionmenu.h>

class QTreeView;
class KMainWindow;
class KToolBar;
class KMultiTabBar;
class QHBoxLayout;
class TagPropWidget;

namespace Digikam
{


class TagsManager : public KDialog
{
    Q_OBJECT

public:
    TagsManager();
    ~TagsManager();

    void setupUi(KDialog *Dialog);


private Q_SLOTS:
    void slotOpenProperties();

private:
    class PrivateTagMngr;
    PrivateTagMngr* d;
};

}

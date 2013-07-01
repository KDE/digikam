#include <kdialog.h>

#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QListView>
#include <QtGui/QPushButton>
#include <QtGui/QToolButton>
#include <QTreeView>
#include <kactionmenu.h>
#include <sidebar.h>

class KMainWindow;
class KToolBar;

namespace Digikam
{


class TagsManager : public KDialog
{
    Q_OBJECT

public:
    TagsManager();
    ~TagsManager();

    void setupUi(KDialog *Dialog);

    QTreeView *treeModel;
    QLabel *tagmngrLabel;
    QLabel          *tagPixmap;
    QLabel          *digikamPixmap;
    QLineEdit       *lineEdit;

    KMainWindow     *treeWindow;
    KToolBar        *mainToolbar;
    //KToolBar        *rightToolBar;
    Sidebar*        rightSidebar;
    KActionMenu     *organizeAction;
    KActionMenu     *syncexportAction;
    KAction*        tagProperties;
    KAction*        addAction;
    KAction*        delAction;
    QListView *listView;
};

}

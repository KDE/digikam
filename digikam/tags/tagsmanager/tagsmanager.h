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

class TagsManager : public KDialog
{
    Q_OBJECT

public:
    TagsManager();
    ~TagsManager();

    void setupUi(KDialog *Dialog);

    QTreeView *treeModel;
    QLabel *tagmngrLabel;
    QLabel *tagPixmap;
    QLabel *digikamPixmap;
    QComboBox *organizeBox;
    QLineEdit *lineEdit;
    QComboBox *syncExportBox;
    QPushButton *pushButton;
    QListView *listView;
    QToolButton *addBttn;
    QToolButton *removeBttn;
    QWidget* tagProperties;
};

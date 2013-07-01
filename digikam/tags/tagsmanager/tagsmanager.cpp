#include "tagsmanager.h"
#include <kdebug.h>
#include <klocale.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <kaction.h>
#include <ktoolbar.h>
#include <kmainwindow.h>

#include "tagpropwidget.h"

namespace Digikam
{

TagsManager::TagsManager()
    : KDialog(0)
{
    //this->resize(800,600);

    /** No buttons **/
    this->setButtons(0x00);

    setupUi(this);
    //this->show();

    kDebug() << "My new Tags Manager class";
}

TagsManager::~TagsManager()
{
}

void TagsManager::setupUi(KDialog *Dialog)
{

     Dialog->resize(972, 722);
     Dialog->setWindowTitle(i18n("Tags Manager"));

     QVBoxLayout* mainLayout = new QVBoxLayout(Dialog);
     QHBoxLayout* firstLine = new QHBoxLayout(Dialog);

     tagPixmap = new QLabel();
     tagPixmap->setText("Tag Pixmap");
     tagPixmap->setMaximumWidth(40);
     tagPixmap->setPixmap(KIcon("tag").pixmap(30,30));

     lineEdit = new QLineEdit();
     lineEdit->setText(i18n("Search..."));

     lineEdit->setMaximumWidth(200);

     digikamPixmap = new QLabel();
     QPixmap dpix (KStandardDirs::locate("data", "digikam/about/top-left-digikam.png"));
     digikamPixmap->setPixmap(dpix.scaled(40,40,Qt::KeepAspectRatio));
     digikamPixmap->setMaximumHeight(40);
     digikamPixmap->setScaledContents(true);


     QHBoxLayout* tempLayout = new QHBoxLayout();
     tempLayout->setAlignment(Qt::AlignLeft);
     tempLayout->addWidget(tagPixmap);
     tempLayout->addWidget(lineEdit);
     firstLine->addLayout(tempLayout);
     firstLine->addWidget(digikamPixmap);

     tagmngrLabel = new QLabel(Dialog);
     tagmngrLabel->setObjectName(QString::fromUtf8("label"));
     tagmngrLabel->setText(i18n("Tags Manager"));
     tagmngrLabel->setAlignment(Qt::AlignCenter);
     QFont font2;
     font2.setPointSize(12);
     font2.setBold(true);
     font2.setWeight(75);
     tagmngrLabel->setFont(font2);
     tagmngrLabel->setAutoFillBackground(true);

     treeWindow = new KMainWindow();
     mainToolbar = new KToolBar(treeWindow);

     addAction = new KAction(i18n("Add"),this);
     delAction = new KAction(i18n("Del"),this);

     organizeAction   = new KActionMenu(i18n("Organize"),this);
     organizeAction->setDelayed(false);
     organizeAction->addAction(new KAction("New Tag",this));

     syncexportAction = new KActionMenu(i18n("Sync & Export"),this);

     tagProperties = new KAction(i18n("Tag Properties"),this);

     mainToolbar->addAction(addAction);
     mainToolbar->addAction(delAction);
     mainToolbar->addAction(organizeAction);
     mainToolbar->addAction(syncexportAction);
     //mainToolbar->addAction(tagProperties);
     treeWindow->addToolBar(mainToolbar);

     SidebarSplitter* splitter    = new SidebarSplitter;
     splitter->setFrameStyle( QFrame::NoFrame );
     splitter->setFrameShadow( QFrame::Plain );
     splitter->setFrameShape( QFrame::NoFrame );
     splitter->setOpaqueResize(false);

     rightSidebar = new Sidebar(treeWindow, splitter, KMultiTabBar::Right);
     rightSidebar->setObjectName("Digikam Left Sidebar");
     splitter->setParent(this);

     TagPropWidget* tagprop = new TagPropWidget(this);
     rightSidebar->appendTab(tagprop,dpix,"Test");
     //rightToolBar = new KToolBar(treeWindow);
     //rightToolBar->addAction(tagProperties);
     //treeWindow->addToolBar(Qt::RightToolBarArea,rightToolBar);

     QHBoxLayout* thirdLine = new QHBoxLayout(Dialog);
     listView = new QListView(Dialog);
     listView->setObjectName(QString::fromUtf8("listView"));
     //listView->setGeometry(QRect(0, 100, 221, 611));
     listView->setContextMenuPolicy(Qt::CustomContextMenu);
     listView->setMaximumWidth(300);

     treeModel = new QTreeView(Dialog);
     treeModel->setObjectName(QString::fromUtf8("treeModel"));


     QVBoxLayout* listLayout = new QVBoxLayout();
     listLayout->addWidget(tagmngrLabel);
     listLayout->addWidget(listView);

     treeWindow->setCentralWidget(treeModel);

     thirdLine->addLayout(listLayout);
     thirdLine->setStretchFactor(listLayout,3);
     thirdLine->addWidget(treeWindow);
     thirdLine->setStretchFactor(treeWindow,9);
     //thirdLine->addWidget(tagprop);
     thirdLine->addWidget(rightSidebar);

     mainLayout->addLayout(firstLine);
     mainLayout->addLayout(thirdLine);

     this->mainWidget()->setLayout(mainLayout);

}

} // namespace Digikam

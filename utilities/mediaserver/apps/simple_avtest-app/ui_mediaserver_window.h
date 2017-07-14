/********************************************************************************
** Form generated from reading UI file 'mediaserver_window.ui'
**
** Created by: Qt User Interface Compiler version 5.9.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MEDIASERVER_WINDOW_H
#define UI_MEDIASERVER_WINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MediaServerWindow
{
public:
    QWidget *centralwidget;
    QVBoxLayout *verticalLayout;
    QTableWidget *sharedItemsTable;
    QGridLayout *gridLayout;
    QCheckBox *watchCheckBox;
    QPushButton *addItemButton;
    QCheckBox *scanRecursivelyCheckbox;
    QPushButton *DeleteDirectoriesButton;
    QPushButton *addContentButton;

    void setupUi(QMainWindow *MediaServerWindow)
    {
        if (MediaServerWindow->objectName().isEmpty())
            MediaServerWindow->setObjectName(QStringLiteral("MediaServerWindow"));
        MediaServerWindow->resize(905, 382);
        MediaServerWindow->setMinimumSize(QSize(360, 170));
        centralwidget = new QWidget(MediaServerWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        verticalLayout = new QVBoxLayout(centralwidget);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        sharedItemsTable = new QTableWidget(centralwidget);
        if (sharedItemsTable->columnCount() < 3)
            sharedItemsTable->setColumnCount(3);
        QTableWidgetItem *__qtablewidgetitem = new QTableWidgetItem();
        sharedItemsTable->setHorizontalHeaderItem(0, __qtablewidgetitem);
        QTableWidgetItem *__qtablewidgetitem1 = new QTableWidgetItem();
        sharedItemsTable->setHorizontalHeaderItem(1, __qtablewidgetitem1);
        QTableWidgetItem *__qtablewidgetitem2 = new QTableWidgetItem();
        sharedItemsTable->setHorizontalHeaderItem(2, __qtablewidgetitem2);
        sharedItemsTable->setObjectName(QStringLiteral("sharedItemsTable"));
        sharedItemsTable->setAlternatingRowColors(true);
        sharedItemsTable->setSelectionMode(QAbstractItemView::MultiSelection);
        sharedItemsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
        sharedItemsTable->setSortingEnabled(true);
        sharedItemsTable->horizontalHeader()->setCascadingSectionResizes(true);
        sharedItemsTable->horizontalHeader()->setMinimumSectionSize(40);
        sharedItemsTable->horizontalHeader()->setStretchLastSection(true);
        sharedItemsTable->verticalHeader()->setVisible(false);

        verticalLayout->addWidget(sharedItemsTable);

        gridLayout = new QGridLayout();
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setSizeConstraint(QLayout::SetMinimumSize);
        watchCheckBox = new QCheckBox(centralwidget);
        watchCheckBox->setObjectName(QStringLiteral("watchCheckBox"));
        watchCheckBox->setEnabled(false);
        watchCheckBox->setLayoutDirection(Qt::RightToLeft);

        gridLayout->addWidget(watchCheckBox, 9, 3, 1, 1);

        addItemButton = new QPushButton(centralwidget);
        addItemButton->setObjectName(QStringLiteral("addItemButton"));

        gridLayout->addWidget(addItemButton, 9, 1, 1, 1);

        scanRecursivelyCheckbox = new QCheckBox(centralwidget);
        scanRecursivelyCheckbox->setObjectName(QStringLiteral("scanRecursivelyCheckbox"));
        scanRecursivelyCheckbox->setLayoutDirection(Qt::RightToLeft);
        scanRecursivelyCheckbox->setChecked(true);

        gridLayout->addWidget(scanRecursivelyCheckbox, 9, 4, 1, 1);

        DeleteDirectoriesButton = new QPushButton(centralwidget);
        DeleteDirectoriesButton->setObjectName(QStringLiteral("DeleteDirectoriesButton"));

        gridLayout->addWidget(DeleteDirectoriesButton, 9, 6, 1, 1);

        addContentButton = new QPushButton(centralwidget);
        addContentButton->setObjectName(QStringLiteral("addContentButton"));

        gridLayout->addWidget(addContentButton, 9, 5, 1, 1);


        verticalLayout->addLayout(gridLayout);

        MediaServerWindow->setCentralWidget(centralwidget);

        retranslateUi(MediaServerWindow);

        QMetaObject::connectSlotsByName(MediaServerWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MediaServerWindow)
    {
        MediaServerWindow->setWindowTitle(QApplication::translate("MediaServerWindow", "Simple Media Server Test", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem = sharedItemsTable->horizontalHeaderItem(0);
        ___qtablewidgetitem->setText(QApplication::translate("MediaServerWindow", "Recursive", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem1 = sharedItemsTable->horizontalHeaderItem(1);
        ___qtablewidgetitem1->setText(QApplication::translate("MediaServerWindow", "Monitored", Q_NULLPTR));
        QTableWidgetItem *___qtablewidgetitem2 = sharedItemsTable->horizontalHeaderItem(2);
        ___qtablewidgetitem2->setText(QApplication::translate("MediaServerWindow", "Path", Q_NULLPTR));
        watchCheckBox->setText(QApplication::translate("MediaServerWindow", "Watch for changes", Q_NULLPTR));
        addItemButton->setText(QApplication::translate("MediaServerWindow", "Add Items", Q_NULLPTR));
        scanRecursivelyCheckbox->setText(QApplication::translate("MediaServerWindow", "Scan recursively", Q_NULLPTR));
        DeleteDirectoriesButton->setText(QApplication::translate("MediaServerWindow", "Delete Selected", Q_NULLPTR));
        addContentButton->setText(QApplication::translate("MediaServerWindow", "&Add New Root Directory", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MediaServerWindow: public Ui_MediaServerWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MEDIASERVER_WINDOW_H

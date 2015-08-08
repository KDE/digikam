#include "trashview.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTableView>
#include <QPushButton>

#include "klocalizedstring.h"

namespace Digikam
{

TrashView::TrashView(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    QTableView* tableView = new QTableView(this);

    QPushButton* restoreButton = new QPushButton(i18n("Restore"));
    QPushButton* deleteButton = new QPushButton(i18n("Delete Permanently"));

    mainLayout->addWidget(tableView);
    mainLayout->addWidget(restoreButton);
    mainLayout->addWidget(deleteButton);

}

} // namespace Digikam

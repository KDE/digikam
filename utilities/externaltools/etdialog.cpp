
#include <QTimer>
#include <QMessageBox>
#include <QVBoxLayout>

#include <kcomponentdata.h>
#include <kprogressdialog.h>
#include <klocalizedstring.h>

#include <libkipi/imagecollection.h>
#include <libkipi/interface.h>

#include "etdialog.h"
#include "etrunner.h"


ETDialog::ETDialog(QWidget* parent, const QList<KUrl>& images, const QString& tool)
    : KDialog(parent)
    , m_images(images)
{
    setButtons(KDialog::Try | KDialog::Close);

    QWidget* main = mainWidget();

    if (!main->layout())
    {
        main->setLayout(new QVBoxLayout(mainWidget()));
    }

    main->layout()->setContentsMargins(0,0,0,0);

    m_etwidget = new ETWidget(main, tool);
    main->layout()->addWidget(m_etwidget);

    connect(this, SIGNAL(finished()), m_etwidget, SLOT(save()));
    connect(this, SIGNAL(tryClicked()), m_etwidget, SLOT(save()));

    restoreDialogSize(ETConfig::config()->cfg);

    if (!tool.isEmpty())
    {
        hide();
        slotButtonClicked(KDialog::Try);
        connect(this, SIGNAL(processFinished()), SLOT(accept()));
    }
}

ETDialog::~ETDialog()
{}

void ETDialog::slotButtonClicked(int button)
{
    saveDialogSize(ETConfig::config()->cfg);

    if (button == KDialog::Try)
    {
        ETRunner* runner = new ETRunner(this, m_etwidget->currentTool(), m_images);
        connect(runner, SIGNAL(started()), this, SLOT(onStarted()));
        connect(runner, SIGNAL(error(QString,QString)), this, SLOT(onError(QString,QString)));
        connect(runner, SIGNAL(finished()), this, SIGNAL(processFinished()));
        connect(runner, SIGNAL(finished()), runner, SLOT(deleteLater()));
        runner->run();
    }

    KDialog::slotButtonClicked(button);
}

void ETDialog::onError(const QString& title, const QString& text)
{
    QMessageBox::critical(this, title, text);
}

void ETDialog::onStarted()
{
    ETRunner* runner = qobject_cast<ETRunner*>(sender());

    KProgressDialog* progress = new KProgressDialog(this, i18n("Executing..."), runner->config()->cfg.readEntry(ETConfig::name, QString()));
    progress->setAttribute(Qt::WA_DeleteOnClose);
    progress->progressBar()->setMaximum(0);
    progress->progressBar()->setMinimum(0);
    progress->progressBar()->setValue(0);
    connect(progress, SIGNAL(rejected()), runner, SLOT(terminate()));
    connect(progress, SIGNAL(accepted()), progress, SLOT(deleteLater()));

    connect(runner, SIGNAL(finished()), progress, SLOT(accept()));
    progress->exec();
}



#include <iostream>

#include <QMessageBox>

#include <kcomponentdata.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kkeysequencewidget.h>
#include <krun.h>

#include "etwidget.h"

#include "ui_etwidget.h"

const QString ETConfig::firstStart    = "firstStart";
const QString ETConfig::showInContext = "showInContext";
const QString ETConfig::shortcut      = "shortcut";
const QString ETConfig::name          = "name";
const QString ETConfig::type          = "type";

const QString ETConfig::Type::Executable::type      = "executable";
const QString ETConfig::Type::Executable::path      = "path";
const QString ETConfig::Type::Executable::arguments = "arguments";

const QString ETConfig::Type::Script::type      = "script";
const QString ETConfig::Type::Script::path      = "path";
const QString ETConfig::Type::Script::script    = "script";

const QString ETConfig::Type::SimpleScript::type= "simplescript";
const QString ETConfig::Type::SimpleScript::path= "path";
const QString ETConfig::Type::SimpleScript::body= "body";

typedef ETConfig::Type::Executable   Exec;
typedef ETConfig::Type::Script       Script;
typedef ETConfig::Type::SimpleScript SimpleScript;

ETConfig::ETConfig()
    : main("externaltoolsrc")
{
    cfg = main.group(ETConfig::pluginName());
}

ETConfig::Ptr ETConfig::config(const QString& tool)
{
    ETConfig::Ptr cfg(new ETConfig());

    if (!tool.isEmpty())
    {
        //access to selected config group
        cfg->cfg = cfg->cfg.group(tool);
    }

    return cfg;
}

const QString ETConfig::pluginName()
{
    return i18n("External Tools");
}


struct ETWidget::Private
{
    QScopedPointer<Ui::etwidget> ui;
    QMap<QString, int> name2index;
};

ETWidget::ETWidget(QWidget* parent, const QString& tool)
    : QWidget(parent)
    , d(new Private())
{
    d->ui.reset(new Ui::etwidget());
    d->ui->setupUi(this);

    d->ui->add->setIcon(KIcon("list-add"));
    d->ui->remove->setIcon(KIcon("list-remove"));
    d->ui->scriptedit->setIcon(KIcon("accessories-text-editor"));    

    connect(d->ui->name, SIGNAL(currentIndexChanged(int)), SLOT(scriptSelected(int)));
    connect(d->ui->add, SIGNAL(clicked(bool)), SLOT(save()));
    connect(d->ui->remove, SIGNAL(clicked(bool)), SLOT(remove()));

    if (ETConfig::config()->readEntry<bool>(ETConfig::firstStart, true))
    {
        ETConfig::config()->writeEntry(ETConfig::firstStart, false);    
        
        ETConfig::Ptr newcfg = ETConfig::config("archive");
        newcfg->writeEntry(ETConfig::name, QString("/tmp/images.tar.gz"));
        newcfg->writeEntry(ETConfig::type, SimpleScript::type);
        newcfg->writeTypeEntry(SimpleScript::type, SimpleScript::path, QString("/bin/sh"));
        newcfg->writeTypeEntry(SimpleScript::type, SimpleScript::body, QString("exec tar -czf /tmp/images.tar.gz --transform='s,.*/,,' -T -"));
    }
    
    d->name2index[Exec::type]        = d->ui->types->indexOf(d->ui->types->findChild<QWidget*>(Exec::type));
    d->name2index[Script::type]      = d->ui->types->indexOf(d->ui->types->findChild<QWidget*>(Script::type));
    d->name2index[SimpleScript::type]= d->ui->types->indexOf(d->ui->types->findChild<QWidget*>(SimpleScript::type));
    
    d->ui->type->clear();
    connect(d->ui->type, SIGNAL(currentIndexChanged(int)), SLOT(typeSelected(int)));    
    d->ui->type->addItem(i18n("Executable"),    Exec::type);
    d->ui->type->addItem(i18n("Script"),        Script::type);
    d->ui->type->addItem(i18n("Simple Script"), SimpleScript::type);
    
    if (QPushButton* prototype = d->ui->scripturl->findChild<QPushButton*>())
    {
        d->ui->scriptedit->setFixedSize(prototype->size());
    }
    connect(d->ui->scriptedit, SIGNAL(clicked(bool)), SLOT(scriptEdit(bool)));
    
    update(tool);
}

ETWidget::~ETWidget()
{}

void ETWidget::update(const QString& tool)
{
    ETConfig::Ptr cfg = ETConfig::config(tool);
    
    d->ui->name->clear();

    foreach(const QString& group, cfg->cfg.groupList())
    {
        KConfigGroup toolcfg = cfg->cfg.group(group);
        d->ui->name->addItem(toolcfg.readEntry<QString>(ETConfig::name, QString()), group);

        if (group == tool)
        {
            d->ui->name->setCurrentIndex(d->ui->name->count() - 1);
        }
    }
}

void ETWidget::typeSelected(int index)
{
    d->ui->types->setCurrentIndex(d->name2index[d->ui->type->itemData(index).toString()]);

}

QString ETWidget::currentTool()
{
    if (d->ui->name->findText(d->ui->name->currentText()) == -1)
    {
        save();
    }

    return d->ui->name->itemData(d->ui->name->currentIndex()).toString();
}

void ETWidget::scriptSelected(int index)
{
    const QString script = d->ui->name->itemData(index).toString();
    ETConfig::Ptr cfg = ETConfig::config(script);

    const QString type = cfg->cfg.readEntry<QString>(ETConfig::type, Exec::type);
    d->ui->type->setCurrentIndex(d->ui->type->findData(type));
        
    d->ui->execurl->setText(cfg->readTypeEntry(Exec::type, Exec::path, QString()));
    d->ui->execargs->setText(cfg->readTypeEntry(Exec::type, Exec::arguments, QString()));
    
    d->ui->scriptinterpreter->setText(cfg->readTypeEntry(Script::type, Script::path, QString("/bin/sh")));
    d->ui->scripturl->setText(cfg->readTypeEntry(Script::type, Script::script, QString()));
    
    d->ui->simpleinterpreter->setText(cfg->readTypeEntry(SimpleScript::type, SimpleScript::path, QString("/bin/sh")));
    d->ui->simplescriptbody->setText(cfg->readTypeEntry(SimpleScript::type, SimpleScript::body, QString()));
    
    d->ui->context->setChecked(cfg->readEntry<bool>(ETConfig::showInContext, false));
    d->ui->shortcut->setKeySequence(cfg->readEntry<QString>(ETConfig::shortcut, QString()));  
}

void ETWidget::save()
{
    ETConfig::Ptr cfg;

    if (d->ui->name->findText(d->ui->name->currentText()) != -1)
    {
        cfg = ETConfig::config(d->ui->name->itemData(d->ui->name->currentIndex()).toString());
    }
    else
    {
        cfg = ETConfig::config(d->ui->name->currentText());
        d->ui->name->addItem(d->ui->name->currentText(), d->ui->name->currentText());
    }

    cfg->writeEntry(ETConfig::name, d->ui->name->currentText());
    cfg->writeEntry(ETConfig::type, d->ui->type->itemData(d->ui->type->currentIndex()).toString());

    cfg->writeEntry(ETConfig::showInContext, d->ui->context->isChecked());
    cfg->writeEntry(ETConfig::shortcut,      d->ui->shortcut->keySequence().toString());
    
    cfg->writeTypeEntry(Exec::type, Exec::path,      d->ui->execurl->text());
    cfg->writeTypeEntry(Exec::type, Exec::arguments, d->ui->execargs->text());
    
    cfg->writeTypeEntry(Script::type, Script::path,   d->ui->scriptinterpreter->text());
    cfg->writeTypeEntry(Script::type, Script::script, d->ui->scripturl->text());
    
    cfg->writeTypeEntry(SimpleScript::type, SimpleScript::path, d->ui->simpleinterpreter->text());
    cfg->writeTypeEntry(SimpleScript::type, SimpleScript::body, d->ui->simplescriptbody->toPlainText());

    cfg->cfg.sync();
    Q_EMIT configChanged();
}

void ETWidget::remove()
{
    if (d->ui->name->currentIndex() != -1)
    {
        QMessageBox::StandardButton answer =
            QMessageBox::question(this,
                                  i18n("Removing tool"),
                                  i18n("Are you sure"),
                                  QMessageBox::Ok | QMessageBox::Abort);

        if (answer != QMessageBox::Ok)
        {
            return;
        }

        ETConfig::Ptr cfg(ETConfig::config());
        cfg->cfg.deleteGroup(d->ui->name->itemData(d->ui->name->currentIndex()).toString());
        cfg->cfg.sync();
        d->ui->name->removeItem(d->ui->name->currentIndex());
    }
}

void ETWidget::scriptEdit(bool)
{
    KRun::runUrl(d->ui->scripturl->url(), "text/plain", this);
}

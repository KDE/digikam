
#include <iostream>

#include <QMessageBox>

#include <kcomponentdata.h>
#include <kdebug.h>
#include <klocalizedstring.h>
#include <kkeysequencewidget.h>

#include "etwidget.h"

#include "ui_etwidget.h"

const QString ETConfig::firstStart    = "firstStart";
const QString ETConfig::showInContext = "showInContext";
const QString ETConfig::shortcut      = "shortcut";
const QString ETConfig::name          = "name";
const QString ETConfig::interpretter  = "interpretter";
const QString ETConfig::script        = "scriptbody";

ETConfig::ETConfig()
    : main("externaltoolsrc")
    , cfg(main.group(ETConfig::pluginName()))
{}

ETConfig::Ptr ETConfig::config(const QString& tool)
{
    ETConfig::Ptr cfg(new ETConfig());

    if (!tool.isEmpty())
    {
        cfg->cfg = cfg->cfg.group(tool);
    }

    return cfg;
}


const QString ETConfig::pluginName()
{
    return i18n("External Tools");
}


ETWidget::ETWidget(QWidget* parent, const QString& tool)
    : QWidget(parent)
    , ui_(new Ui::etwidget())
{
    ui_->setupUi(this);

    ui_->add->setIcon(QIcon::fromTheme("list-add"));
    ui_->remove->setIcon(QIcon::fromTheme("list-remove"));

    connect(ui_->name, SIGNAL(currentIndexChanged(int)), SLOT(scriptSelected(int)));
    connect(ui_->add, SIGNAL(clicked(bool)), SLOT(save()));
    connect(ui_->remove, SIGNAL(clicked(bool)), SLOT(remove()));

    update(tool);
}

ETWidget::~ETWidget()
{}

void ETWidget::update(const QString& tool)
{
    ETConfig::Ptr cfg(ETConfig::config());

    ui_->name->clear();

    foreach(const QString& group, cfg->cfg.groupList())
    {
        KConfigGroup toolcfg = cfg->cfg.group(group);
        ui_->name->addItem(toolcfg.readEntry<QString>(ETConfig::name, QString()), group);

        if (group == tool)
        {
            ui_->name->setCurrentIndex(ui_->name->count() - 1);
        }
    }
}

QString ETWidget::currentTool()
{
    if (ui_->name->findText(ui_->name->currentText()) == -1)
    {
        save();
    }

    return ui_->name->itemData(ui_->name->currentIndex()).toString();
}

void ETWidget::scriptSelected(int index)
{
    const QString script = ui_->name->itemData(index).toString();
    ETConfig::Ptr cfg(ETConfig::config());
    cfg->cfg = cfg->cfg.group(script);

    ui_->interpret->setText(cfg->cfg.readEntry<QString>(ETConfig::interpretter, QString()));
    ui_->script->setPlainText(cfg->cfg.readEntry<QString>(ETConfig::script, QString()));
    ui_->context->setChecked(cfg->cfg.readEntry<bool>(ETConfig::showInContext, false));
    ui_->shortcut->setKeySequence(cfg->cfg.readEntry<QString>(ETConfig::shortcut, QString()));
}

void ETWidget::save()
{
    ETConfig::Ptr cfg(ETConfig::config());

    if (ui_->name->findText(ui_->name->currentText()) != -1)
    {
        cfg->cfg = cfg->cfg.group(ui_->name->itemData(ui_->name->currentIndex()).toString());
    }
    else
    {
        cfg->cfg = cfg->cfg.group(ui_->name->currentText());
        ui_->name->addItem(ui_->name->currentText(), ui_->name->currentText());
    }

    cfg->cfg.writeEntry(ETConfig::name, ui_->name->currentText());
    cfg->cfg.writeEntry(ETConfig::interpretter, ui_->interpret->text());
    cfg->cfg.writeEntry(ETConfig::script, ui_->script->toPlainText());
    cfg->cfg.writeEntry(ETConfig::showInContext, ui_->context->isChecked());
    cfg->cfg.writeEntry(ETConfig::shortcut, ui_->shortcut->keySequence().toString());

    cfg->cfg.sync();
}

void ETWidget::remove()
{
    ETConfig::Ptr cfg(ETConfig::config());

    if (ui_->name->currentIndex() != -1)
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

        cfg->cfg.deleteGroup(ui_->name->itemData(ui_->name->currentIndex()).toString());
        cfg->cfg.sync();
        ui_->name->removeItem(ui_->name->currentIndex());
    }
}


/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2014-10-05
 * Description : slideshow help dialog
 *
 * Copyright (C) 2014-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "slidehelp.h"

// Qt includes

#include <QLabel>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>

// KDE includes

#include <klocalizedstring.h>

namespace Digikam
{

SlideHelp::SlideHelp()
    : QDialog(0)
{
    setWindowTitle(i18n("Slideshow Usage"));

    QDialogButtonBox* const buttons = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    buttons->button(QDialogButtonBox::Ok)->setDefault(true);

    // -------------------------------------------------------------------------------------------------------------------

    QLabel* const label = new QLabel(this);
    label->setText(i18n("<qt>"
                        "<table>"
                                "<tr><td colspan=\"2\"><nobr><center><b>"
                                "<h1>Item Access</h1>"
                                "</b></center></nobr></td></tr>"

                                    "<tr><td>Previous Item:</td>"      "<td><i>Up</i> key</td></tr>"
                                    "<tr><td></td>"                    "<td><i>PgUp</i> key</td>"
                                    "<tr><td></td>"                    "<td><i>Left</i> key</td>"
                                    "<tr><td></td>"                    "<td>Mouse wheel up</td>"
                                    "<tr><td></td>"                    "<td>Left mouse button</td>"
                                    "<tr><td>Next Item:</td>"          "<td><i>Down</i> key</td></tr>"
                                    "<tr><td></td>"                    "<td><i>PgDown</i> key</td>"
                                    "<tr><td></td>"                    "<td><i>Right</i> key</td>"
                                    "<tr><td></td>"                    "<td>Mouse wheel down</td>"
                                    "<tr><td></td>"                    "<td>Right mouse button</td>"
                                    "<tr><td>Pause/Start:</td>"        "<td><i>Space</i> key</td></tr>"
                                    "<tr><td>Quit:</td>"               "<td><i>Esc</i> key<td></tr>"

                                "<tr><td colspan=\"2\"><nobr><center><b>"
                                "<h1>Item Properties</h1>"
                                "</b></center></nobr></td></tr>"

                                    "<tr><td>Change Tags:</td>"        "<td>Use Tags keyboard shortcuts</td></tr>"
                                    "<tr><td>Change Rating:</td>"      "<td>Use Rating keyboard shortcuts</td></tr>"
                                    "<tr><td>Change Color Label:</td>" "<td>Use Color label keyboard shortcuts</td></tr>"
                                    "<tr><td>Change Pick Label:</td>"  "<td>Use Pick label keyboard shortcuts</td></tr>"

                                "<tr><td colspan=\"2\"><nobr><center><b>"
                                "<h1>Others</h1>"
                                "</b></center></nobr></td></tr>"

                                    "<tr><td>Show this help:</td>"     "<td><i>F1</i> key</td></tr>"
                        "</table>"
                        "</qt>"));

    QVBoxLayout* const vbx = new QVBoxLayout(this);
    vbx->addWidget(label);
    vbx->addWidget(buttons);
    setLayout(vbx);

    // ---------------------------------------------------------------------------------------------------------------------

    connect(buttons->button(QDialogButtonBox::Ok), SIGNAL(clicked()),
            this, SLOT(accept()));

    adjustSize();
}

SlideHelp::~SlideHelp()
{
}

} // namespace Digikam

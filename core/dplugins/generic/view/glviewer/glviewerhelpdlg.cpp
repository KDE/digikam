/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2012-04-04
 * Description : a tool to show image using an OpenGL interface.
 *
 * Copyright (C) 2012-2016 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "glviewerhelpdlg.h"

// Qt includes

#include <QTextBrowser>
#include <QPushButton>
#include <QVBoxLayout>

// KDE includes

#include <klocalizedstring.h>

namespace GenericDigikamGLViewerPlugin
{

GLViewerHelpDlg::GLViewerHelpDlg(DPlugin* const plugin)
    : DPluginDialog(0, QLatin1String("GLViewerPluginHelpDlg"))
{
    setPlugin(plugin);
    setWindowIcon(plugin->icon());
    setWindowTitle(i18n("Usage of OpenGL Image Viewer"));

    m_buttons->addButton(QDialogButtonBox::Close);

    connect(m_buttons->button(QDialogButtonBox::Close), SIGNAL(clicked()),
            this, SLOT(close()));

    QTextBrowser* const brw = new QTextBrowser(this);
    QVBoxLayout* const vbx  = new QVBoxLayout(this);
    vbx->addWidget(brw);
    vbx->addWidget(m_buttons);
    setLayout(vbx);

    brw->setHtml(i18n(
        "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\""
        "\"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
        "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
        "p, li { white-space: pre-wrap; }\n"
        "</style></head><body style=\" font-family:'Sans Serif'; font-size:10pt; "
        "font-weight:400; font-style:normal;\">\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
        "-qt-block-indent:0; text-indent:0px;\"><span style=\" font-size:x-large; "
        "font-weight:600; color:#5500ff;\">Image Access</span><br /></p>\n"
        "<table border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\">\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">next image  </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">scrollwheel down/down arrow"
                            "/right arrow/PgDown/Space/n   </p></td></tr>\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">previous image   </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">scrollwheel up/up arrow/left arrow/PgUp/p   </p></td></tr>\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">quit  </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">Esc    </p></td></tr></table>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"></p>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-weight:600;"
                            "\"><span style=\" font-size:x-large; color:#5500ff;\">Display</span></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600; color:#5500ff;\"></p>\n"
        "<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600; color:#5500ff;\"></p>\n"
        "<table border=\"0\" style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px;\" cellspacing=\"2\" cellpadding=\"0\">\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">toggle fullscreen/normal   </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">f    </p></td></tr>\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:"
                            "0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">toggle scrollwheel action  </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">c (either zoom or change image)   </p></td></tr>\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">rotation   </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">r    </p></td></tr>\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">reset view   </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">double click    </p></td></tr>\n"
        "<tr>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-l"
                            "eft:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">original size  </p></td>\n"
        "<td>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">o   </p></td></tr></table>\n"
        "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><br /><span style=\" font-size:x-large; font-weight:600; color:#5500ff;\">Zooming</span></p>\n"
        "<ul style=\"-qt-list-indent: 1;\"><li style=\" margin-top:12px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">move mouse in up/down-direction while pressing the right mouse button</li>\n"
        "<li style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">alternatively, press c and use the scrollwheel<br /></li>\n"
        "<li style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">plus/minu"
                            "s</li>\n"
        "<li style=\" margin-top:0px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">ctrl + scrollwheel</li></ul>\n"
        "<p style=\" margin-top:0px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; font-size:x-large; font-weight:600; color:#5500ff;\"><span style=\" font-size:x-large;\">Panning</span></p>\n"
        "<ul style=\"-qt-list-indent: 1;\"><li style=\" margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">move mouse while pressing the left button</li></ul></body></html>",
    0));

    brw->setProperty("text", QVariant(i18n(
        "<b><font color=\"#5500ff\">"
        "<font size=\"+2\">Image Access</font></font></b><br>\n"
        "<TABLE> \n"
        " <TR> \n"
        " <TD>next image</TD> \n"
        " <TD>scrollwheel down/down arrow/right arrow/PgDown/Space/n</TD>\n"
        " </TR> \n"
        " <TR> \n"
        " <TD>previous image </TD> \n"
        " <TD>scrollwheel up/up arrow/left arrow/PgUp/p </TD> \n"
        " </TR>\n"
        "<TR> \n"
        " <TD>quit</TD> \n"
        " <TD>Esc</TD> \n"
        " </TR> \n"
        " </TABLE>\n"
        "<br>\n"
        " <TH><b><font color=\"#5500ff\"><font size=\"+2\">Display</font></font></b></TH> </br>\n"
        "<TABLE> \n"
        " <TR> \n"
        " <TD>toggle fullscreen/normal </TD> \n"
        " <TD>f</TD> \n"
        " </TR> \n"
        " <TR> \n"
        " <TD>toggle scrollwheel action</TD> \n"
        " <TD>c (either zoom or change image)</TD> \n"
        " </TR>\n"
        " <TR> \n"
        " <TD>rotation </TD> \n"
        " <TD>r</TD> \n"
        " </TR> \n"
        " <TR> \n"
        " <TD>reset view </TD> \n"
        " <TD>double click</TD> \n"
        " </TR> \n"
        " <TR> \n"
        " <TD>original size</TD> \n"
        " <TD>o</TD> \n"
        " </TR>\n"
        " </TABLE>\n"
        "<br>\n"
        "\n"
        "\n"
        "<b><font color=\"#5500ff\" size=\"+2\">Zooming</font></b><br> \n"
        "<UL>\n"
        ""
        "<LI>move mouse in up/down-direction while pressing the right mouse button\n"
        "<LI>alternatively, press c and use the scrollwheel<br>\n"
        "<LI>plus/minus\n"
        "<LI>ctrl + scrollwheel\n"
        "</UL>\n"
        "\n"
        "<b><font color=\"#5500ff\" size=\"+2\">Panning</font></b><br>\n"
        "<UL> \n"
        "<LI>move mouse while pressing the left button\n"
        "</UL>",
    0)));

    resize(700, 550);
}

GLViewerHelpDlg::~GLViewerHelpDlg()
{
}

} // namespace GenericDigikamGLViewerPlugin

/*
 * Grace - GRaphing, Advanced Computation and Exploration of data
 * 
 * Home page: http://plasma-gate.weizmann.ac.il/Grace/
 * 
 * Copyright (c) 1991-1995 Paul J Turner, Portland, OR
 * Copyright (c) 1996-2003 Grace Development Team
 * 
 * Maintained by Evgeny Stambulchik
 * 
 * 
 *                           All Rights Reserved
 * 
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 * 
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 * 
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "mainwindow.h"
#include <QCloseEvent>
#include <QSettings>

extern "C" {
#include <globals.h>
#include <utils.h>
}

MainWindow::MainWindow(QMainWindow *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.scrollArea->setWidget(ui.canvasWidget);

    QSettings settings("GraceProject", "Grace");
    restoreGeometry(settings.value("geometry").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings("GraceProject", "Grace");
    settings.setValue("geometry", saveGeometry());
    bailout(gapp);
}


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

#include "fileselectiondialog.h"
#include <QCloseEvent>

FileSelectionDialog::FileSelectionDialog(QWidget *parent) 
    : QDialog(parent)
{
    ui.setupUi(this);

    dirModel = new QFileSystemModel(this);
    //The call to setRootPath() tell the model which drive on the file system
    //to expose to the views.
    dirModel->setRootPath(QDir::currentPath());
    dirModel->setFilter(QDir::AllDirs);
    
    ui.dirListView->setModel(dirModel);
    //We filter the data supplied by the model by calling the setRootIndex()
    //function on each view, passing a suitable model index from the file
    //system model for the current directory
    ui.dirListView->setRootIndex(dirModel->index(QDir::currentPath()));

    connect(ui.dirListView, SIGNAL(doubleClicked(const QModelIndex)),
            this, SLOT(dirDoubleClicked(const QModelIndex)));

    fileModel = new QFileSystemModel(this);
    fileModel->setRootPath(QDir::currentPath());
    //TODO: http://bugreports.qt.nokia.com/browse/QTBUG-9811
    // maybe they are related http://bugreports.qt.nokia.com/browse/QTBUG-8632
    fileModel->setFilter(QDir::Files);
    
    ui.filesListView->setModel(fileModel);
    ui.filesListView->setRootIndex(fileModel->index(QDir::currentPath()));

}

void FileSelectionDialog::closeEvent(QCloseEvent *event)
{
    //event->ignore();
}

void FileSelectionDialog::dirDoubleClicked(const QModelIndex index)
{
    QString path = dirModel->filePath(index);
    path = QDir::cleanPath(path);
    QByteArray ba = path.toLatin1();
    qDebug(ba.data());
    dirModel->setRootPath(path);
    ui.dirListView->setRootIndex(dirModel->index(path));

    fileModel->setRootPath(path);
    ui.filesListView->setRootIndex(fileModel->index(path));
}

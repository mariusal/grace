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
extern "C" {
  #include <globals.h>
  #include "utils.h"
}

FileSelectionDialog::FileSelectionDialog(QWidget *parent) 
    : QDialog(parent)
{
    ui.setupUi(this);

    dirModel = new QFileSystemModel(this);
    fileModel = new QFileSystemModel(this);
    //The call to setRootPath() tell the model which drive on the file system
    //to expose to the views.
    dirModel->setRootPath(QDir::currentPath());
    fileModel->setRootPath(QDir::currentPath());

    showHidden(false);
    
    ui.dirListView->setModel(dirModel);
    ui.filesListView->setModel(fileModel);
    //We filter the data supplied by the model by calling the setRootIndex()
    //function on each view, passing a suitable model index from the file
    //system model for the current directory
    ui.dirListView->setRootIndex(dirModel->index(QDir::currentPath()));
    ui.filesListView->setRootIndex(fileModel->index(QDir::currentPath()));

    connect(ui.dirListView, SIGNAL(doubleClicked(const QModelIndex)),
            this, SLOT(dirDoubleClicked(const QModelIndex)));

    connect(ui.filesListView, SIGNAL(clicked(const QModelIndex)),
            this, SLOT(fileClicked(const QModelIndex)));

    connect(ui.showHiddenFilesCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showHidden(bool)));
   
    ui.chDirComboBox->addItem("Cwd");
    ui.chDirComboBox->addItem("Home");
    ui.chDirComboBox->addItem("/");
#ifdef __WIN32
    ui.chDirComboBox->addItem("My Computer");
#endif

    connect(ui.chDirComboBox, SIGNAL(activated(int)),
            this, SLOT(cdToDir(int)));

    connect(ui.setAsCwdPushButton, SIGNAL(clicked()),
            this, SLOT(setAsCwd()));
}

char* qstring_to_char(QString s)
{
    QByteArray *ba = new QByteArray(s.toLatin1());
    return ba->data();
}

void FileSelectionDialog::dirDoubleClicked(const QModelIndex index)
{
    setDirectory(dirModel->filePath(index));
}

void FileSelectionDialog::setDirectory(QString dir)
{
    dir = QDir::cleanPath(dir);

    QByteArray ba = dir.toLatin1();
    qDebug(ba.data());

    dirModel->setRootPath(dir);
    ui.dirListView->setRootIndex(dirModel->index(dir));

    fileModel->setRootPath(dir);
    ui.filesListView->setRootIndex(fileModel->index(dir));
    
    reapplyFilter();

    ui.selectioLineEdit->setText(dir);
}

void FileSelectionDialog::reapplyFilter()
{
    //TODO: this function exists only to fix a buggy behavior
    // http://bugreports.qt.nokia.com/browse/QTBUG-9811
    // maybe they are related http://bugreports.qt.nokia.com/browse/QTBUG-8632
    QFlags<QDir::Filter> dirFilter = dirModel->filter();
    QFlags<QDir::Filter> fileFilter = fileModel->filter();

    dirModel->setFilter(QDir::System);
    fileModel->setFilter(QDir::System);

    dirModel->setFilter(dirFilter);
    fileModel->setFilter(fileFilter);
}

void FileSelectionDialog::fileClicked(const QModelIndex index)
{
    QString dir = fileModel->filePath(index);
    ui.selectioLineEdit->setText(dir);
}

void FileSelectionDialog::showHidden(bool onoff)
{
    if (onoff) {
       qDebug("show hidden");
       dirModel->setFilter(QDir::AllDirs | QDir::Hidden | QDir::System);
       fileModel->setFilter(QDir::Files | QDir::Hidden | QDir::System);
    } else {
       qDebug("hide hidden");
       dirModel->setFilter(QDir::AllDirs);
       fileModel->setFilter(QDir::Files);
    }
}

#define FSB_CWD     0
#define FSB_HOME    1
#define FSB_ROOT    2
#define FSB_CYGDRV  3

void FileSelectionDialog::cdToDir(int value)
{
    char *bufp;
    bool show_drives = false;

    switch (value) {
    case FSB_CWD:
        bufp = get_workingdir(gapp);
        break;
    case FSB_HOME:
#ifdef __WIN32
        bufp = qstring_to_char(QDir::homePath());
#else
        bufp = grace_get_userhome(gapp->grace);
#endif
        break;
    case FSB_ROOT:
#ifdef __WIN32
        bufp = qstring_to_char(QDir::rootPath());
#else
        bufp = "/";
#endif
        break;
    case FSB_CYGDRV:
#ifndef __CYGWIN__
        show_drives = true;
#else
        bufp = "/cygdrive/";
#endif
        break;
    default:
        return;
    }

    if (show_drives) {
        showDrives();
    } else {
        setDirectory(bufp);
    }
}

void FileSelectionDialog::showDrives()
{
    setDirectory("");

    //ui.dirListView->setRootIndex(QModelIndex());
    // TODO: show empty view
    //ui.filesListView->setRootIndex(QModelIndex());
}

void FileSelectionDialog::setAsCwd()
{
    char *bufp;

    QString dir = dirModel->filePath(ui.dirListView->rootIndex());
    dir = QDir::cleanPath(dir);
    bufp = qstring_to_char(dir);
    qDebug(bufp);
    set_workingdir(gapp, bufp);
}


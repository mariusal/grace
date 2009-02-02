#include <QApplication>
#include <QAbstractButton>
#include <QMessageBox>

#include "mainwindow.h"

extern "C" {
  #include "qtgrace.h"
}

int main_cpp(int argc, char *argv[], GraceApp *gapp)
{
  QApplication app(argc, argv);
  MainWindow mainWin(gapp);
  mainWin.show();
  return app.exec();
}

void startup_qt_gui(GraceApp *gapp)
{
  char *ch[8] = {"qtgrace"};
  main_cpp(1, ch, gapp);
  exit(0);
}

int yesnowin(char *msg, char *s1, char *s2, char *help_anchor)
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Grace: Warning");
    msgBox.setIcon(QMessageBox::Question);
    if (msg != NULL) {
	msgBox.setText(msg);
    } else {
	msgBox.setText("Warning");
    }

    if (help_anchor) {
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel | QMessageBox::Help);
    } else {    
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    }

    if (s1) {
    	msgBox.button(QMessageBox::Ok)->setText(s1);
    }    

    if (s2) {
    	msgBox.button(QMessageBox::Cancel)->setText(s2);
    }

    msgBox.setDefaultButton(QMessageBox::Cancel);
    msgBox.setEscapeButton(QMessageBox::Cancel);
    int yesno_retval = FALSE;
    int ret = msgBox.exec();
    switch (ret) {
	case QMessageBox::Ok:
	    yesno_retval = TRUE;
	    break;
	case QMessageBox::Cancel:
	    yesno_retval = FALSE;
	    break;
	case QMessageBox::Help:
	    // HelpCB(help_anchor); // TODO: implement help viewer
	    // help was clicked
	    break;
	default:
	    // should never be reached
	    break;
    }

    return yesno_retval;
}


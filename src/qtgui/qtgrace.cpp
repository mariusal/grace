#include <QApplication>

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


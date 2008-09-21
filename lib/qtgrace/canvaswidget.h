#include <QWidget>

class CanvasWidget : public QWidget
{
   Q_OBJECT

public:
  CanvasWidget(QWidget *parent = 0);

private:
  void paintEvent(QPaintEvent *event);

};


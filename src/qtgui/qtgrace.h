#ifndef __QTGRACE_H_
#define __QTGRACE_H_

#include <QObject>
extern "C" {
#include <motifinc.h>
}

typedef struct {
    Widget but;
    Button_CBProc cbproc;
    void *anydata;
} Button_CBdata;

class Callback_Data : public QObject
{
    Q_OBJECT

public:
    Button_CBdata *cbdata;
};

class ButtonCallback : public QObject
{
    Q_OBJECT

public slots:
    void buttonClicked(QObject* data) {
        Button_CBdata *cbdata = (Button_CBdata *) ((Callback_Data*)data)->cbdata;
        cbdata->cbproc(cbdata->but, cbdata->anydata);
    };
};

#endif /* __QTGRACE_H_ */
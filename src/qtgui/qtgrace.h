#ifndef __QTGRACE_H_
#define __QTGRACE_H_

#include <QObject>
extern "C" {
#include <config.h>
#include <motifinc.h>
}

class CallBack : public QObject
{
    Q_OBJECT

public:
    Widget sender;
    void (*callback)(Widget, XtPointer, XtPointer);
    void *data;

    void setCallBack(const QObject *sender,
                     void (*callback)(Widget, XtPointer, XtPointer),
                     void *data) {
        this->sender = (Widget) sender;
        this->callback = callback;
        this->data = data;
    };

public slots:
    void callBack() {
        callback(sender, (XtPointer) data, NULL);
    };
};

#endif /* __QTGRACE_H_ */


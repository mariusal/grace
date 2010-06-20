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
    void (*callback)(Widget, XtPointer, XtPointer);
    void *data;

    void setCallBack(void (*callback)(Widget, XtPointer, XtPointer),
                     void *data) {
        this->callback = callback;
        this->data = data;
    };

public slots:
    void callBack() {
        callback(NULL, (XtPointer) data, NULL);
    };
};

#endif /* __QTGRACE_H_ */


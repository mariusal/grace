#include <QPainter>
#include <QFont>

#include "mainwindow.h"
#include <QCloseEvent>
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>
#include <QBitmap>
#include <QScrollBar>
extern "C" {
#include <utils.h>
#include <files.h>
#include <bitmaps.h>
#include <core_utils.h>
#include <xprotos.h>
}

MainWindow::MainWindow(GraceApp *gapp, QMainWindow *parent) : QMainWindow(parent)
{
    ui.setupUi(this);

    ui.scrollArea->setWidget(ui.widget);
    this->gapp = gapp;
    canvasWidget = ui.widget;
    canvasWidget->setStatic();
    canvasWidget->setMainWindow(this);

    setToolBarIcons();

    QSettings settings("GraceProject", "Grace");
    restoreGeometry(settings.value("geometry").toByteArray());
}

MainWindow::~MainWindow()
{
}

void MainWindow::setToolBarIcons()
{
    ui.actionRedraw->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), redraw_bits, QImage::Format_MonoLSB)));

    ui.actionZoom->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), zoom_bits, QImage::Format_MonoLSB)));
    ui.actionZoomX->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), zoom_x_bits, QImage::Format_MonoLSB)));
    ui.actionZoomY->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), zoom_y_bits, QImage::Format_MonoLSB)));

    ui.actionAutoScale->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), auto_bits, QImage::Format_MonoLSB)));
    ui.actionAutoScaleX->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), auto_x_bits, QImage::Format_MonoLSB)));
    ui.actionAutoScaleY->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), auto_y_bits, QImage::Format_MonoLSB)));
    ui.actionAutoTick->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), auto_tick_bits, QImage::Format_MonoLSB)));

    ui.actionScrollLeft->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), left_bits, QImage::Format_MonoLSB)));
    ui.actionScrollRight->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), right_bits, QImage::Format_MonoLSB)));
    ui.actionScrollUp->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), up_bits, QImage::Format_MonoLSB)));
    ui.actionScrollDown->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), down_bits, QImage::Format_MonoLSB)));

    ui.actionZoomIn->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), expand_bits, QImage::Format_MonoLSB)));
    ui.actionZoomOut->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), shrink_bits, QImage::Format_MonoLSB)));

    ui.actionAddText->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), atext_bits, QImage::Format_MonoLSB)));

    ui.actionExit->setIcon(QIcon(QBitmap::fromData(QSize(16, 16), exit_bits, QImage::Format_MonoLSB)));
}

void MainWindow::on_actionNew_triggered()
{
    new_project(gapp, NULL);
    xdrawgraph(gapp->gp);
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("Open project"),
                                                    "",
                                                    tr("Grace Files (*.xgr *.agr)")
                                                   );
    if (!fileName.isEmpty()) {
        if (load_project(gapp, fileName.toUtf8().data()) == RETURN_SUCCESS) {
            xdrawgraph(gapp->gp);
            update_all();
            //statusBar()->showMessage(tr("File loaded"), 2000);
        } else {
            //statusBar()->showMessage(tr("File failed to load"), 2000);
        }
    }
}

void MainWindow::on_actionSave_triggered()
{
    if (gproject_get_docname(gapp->gp)) {
        //set_wait_cursor();

        save_project(gapp->gp, gproject_get_docname(gapp->gp));
        update_all();

        //unset_wait_cursor();
    } else {
        on_actionSaveAs_triggered();
    }
}

void MainWindow::on_actionSaveAs_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save project"),
                                                    "",
                                                    tr("Grace File (*.xgr)")
                                                   );
    if (!fileName.isEmpty()) {
        if (save_project(gapp->gp, fileName.toUtf8().data()) == RETURN_SUCCESS) {
            update_all();
            //return TRUE;
        } else {
            //return FALSE;
        }
    }
}

void MainWindow::on_actionRevertToSaved_triggered()
{
    char *docname;

    //    set_wait_cursor();
    docname = gproject_get_docname(gapp->gp);
    if (docname) {
        load_project(gapp, docname);
    } else {
        new_project(gapp, NULL);
    }
    xdrawgraph(gapp->gp);
    //    unset_wait_cursor();
}

void MainWindow::on_actionZoom_triggered()
{
    canvasWidget->actionZoom();
}

void MainWindow::on_actionZoomX_triggered()
{
    canvasWidget->actionZoomX();
}

void MainWindow::on_actionZoomY_triggered()
{
    canvasWidget->actionZoomY();
}

void MainWindow::autoscale_proc(int type)
{
    Quark *cg = graph_get_current(gproject_get_top(gapp->gp));

    if (autoscale_graph(cg, type) == RETURN_SUCCESS) {
        snapshot_and_update(gapp->gp, TRUE);
    } else {
        errmsg("Can't autoscale (no active sets?)");
    }
}

void MainWindow::on_actionAutoScale_triggered()
{
    autoscale_proc(AUTOSCALE_XY);
}

void MainWindow::on_actionAutoScaleX_triggered()
{
    autoscale_proc(AUTOSCALE_X);
}

void MainWindow::on_actionAutoScaleY_triggered()
{
    autoscale_proc(AUTOSCALE_Y);
}

/*
 * service the autoticks button on the main panel
 */
void MainWindow::on_actionAutoTick_triggered()
{
    autotick_graph_axes(graph_get_current(gproject_get_top(gapp->gp)), AXIS_MASK_XY);
    snapshot_and_update(gapp->gp, TRUE);
}

void MainWindow::page_zoom_inout(int inout)
{
    if (!gui_is_page_free(gapp->gui)) {
        if (inout > 0) {
            gapp->gui->zoom *= ZOOM_STEP;
        } else
        if (inout < 0) {
            gapp->gui->zoom /= ZOOM_STEP;
        } else {
            gapp->gui->zoom = 1.0;
        }
        xdrawgraph(gapp->gp);
        set_left_footer(NULL);
    }
}

void MainWindow::on_actionSmaller_triggered()
{
    page_zoom_inout(-1);
}

void MainWindow::on_actionLarger_triggered()
{
    page_zoom_inout(+1);
}

void MainWindow::on_actionOriginalSize_triggered()
{
    page_zoom_inout(0);
}

void MainWindow::on_actionRedraw_triggered()
{
    xdrawgraph(gapp->gp);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if ((gapp->gp && !quark_dirtystate_get(gproject_get_top(gapp->gp)))
            || yesno("Exit losing unsaved changes?", NULL, NULL, NULL)) {
        gapp_free(gapp);
        QSettings settings("GraceProject", "Grace");
        settings.setValue("geometry", saveGeometry());
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

int scroll_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_scroll(q, *type);
    }

    return TRUE;
}

void MainWindow::graph_scroll_proc(int type)
{
    Quark *cg, *f;

    cg = graph_get_current(gproject_get_top(gapp->gp));
    f = get_parent_frame(cg);

    quark_traverse(f, scroll_hook, &type);

    snapshot_and_update(gapp->gp, TRUE);
}

void MainWindow::on_actionScrollLeft_triggered()
{
    graph_scroll_proc(GSCROLL_LEFT);
}

void MainWindow::on_actionScrollRight_triggered()
{
    graph_scroll_proc(GSCROLL_RIGHT);
}

void MainWindow::on_actionScrollUp_triggered()
{
    graph_scroll_proc(GSCROLL_UP);
}

void MainWindow::on_actionScrollDown_triggered()
{
    graph_scroll_proc(GSCROLL_DOWN);
}

int zoom_hook(Quark *q, void *udata, QTraverseClosure *closure)
{
    if (quark_fid_get(q) == QFlavorGraph) {
        int *type = (int *) udata;
        closure->descend = FALSE;
        graph_zoom(q, *type);
    }

    return TRUE;
}

void MainWindow::graph_zoom_proc(int type)
{
    Quark *cg, *f;

    cg = graph_get_current(gproject_get_top(gapp->gp));
    f = get_parent_frame(cg);

    quark_traverse(f, zoom_hook, &type);

    snapshot_and_update(gapp->gp, TRUE);
}

void MainWindow::on_actionZoomIn_triggered()
{
    graph_zoom_proc(GZOOM_EXPAND);
}

void MainWindow::on_actionZoomOut_triggered()
{
    graph_zoom_proc(GZOOM_SHRINK);
}

void MainWindow::on_actionAddText_triggered() {
    //set_cursor(gapp->gui, 2);
    canvasWidget->actionAddText();
    set_left_footer("Select an anchor point");
}

void MainWindow::on_actionPreferences_triggered()
{
    create_props_frame(0, 0);
}

static void scroll_bar_pix(QScrollBar *bar, int pix)
{
    int value = bar->value();
    int maxvalue = bar->maximum();
    int slider_size = bar->pageStep();

    value += pix;
    if (value < 0) {
        value = 0;
    } else
    if (value > maxvalue + slider_size) {
        value = maxvalue + slider_size;
    }
    bar->setValue(value);
}

void MainWindow::scroll_pix(int dx, int dy)
{
    if (dx) {
        scroll_bar_pix(ui.scrollArea->horizontalScrollBar(), dx);
    }
    if (dy) {
        scroll_bar_pix(ui.scrollArea->verticalScrollBar(), dy);
    }
}


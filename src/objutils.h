#ifndef __OBJUTILS_H_
#define __OBJUTILS_H_

typedef struct {
    double x;
    double y;
} APoint;

/* Object types */
typedef enum {
    DO_LINE,
    DO_BOX,
    DO_ARC,
    DO_STRING
} OType;

typedef struct _DObject {
    int active;

    int gno;

    int loctype;
    APoint ap;
    
    VPoint offset;
    double angle;
    
    Pen pen;
    int lines;
    double linew;
    Pen fillpen;

    OType type;     /* object type */
    void *odata;
    
    view bb;        /* BBox (calculated at run-time) */
} DObject;

typedef struct _DOLineData {
    double length;
    int arrow_end;
    Arrow arrow;
} DOLineData;

typedef struct _DOBoxData {
    double width;
    double height;
} DOBoxData;

typedef struct _DOArcData {
    double width;
    double height;
    
    double angle1;
    double angle2;
    
    int fillmode;
} DOArcData;

typedef struct _DOStringData {
    char *s;
    int font;
    double size;
    int just;
} DOStringData;

DObject *object_get(int id);
void object_free(DObject *o);

void do_clear_objects(void);

int isactive_object(DObject *o);

void copy_object(int type, int from, int to);
int kill_object(int id);
int next_object(OType type);
int duplicate_object(int type, int id);

int get_object_bb(DObject *o, view *bb);
void move_object(int type, int id, VVector shift);

void init_string(int id, VPoint vp);
void init_line(int id, VPoint vp1, VPoint vp2);
void init_box(int id, VPoint vp1, VPoint vp2);
void init_ellipse(int id, VPoint vp1, VPoint vp2);

void do_clear_lines(void);
void do_clear_boxes(void);
void do_clear_text(void);
void do_clear_ellipses(void);

void set_default_string(plotstr * s);
void set_default_arrow(Arrow *arrowp);

#endif /* __OBJUTILS_H_ */

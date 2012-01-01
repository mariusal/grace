extern "C" {
#include <config.h>

typedef struct QWidget QWidget;
#include "../src/motifinc.h"
}

#include <gtest/gtest.h>


static int return_true(char **value, int *length, void *data) {
    return TRUE;
}

static int return_false(char **value, int *length, void *data) {
    return FALSE;
}

TEST(GUITest, TextValidator) {
    TextStructure *cstext = CreateCSText(NULL, "");
    AddTextValidateCB(cstext->text, return_true, NULL);

    SetTextString(cstext, "A String");
    char *s = GetTextString(cstext);

    EXPECT_STREQ("A String", s);

    xfree(s);
}


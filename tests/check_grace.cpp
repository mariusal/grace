extern "C" {
#include <grace/grace.h>
}

#include <gtest/gtest.h>

void errmsg(const char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
}

TEST(ColorsMergeTest, NullArgumentsAreHandledCorrectly) {
    ASSERT_FALSE(colorlists_are_equal(NULL, NULL));
}

TEST(ColorsMergeTest, EmptyColorListsAreNotEqual) {
    ColorList clist1 = {NULL, 0};
    ColorList clist2 = {NULL, 0};

    ASSERT_FALSE(colorlists_are_equal(&clist1, &clist2));
}

//TEST(ColorsMergeTest, TestColorListsAreEqual) {
//    ColorList clist1 = {NULL, 0};
//    ColorList clist2 = {NULL, 0};

//    ASSERT_TRUE(colorlists_are_equal(&clist1, &clist2));
//}

TEST(ColorsMergeTest, ThereAreNoDuplicateColors) {
    QuarkFactory *qfactory = qfactory_new();
    project_qf_register(qfactory);
    Quark *pr1 = project_new(NULL, qfactory, AMEM_MODEL_SIMPLE);
    Quark *pr2 = project_new(NULL, qfactory, AMEM_MODEL_SIMPLE);

    char *black_name, *white_name, *red_name;
    black_name = copy_string(NULL, "black");
    white_name = copy_string(NULL, "white");
    red_name = copy_string(NULL, "red");
    Colordef color_black = {0, {0x0,  0x0,  0x0},  black_name};
    Colordef color_white = {0, {0xff, 0xff, 0xff}, white_name};
    Colordef color_red   = {0, {0xff, 0x0,  0x0},  red_name};

    project_add_color(pr1, &color_black);
    project_add_color(pr1, &color_white);
    project_add_color(pr1, &color_red);

    project_add_color(pr2, &color_black);
    project_add_color(pr2, &color_white);
    project_add_color(pr2, &color_red);

    ColorList clist1 = project_get_colorlist(pr1);
    ColorList clist2 = project_get_colorlist(pr2);

    ColorList *clist = colorlist_merge_colors(&clist1, &clist2);

    ASSERT_TRUE(colorlists_are_equal(clist, &clist1));

    xfree(black_name);
    xfree(white_name);
    xfree(red_name);
    quark_free(pr1);
    quark_free(pr2);
    qfactory_free(qfactory);
}


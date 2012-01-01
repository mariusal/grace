extern "C" {
#include <grace/grace.h>
}

#include <gtest/gtest.h>

void errmsg(const char *msg)
{
    fputs(msg, stderr);
    fputc('\n', stderr);
}

TEST(CoreTest, QuarkIDStr) {
    Quark *q = quark_new(NULL, QFlavorProject);
    quark_idstr_set(q, "A string");

    EXPECT_STREQ("A String", quark_idstr_get(q));

    quark_free(q);
}


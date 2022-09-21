#include "../pico_math.h"
#include "../pico_unit.h"

#include <stdio.h>

PU_TEST(test_t2_equal)
{
    { // Should be equal
        pm_t2 t = pm_t2_make(2, 3, 3, 2, 1, 1);
        PU_ASSERT(pm_t2_equal(&t, &t));
    }

    { // Should not be equal
        pm_t2 t1 = pm_t2_make(2, 3, 3, 2, 1, 1);
        pm_t2 t2 = pm_t2_make(2, 3, 3, 5, 1, 1);
        PU_ASSERT(!pm_t2_equal(&t1, &t2));
    }

    return true;
}

PU_TEST(test_t2_identity)
{
    pm_t2 exp = pm_t2_make(1, 0, 0,
                             0, 1, 0);

    pm_t2 res = pm_t2_identity();
    PU_ASSERT(pm_t2_equal(&res, &exp));

    return true;
}

PU_TEST(test_t2_get_pos)
{
    pm_t2 t = pm_t2_make(1, 1, 2, 0, 0, 3);
    pm_v2 exp = pm_v2_make(2, 3);

    pm_v2 res = pm_t2_get_pos(&t);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_t2_set_pos)
{
    pm_t2 t = pm_t2_identity();
    pm_v2 pos = pm_v2_make(2, 3);
    pm_v2 exp = pm_v2_make(2, 3);

    pm_t2_set_pos(&t, pos);
    pm_v2 res = pm_t2_get_pos(&t);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_t2_get_angle)
{
    pm_t2 t = pm_t2_rotation(PM_PI / 8.0f);
    PU_ASSERT(pm_equal(pm_t2_get_angle(&t), PM_PI / 8.0f));

    t = pm_t2_rotation(PM_PI / 2.0f);
    PU_ASSERT(pm_equal(pm_t2_get_angle(&t), PM_PI / 2.0f));

    return true;
}

PU_TEST(test_t2_get_scale)
{
    { // Acute angle
        pm_t2 t1 = pm_t2_rotation(PM_PI / 4.0f);
        pm_t2 t2 = pm_t2_scaling(pm_v2_make(2, 3));
        pm_t2 t3 = pm_t2_mult(&t2, &t1);
        pm_v2 exp = pm_v2_make(2, 3);
        pm_v2 res = pm_t2_get_scale(&t3);
        PU_ASSERT(pm_v2_equal(res, exp));
    }

    { // Obtuse angle
        pm_t2 t1 = pm_t2_rotation(PM_PI * 3.0/ 4.0f);
        pm_t2 t2 = pm_t2_scaling(pm_v2_make(2, 3));
        pm_t2 t3 = pm_t2_mult(&t2, &t1);
        pm_v2 exp = pm_v2_make(2, 3);
        pm_v2 res = pm_t2_get_scale(&t3);
        PU_ASSERT(pm_v2_equal(res, exp));
    }

    {
        pm_t2 t1 = pm_t2_rotation(PM_PI / 2.0f);
        pm_t2 t2 = pm_t2_scaling(pm_v2_make(2, 2));
        pm_t2 t3 = pm_t2_mult(&t2, &t1);
        pm_v2 exp = pm_v2_make(2, 2);
        pm_v2 res = pm_t2_get_scale(&t3);
        PU_ASSERT(pm_v2_equal(res, exp));
    }

    return true;
}

PU_TEST(test_t2_set_angle)
{
    pm_t2 t1 = pm_t2_rotation(PM_PI / 2.0f);
    pm_t2 t2 = pm_t2_scaling(pm_v2_make(2, 3));
    pm_t2 t3 = pm_t2_mult(&t2, &t1);

    { // Case 0
        pm_t2_set_angle(&t3, PM_PI / 8.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI / 8.0f));
    }

    { // Case 1
        pm_t2_set_angle(&t3, PM_PI / 4.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI / 4.0f));
    }

    { // Case 2
        pm_t2_set_angle(&t3, PM_PI * 3.0f / 8.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI * 3.0f / 8.0f));
    }

    { // Case 3
        pm_t2_set_angle(&t3, PM_PI * 7.0f / 8.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI * 7.0f / 8.0f));
    }

    { // Case 4
        pm_t2_set_angle(&t3, PM_PI / 2.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI / 2.0f));
    }

    { // Case 5
        pm_t2_set_angle(&t3, PM_PI);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI));
    }

    { // Case 6
        pm_t2_set_angle(&t3, PM_PI * 3.0f / 4.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI * 3.0f / 4.0f));
    }

    { // Case 9
        pm_t2_set_angle(&t3, PM_PI * 9.0f / 8.0f);
        pm_float angle = pm_t2_get_angle(&t3);
        PU_ASSERT(pm_equal(angle, PM_PI * 9.0f / 8.0f));
    }

    return true;
}

PU_TEST(test_t2_map)
{
    pm_t2 t1 = pm_t2_rotation(PM_PI / 4.0f);
    pm_t2 t2 = pm_t2_scaling(pm_v2_make(2, 2));
    pm_t2 t3 = pm_t2_mult(&t1, &t2);

    pm_v2 v = pm_v2_make(1, 0);

    pm_v2 exp = pm_v2_make(pm_sqrt(2), pm_sqrt(2));
    pm_v2 res = pm_t2_map(&t3, v);

    PU_ASSERT(pm_v2_equal(res, exp));

    return true;
}

PU_TEST(test_t2_mult)
{
    pm_t2 t1 = pm_t2_rotation(PM_PI / 8.0f);
    pm_t2 t2 = pm_t2_rotation(PM_PI / 8.0f);
    pm_t2 t3 = pm_t2_mult(&t1, &t2);

    pm_float angle = pm_t2_get_angle(&t3);
    PU_ASSERT(pm_equal(angle, PM_PI / 4.0f));

    t2 = pm_t2_scaling(pm_v2_make(2, 2));
    t3 = pm_t2_mult(&t3, &t2);

    pm_v2 scale = pm_t2_get_scale(&t3);

    PU_ASSERT(pm_equal(angle, PM_PI / 4.0f));

    pm_v2 exp = pm_v2_make(2, 2);

    PU_ASSERT(pm_v2_equal(scale, exp));

    return true;
}

PU_TEST(test_t2_inv)
{
    pm_t2 t1 = pm_t2_rotation(PM_PI / 8.0f);
    pm_t2 t2 = pm_t2_rotation(PM_PI / 8.0f);
    pm_t2 t3 = pm_t2_mult(&t1, &t2);

    pm_t2 inv = pm_t2_inv(&t3);
    pm_t2 exp = pm_t2_identity();
    pm_t2 res = pm_t2_mult(&t3, &inv);

    PU_ASSERT(pm_t2_equal(&res, &exp));

    t1 = pm_t2_translation(pm_v2_make(1, 2));
    t2 = pm_t2_scaling(pm_v2_make(2, 2));
    t3 = pm_t2_mult(&t3, &t2);
    t3 = pm_t2_mult(&t3, &t1);
    inv = pm_t2_inv(&t3);
    res = pm_t2_mult(&t3, &inv);

    PU_ASSERT(pm_t2_equal(&res, &exp));

    return true;
}

PU_TEST(test_t2_lerp)

{
    pm_t2 t1, t2, t3;

    t1 = pm_t2_translation(pm_v2_make(1, 1));
    t3 = pm_t2_scaling(pm_v2_make(1, 1));
    t1 = pm_t2_mult(&t1, &t3);
    t3 = pm_t2_rotation(PM_PI / 4.0f);
    t1 = pm_t2_mult(&t1, &t3);

    /*t1 = pm_t2_rotation(PM_PI / 4.0f);
    t3 = pm_t2_scaling(pm_v2_make(1, 1));
    t1 = pm_t2_mult(&t3, &t1);
    t3 = pm_t2_translation(pm_v2_make(1, 1));
    t1 = pm_t2_mult(&t3, &t1);*/

    t2 = pm_t2_rotation(PM_PI / 2.0f);
    t3 = pm_t2_scaling(pm_v2_make(2, 2));
    t2 = pm_t2_mult(&t3, &t2);
    t3 = pm_t2_translation(pm_v2_make(1, 1));
    t2 = pm_t2_mult(&t3, &t2);

    t3 = pm_t2_lerp(&t1, &t2, 0.5f);

    pm_v2 scale = pm_t2_get_scale(&t3);
    pm_v2 pos = pm_t2_get_pos(&t3);

    pm_float angle = pm_t2_get_angle(&t3);

    PU_ASSERT(pm_equal(angle, PM_PI * 3.0f / 8.0f));

    pm_v2 exp_scale = pm_v2_make(1.5, 1.5);
    pm_v2 exp_pos = pm_v2_make(1, 1);

    PU_ASSERT(pm_v2_equal(scale, exp_scale));
    PU_ASSERT(pm_v2_equal(pos, exp_pos));

    return true;
}

PU_TEST(test_t2_lerp_identity)
{
    pm_t2 t1 = pm_t2_identity();
    pm_t2 t2 = pm_t2_identity();

    pm_t2 res = pm_t2_lerp(&t1, &t2, 0.3f);
    pm_t2 exp = pm_t2_identity();

    PU_ASSERT(pm_t2_equal(&res, &exp));

    return true;
}

PU_SUITE(suite_t2)
{
    PU_RUN_TEST(test_t2_equal);
    PU_RUN_TEST(test_t2_identity);
    PU_RUN_TEST(test_t2_get_pos);
    PU_RUN_TEST(test_t2_set_pos);
    PU_RUN_TEST(test_t2_get_angle);
    PU_RUN_TEST(test_t2_get_scale);
    PU_RUN_TEST(test_t2_set_angle);
    PU_RUN_TEST(test_t2_map);
    PU_RUN_TEST(test_t2_mult);
    PU_RUN_TEST(test_t2_inv);
    PU_RUN_TEST(test_t2_lerp);
    PU_RUN_TEST(test_t2_lerp_identity);
}

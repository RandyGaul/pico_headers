/*
    @file pico_sat.h
    @brief Separating Axis Test (SAT) written in C99.

    ----------------------------------------------------------------------------
    Licensing information at end of header
    ----------------------------------------------------------------------------

    Later:
    pm_b2 sat_polygon_to_aabb(const sat_polygon_t* poly);
    pm_b2 sat_circle_to_aabb(const sat_circle_t* circle);

*/

#ifndef PICO_SAT_H
#define PICO_SAT_H

#include <float.h>

#include "pico_math.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum number of vertices in a polygon
#ifndef PICO_SAT_MAX_POLY_VERTS
#define PICO_SAT_MAX_POLY_VERTS 8
#endif

/**
 * @brief A circle shape
 */
typedef struct
{
    pm_v2  pos;      //!< Center of circle
    pm_float radius; //!< Radius of the circle
} sat_circle_t;

/**
 * @brief A polygon shape
 * Must use CCW (counter-clockwise) winding
 */
typedef struct
{
    int   vertex_count;                      //!< Number of vertices in polygon
    pm_v2 vertices[PICO_SAT_MAX_POLY_VERTS]; //!< Polygon vertices
    pm_v2 normals[PICO_SAT_MAX_POLY_VERTS];  //!< Polygon face normals
    pm_v2 edges[PICO_SAT_MAX_POLY_VERTS];    //!< Edges of polygon
} sat_poly_t;

/**
 * @brief A collision manifold
 * Provides information about a collision
 */
typedef struct
{
    pm_v2    normal;  //!< Normal to colliding edge (in direction of MTV)
    pm_float overlap; //!< Amount of overlap between two shapes along colliding axis (MTD)
    pm_v2    vector;  //!< Vector defined by `vector = normal * overlap`
} sat_manifold_t;

/**
 * @brief Initializes a circle
 * @param pos    Circle center
 * @param radius Circle radius
 */
sat_circle_t sat_make_circle(pm_v2 pos, pm_float radius);

/**
 * @brief Initializes a polygon
 * @param vertex_count The number of vertices of the polygon
 * @param vertices     The vertices of the polygon (must use CCW winding)
 * @returns The polygon with the given vertices
 */
sat_poly_t sat_make_polygon(int vertex_count, pm_v2 vertices[]);

/**
 * @brief Converts and axis-aligned bounding box (AABB) to a polygon
 * @brief aabb The AABB
 * @returns the AABB as a polygon
 */
sat_poly_t sat_aabb_to_poly(const pm_b2* aabb);

/**
 * @brief Tests to see if one polygon overlaps with another
 * @param poly1    The colliding polygon
 * @param poly2    The target polygon
 * @param manifold The collision information (or NULL)
 * @returns True if the polygons overlap and false otherwise
 */
bool sat_test_poly_poly(const sat_poly_t* poly1,
                        const sat_poly_t* poly2,
                        sat_manifold_t* manifold);

/**
 * @brief Tests to see if a polygon overlaps a circle
 * @param poly     The colliding polygon
 * @param circle   The target circle
 * @param manifold The collision information (or NULL)
 * @returns True if the polygon and circle overlap, and false otherwise
 */
bool sat_test_poly_circle(const sat_poly_t* poly,
                          const sat_circle_t* circle,
                          sat_manifold_t* manifold);

/**
 * @brief Tests to see if a circle overlaps a polygon
 * @param circle   The colliding circle
 * @param poly     The target polygon
 * @param manifold The collision information (or NULL)
 * @returns True if the circle overlaps the polygon, and false otherwise
 */
bool sat_test_circle_poly(const sat_circle_t* circle,
                          const sat_poly_t* poly,
                          sat_manifold_t* manifold);

/**
 * @brief Tests to see if two circles overlap
 * @param circle1  The colliding circle
 * @param circle2  The target circle
 * @param manifold The collision information (or NULL)
 * @returns True if the circle the other circle, and false otherwise
 */
bool sat_test_circle_circle(const sat_circle_t* circle1,
                            const sat_circle_t* circle2,
                            sat_manifold_t* manifold);

#ifdef __cplusplus
}
#endif

#endif // PICO_SAT_H

#ifdef PICO_SAT_IMPLEMENTATION // Define once

#ifdef NDEBUG
    #define PICO_SAT_ASSERT(expr) ((void)0)
#else
    #ifndef PICO_SAT_ASSERT
        #include <assert.h>
        #define PICO_SAT_ASSERT(expr) (assert(expr))
    #endif
#endif

#define SAT_ASSERT PICO_SAT_ASSERT

/*=============================================================================
 * Internal function declarations
 *============================================================================*/

static void sat_init_manifold(sat_manifold_t* manifold);
static void sat_update_manifold(sat_manifold_t* manifold, pm_v2 normal, pm_float overlap);

static void sat_axis_range(const sat_poly_t* poly, pm_v2 normal, pm_float range[2]);
static pm_float sat_axis_overlap(const sat_poly_t* poly1, const sat_poly_t* poly2, pm_v2 axis);

typedef enum
{
    SAT_VORONOI_LEFT,
    SAT_VORONOI_RIGHT,
    SAT_VORONOI_MIDDLE
} sat_voronoi_region_t;

static sat_voronoi_region_t sat_voronoi_region(pm_v2 point, pm_v2 line);

/*=============================================================================
 * Public API implementation
 *============================================================================*/

sat_circle_t sat_make_circle(pm_v2 pos, pm_float radius)
{
    sat_circle_t circle;
    circle.pos = pos;
    circle.radius = radius;
    return circle;
}

sat_poly_t sat_make_poly(int vertex_count, pm_v2 vertices[])
{
    SAT_ASSERT(vertex_count <= PICO_SAT_MAX_POLY_VERTS);

    sat_poly_t poly;

    poly.vertex_count = vertex_count;

    for (int i = 0; i < vertex_count; i++)
    {
        poly.vertices[i] = vertices[i];
    }

    for (int i = 0; i < vertex_count; i++)
    {
        int next = (i + 1) == vertex_count ? 0 : i + 1;

        pm_v2 v1 = vertices[i];
        pm_v2 v2 = vertices[next];
        poly.edges[i] = pm_v2_sub(v2, v1);
        poly.normals[i] = pm_v2_perp(poly.edges[i]);
        poly.normals[i] = pm_v2_normalize(poly.normals[i]);
    }

    return poly;
}

sat_poly_t sat_aabb_to_poly(const pm_b2* aabb)
{
    pm_v2 pos = pm_b2_pos(aabb);
    pm_v2 size = pm_b2_size(aabb);

    pm_v2 vertices[] =
    {
        { pos.x, pos.y                   },
        { pos.x,          pos.y + size.y },
        { pos.x + size.x, pos.y + size.y },
        { pos.x + size.x, pos.y          }
    };

    return sat_make_poly(4, vertices);
}

bool sat_test_circle_circle(const sat_circle_t* circle1,
                            const sat_circle_t* circle2,
                            sat_manifold_t* manifold)
{
    if (manifold)
        sat_init_manifold(manifold);

    pm_v2 diff = pm_v2_sub(circle2->pos, circle1->pos);
    pm_float dist2 = pm_v2_len2(diff);
    pm_float total_radius = circle1->radius + circle2->radius;
    pm_float total_radius2 = total_radius * total_radius;

    if (dist2 >= total_radius2)
        return false;

    if (manifold)
    {
        pm_float dist = pm_sqrt(dist2);
        pm_float overlap = total_radius - dist;
        pm_v2 normal = pm_v2_normalize(diff);
        sat_update_manifold(manifold, normal, overlap);
    }

    return true;
}

bool sat_test_poly_poly(const sat_poly_t* poly1,
                        const sat_poly_t* poly2,
                        sat_manifold_t* manifold)
{
    if (manifold)
        sat_init_manifold(manifold);

    for (int i = 0; i < poly1->vertex_count; i++)
    {
        pm_float overlap = sat_axis_overlap(poly1, poly2, poly1->normals[i]);

        if (overlap == 0.0f)
            return false;

        if (manifold)
            sat_update_manifold(manifold, poly2->normals[i], overlap);
    }

    for (int i = 0; i < poly2->vertex_count; i++)
    {
        pm_float overlap = sat_axis_overlap(poly2, poly1, poly2->normals[i]);

        if (overlap == 0.0f)
            return false;

        if (manifold)
            sat_update_manifold(manifold, poly2->normals[i], overlap);
    }

    return true;
}

bool sat_test_poly_circle(const sat_poly_t* poly,
                          const sat_circle_t* circle,
                          sat_manifold_t* manifold)
{
    if (manifold)
        sat_init_manifold(manifold);

    pm_float radius2 = circle->radius * circle->radius;

    int count = poly->vertex_count;

    for (int i = 0; i < count; i++)
    {
        int next = (i + 1) == count ? 0 : i + 1;
        int prev = (i - 1) <= 0 ? count - 1 : i - 1;

        pm_v2 normal = pm_v2_zero();
        pm_float overlap = FLT_MAX;

        pm_v2 edge = poly->edges[i];
        pm_v2 point = pm_v2_sub(circle->pos, poly->vertices[i]);

        sat_voronoi_region_t region = sat_voronoi_region(point, edge);

        if (region == SAT_VORONOI_LEFT)
        {
            pm_v2 point2 = pm_v2_sub(circle->pos, poly->vertices[prev]);
            edge = poly->edges[prev];

            region = sat_voronoi_region(point2, edge);

            if (region == SAT_VORONOI_RIGHT)
            {
                pm_float diff2 = pm_v2_len2(point);

                if (diff2 > radius2)
                    return false;

                if (manifold)
                {
                    pm_float diff = pm_sqrt(diff2);
                    overlap = circle->radius - diff;
                    normal = pm_v2_normalize(point);
                }
            }
        }
        else if (region == SAT_VORONOI_RIGHT)
        {
            pm_v2 point2 = pm_v2_sub(circle->pos, poly->vertices[next]);
            edge = poly->edges[next];

            region = sat_voronoi_region(point2, edge);

            if (region == SAT_VORONOI_LEFT)
            {
                pm_float diff2 = pm_v2_len2(point);

                if (diff2 > radius2)
                    return false;

                if (manifold)
                {
                    pm_float diff = pm_sqrt(diff2);
                    overlap = circle->radius - diff;
                    normal = pm_v2_normalize(point);
                }
            }
        }
        else // SAT_VORONOI_MIDDLE
        {
            normal = poly->normals[i];
            pm_float diff = pm_v2_dot(normal, point);
            pm_float abs_diff = pm_abs(diff);

            if (diff > 0.0f && abs_diff > circle->radius)
                return false;

            overlap = circle->radius - diff;
        }

        if (manifold)
            sat_update_manifold(manifold, normal, overlap);
    }

    return true;
}

bool sat_test_circle_poly(const sat_circle_t* circle,
                          const sat_poly_t* poly,
                          sat_manifold_t* manifold)
{
    bool collides = sat_test_poly_circle(poly, circle, (manifold) ? manifold : NULL);

    if (manifold)
    {
        manifold->normal = pm_v2_neg(manifold->normal);
        manifold->vector = pm_v2_neg(manifold->vector);
    }

    return collides;
}

/*=============================================================================
 * Internal function definitions
 *============================================================================*/

static void sat_init_manifold(sat_manifold_t* manifold)
{
    manifold->overlap = FLT_MAX;
    manifold->normal  = pm_v2_zero();
    manifold->vector  = pm_v2_zero();
}

static void sat_update_manifold(sat_manifold_t* manifold, pm_v2 normal, pm_float overlap)
{
    pm_float abs_overlap = pm_abs(overlap);

    if (abs_overlap < manifold->overlap)
    {
        manifold->overlap = abs_overlap;

        if (overlap < 0.0f)
            manifold->normal = pm_v2_neg(normal);
        else if (overlap > 0.0f)
            manifold->normal = normal;

        manifold->vector = pm_v2_scale(manifold->normal, manifold->overlap);
    }
}

static void sat_axis_range(const sat_poly_t* poly, pm_v2 normal, pm_float range[2])
{
    pm_float dot = pm_v2_dot(poly->vertices[0], normal);
    pm_float min = dot;
    pm_float max = dot;

    for (int i = 1; i < poly->vertex_count; i++)
    {
        dot = pm_v2_dot(poly->vertices[i], normal);

        if (dot < min)
            min = dot;

        if (dot > max)
            max = dot;
    }

    range[0] = min;
    range[1] = max;
}

static pm_float sat_axis_overlap(const sat_poly_t* poly1,
                                 const sat_poly_t* poly2,
                                 pm_v2 axis)

{
    pm_float range1[2];
    pm_float range2[2];

    sat_axis_range(poly1, axis, range1);
    sat_axis_range(poly2, axis, range2);

    if (range1[1] < range2[0] || range2[1] < range1[0])
        return 0.0f;

    pm_float overlap1 = range1[1] - range2[0];
    pm_float overlap2 = range2[1] - range1[0];

    return (overlap2 > overlap1) ? overlap1 : -overlap2;
}

static sat_voronoi_region_t sat_voronoi_region(pm_v2 point, pm_v2 line)
{
    pm_float len2 = pm_v2_len2(line);
    pm_float dot  = pm_v2_dot(point, line);

    if (dot < 0.0f)
        return SAT_VORONOI_LEFT;
    else if (dot > len2)
        return SAT_VORONOI_RIGHT;
    else
        return SAT_VORONOI_MIDDLE;
}

#endif // PICO_SAT_IMPLEMENTATION


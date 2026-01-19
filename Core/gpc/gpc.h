/*
===========================================================================

Project:   Generic Polygon Clipper 普通多边形裁剪
			一种计算任意多边形集的差、交、异或或并的新算法。
           A new algorithm for calculating the difference, intersection,
           exclusive-or or union of arbitrary polygon sets.

File:      gpc.h
Author:    Alan Murta (email: gpc@cs.man.ac.uk)
Version:   2.32
Date:      17th December 2004

Copyright: (C) 1997-2004, Advanced Interfaces Group,
           University of Manchester.

           This software is free for non-commercial use. It may be copied,
           modified, and redistributed provided that this copyright notice
           is preserved on all copies. The intellectual property rights of
           the algorithms used reside with the University of Manchester
           Advanced Interfaces Group.

           You may not use this software, in whole or in part, in support
           of any commercial product without the express consent of the
           author.

           There is no warranty or other guarantee of fitness of this
           software for any purpose. It is provided solely "as is".

===========================================================================
*/

#ifndef __gpc_h
#define __gpc_h

#include <stdio.h>


/*
===========================================================================
                               Constants
===========================================================================
*/

/* Increase GPC_EPSILON to encourage merging of near coincident edges    */

#define GPC_EPSILON (DBL_EPSILON)

#define GPC_VERSION "2.32"


/*
===========================================================================
                           Public Data Types
===========================================================================
*/

typedef enum                        /* Set operation type                *///操作类型
{
  GPC_DIFF,                         /* Difference                        *///差
  GPC_INT,                          /* Intersection                      *///交
  GPC_XOR,                          /* Exclusive or                      *///异或
  GPC_UNION                         /* Union                             *///并
} gpc_op;

typedef struct                      /* Polygon vertex structure          *///多边形顶点结构
{
  double              x;            /* Vertex x component                *///顶点x坐标
  double              y;            /* vertex y component                *///顶点y坐标
} gpc_vertex;

typedef struct                      /* Vertex list structure             *///顶点列表结构
{
  int                 num_vertices; /* Number of vertices in list        *///顶点数
  gpc_vertex         *vertex;       /* Vertex array pointer              *///顶点数组
} gpc_vertex_list;

typedef struct                      /* Polygon set structure             *///多边形的设置结构
{
  int                 num_contours; /* Number of contours in polygon     *///构成多边形的多边形数量
  int                *hole;         /* Hole / external contour flags     *///每个多边形的内外环标识 true/false
  gpc_vertex_list    *contour;      /* Contour array pointer             *///每个多边形的点
} gpc_polygon;

typedef struct                      /* Tristrip set structure            *///三角面的设置结构
{
  int                 num_strips;   /* Number of tristrips               *///三角面个数
  gpc_vertex_list    *strip;        /* Tristrip array pointer            *///三角面的顶点列表
} gpc_tristrip;


/*
===========================================================================
                       Public Function Prototypes
===========================================================================
*/

void gpc_read_polygon        (FILE            *infile_ptr, 
                              int              read_hole_flags,
                              gpc_polygon     *polygon);

void gpc_write_polygon       (FILE            *outfile_ptr,
                              int              write_hole_flags,
                              gpc_polygon     *polygon);

void gpc_add_contour         (gpc_polygon     *polygon,
                              gpc_vertex_list *contour,
                              int              hole);

void gpc_polygon_clip        (gpc_op           set_operation,
                              gpc_polygon     *subject_polygon,
                              gpc_polygon     *clip_polygon,
                              gpc_polygon     *result_polygon);

void gpc_tristrip_clip       (gpc_op           set_operation,
                              gpc_polygon     *subject_polygon,
                              gpc_polygon     *clip_polygon,
                              gpc_tristrip    *result_tristrip);

void gpc_polygon_to_tristrip (gpc_polygon     *polygon,
                              gpc_tristrip    *tristrip);

void gpc_free_polygon        (gpc_polygon     *polygon);

void gpc_free_tristrip       (gpc_tristrip    *tristrip);

#endif

/*
===========================================================================
                           End of file: gpc.h
===========================================================================
*/

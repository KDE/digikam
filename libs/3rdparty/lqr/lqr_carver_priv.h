/* LiquidRescaling Library
 * Copyright (C) 2007-2009 Carlo Baldassi (the "Author") <carlobaldassi@gmail.com>.
 * All Rights Reserved.
 *
 * This library implements the algorithm described in the paper
 * "Seam Carving for Content-Aware Image Resizing"
 * by Shai Avidan and Ariel Shamir
 * which can be found at http://www.faculty.idc.ac.il/arik/imret.pdf
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3 dated June, 2007.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>
 */

#ifndef __LQR_CARVER_PRIV_H__
#define __LQR_CARVER_PRIV_H__

#ifndef __LQR_BASE_H__
#error "lqr_base.h must be included prior to lqr_carver.h"
#endif /* __LQR_BASE_H__ */

#ifndef __LQR_GRADIENT_H__
#error "lqr_gradient.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_GRADIENT_H__ */

#ifndef __LQR_READER_WINDOW_PUB_H__
#error "lqr_rwindow_pub.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_READER_WINDOW_PUB_H__ */

#ifndef __LQR_ENERGY_H__
#error "lqr_energy.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_ENERGY_H__ */

#ifndef __LQR_CARVER_LIST_H__
#error "lqr_carver_list.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_CARVER_LIST_H__ */

#ifndef __LQR_VMAP_H__
#error "lqr_vmap_priv.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_VMAP_H__ */

#ifndef __LQR_VMAP_LIST_H__
#error "lqr_vmap_list.h must be included prior to lqr_carver_priv.h"
#endif /* __LQR_VMAP_LIST_H__ */

/* Macros for internal use */

#define AS0_8I(x) ((lqr_t_8i)(x))
#define AS0_16I(x) ((lqr_t_16i)(x))
#define AS0_32F(x) ((lqr_t_32f)(x))
#define AS0_64F(x) ((lqr_t_64f)(x))

#define AS_8I(x) ((lqr_t_8i*)(x))
#define AS_16I(x) ((lqr_t_16i*)(x))
#define AS_32F(x) ((lqr_t_32f*)(x))
#define AS_64F(x) ((lqr_t_64f*)(x))

#define AS2_8I(x) ((lqr_t_8i**)(x))
#define AS2_16I(x) ((lqr_t_16i**)(x))
#define AS2_32F(x) ((lqr_t_32f**)(x))
#define AS2_64F(x) ((lqr_t_64f**)(x))

#define PXL_COPY(dest, dest_ind, src, src_ind, col_depth) G_STMT_START { \
  switch (col_depth) \
    { \
      case LQR_COLDEPTH_8I: \
        AS_8I((dest))[(dest_ind)] = AS_8I((src))[(src_ind)]; \
        break; \
      case LQR_COLDEPTH_16I: \
        AS_16I((dest))[(dest_ind)] = AS_16I((src))[(src_ind)]; \
        break; \
      case LQR_COLDEPTH_32F: \
        AS_32F((dest))[(dest_ind)] = AS_32F((src))[(src_ind)]; \
        break; \
      case LQR_COLDEPTH_64F: \
        AS_64F((dest))[(dest_ind)] = AS_64F((src))[(src_ind)]; \
        break; \
    } \
} G_STMT_END

#define BUF_POINTER_COPY(dest, src, col_depth) G_STMT_START { \
  switch (col_depth) \
    { \
      case LQR_COLDEPTH_8I: \
        *AS2_8I((dest)) = AS_8I((src)); \
        break; \
      case LQR_COLDEPTH_16I: \
        *AS2_16I((dest)) = AS_16I((src)); \
        break; \
      case LQR_COLDEPTH_32F: \
        *AS2_32F((dest)) = AS_32F((src)); \
        break; \
      case LQR_COLDEPTH_64F: \
        *AS2_64F((dest)) = AS_64F((src)); \
        break; \
    } \
} G_STMT_END

#define BUF_TRY_NEW_RET_POINTER(dest, size, col_depth) G_STMT_START { \
  switch (col_depth) \
    { \
      case LQR_COLDEPTH_8I: \
        LQR_TRY_N_N ((dest) = g_try_new (lqr_t_8i, (size))); \
        break; \
      case LQR_COLDEPTH_16I: \
        LQR_TRY_N_N ((dest) = g_try_new (lqr_t_16i, (size))); \
        break; \
      case LQR_COLDEPTH_32F: \
        LQR_TRY_N_N ((dest) = g_try_new (lqr_t_32f, (size))); \
        break; \
      case LQR_COLDEPTH_64F: \
        LQR_TRY_N_N ((dest) = g_try_new (lqr_t_64f, (size))); \
        break; \
    } \
} G_STMT_END

#define BUF_TRY_NEW0_RET_POINTER(dest, size, col_depth) G_STMT_START { \
  switch (col_depth) \
    { \
      case LQR_COLDEPTH_8I: \
        LQR_TRY_N_N ((dest) = g_try_new0 (lqr_t_8i, (size))); \
        break; \
      case LQR_COLDEPTH_16I: \
        LQR_TRY_N_N ((dest) = g_try_new0 (lqr_t_16i, (size))); \
        break; \
      case LQR_COLDEPTH_32F: \
        LQR_TRY_N_N ((dest) = g_try_new0 (lqr_t_32f, (size))); \
        break; \
      case LQR_COLDEPTH_64F: \
        LQR_TRY_N_N ((dest) = g_try_new0 (lqr_t_64f, (size))); \
        break; \
    } \
} G_STMT_END

#define BUF_TRY_NEW0_RET_LQR(dest, size, col_depth) G_STMT_START { \
  switch (col_depth) \
    { \
      case LQR_COLDEPTH_8I: \
        LQR_CATCH_MEM ((dest) = g_try_new0 (lqr_t_8i, (size))); \
        break; \
      case LQR_COLDEPTH_16I: \
        LQR_CATCH_MEM ((dest) = g_try_new0 (lqr_t_16i, (size))); \
        break; \
      case LQR_COLDEPTH_32F: \
        LQR_CATCH_MEM ((dest) = g_try_new0 (lqr_t_32f, (size))); \
        break; \
      case LQR_COLDEPTH_64F: \
        LQR_CATCH_MEM ((dest) = g_try_new0 (lqr_t_64f, (size))); \
        break; \
    } \
} G_STMT_END

#define LQR_CATCH_CANC(carver) G_STMT_START { \
  if (g_atomic_int_get(&((carver)->state)) == LQR_CARVER_STATE_CANCELLED) \
    { \
      return LQR_USRCANCEL; \
    } \
} G_STMT_END

#define LQR_CATCH_CANC_N(carver) G_STMT_START { \
  if (g_atomic_int_get(&((carver)->state)) == LQR_CARVER_STATE_CANCELLED) \
    { \
      return NULL; \
    } \
} G_STMT_END

/* Tolerance for update_mmap */
#define UPDATE_TOLERANCE (1e-5)

/* Carver states */

enum _LqrCarverState {
    LQR_CARVER_STATE_STD,
    LQR_CARVER_STATE_RESIZING,
    LQR_CARVER_STATE_INFLATING,
    LQR_CARVER_STATE_TRANSPOSING,
    LQR_CARVER_STATE_FLATTENING,
    LQR_CARVER_STATE_CANCELLED
};

typedef enum _LqrCarverState LqrCarverState;

/**** LQR_CARVER CLASS DEFINITION ****/

/* This is the representation of the multisize image */
struct _LqrCarver {
    gint w_start, h_start;              /* original width & height */
    gint w, h;                          /* current width & height */
    gint w0, h0;                        /* map array width & height */

    gint level;                         /* (in)visibility level (1 = full visibility) */
    gint max_level;                     /* max level computed so far
                                         * it is not: level <= max_level
                                         * but rather: level <= 2 * max_level - 1
                                         * since levels are shifted upon inflation
                                         */

    LqrImageType image_type;            /* image type */
    gint channels;                      /* number of colour channels of the image */
    gint alpha_channel;                 /* opacity channel index (-1 if absent) */
    gint black_channel;                 /* black channel index (-1 if absent) */
    LqrColDepth col_depth;              /* image colour depth */

    gint transposed;                    /* flag to set transposed state */
    gboolean active;                    /* flag to set if carver is active */
    gboolean nrg_active;                /* flag to set if carver energy is active */
    LqrCarver *root;                    /* pointer to the root carver */

    gboolean resize_aux_layers;         /* flag to determine whether the auxiliary layers are resized */
    gboolean dump_vmaps;                /* flag to determine whether to output the seam map */
    LqrResizeOrder resize_order;        /* resize order */

    LqrCarverList *attached_list;       /* list of attached carvers */

    gfloat rigidity;                    /* rigidity value (can straighten seams) */
    gfloat *rigidity_map;               /* the rigidity function */
    gfloat *rigidity_mask;              /* the rigidity mask */
    gint delta_x;                       /* max displacement of seams (currently is only meaningful if 0 or 1 */

    void *rgb;                          /* array of rgb points */
    gint *vs;                           /* array of visibility levels */
    gfloat *en;                         /* array of energy levels */
    gfloat *bias;                       /* bias mask */
    gfloat *m;                          /* array of auxiliary energy values */
    gint *least;                        /* array of pointers */
    gint *_raw;                         /* array of array-coordinates, for seam computation */
    gint **raw;                         /* array of array-coordinates, for seam computation */

    LqrCursor *c;                       /* cursor to be used as image reader */
    void *rgb_ro_buffer;                /* readout buffer */

    gint *vpath;                        /* array of array-coordinates representing a vertical seam */
    gint *vpath_x;                      /* array of abscisses representing a vertical seam */

    gint leftright;                     /* whether to favor left or right seams */
    gint lr_switch_frequency;           /* interval between leftright switches */
    gfloat enl_step;                    /* maximum enlargement ratio in a single step */

    LqrProgress *progress;              /* pointer to progress update functions */
    gint session_update_step;           /* update step for the rescaling session */
    gint session_rescale_total;         /* total amount of rescaling for the session */
    gint session_rescale_current;       /* current amount of rescaling for the session */

    LqrEnergyFunc nrg;                  /* pointer to a general energy function */
    gint nrg_radius;                    /* energy function radius */
    LqrEnergyReaderType nrg_read_t;     /* energy function reader type */
    gpointer nrg_extra_data;            /* extra data to pass on to the energy function */
    LqrReadingWindow *rwindow;          /* reading window for energy computation */

    gint *nrg_xmin;                     /* auxiliary vector for energy update */
    gint *nrg_xmax;                     /* auxiliary vector for energy update */

    gboolean nrg_uptodate;              /* flag set if energy map is up to date */

    gdouble *rcache;                    /* array of brightness (or luma or else) levels for energy computation */
    gboolean use_rcache;                /* wheter to cache brightness, luma etc. */

    LqrVMapList *flushed_vs;            /* linked list of pointers to flushed visibility maps buffers */

    gboolean preserve_in_buffer;        /* whether to preserve the buffer given to lqr_carver_new */

    volatile gint state;                /* current state of the carver (actually a LqrCarverState enum) */
    volatile gint state_lock;           /* lock for state changing routines */
    volatile gint state_lock_queue;     /* lock queue for state changing routines */

};

/* LQR_CARVER CLASS PRIVATE FUNCTIONS */

/* constructor base */
LqrCarver *lqr_carver_new_common(gint width, gint height, gint channels);

/* Init energy related structures only */
LqrRetVal lqr_carver_init_energy_related(LqrCarver *r);

/* build maps */
LqrRetVal lqr_carver_build_maps(LqrCarver *r, gint depth);      /* build all */
LqrRetVal lqr_carver_build_emap(LqrCarver *r);  /* energy */
LqrRetVal lqr_carver_build_mmap(LqrCarver *r);  /* minpath */
LqrRetVal lqr_carver_build_vsmap(LqrCarver *r, gint depth);     /* visibility */

/* internal functions for maps computation */
LqrRetVal lqr_carver_compute_e(LqrCarver *r, gint x, gint y);   /* compute energy of point at c */
LqrRetVal lqr_carver_update_emap(LqrCarver *r); /* update energy map after seam removal */
LqrRetVal lqr_carver_update_mmap(LqrCarver *r); /* minpath */
void lqr_carver_build_vpath(LqrCarver *r);      /* compute seam path */
void lqr_carver_carve(LqrCarver *r);    /* updates the "raw" buffer */
void lqr_carver_update_vsmap(LqrCarver *r, gint l);     /* update visibility map after seam removal */
void lqr_carver_finish_vsmap(LqrCarver *r);     /* complete visibility map (last seam) */
LqrRetVal lqr_carver_inflate(LqrCarver *r, gint l);     /* adds enlargment info to map */
LqrRetVal lqr_carver_propagate_vsmap(LqrCarver *r);     /* propagates vsmap on attached carvers */

/* image manipulations */
LqrRetVal lqr_carver_resize_width(LqrCarver *r, gint w1);       /* liquid resize width */
LqrRetVal lqr_carver_resize_height(LqrCarver *r, gint h1);      /* liquid resize height */
void lqr_carver_set_width(LqrCarver *r, gint w1);
LqrRetVal lqr_carver_transpose(LqrCarver *r);
void lqr_carver_scan_reset_all(LqrCarver *r);

/* auxiliary */
LqrRetVal lqr_carver_scan_reset_attached(LqrCarver *r, LqrDataTok data);
LqrRetVal lqr_carver_set_width_attached(LqrCarver *r, LqrDataTok data);
LqrRetVal lqr_carver_inflate_attached(LqrCarver *r, LqrDataTok data);
LqrRetVal lqr_carver_flatten_attached(LqrCarver *r, LqrDataTok data);
LqrRetVal lqr_carver_transpose_attached(LqrCarver *r, LqrDataTok data);
LqrRetVal lqr_carver_propagate_vsmap_attached(LqrCarver *r, LqrDataTok data);
LqrRetVal lqr_carver_set_state(LqrCarver *r, LqrCarverState state, gboolean skip_canceled);
LqrRetVal lqr_carver_set_state_attached(LqrCarver *r, LqrDataTok data);

#ifdef __LQR_DEBUG__
/* debug */
void lqr_carver_debug_check_rows(LqrCarver *r);
#endif /* __LQR_DEBUG__ */

#endif /* __LQR_CARVER_PRIV_H__ */

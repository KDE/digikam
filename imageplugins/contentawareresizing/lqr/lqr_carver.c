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

#include <math.h>

#include <lqr/lqr_all.h>

#ifdef __LQR_VERBOSE__
#include <stdio.h>
#endif /* __LQR_VERBOSE__ */

#ifdef __LQR_DEBUG__
#include <stdio.h>
#include <assert.h>
#endif /* __LQR_DEBUG__ */

/**** LQR_CARVER CLASS FUNCTIONS ****/

/*** constructor & destructor ***/

/* constructors */
LqrCarver * lqr_carver_new_common (gint width, gint height, gint channels)
{
  LqrCarver *r;

  TRY_N_N (r = g_try_new (LqrCarver, 1));

  g_atomic_int_set(&r->state, LQR_CARVER_STATE_STD);
  g_atomic_int_set(&r->state_lock, 0);
  g_atomic_int_set(&r->state_lock_queue, 0);

  r->level = 1;
  r->max_level = 1;
  r->transposed = 0;
  r->active = FALSE;
  r->nrg_active = FALSE;
  r->root = NULL;
  r->rigidity = 0;
  r->resize_aux_layers = FALSE;
  r->dump_vmaps = FALSE;
  r->resize_order = LQR_RES_ORDER_HOR;
  r->attached_list = NULL;
  r->flushed_vs = NULL;
  r->preserve_in_buffer = FALSE;
  TRY_N_N (r->progress = lqr_progress_new());

  r->en = NULL;
  r->bias = NULL;
  r->m = NULL;
  r->least = NULL;
  r->_raw = NULL;
  r->raw = NULL;
  r->vpath = NULL;
  r->vpath_x = NULL;
  r->rigidity_map = NULL;
  r->rigidity_mask = NULL;
  r->delta_x = 1;

  r->h = height;
  r->w = width;
  r->channels = channels;

  r->w0 = r->w;
  r->h0 = r->h;
  r->w_start = r->w;
  r->h_start = r->h;

  r->rcache = NULL;
  r->use_rcache = TRUE;

  r->rwindow = NULL;
  lqr_carver_set_energy_function_builtin(r, LQR_EF_GRAD_XABS);
  r->nrg_xmin = NULL;
  r->nrg_xmax = NULL;
  r->nrg_uptodate = FALSE;

  r->leftright = 0;
  r->lr_switch_frequency = 0;

  r->enl_step = 2.0;

  TRY_N_N (r->vs = g_try_new0 (gint, r->w * r->h));

  /* initialize cursor */

  TRY_N_N (r->c = lqr_cursor_create (r));

  switch (channels)
  {
    case 1:
      lqr_carver_set_image_type (r, LQR_GREY_IMAGE);
      break;
    case 2:
      lqr_carver_set_image_type (r, LQR_GREYA_IMAGE);
      break;
    case 3:
      lqr_carver_set_image_type (r, LQR_RGB_IMAGE);
      break;
    case 4:
      lqr_carver_set_image_type (r, LQR_RGBA_IMAGE);
      break;
    case 5:
      lqr_carver_set_image_type (r, LQR_CMYKA_IMAGE);
      break;
    default:
      lqr_carver_set_image_type (r, LQR_CUSTOM_IMAGE);
      break;
  }

  return r;
}

LQR_PUBLIC
LqrCarver *
lqr_carver_new (guchar * buffer, gint width, gint height, gint channels)
{
  return lqr_carver_new_ext (buffer, width, height, channels, LQR_COLDEPTH_8I);
}

LQR_PUBLIC
LqrCarver *
lqr_carver_new_ext (void * buffer, gint width, gint height, gint channels, LqrColDepth colour_depth)
{
  LqrCarver *r;

  TRY_N_N (r = lqr_carver_new_common (width, height, channels));

  r->rgb = (void*) buffer;

  BUF_TRY_NEW_RET_POINTER(r->rgb_ro_buffer, r->channels * r->w, colour_depth);

  r->col_depth = colour_depth;

  return r;
}

/* destructor */
LQR_PUBLIC
void
lqr_carver_destroy (LqrCarver * r)
{
  if (!r->preserve_in_buffer)
    {
      g_free (r->rgb);
    }
  if (r->root == NULL)
    {
      g_free (r->vs);
    }
  g_free (r->en);
  g_free (r->bias);
  g_free (r->m);
  g_free (r->rcache);
  g_free (r->least);
  lqr_cursor_destroy (r->c);
  g_free (r->vpath);
  g_free (r->vpath_x);
  if (r->rigidity_map != NULL)
    {
      r->rigidity_map -= r->delta_x;
      g_free (r->rigidity_map);
    }
  g_free (r->rigidity_mask);
  lqr_rwindow_destroy (r->rwindow);
  g_free (r->nrg_xmin);
  g_free (r->nrg_xmax);
  lqr_vmap_list_destroy(r->flushed_vs);
  lqr_carver_list_destroy(r->attached_list);
  g_free (r->progress);
  g_free (r->_raw);
  g_free (r->raw);
  g_free (r);
}

/*** initialization ***/

LqrRetVal
lqr_carver_init_energy_related (LqrCarver *r)
{
  gint y, x;

  CATCH_F (r->active == FALSE);
  CATCH_F (r->nrg_active == FALSE);

  CATCH_MEM (r->en = g_try_new (gfloat, r->w * r->h));
  CATCH_MEM (r->_raw = g_try_new (gint, r->h_start * r->w_start));
  CATCH_MEM (r->raw = g_try_new (gint *, r->h_start));

  for (y = 0; y < r->h; y++)
    {
      r->raw[y] = r->_raw + y * r->w_start;
      for (x = 0; x < r->w_start; x++)
        {
          r->raw[y][x] = y * r->w_start + x;
        }
    }

  r->nrg_active = TRUE;

  return LQR_OK;
}


LQR_PUBLIC
LqrRetVal
lqr_carver_init (LqrCarver *r, gint delta_x, gfloat rigidity)
{
  gint x;

  CATCH_CANC (r);

  CATCH_F (r->active == FALSE);

  /* CATCH_MEM (r->bias = g_try_new0 (gfloat, r->w * r->h)); */
  CATCH_MEM (r->m = g_try_new (gfloat, r->w * r->h));
  CATCH_MEM (r->least = g_try_new (gint, r->w * r->h));

  CATCH (lqr_carver_init_energy_related (r));

  CATCH_MEM (r->vpath = g_try_new (gint, r->h));
  CATCH_MEM (r->vpath_x = g_try_new (gint, r->h));

  CATCH_MEM (r->nrg_xmin = g_try_new (gint, r->h));
  CATCH_MEM (r->nrg_xmax = g_try_new (gint, r->h));

  /* set rigidity map */
  r->delta_x = delta_x;
  r->rigidity = rigidity;

  r->rigidity_map = g_try_new0 (gfloat, 2 * r->delta_x + 1);
  r->rigidity_map += r->delta_x;
  for (x = -r->delta_x; x <= r->delta_x; x++)
    {
      r->rigidity_map[x] =
        r->rigidity * powf(fabsf(x), 1.5) / r->h;
    }

  r->active = TRUE;

  return LQR_OK;
}

/*** set attributes ***/

LQR_PUBLIC
LqrRetVal
lqr_carver_set_image_type (LqrCarver * r, LqrImageType image_type)
{
  CATCH_CANC (r);

  switch (image_type) {
    case LQR_GREY_IMAGE:
      if (r->channels != 1) {
        return LQR_ERROR;
      }
      r->alpha_channel = -1;
      r->black_channel = -1;
      break;
    case LQR_GREYA_IMAGE:
      if (r->channels != 2)
        {
          return LQR_ERROR;
        }
      r->alpha_channel = 1;
      r->black_channel = -1;
      break;
    case LQR_CMY_IMAGE:
    case LQR_RGB_IMAGE:
      if (r->channels != 3)
        {
          return LQR_ERROR;
        }
      r->alpha_channel = -1;
      r->black_channel = -1;
      break;
    case LQR_CMYK_IMAGE:
      if (r->channels != 4)
        {
          return LQR_ERROR;
        }
      r->alpha_channel = -1;
      r->black_channel = 3;
      break;
    case LQR_RGBA_IMAGE:
      if (r->channels != 4)
        {
          return LQR_ERROR;
        }
      r->alpha_channel = 3;
      r->black_channel = -1;
      break;
    case LQR_CMYKA_IMAGE:
      if (r->channels != 5)
        {
          return LQR_ERROR;
        }
      r->alpha_channel = 4;
      r->black_channel = 3;
      break;
    case LQR_CUSTOM_IMAGE:
      r->alpha_channel = r->channels - 1;
      r->black_channel = -1;
      break;
    default:
      return LQR_ERROR;
  }
  r->image_type = image_type;

  g_free(r->rcache);
  r->rcache = NULL;
  r->nrg_uptodate = FALSE;

  return LQR_OK;
}

LQR_PUBLIC
LqrRetVal
lqr_carver_set_alpha_channel (LqrCarver * r, gint channel_index)
{
  CATCH_CANC (r);

  if (channel_index < 0) {
    r->alpha_channel = -1;
  } else if (channel_index < r->channels) {
    r->alpha_channel = channel_index;
  } else {
    return LQR_ERROR;
  }
  r->image_type = LQR_CUSTOM_IMAGE;

  g_free(r->rcache);
  r->rcache = NULL;
  r->nrg_uptodate = FALSE;

  return LQR_OK;
}

LQR_PUBLIC
LqrRetVal
lqr_carver_set_black_channel (LqrCarver * r, gint channel_index)
{
  CATCH_CANC (r);

  if (channel_index < 0) {
    r->black_channel = -1;
  } else if (channel_index < r->channels) {
    r->black_channel = channel_index;
  } else {
    return LQR_ERROR;
  }
  r->image_type = LQR_CUSTOM_IMAGE;

  g_free(r->rcache);
  r->rcache = NULL;
  r->nrg_uptodate = FALSE;

  return LQR_OK;
}

/* set gradient function */
/* WARNING: THIS FUNCTION IS ONLY MAINTAINED FOR BACK-COMPATIBILITY PURPOSES */
/* lqr_carver_set_energy_function_builtin() should be used in newly written code instead */
LQR_PUBLIC
void
lqr_carver_set_gradient_function (LqrCarver * r, LqrGradFuncType gf_ind)
{
  switch (gf_ind)
    {
    case LQR_GF_NORM:
      lqr_carver_set_energy_function_builtin(r, LQR_EF_GRAD_NORM);
      break;
    case LQR_GF_SUMABS:
      lqr_carver_set_energy_function_builtin(r, LQR_EF_GRAD_SUMABS);
      break;
    case LQR_GF_XABS:
      lqr_carver_set_energy_function_builtin(r, LQR_EF_GRAD_XABS);
      break;
    case LQR_GF_NULL:
      lqr_carver_set_energy_function_builtin(r, LQR_EF_NULL);
      break;
    case LQR_GF_NORM_BIAS:
    case LQR_GF_YABS:
      lqr_carver_set_energy_function_builtin(r, LQR_EF_NULL);
      break;
#ifdef __LQR_DEBUG__
    default:
      assert (0);
#endif /* __LQR_DEBUG__ */
    }
}

/* attach carvers to be scaled along with the main one */
LQR_PUBLIC
LqrRetVal
lqr_carver_attach (LqrCarver * r, LqrCarver * aux)
{
  CATCH_F (r->w0 == aux->w0);
  CATCH_F (r->h0 == aux->h0);
  CATCH_F (g_atomic_int_get(&r->state) == LQR_CARVER_STATE_STD);
  CATCH_F (g_atomic_int_get(&aux->state) == LQR_CARVER_STATE_STD);
  CATCH_MEM (r->attached_list = lqr_carver_list_append (r->attached_list, aux));
  g_free(aux->vs);
  aux->vs = r->vs;
  aux->root = r;

  return LQR_OK;
}

/* set the seam output flag */
LQR_PUBLIC
void
lqr_carver_set_dump_vmaps (LqrCarver *r)
{
  r->dump_vmaps = TRUE;
}

/* unset the seam output flag */
LQR_PUBLIC
void
lqr_carver_set_no_dump_vmaps (LqrCarver *r)
{
  r->dump_vmaps = FALSE;
}

/* set order if rescaling in both directions */
LQR_PUBLIC
void
lqr_carver_set_resize_order (LqrCarver *r, LqrResizeOrder resize_order)
{
  r->resize_order = resize_order;
}

/* set leftright switch interval */
LQR_PUBLIC
void
lqr_carver_set_side_switch_frequency (LqrCarver *r, guint switch_frequency)
{
  r->lr_switch_frequency = switch_frequency;
}

/* set enlargement step */
LQR_PUBLIC
LqrRetVal
lqr_carver_set_enl_step (LqrCarver *r, gfloat enl_step)
{
  CATCH_F ((enl_step > 1) && (enl_step <= 2));
  CATCH_CANC (r);
  r->enl_step = enl_step;
  return LQR_OK;
}

LQR_PUBLIC
void
lqr_carver_set_use_cache (LqrCarver *r, gboolean use_cache)
{
  if (!use_cache)
    {
      g_free(r->rcache);
      r->rcache = NULL;
    }
  r->use_rcache = use_cache;
  r->rwindow->use_rcache = use_cache;
}

/* set progress reprot */
LQR_PUBLIC
void
lqr_carver_set_progress (LqrCarver *r, LqrProgress *p)
{
  g_free(r->progress);
  r->progress = p;
}

/* flag the input buffer to avoid destruction */
LQR_PUBLIC
void
lqr_carver_set_preserve_input_image(LqrCarver *r)
{
  r->preserve_in_buffer = TRUE;
}


/*** compute maps (energy, minpath & visibility) ***/

/* build multisize image up to given depth
 * it is progressive (can be called multilple times) */
LqrRetVal
lqr_carver_build_maps (LqrCarver * r, gint depth)
{
#ifdef __LQR_DEBUG__
  assert (depth <= r->w_start);
  assert (depth >= 1);
#endif /* __LQR_DEBUG__ */

  CATCH_CANC (r);

  /* only go deeper if needed */
  if (depth > r->max_level)
    {
      CATCH_F (r->active);
      CATCH_F (r->root == NULL);

      /* set to minimum width reached so far */
      lqr_carver_set_width (r, r->w_start - r->max_level + 1);

      /* compute energy & minpath maps */
      CATCH (lqr_carver_build_emap (r));
      CATCH (lqr_carver_build_mmap (r));

      /* compute visibility map */
      CATCH (lqr_carver_build_vsmap (r, depth));
    }
  return LQR_OK;
}

/* compute energy map */
LqrRetVal
lqr_carver_build_emap (LqrCarver * r)
{
  gint x, y;

  CATCH_CANC(r);

  if (r->nrg_uptodate)
    {
      return LQR_OK;
    }

  if (r->use_rcache && r->rcache == NULL)
    {
      CATCH_MEM (r->rcache = lqr_carver_generate_rcache (r));
    }

  for (y = 0; y < r->h; y++)
    {
      CATCH_CANC(r);
      /* r->nrg_xmin[y] = 0; */
      /* r->nrg_xmax[y] = r->w - 1; */
      for (x = 0; x < r->w; x++)
        {
          CATCH (lqr_carver_compute_e(r, x, y));
        }
    }

  r->nrg_uptodate = TRUE;

  return LQR_OK;
}

LqrRetVal
lqr_carver_compute_e (LqrCarver * r, gint x, gint y)
{
  gint data;
  gfloat b_add = 0;

  /* removed CANC check for performance reasons */
  /* CATCH_CANC (r); */

  data = r->raw[y][x];

  CATCH (lqr_rwindow_fill (r->rwindow, r, x, y));
  if (r->bias != NULL)
    {
      b_add = r->bias[data] / r->w_start;
    }
  r->en[data] = r->nrg(x, y, r->w, r->h, r->rwindow, r->nrg_extra_data) + b_add;

  return LQR_OK;
}

/* compute auxiliary minpath map
 * defined as
 *   y = 1 : m(x,y) = e(x,y)
 *   y > 1 : m(x,y) = min_{x'=-dx,..,dx} ( m(x-x',y-1) + rig(x') ) + e(x,y)
 * where
 *   e(x,y)  is the energy at point (x,y)
 *   dx      is the max seam step delta_x
 *   rig(x') is the rigidity for step x'
 */
LqrRetVal
lqr_carver_build_mmap (LqrCarver * r)
{
  gint x, y;
  gint data;
  gint data_down;
  gint x1_min, x1_max, x1;
  gfloat m, m1, r_fact;


  CATCH_CANC(r);

  /* span first row */
  for (x = 0; x < r->w; x++)
    {
      data = r->raw[0][x];
#ifdef __LQR_DEBUG__
      assert (r->vs[data] == 0);
#endif /* __LQR_DEBUG__ */
      r->m[data] = r->en[data];
    }

  /* span all other rows */
  for (y = 1; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          CATCH_CANC(r);

          data = r->raw[y][x];
#ifdef __LQR_DEBUG__
          assert (r->vs[data] == 0);
#endif /* __LQR_DEBUG__ */
          /* watch for boundaries */
          x1_min = MAX (-x, -r->delta_x);
          x1_max = MIN (r->w - 1 - x, r->delta_x);
          if (r->rigidity_mask) {
                  r_fact = r->rigidity_mask[data];
          } else {
                  r_fact = 1;
          }

          /* we use the data_down pointer to be able to
           * track the seams later (needed for rigidity) */
          data_down = r->raw[y - 1][x + x1_min];
          r->least[data] = data_down;
          if (r->rigidity)
            {
              m = r->m[data_down] + r_fact * r->rigidity_map[x1_min];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  /* find the min among the neighbors
                   * in the last row */
                  m1 = r->m[data_down] + r_fact * r->rigidity_map[x1];
                  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                    {
                      m = m1;
                      r->least[data] = data_down;
                    }
                  /* m = MIN(m, r->m[data_down] + r->rigidity_map[x1]); */
                }
            }
          else
            {
              m = r->m[data_down];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  /* find the min among the neighbors
                   * in the last row */
                  m1 = r->m[data_down];
                  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                    {
                      m = m1;
                      r->least[data] = data_down;
                    }
                  m = MIN (m, r->m[data_down]);
                }
            }

          /* set current m */
          r->m[data] = r->en[data] + m;
        }
    }
  return LQR_OK;
}

/* compute (vertical) visibility map up to given depth
 * (it also calls inflate() to add image enlargment information) */
LqrRetVal
lqr_carver_build_vsmap (LqrCarver * r, gint depth)
{
  gint l;
  gint update_step;
  gint lr_switch_interval = 0;
  LqrDataTok data_tok;

#ifdef __LQR_VERBOSE__
  printf ("[ building visibility map ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */


#ifdef __LQR_DEBUG__
  assert (depth <= r->w_start + 1);
  assert (depth >= 1);
#endif /* __LQR_DEBUG__ */

  /* default behaviour : compute all possible levels
   * (complete map) */
  if (depth == 0)
    {
      depth = r->w_start + 1;
    }

  /* here we assume that
   * lqr_carver_set_width(w_start - max_level + 1);
   * has been given */

  /* update step for progress reprt*/
  update_step = (gint) MAX ((depth - r->max_level) * r->progress->update_step, 1);

  /* left-right switch interval */
  if (r->lr_switch_frequency)
    {
      lr_switch_interval = (depth - r->max_level - 1) / r->lr_switch_frequency + 1;
    }

  /* cycle over levels */
  for (l = r->max_level; l < depth; l++)
    {
      CATCH_CANC(r);

      if ((l - r->max_level) % update_step == 0)
        {
          lqr_progress_update (r->progress, (gdouble) (l - r->max_level) /
                                (gdouble) (depth - r->max_level));
        }

#ifdef __LQR_DEBUG__
      /* check raw rows */
      lqr_carver_debug_check_rows (r);
#endif /* __LQR_DEBUG__ */

      /* compute vertical seam */
      lqr_carver_build_vpath (r);

      /* update visibility map
       * (assign level to the seam) */
      lqr_carver_update_vsmap (r, l + r->max_level - 1);

      /* increase (in)visibility level
       * (make the last seam invisible) */
      r->level++;
      r->w--;

      /* update raw data */
      lqr_carver_carve (r);

      if (r->w > 1)
        {
          /* update the energy */
          /* CATCH (lqr_carver_build_emap (r));  */
          CATCH (lqr_carver_update_emap (r));

          /* recalculate the minpath map */
          if ((r->lr_switch_frequency) && (((l - r->max_level + lr_switch_interval / 2) % lr_switch_interval) == 0))
            {
              r->leftright ^= 1;
              CATCH (lqr_carver_build_mmap (r));
            }
          else
            {
              /* lqr_carver_build_mmap (r); */
              CATCH (lqr_carver_update_mmap (r));
            }
        }
      else
        {
          /* complete the map (last seam) */
          lqr_carver_finish_vsmap (r);
        }
    }

  /* insert seams for image enlargement */
  CATCH (lqr_carver_inflate (r, depth - 1));

  /* reset image size */
  lqr_carver_set_width (r, r->w_start);
  /* repeat for auxiliary layers */
  data_tok.integer = r->w_start;
  CATCH (lqr_carver_list_foreach_recursive (r->attached_list, lqr_carver_set_width_attached, data_tok));

#ifdef __LQR_VERBOSE__
  printf ("[ visibility map OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  return LQR_OK;
}

/* enlarge the image by seam insertion
 * visibility map is updated and the resulting multisize image
 * is complete in both directions */
LqrRetVal
lqr_carver_inflate (LqrCarver * r, gint l)
{
  gint w1, z0, vs, k;
  gint x, y;
  gint c_left;
  void *new_rgb = NULL;
  gint *new_vs = NULL;
  gdouble tmp_rgb;
  gfloat *new_bias = NULL;
  gfloat *new_rigmask = NULL;
  LqrDataTok data_tok;
  LqrCarverState prev_state = LQR_CARVER_STATE_STD;

#ifdef __LQR_VERBOSE__
  printf ("  [ inflating (active=%i) ]\n", r->active);
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

#ifdef __LQR_DEBUG__
  assert (l + 1 > r->max_level);        /* otherwise is useless */
#endif /* __LQR_DEBUG__ */

  CATCH_CANC (r);

  if (r->root == NULL)
    {
      prev_state = g_atomic_int_get(&r->state);
      CATCH (lqr_carver_set_state(r, LQR_CARVER_STATE_INFLATING, TRUE));
    }

  /* first iterate on attached carvers */
  data_tok.integer = l;
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_inflate_attached, data_tok));

  /* scale to current maximum size
   * (this is the original size the first time) */
  lqr_carver_set_width (r, r->w0);

  /* final width */
  w1 = r->w0 + l - r->max_level + 1;

  /* allocate room for new maps */
  BUF_TRY_NEW0_RET_LQR(new_rgb, w1 * r->h0 * r->channels, r->col_depth);

  if (r->root == NULL)
    {
      CATCH_MEM (new_vs = g_try_new0 (gint, w1 * r->h0));
    }
  if (r->active)
    {
      if (r->bias)
        {
          CATCH_MEM (new_bias = g_try_new0 (gfloat, w1 * r->h0));
        }
      if (r->rigidity_mask)
        {
          CATCH_MEM (new_rigmask = g_try_new (gfloat, w1 * r->h0));
        }
    }

  /* span the image with a cursor
   * and build the new image */
  lqr_cursor_reset (r->c);
  x = 0;
  y = 0;
  for (z0 = 0; z0 < w1 * r->h0; z0++, lqr_cursor_next (r->c))
    {

      CATCH_CANC (r);

      /* read visibility */
      vs = r->vs[r->c->now];
      if ((vs != 0) && (vs <= l + r->max_level - 1)
          && (vs >= 2 * r->max_level - 1))
        {
          /* the point belongs to a previously computed seam
           * and was not inserted during a previous
           * inflate() call : insert another seam */

          /* the new pixel value is equal to the average of its
           * left and right neighbors */

          if (r->c->x > 0)
            {
              c_left = lqr_cursor_left (r->c);
            }
          else
            {
              c_left = r->c->now;
            }

          for (k = 0; k < r->channels; k++)
            {
              switch (r->col_depth)
                {
                  case LQR_COLDEPTH_8I:
                    tmp_rgb = (AS_8I(r->rgb)[c_left * r->channels + k] +
                               AS_8I(r->rgb)[r->c->now * r->channels + k]) / 2;
                    AS_8I(new_rgb)[z0 * r->channels + k] = (lqr_t_8i) (tmp_rgb + 0.499999);
                    break;
                  case LQR_COLDEPTH_16I:
                    tmp_rgb = (AS_16I(r->rgb)[c_left * r->channels + k] +
                               AS_16I(r->rgb)[r->c->now * r->channels + k]) / 2;
                    AS_16I(new_rgb)[z0 * r->channels + k] = (lqr_t_16i) (tmp_rgb + 0.499999);
                    break;
                  case LQR_COLDEPTH_32F:
                    tmp_rgb = (AS_32F(r->rgb)[c_left * r->channels + k] +
                               AS_32F(r->rgb)[r->c->now * r->channels + k]) / 2;
                    AS_32F(new_rgb)[z0 * r->channels + k] = (lqr_t_32f) tmp_rgb;
                    break;
                  case LQR_COLDEPTH_64F:
                    tmp_rgb = (AS_64F(r->rgb)[c_left * r->channels + k] +
                               AS_64F(r->rgb)[r->c->now * r->channels + k]) / 2;
                    AS_64F(new_rgb)[z0 * r->channels + k] = (lqr_t_64f) tmp_rgb;
                    break;
                }
            }
          if (r->active)
            {
              if (r->bias)
                {
                  new_bias[z0] = (r->bias[c_left] + r->bias[r->c->now]) / 2;
                }
              if (r->rigidity_mask)
                {
                  new_rigmask[z0] = (r->rigidity_mask[c_left] + r->rigidity_mask[r->c->now]) / 2;
                }
            }
          /* the first time inflate() is called
           * the new visibility should be -vs + 1 but we shift it
           * so that the final minimum visibiliy will be 1 again
           * and so that vs=0 still means "uninitialized".
           * Subsequent inflations account for that */
          if (r->root == NULL)
            {
              new_vs[z0] = l - vs + r->max_level;
            }
          z0++;
        }
      for (k = 0; k < r->channels; k++)
        {
          PXL_COPY(new_rgb, z0 * r->channels + k, r->rgb, r->c->now * r->channels + k, r->col_depth);
        }
      if (r->active)
        {
          if (r->bias)
            {
              new_bias[z0] = r->bias[r->c->now];
            }
          if (r->rigidity_mask)
            {
              new_rigmask[z0] = r->rigidity_mask[r->c->now];
            }
        }
      if (vs != 0)
        {
          /* visibility has to be shifted up */
          if (r->root == NULL)
            {
              new_vs[z0] = vs + l - r->max_level + 1;
            }
        }
      else if (r->raw != NULL)
        {
#ifdef __LQR_DEBUG__
          assert (y < r->h_start);
          assert (x < r->w_start - l);
#endif /* __LQR_DEBUG__ */
          r->raw[y][x] = z0;
          x++;
          if (x >= r->w_start - l)
            {
              x = 0;
              y++;
            }
        }
    }

#ifdef __LQR_DEBUG__
  if (r->raw != NULL)
    {
      assert (x == 0);
      if (w1 != 2 * r->w_start - 1)
        {
          assert ((y == r->h_start)
                  || (printf ("y=%i hst=%i w1=%i\n", y, r->h_start, w1)
                      && fflush (stdout) && 0));
        }
    }
#endif /* __LQR_DEBUG__ */

  /* substitute maps */
  if (!r->preserve_in_buffer)
    {
      g_free (r->rgb);
    }
  /* g_free (r->vs); */
  g_free (r->en);
  g_free (r->m);
  g_free (r->rcache);
  g_free (r->least);
  g_free (r->bias);
  g_free (r->rigidity_mask);

  r->rcache = NULL;
  r->nrg_uptodate = FALSE;

  r->rgb = new_rgb;
  r->preserve_in_buffer = FALSE;

  if (r->root == NULL)
    {
      g_free (r->vs);
      r->vs = new_vs;
      CATCH (lqr_carver_propagate_vsmap(r));
    }
  else
    {
      /* r->vs = NULL; */
    }
  if (r->nrg_active)
    {
      CATCH_MEM (r->en = g_try_new0 (gfloat, w1 * r->h0));
    }
  if (r->active)
    {
      r->bias = new_bias;
      r->rigidity_mask = new_rigmask;
      CATCH_MEM (r->m = g_try_new0 (gfloat, w1 * r->h0));
      CATCH_MEM (r->least = g_try_new0 (gint, w1 * r->h0));
    }

  /* set new widths & levels (w_start is kept for reference) */
  r->level = l + 1;
  r->max_level = l + 1;
  r->w0 = w1;
  r->w = r->w_start;

  /* reset readout buffer */
  g_free (r->rgb_ro_buffer);
  BUF_TRY_NEW0_RET_LQR(r->rgb_ro_buffer, r->w0 * r->channels, r->col_depth);

#ifdef __LQR_VERBOSE__
  printf ("  [ inflating OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  if (r->root == NULL)
    {
      CATCH (lqr_carver_set_state(r, prev_state, TRUE));
    }

  return LQR_OK;
}

LqrRetVal
lqr_carver_inflate_attached (LqrCarver * r, LqrDataTok data)
{
  return lqr_carver_inflate (r, data.integer);
}


/*** internal functions for maps computations ***/

/* do the carving
 * this actually carves the raw array,
 * which holds the indices to be used
 * in all the other maps */
void
lqr_carver_carve (LqrCarver * r)
{
  gint x, y;

#ifdef __LQR_DEBUG__
  assert (r->root == NULL);
#endif /* __LQR_DEBUG__ */

  for (y = 0; y < r->h_start; y++)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->raw[y][r->vpath_x[y]]] != 0);
      for (x = 0; x < r->vpath_x[y]; x++)
        {
          assert (r->vs[r->raw[y][x]] == 0);
        }
#endif /* __LQR_DEBUG__ */
      for (x = r->vpath_x[y]; x < r->w; x++)
        {
          r->raw[y][x] = r->raw[y][x + 1];
#ifdef __LQR_DEBUG__
          assert (r->vs[r->raw[y][x]] == 0);
#endif /* __LQR_DEBUG__ */
        }
    }

  r->nrg_uptodate = FALSE;
}


/* update energy map after seam removal */
LqrRetVal
lqr_carver_update_emap (LqrCarver * r)
{
  gint x, y;
  gint y1, y1_min, y1_max;

  if (r->nrg_uptodate)
    {
      return LQR_OK;
    }
  if (r->use_rcache)
    {
      CATCH_F (r->rcache != NULL);
    }

  CATCH_CANC (r);

  for (y = 0; y < r->h; y++)
    {
      /* note: here the vpath has already
       * been carved */
      x = r->vpath_x[y];
      r->nrg_xmin[y] = x;
      r->nrg_xmax[y] = x - 1;
    }
  for (y = 0; y < r->h; y++)
    {
      x = r->vpath_x[y];
      y1_min = MAX (y - r->nrg_radius, 0);
      y1_max = MIN (y + r->nrg_radius, r->h - 1);

      for (y1 = y1_min; y1 <= y1_max; y1++)
        {
          r->nrg_xmin[y1] = MIN (r->nrg_xmin[y1], x - r->nrg_radius);
          r->nrg_xmin[y1] = MAX (0, r->nrg_xmin[y1]);
          /* note: the -1 below is because of the previous carving */
          r->nrg_xmax[y1] = MAX (r->nrg_xmax[y1], x + r->nrg_radius - 1);
          r->nrg_xmax[y1] = MIN (r->w - 1, r->nrg_xmax[y1]);
        }
    }

  for (y = 0; y < r->h; y++)
    {
      CATCH_CANC (r);

      for (x = r->nrg_xmin[y]; x <= r->nrg_xmax[y]; x++)
        {
          CATCH (lqr_carver_compute_e (r, x, y));
        }
    }

  r->nrg_uptodate = TRUE;

  return LQR_OK;
}

/* update the auxiliary minpath map
 * this only updates the affected pixels,
 * which start form the beginning of the changed
 * energy region around the seam and expand
 * at most by delta_x (in both directions)
 * at each row */
LqrRetVal
lqr_carver_update_mmap (LqrCarver * r)
{
  gint x, y;
  gint x_min, x_max;
  gint x1;
  gint x1_min, x1_max;
  gint data, data_down, least;
  gfloat m, m1, r_fact;
  gint stop;
  gint x_stop;

  CATCH_CANC(r);
  CATCH_F (r->nrg_uptodate);

  /* span first row */
  /* x_min = MAX (r->vpath_x[0] - r->delta_x, 0); */
  x_min = MAX (r->nrg_xmin[0], 0);
  /* x_max = MIN (r->vpath_x[0] + r->delta_x - 1, r->w - 1); */
  /* x_max = MIN (r->vpath_x[0] + r->delta_x, r->w - 1); */
  x_max = MIN (r->nrg_xmax[0], r->w - 1);

  for (x = x_min; x <= x_max; x++)
    {
      data = r->raw[0][x];
      r->m[data] = r->en[data];
    }

  /* other rows */
  for (y = 1; y < r->h; y++)
    {
      CATCH_CANC(r);

      /* make sure to include the changed energy region */
      x_min = MIN (x_min, r->nrg_xmin[y]);
      x_max = MAX (x_max, r->nrg_xmax[y]);

      /* expand the affected region by delta_x */
      x_min = MAX (x_min - r->delta_x, 0);
      x_max = MIN (x_max + r->delta_x, r->w - 1);

      /* span the affected region */
      stop = 0;
      x_stop = 0;
      for (x = x_min; x <= x_max; x++)
        {
          data = r->raw[y][x];
          if (r->rigidity_mask) {
                  r_fact = r->rigidity_mask[data];
          } else {
                  r_fact = 1;
          }

          /* find the minimum in the previous rows
           * as in build_mmap() */
          x1_min = MAX (-x, -r->delta_x);
          x1_max = MIN (r->w - 1 - x, r->delta_x);
          data_down = r->raw[y - 1][x + x1_min];
          least = data_down;
          if (r->rigidity)
            {
              m = r->m[data_down] + r_fact * r->rigidity_map[x1_min];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  m1 = r->m[data_down] + r_fact * r->rigidity_map[x1];
                  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                    {
                      m = m1;
                      least = data_down;
                    }
                }
            }
          else
            {
              m = r->m[data_down];
              for (x1 = x1_min + 1; x1 <= x1_max; x1++)
                {
                  data_down = r->raw[y - 1][x + x1];
                  m1 = r->m[data_down];
                  if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
                    {
                      m = m1;
                      least = data_down;
                    }
                }
            }

          /* reduce the range if there's no difference
           * with the previous map */
          if (r->least[data] == least)
            {
              if ((x == x_min) && (x < r->nrg_xmin[y])
                  && (r->m[data] == r->en[data] + m))
                {
                  x_min++;
                }
              if ((x > r->nrg_xmax[y]) && (r->m[data] == r->en[data] + m))
                {
                  if (stop == 0)
                    {
                      x_stop = x;
                    }
                  stop = 1;
                }
              else
                {
                  stop = 0;
                }
            }
          else
            {
              stop = 0;
            }

          /* set current m */
          r->m[data] = r->en[data] + m;
          r->least[data] = least;

          if ((x == x_max) && (stop))
            {
              x_max = x_stop;
            }
        }

    }
  return LQR_OK;
}



/* compute seam path from minpath map */
void
lqr_carver_build_vpath (LqrCarver * r)
{
  gint x, y, z0;
  gfloat m, m1;
  gint last = -1;
  gint last_x = 0;
  gint x_min, x_max;

  /* we start at last row */
  y = r->h - 1;

  /* span the last row for the minimum mmap value */
  m = (1 << 29);
  for (x = 0, z0 = y * r->w_start; x < r->w; x++, z0++)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->raw[y][x]] == 0);
#endif /* __LQR_DEBUG__ */

      m1 = r->m[r->raw[y][x]];
      if ((m1 < m) || ((m1 == m) && (r->leftright == 1)))
        {
          last = r->raw[y][x];
          last_x = x;
          m = m1;
        }
    }

#ifdef __LQR_DEBUG__
  assert (last >= 0);
#endif /* __LQR_DEBUG__ */

  /* follow the track for the other rows */
  for (y = r->h0 - 1; y >= 0; y--)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[last] == 0);
      assert (last_x < r->w);
#endif /* __LQR_DEBUG__ */
      r->vpath[y] = last;
      r->vpath_x[y] = last_x;
      if (y > 0)
        {
          last = r->least[r->raw[y][last_x]];
          /* we also need to retrieve the x coordinate */
          x_min = MAX (last_x - r->delta_x, 0);
          x_max = MIN (last_x + r->delta_x, r->w - 1);
          for (x = x_min; x <= x_max; x++)
            {
              if (r->raw[y - 1][x] == last)
                {
                  last_x = x;
                  break;
                }
            }
#ifdef __LQR_DEBUG__
          assert (x < x_max + 1);
#endif /* __LQR_DEBUG__ */
        }
    }


#if 0
  /* we backtrack the seam following the min mmap */
  for (y = r->h0 - 1; y >= 0; y--)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[last] == 0);
      assert (last_x < r->w);
#endif /* __LQR_DEBUG__ */

      r->vpath[y] = last;
      r->vpath_x[y] = last_x;
      if (y > 0)
        {
          m = (1 << 29);
          x_min = MAX (0, last_x - r->delta_x);
          x_max = MIN (r->w - 1, last_x + r->delta_x);
          for (x = x_min; x <= x_max; x++)
            {
              m1 = r->m[r->raw[y - 1][x]];
              if (m1 < m)
                {
                  last = r->raw[y - 1][x];
                  last_x = x;
                  m = m1;
                }
            }
        }
    }
#endif
}

/* update visibility map after seam computation */
void
lqr_carver_update_vsmap (LqrCarver * r, gint l)
{
  gint y;
#ifdef __LQR_DEBUG__
  assert(r->root == NULL);
#endif /* __LQR_DEBUG__ */
  for (y = 0; y < r->h; y++)
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->vpath[y]] == 0);
      assert (r->vpath[y] == r->raw[y][r->vpath_x[y]]);
#endif /* __LQR_DEBUG__ */
      r->vs[r->vpath[y]] = l;
    }
}

/* complete visibility map (last seam) */
/* set the last column of pixels to vis. level w0 */
void
lqr_carver_finish_vsmap (LqrCarver * r)
{
  gint y;

#ifdef __LQR_DEBUG__
  assert (r->w == 1);
  assert (r->root == NULL);
#endif /* __LQR_DEBUG__ */
  lqr_cursor_reset (r->c);
  for (y = 1; y <= r->h; y++, lqr_cursor_next (r->c))
    {
#ifdef __LQR_DEBUG__
      assert (r->vs[r->c->now] == 0);
#endif /* __LQR_DEBUG__ */
      r->vs[r->c->now] = r->w0;
    }
  lqr_cursor_reset (r->c);
}

/* propagate the root carver's visibility map */
LqrRetVal
lqr_carver_propagate_vsmap (LqrCarver * r)
{
  LqrDataTok data_tok;

  CATCH_CANC (r);

  data_tok.data = NULL;
  CATCH (lqr_carver_list_foreach_recursive (r->attached_list,  lqr_carver_propagate_vsmap_attached, data_tok));
  return LQR_OK;
}

LqrRetVal
lqr_carver_propagate_vsmap_attached (LqrCarver * r, LqrDataTok data)
{
  LqrDataTok data_tok;
  data_tok.data = NULL;
  r->vs = r->root->vs;
  lqr_carver_scan_reset(r);
  /* CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_propagate_vsmap_attached, data_tok)); */
  return LQR_OK;
}

/*** image manipulations ***/

/* set width of the multisize image
 * (maps have to be computed already) */
void
lqr_carver_set_width (LqrCarver * r, gint w1)
{
#ifdef __LQR_DEBUG__
  assert (w1 <= r->w0);
  assert (w1 >= r->w_start - r->max_level + 1);
#endif /* __LQR_DEBUG__ */
  r->w = w1;
  r->level = r->w0 - w1 + 1;
}

LqrRetVal
lqr_carver_set_width_attached (LqrCarver * r, LqrDataTok data)
{
  lqr_carver_set_width (r, data.integer);
  return LQR_OK;
}



/* flatten the image to its current state
 * (all maps are reset, invisible points are lost) */
LQR_PUBLIC
LqrRetVal
lqr_carver_flatten (LqrCarver * r)
{
  void *new_rgb = NULL;
  gfloat *new_bias = NULL;
  gfloat *new_rigmask = NULL;
  gint x, y, k;
  gint z0;
  LqrDataTok data_tok;
  LqrCarverState prev_state = LQR_CARVER_STATE_STD;

#ifdef __LQR_VERBOSE__
  printf ("    [ flattening (active=%i) ]\n", r->active);
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  CATCH_CANC(r);

  if (r->root == NULL)
    {
      prev_state = g_atomic_int_get(&r->state);
      CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_FLATTENING, TRUE));
    }

  /* first iterate on attached carvers */
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_flatten_attached, data_tok));

  /* free non needed maps first */
  g_free (r->en);
  g_free (r->m);
  g_free (r->rcache);
  g_free (r->least);

  r->rcache = NULL;
  r->nrg_uptodate = FALSE;

  /* allocate room for new map */
  BUF_TRY_NEW0_RET_LQR(new_rgb, r->w * r->h * r->channels, r->col_depth);

  if (r->active)
    {
      if (r->bias)
        {
          CATCH_MEM (new_bias = g_try_new0 (gfloat, r->w * r->h));
        }
      if (r->rigidity_mask)
        {
          CATCH_MEM (new_rigmask = g_try_new (gfloat, r->w * r->h));
        }
    }
  if (r->nrg_active)
    {
      g_free (r->_raw);
      g_free (r->raw);
      CATCH_MEM (r->_raw = g_try_new (gint, r->w * r->h));
      CATCH_MEM (r->raw = g_try_new (gint *, r->h));
    }

  /* span the image with the cursor and copy
   * it in the new array  */
  lqr_cursor_reset (r->c);
  for (y = 0; y < r->h; y++)
    {
      CATCH_CANC (r);

      if (r->nrg_active)
        {
          r->raw[y] = r->_raw + y * r->w;
        }
      for (x = 0; x < r->w; x++)
        {
          z0 = y * r->w + x;
          for (k = 0; k < r->channels; k++)
            {
              PXL_COPY(new_rgb, z0 * r->channels + k, r->rgb, r->c->now * r->channels + k, r->col_depth);
            }
          if (r->active)
            {
              if (r->bias)
                {
                  new_bias[z0] = r->bias[r->c->now];
                }
              if (r->rigidity_mask)
                {
                  new_rigmask[z0] = r->rigidity_mask[r->c->now];
                }
            }
          if (r->nrg_active)
            {
              r->raw[y][x] = z0;
            }
          lqr_cursor_next (r->c);
        }
    }

  /* substitute the old maps */
  if (!r->preserve_in_buffer)
    {
      g_free (r->rgb);
    }
  r->rgb = new_rgb;
  r->preserve_in_buffer = FALSE;
  if (r->active)
    {
      g_free (r->bias);
      r->bias = new_bias;
      g_free (r->rigidity_mask);
      r->rigidity_mask = new_rigmask;
    }

  /* init the other maps */
  if (r->root == NULL)
    {
      g_free (r->vs);
      CATCH_MEM (r->vs = g_try_new0 (gint, r->w * r->h));
      CATCH (lqr_carver_propagate_vsmap(r));
    }
  if (r->nrg_active)
    {
      CATCH_MEM (r->en = g_try_new0 (gfloat, r->w * r->h));
    }
  if (r->active)
    {
      CATCH_MEM (r->m = g_try_new0 (gfloat, r->w * r->h));
      CATCH_MEM (r->least = g_try_new (gint, r->w * r->h));
    }

  /* reset widths, heights & levels */
  r->w0 = r->w;
  r->h0 = r->h;
  r->w_start = r->w;
  r->h_start = r->h;
  r->level = 1;
  r->max_level = 1;

#ifdef __LQR_VERBOSE__
  printf ("    [ flattening OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  if (r->root == NULL)
    {
      CATCH (lqr_carver_set_state (r, prev_state, TRUE));
    }

  return LQR_OK;
}

LqrRetVal
lqr_carver_flatten_attached(LqrCarver *r, LqrDataTok data)
{
  return lqr_carver_flatten(r);
}

/* transpose the image, in its current state
 * (all maps and invisible points are lost) */
LqrRetVal
lqr_carver_transpose (LqrCarver * r)
{
  gint x, y, k;
  gint z0, z1;
  gint d;
  void *new_rgb = NULL;
  gfloat *new_bias = NULL;
  gfloat *new_rigmask = NULL;
  LqrDataTok data_tok;
  LqrCarverState prev_state = LQR_CARVER_STATE_STD;

#ifdef __LQR_VERBOSE__
  printf ("[ transposing (active=%i) ]\n", r->active);
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  CATCH_CANC (r);

  if (r->root == NULL)
    {
      prev_state = g_atomic_int_get(&r->state);
      CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_TRANSPOSING, TRUE));
    }

  if (r->level > 1)
    {
      CATCH (lqr_carver_flatten (r));
    }

  /* first iterate on attached carvers */
  CATCH (lqr_carver_list_foreach (r->attached_list,  lqr_carver_transpose_attached, data_tok));

  /* free non needed maps first */
  if (r->root == NULL)
    {
      g_free (r->vs);
    }
  g_free (r->en);
  g_free (r->m);
  g_free (r->rcache);
  g_free (r->least);
  g_free (r->rgb_ro_buffer);

  r->rcache = NULL;
  r->nrg_uptodate = FALSE;

  /* allocate room for the new maps */
  BUF_TRY_NEW0_RET_LQR(new_rgb, r->w0 * r->h0 * r->channels, r->col_depth);

  if (r->active)
    {
      if (r->bias)
        {
          CATCH_MEM (new_bias = g_try_new0 (gfloat, r->w0 * r->h0));
        }
      if (r->rigidity_mask)
        {
          CATCH_MEM (new_rigmask = g_try_new (gfloat, r->w0 * r->h0));
        }
    }
  if (r->nrg_active)
    {
      g_free (r->_raw);
      g_free (r->raw);
      CATCH_MEM (r->_raw = g_try_new0 (gint, r->h0 * r->w0));
      CATCH_MEM (r->raw = g_try_new0 (gint *, r->w0));
    }

  /* compute trasposed maps */
  for (x = 0; x < r->w; x++)
    {
      if (r->nrg_active)
        {
          r->raw[x] = r->_raw + x * r->h0;
        }
      for (y = 0; y < r->h; y++)
        {
          z0 = y * r->w0 + x;
          z1 = x * r->h0 + y;
          for (k = 0; k < r->channels; k++)
            {
              PXL_COPY(new_rgb, z1 * r->channels + k, r->rgb, z0 * r->channels + k, r->col_depth);
            }
          if (r->active)
            {
              if (r->bias)
                {
                  new_bias[z1] = r->bias[z0];
                }
              if (r->rigidity_mask)
                {
                  new_rigmask[z1] = r->rigidity_mask[z0];
                }
            }
          if (r->nrg_active)
            {
              r->raw[x][y] = z1;
            }
        }
    }

  /* substitute the map */
  if (!r->preserve_in_buffer)
    {
      g_free (r->rgb);
    }
  r->rgb = new_rgb;
  r->preserve_in_buffer = FALSE;

  if (r->active)
    {
      g_free (r->bias);
      r->bias = new_bias;
      g_free (r->rigidity_mask);
      r->rigidity_mask = new_rigmask;
    }

  /* init the other maps */
  if (r->root == NULL)
    {
      CATCH_MEM (r->vs = g_try_new0 (gint, r->w0 * r->h0));
      CATCH (lqr_carver_propagate_vsmap(r));
    }
  if (r->nrg_active)
    {
      CATCH_MEM (r->en = g_try_new0 (gfloat, r->w0 * r->h0));
    }
  if (r->active)
    {
      CATCH_MEM (r->m = g_try_new0 (gfloat, r->w0 * r->h0));
      CATCH_MEM (r->least = g_try_new (gint, r->w0 * r->h0));
    }

  /* switch widths & heights */
  d = r->w0;
  r->w0 = r->h0;
  r->h0 = d;
  r->w = r->w0;
  r->h = r->h0;

  /* reset w_start, h_start & levels */
  r->w_start = r->w0;
  r->h_start = r->h0;
  r->level = 1;
  r->max_level = 1;

  /* reset seam path, cursor and readout buffer */
  if (r->active)
    {
      g_free (r->vpath);
      CATCH_MEM (r->vpath = g_try_new (gint, r->h));
      g_free (r->vpath_x);
      CATCH_MEM (r->vpath_x = g_try_new (gint, r->h));
      g_free (r->nrg_xmin);
      CATCH_MEM (r->nrg_xmin = g_try_new (gint, r->h));
      g_free (r->nrg_xmax);
      CATCH_MEM (r->nrg_xmax = g_try_new (gint, r->h));
    }

  BUF_TRY_NEW0_RET_LQR(r->rgb_ro_buffer, r->w0 * r->channels, r->col_depth);

  /* rescale rigidity */

  if (r->active)
    {
      for (x = -r->delta_x; x <= r->delta_x; x++)
        {
          r->rigidity_map[x] = r->rigidity_map[x] * r->w0 / r->h0;
        }
    }

  /* set transposed flag */
  r->transposed = (r->transposed ? 0 : 1);

#ifdef __LQR_VERBOSE__
  printf ("[ transpose OK ]\n");
  fflush (stdout);
#endif /* __LQR_VERBOSE__ */

  if (r->root == NULL)
    {
      CATCH (lqr_carver_set_state (r, prev_state, TRUE));
    }

  return LQR_OK;
}

LqrRetVal
lqr_carver_transpose_attached (LqrCarver * r, LqrDataTok data)
{
  return lqr_carver_transpose(r);
}

/* resize w + h: these are the liquid rescale methods.
 * They automatically determine the depth of the map
 * according to the desired size, can be called multiple
 * times, transpose the image as necessasry */
LqrRetVal
lqr_carver_resize_width (LqrCarver * r, gint w1)
{
  LqrDataTok data_tok;
  gint delta, gamma;
  gint delta_max;
  /* delta is used to determine the required depth
   * gamma to decide if action is necessary */
  if (!r->transposed)
    {
      delta = w1 - r->w_start;
      gamma = w1 - r->w;
      delta_max = (gint) ((r->enl_step - 1) * r->w_start) - 1;
    }
  else
    {
      delta = w1 - r->h_start;
      gamma = w1 - r->h;
      delta_max = (gint) ((r->enl_step - 1) * r->h_start) - 1;
    }
  if (delta_max < 1)
    {
      delta_max = 1;
    }
  if (delta < 0)
    {
      delta = -delta;
      delta_max = delta;
    }

  CATCH_CANC (r);
  CATCH_F (g_atomic_int_get(&r->state) == LQR_CARVER_STATE_STD);
  CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_RESIZING, TRUE));

  while (gamma)
    {
      gint delta0 = MIN (delta, delta_max);
      gint new_w;

      delta -= delta0;
      if (r->transposed)
        {
          CATCH (lqr_carver_transpose (r));
        }
      new_w = MIN (w1, r->w_start + delta_max);
      gamma = w1 - new_w;
      lqr_progress_init (r->progress, r->progress->init_width_message);
      CATCH (lqr_carver_build_maps (r, delta0 + 1));
      lqr_carver_set_width (r, new_w);

      data_tok.integer = new_w;
      lqr_carver_list_foreach_recursive (r->attached_list,  lqr_carver_set_width_attached, data_tok);

      if (r->dump_vmaps)
        {
          CATCH (lqr_vmap_internal_dump (r));
        }
      lqr_progress_end (r->progress, r->progress->end_width_message);
      if (new_w < w1)
        {
          CATCH (lqr_carver_flatten(r));
          delta_max = (gint) ((r->enl_step - 1) * r->w_start) - 1;
          if (delta_max < 1)
            {
              delta_max = 1;
            }
        }
    }

  CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_STD, TRUE));

  return LQR_OK;
}

LqrRetVal
lqr_carver_resize_height (LqrCarver * r, gint h1)
{
  LqrDataTok data_tok;
  gint delta, gamma;
  gint delta_max;
  /* delta is used to determine the required depth
   * gamma to decide if action is necessary */
  if (!r->transposed)
    {
      delta = h1 - r->h_start;
      gamma = h1 - r->h;
      delta_max = (gint) ((r->enl_step - 1) * r->h_start) - 1;
    }
  else
    {
      delta = h1 - r->w_start;
      gamma = h1 - r->w;
      delta_max = (gint) ((r->enl_step - 1) * r->w_start) - 1;
    }
  if (delta_max < 1)
    {
      delta_max = 1;
    }
  if (delta < 0)
    {
      delta_max = -delta;
    }
  delta = delta > 0 ? delta : -delta;

  CATCH_CANC (r);
  CATCH_F (g_atomic_int_get(&r->state) == LQR_CARVER_STATE_STD);
  CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_RESIZING, TRUE));

  while (gamma)
    {
      gint delta0 = MIN (delta, delta_max);
      gint new_w;
      delta -= delta0;
      if (!r->transposed)
        {
          CATCH (lqr_carver_transpose (r));
        }
      new_w = MIN (h1, r->w_start + delta_max);
      gamma = h1 - new_w;
      lqr_progress_init (r->progress, r->progress->init_height_message);
      CATCH (lqr_carver_build_maps (r, delta0 + 1));
      lqr_carver_set_width (r, new_w);

      data_tok.integer = new_w;
      lqr_carver_list_foreach_recursive (r->attached_list,  lqr_carver_set_width_attached, data_tok);

      if (r->dump_vmaps)
        {
          CATCH (lqr_vmap_internal_dump (r));
        }
      lqr_progress_end (r->progress, r->progress->end_height_message);
      if (new_w < h1)
        {
          CATCH (lqr_carver_flatten(r));
          delta_max = (gint) ((r->enl_step - 1) * r->w_start) - 1;
          if (delta_max < 1)
            {
              delta_max = 1;
            }
        }
    }

  CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_STD, TRUE));

  return LQR_OK;
}

/* liquid rescale public method */
LQR_PUBLIC
LqrRetVal
lqr_carver_resize (LqrCarver * r, gint w1, gint h1)
{
#ifdef __LQR_VERBOSE__
  printf("[ Rescale from %i,%i to %i,%i ]\n", (r->transposed ? r->h : r->w), (r->transposed ? r->w : r->h), w1, h1);
  fflush(stdout);
#endif /* __LQR_VERBOSE__ */
  CATCH_F ((w1 >= 1) && (h1 >= 1));
  CATCH_F (r->root == NULL);

  CATCH_CANC (r);
  CATCH_F (g_atomic_int_get(&r->state) == LQR_CARVER_STATE_STD);

  switch (r->resize_order)
    {
      case LQR_RES_ORDER_HOR:
        CATCH (lqr_carver_resize_width(r, w1));
        CATCH (lqr_carver_resize_height(r, h1));
        break;
      case LQR_RES_ORDER_VERT:
        CATCH (lqr_carver_resize_height(r, h1));
        CATCH (lqr_carver_resize_width(r, w1));
        break;
#ifdef __LQR_DEBUG__
      default:
        assert(0);
#endif /* __LQR_DEBUG__ */
    }
  lqr_carver_scan_reset_all(r);

#ifdef __LQR_VERBOSE__
  printf("[ Rescale OK ]\n");
  fflush(stdout);
#endif /* __LQR_VERBOSE__ */
  return LQR_OK;
}

LqrRetVal
lqr_carver_set_state (LqrCarver * r, LqrCarverState state, gboolean skip_canceled)
{
  LqrDataTok data_tok;
  gint lock_pos;

  CATCH_F(r->root == NULL);

  lock_pos = g_atomic_int_exchange_and_add(&r->state_lock_queue, 1);

  while (g_atomic_int_get(&r->state_lock) != lock_pos)
    {
      g_usleep(10000);
    }

  if (skip_canceled && g_atomic_int_get(&r->state) == LQR_CARVER_STATE_CANCELLED) {
    g_atomic_int_inc(&r->state_lock);
    return LQR_OK;
  }

  g_atomic_int_set(&r->state, state);

  data_tok.integer = state;
  CATCH (lqr_carver_list_foreach_recursive (r->attached_list, lqr_carver_set_state_attached, data_tok));

  g_atomic_int_inc(&r->state_lock);

  return LQR_OK;
}

LqrRetVal
lqr_carver_set_state_attached (LqrCarver * r, LqrDataTok data)
{
  g_atomic_int_set (&r->state, data.integer);
  return LQR_OK;
}

/* cancel the current action from a different thread */
LQR_PUBLIC
LqrRetVal
lqr_carver_cancel (LqrCarver * r)
{
  LqrCarverState curr_state;

  CATCH_F (r->root == NULL);

  curr_state = g_atomic_int_get (&r->state);

  if ((curr_state == LQR_CARVER_STATE_RESIZING) ||
      (curr_state == LQR_CARVER_STATE_INFLATING) ||
      (curr_state == LQR_CARVER_STATE_TRANSPOSING) ||
      (curr_state == LQR_CARVER_STATE_FLATTENING))
    {
      CATCH (lqr_carver_set_state (r, LQR_CARVER_STATE_CANCELLED, TRUE));
    }
  return LQR_OK;
}

/* get current size */
LQR_PUBLIC
gint
lqr_carver_get_width(LqrCarver* r)
{
  return (r->transposed ? r->h : r->w);
}

LQR_PUBLIC
gint
lqr_carver_get_height(LqrCarver* r)
{
  return (r->transposed ? r->w : r->h);
}

/* get reference size */
LQR_PUBLIC
gint
lqr_carver_get_ref_width(LqrCarver* r)
{
  return (r->transposed ? r->h_start : r->w_start);
}

LQR_PUBLIC
gint
lqr_carver_get_ref_height(LqrCarver* r)
{
  return (r->transposed ? r->w_start : r->h_start);
}

/* get colour channels */
LQR_PUBLIC
gint
lqr_carver_get_channels (LqrCarver * r)
{
  return r->channels;
}

LQR_PUBLIC
gint
lqr_carver_get_bpp (LqrCarver * r)
{
  return lqr_carver_get_channels(r);
}

/* get colour depth */
LQR_PUBLIC
LqrColDepth
lqr_carver_get_col_depth (LqrCarver * r)
{
  return r->col_depth;
}

/* get enlargement step */
LQR_PUBLIC
gfloat
lqr_carver_get_enl_step (LqrCarver * r)
{
  return r->enl_step;
}

/* get orientation */
LQR_PUBLIC
gint
lqr_carver_get_orientation (LqrCarver* r)
{
  return (r->transposed ? 1 : 0);
}

/* get depth */
LQR_PUBLIC
gint
lqr_carver_get_depth (LqrCarver *r)
{
  return r->w0 - r->w_start;
}


/* readout reset */
LQR_PUBLIC
void
lqr_carver_scan_reset (LqrCarver * r)
{
  lqr_cursor_reset (r->c);
}

LqrRetVal
lqr_carver_scan_reset_attached (LqrCarver * r, LqrDataTok data)
{
  lqr_carver_scan_reset(r);
  return lqr_carver_list_foreach(r->attached_list, lqr_carver_scan_reset_attached, data);
}

void
lqr_carver_scan_reset_all (LqrCarver *r)
{
  LqrDataTok data;
  data.data = NULL;
  lqr_carver_scan_reset(r);
  lqr_carver_list_foreach(r->attached_list, lqr_carver_scan_reset_attached, data);
}



/* readout all, pixel by bixel */
LQR_PUBLIC
gboolean
lqr_carver_scan (LqrCarver * r, gint * x, gint * y, guchar ** rgb)
{
  gint k;
  if (r->col_depth != LQR_COLDEPTH_8I)
    {
      return FALSE;
    }
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  (*x) = (r->transposed ? r->c->y : r->c->x);
  (*y) = (r->transposed ? r->c->x : r->c->y);
  for (k = 0; k < r->channels; k++)
    {
      AS_8I(r->rgb_ro_buffer)[k] = AS_8I(r->rgb)[r->c->now * r->channels + k];
    }
  (*rgb) = AS_8I(r->rgb_ro_buffer);
  lqr_cursor_next(r->c);
  return TRUE;
}

LQR_PUBLIC
gboolean
lqr_carver_scan_ext (LqrCarver * r, gint * x, gint * y, void ** rgb)
{
  gint k;
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  (*x) = (r->transposed ? r->c->y : r->c->x);
  (*y) = (r->transposed ? r->c->x : r->c->y);
  for (k = 0; k < r->channels; k++)
    {
      PXL_COPY(r->rgb_ro_buffer, k, r->rgb, r->c->now * r->channels + k, r->col_depth);
    }

  BUF_POINTER_COPY(rgb, r->rgb_ro_buffer, r->col_depth);

  lqr_cursor_next(r->c);
  return TRUE;
}

/* readout all, by line */
LQR_PUBLIC
gboolean
lqr_carver_scan_by_row (LqrCarver *r)
{
  return r->transposed ? FALSE : TRUE;
}

LQR_PUBLIC
gboolean
lqr_carver_scan_line (LqrCarver * r, gint * n, guchar ** rgb)
{
  if (r->col_depth != LQR_COLDEPTH_8I)
    {
      return FALSE;
    }
  return lqr_carver_scan_line_ext (r, n, (void**) rgb);
}

LQR_PUBLIC
gboolean
lqr_carver_scan_line_ext (LqrCarver * r, gint * n, void ** rgb)
{
  gint k, x;
  if (r->c->eoc)
    {
      lqr_carver_scan_reset (r);
      return FALSE;
    }
  x = r->c->x;
  (*n) = r->c->y;
  while (x > 0)
    {
      lqr_cursor_prev(r->c);
      x = r->c->x;
    }
  for (x = 0; x < r->w; x++)
    {
      for (k = 0; k < r->channels; k++)
        {
          PXL_COPY(r->rgb_ro_buffer, x * r->channels + k, r->rgb, r->c->now * r->channels + k, r->col_depth);
        }
      lqr_cursor_next(r->c);
    }


  BUF_POINTER_COPY(rgb, r->rgb_ro_buffer, r->col_depth);

  return TRUE;
}

#ifdef __LQR_DEBUG__
void lqr_carver_debug_check_rows(LqrCarver * r)
{
  int x, y;
  int data;
  for (y = 0; y < r->h; y++)
    {
      for (x = 0; x < r->w; x++)
        {
          data = r->raw[y][x];
          if (data / r->w0 != y)
            {
              fflush(stderr);
            }
          assert(data / r->w0 == y);
        }
    }
}
#endif /* __LQR_DEBUG__ */


/**** END OF LQR_CARVER CLASS FUNCTIONS ****/

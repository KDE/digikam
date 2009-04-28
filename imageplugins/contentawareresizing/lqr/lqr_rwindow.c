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

#include <glib.h>
#include <lqr/lqr_all.h>

#ifdef __LQR_DEBUG__
#include <stdio.h>
#include <assert.h>
#endif /* __LQR_DEBUG__ */


LqrRetVal
lqr_rwindow_fill_std (LqrReaderWindow * rwindow, LqrCarver * r, gint x, gint y)
{
  gdouble ** buffer;
  gint i, j;

  LqrReadFunc read_float;

  buffer = rwindow->buffer;

  switch (rwindow->read_t)
    {
      case LQR_ER_BRIGHT:
        read_float = lqr_carver_read_brightness;
        break;
      case LQR_ER_LUMA:
        read_float = lqr_carver_read_luma;
        break;
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return LQR_ERROR;
    }

  for (i = -rwindow->radius; i <= rwindow->radius; i++)
    {
      for (j = -rwindow->radius; j <= rwindow->radius; j++)
        {
          if (x + i < 0 || x + i >= r->w || y + j < 0 || y + j >= r->h)
            {
              buffer[i][j] = 0;
            }
          else
            {
              buffer[i][j] = read_float (r, x + i, y + j);
            }
        }
    }

  return LQR_OK;
}

LqrRetVal
lqr_rwindow_fill_rgba (LqrReaderWindow * rwindow, LqrCarver * r, gint x, gint y)
{
  gdouble ** buffer;
  gint i, j, k;

  buffer = rwindow->buffer;

  CATCH_F (lqr_rwindow_get_read_t (rwindow) == LQR_ER_RGBA);

  for (i = -rwindow->radius; i <= rwindow->radius; i++)
    {
      for (j = -rwindow->radius; j <= rwindow->radius; j++)
        {
          if (x + i < 0 || x + i >= r->w || y + j < 0 || y + j >= r->h)
            {
              for (k = 0; k < 4; k++)
                {
                  buffer[i][4 * j + k] = 0;
                }
            }
          else
            {
              for (k = 0; k < 4; k++)
                {
                  buffer[i][4 * j + k] = lqr_carver_read_rgba (r, x + i, y + j, k);
                }
            }
        }
    }

  return LQR_OK;
}

LqrRetVal
lqr_rwindow_fill_custom (LqrReaderWindow * rwindow, LqrCarver * r, gint x, gint y)
{
  gdouble ** buffer;
  gint i, j, k;

  buffer = rwindow->buffer;

  CATCH_F (lqr_rwindow_get_read_t (rwindow) == LQR_ER_CUSTOM);

  for (i = -rwindow->radius; i <= rwindow->radius; i++)
    {
      for (j = -rwindow->radius; j <= rwindow->radius; j++)
        {
          if (x + i < 0 || x + i >= r->w || y + j < 0 || y + j >= r->h)
            {
              for (k = 0; k < r->channels; k++)
                {
                  buffer[i][r->channels * j + k] = 0;
                }
            }
          else
            {
              for (k = 0; k < r->channels; k++)
                {
                  buffer[i][r->channels * j + k] = lqr_carver_read_custom (r, x + i, y + j, k);
                }
            }
        }
    }

  return LQR_OK;
}


LqrRetVal
lqr_rwindow_fill (LqrReaderWindow * rwindow, LqrCarver * r, gint x, gint y)
{
  CATCH_CANC (r);

  if (rwindow->use_rcache)
    {
      rwindow->carver = r;
      rwindow->x = x;
      rwindow->y = y;
      return LQR_OK;
    }

  switch (rwindow->read_t)
    {
      case LQR_ER_BRIGHT:
      case LQR_ER_LUMA:
        CATCH (lqr_rwindow_fill_std(rwindow, r, x, y));
        break;
      case LQR_ER_RGBA:
        CATCH (lqr_rwindow_fill_rgba(rwindow, r, x, y));
        break;
      case LQR_ER_CUSTOM:
        CATCH (lqr_rwindow_fill_custom(rwindow, r, x, y));
        break;
      default:
        return LQR_ERROR;
    }
  return LQR_OK;
}


LqrReaderWindow *
lqr_rwindow_new_std (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache)
{
  LqrReaderWindow * out_rwindow;
  gdouble ** out_buffer;
  gdouble * out_buffer_aux;

  gint buf_size1, buf_size2;
  gint i;

  TRY_N_N (out_rwindow = g_try_new0 (LqrReaderWindow, 1));

  buf_size1 = (2 * radius + 1);
  buf_size2 = buf_size1 * buf_size1;

  TRY_N_N (out_buffer_aux = g_try_new0 (gdouble, buf_size2));
  TRY_N_N (out_buffer = g_try_new0 (gdouble *, buf_size1));
  for (i = 0; i < buf_size1; i++)
    {
      out_buffer[i] = out_buffer_aux + radius;
      out_buffer_aux += buf_size1;
    }
  out_buffer += radius;

  out_rwindow->buffer = out_buffer;
  out_rwindow->radius = radius;
  out_rwindow->read_t = read_func_type;
  out_rwindow->channels = 1;
  out_rwindow->use_rcache = use_rcache;
  out_rwindow->carver = NULL;
  out_rwindow->x = 0;
  out_rwindow->y = 0;

  return out_rwindow;
}

LqrReaderWindow *
lqr_rwindow_new_rgba (gint radius, gboolean use_rcache)
{
  LqrReaderWindow * out_rwindow;
  gdouble ** out_buffer;
  gdouble * out_buffer_aux;

  gint buf_size1, buf_size2;
  gint i;

  TRY_N_N (out_rwindow = g_try_new0 (LqrReaderWindow, 1));

  buf_size1 = (2 * radius + 1);
  buf_size2 = buf_size1 * buf_size1 * 4;

  TRY_N_N (out_buffer_aux = g_try_new0 (gdouble, buf_size2));
  TRY_N_N (out_buffer = g_try_new0 (gdouble *, buf_size1));
  for (i = 0; i < buf_size1; i++)
    {
      out_buffer[i] = out_buffer_aux + radius * 4;
      out_buffer_aux += buf_size1 * 4;
    }
  out_buffer += radius;

  out_rwindow->buffer = out_buffer;
  out_rwindow->radius = radius;
  out_rwindow->read_t = LQR_ER_RGBA;
  out_rwindow->channels = 4;
  out_rwindow->use_rcache = use_rcache;
  out_rwindow->carver = NULL;
  out_rwindow->x = 0;
  out_rwindow->y = 0;

  return out_rwindow;
}

LqrReaderWindow *
lqr_rwindow_new_custom (gint radius, gboolean use_rcache, gint channels)
{
  LqrReaderWindow * out_rwindow;
  gdouble ** out_buffer;
  gdouble * out_buffer_aux;

  gint buf_size1, buf_size2;
  gint i;

  TRY_N_N (out_rwindow = g_try_new0 (LqrReaderWindow, 1));

  buf_size1 = (2 * radius + 1);
  buf_size2 = buf_size1 * buf_size1 * channels;

  TRY_N_N (out_buffer_aux = g_try_new0 (gdouble, buf_size2));
  TRY_N_N (out_buffer = g_try_new0 (gdouble *, buf_size1));
  for (i = 0; i < buf_size1; i++)
    {
      out_buffer[i] = out_buffer_aux + radius * channels;
      out_buffer_aux += buf_size1 * channels;
    }
  out_buffer += radius;

  out_rwindow->buffer = NULL;
  out_rwindow->radius = radius;
  out_rwindow->read_t = LQR_ER_CUSTOM;
  out_rwindow->channels = channels;
  out_rwindow->use_rcache = use_rcache;
  out_rwindow->carver = NULL;
  out_rwindow->x = 0;
  out_rwindow->y = 0;

  return out_rwindow;
}

LqrReaderWindow *
lqr_rwindow_new (gint radius, LqrEnergyReaderType read_func_type, gboolean use_rcache)
{
  switch (read_func_type)
    {
      case LQR_ER_BRIGHT:
      case LQR_ER_LUMA:
        return lqr_rwindow_new_std(radius, read_func_type, use_rcache);
      case LQR_ER_RGBA:
        return lqr_rwindow_new_rgba(radius, use_rcache);
      case LQR_ER_CUSTOM:
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return NULL;
    }
}

void
lqr_rwindow_destroy (LqrReaderWindow * rwindow)
{
  gdouble ** buffer;

  if (rwindow == NULL)
    {
      return;
    }

  if (rwindow->buffer == NULL)
    {
      return;
    }

  buffer = rwindow->buffer;
  buffer -= rwindow->radius;
  buffer[0] -= rwindow->radius * rwindow->channels;
  g_free(buffer[0]);
  g_free(buffer);
}

gdouble
lqr_rwindow_read_bright (LqrReaderWindow * rwindow, gint x, gint y)
{
  if (rwindow->use_rcache)
    {
      return lqr_carver_read_cached_std (rwindow->carver, rwindow->x + x, rwindow->y + y);
    }

  return rwindow->buffer[x][y];
}

gdouble
lqr_rwindow_read_luma (LqrReaderWindow * rwindow, gint x, gint y)
{
  if (rwindow->use_rcache)
    {
      return lqr_carver_read_cached_std (rwindow->carver, rwindow->x + x, rwindow->y + y);
    }

  return rwindow->buffer[x][y];
}

gdouble
lqr_rwindow_read_rgba (LqrReaderWindow * rwindow, gint x, gint y, gint channel)
{
  if (rwindow->use_rcache)
    {
      return lqr_carver_read_cached_rgba (rwindow->carver, rwindow->x + x, rwindow->y + y, channel);
    }

  return rwindow->buffer[x][4 * y + channel];
}

gdouble
lqr_rwindow_read_custom (LqrReaderWindow * rwindow, gint x, gint y, gint channel)
{
  if (rwindow->use_rcache)
    {
      return lqr_carver_read_cached_custom (rwindow->carver, rwindow->x + x, rwindow->y + y, channel);
    }

  return rwindow->buffer[x][rwindow->channels * y + channel];
}

LQR_PUBLIC
gdouble
lqr_rwindow_read (LqrReaderWindow * rwindow, gint x, gint y, gint channel)
{
  if (rwindow == NULL ||
      x < -rwindow->radius || x > rwindow->radius ||
      y < -rwindow->radius || y > rwindow->radius)
    {
      return 0;
    }

  switch (rwindow->read_t)
    {
      case LQR_ER_BRIGHT:
        return lqr_rwindow_read_bright (rwindow, x, y);
      case LQR_ER_LUMA:
        return lqr_rwindow_read_luma (rwindow, x, y);
      case LQR_ER_RGBA:
        return lqr_rwindow_read_rgba (rwindow, x, y, channel);
      case LQR_ER_CUSTOM:
        return lqr_rwindow_read_custom (rwindow, x, y, channel);
      default:
#ifdef __LQR_DEBUG__
        assert(0);
#endif /* __LQR_DEBUG__ */
        return 0;
    }
}


LQR_PUBLIC
LqrEnergyReaderType
lqr_rwindow_get_read_t (LqrReaderWindow * rwindow)
{
  return rwindow->read_t;
}

LQR_PUBLIC
gint
lqr_rwindow_get_radius (LqrReaderWindow * rwindow)
{
  return rwindow->radius;
}

LQR_PUBLIC
gint
lqr_rwindow_get_channels (LqrReaderWindow * rwindow)
{
  return rwindow->channels;
}



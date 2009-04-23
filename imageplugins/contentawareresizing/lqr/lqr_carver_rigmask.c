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

#include <lqr/lqr_all.h>

#ifdef __LQR_DEBUG__
#include <assert.h>
#endif

/**** LQR_CARVER_RIGMASK STRUCT FUNTIONS ****/

LqrRetVal
lqr_carver_rigmask_init (LqrCarver *r)
{
  gint y, x;

  CATCH_CANC (r);
  CATCH_F (r->active == TRUE);

  CATCH_MEM (r->rigidity_mask = g_try_new (gfloat, r->w * r->h));

  for (y = 0; y < r->h; y++)
    {
      for (x = 0; x < r->w_start; x++)
        {
	  r->rigidity_mask[y * r->w_start + x] = 1;
	}
    }
  return LQR_OK;
}


LQR_PUBLIC
LqrRetVal
lqr_carver_rigmask_add_area(LqrCarver *r, gdouble *buffer, gint width, gint height, gint x_off, gint y_off)
{
  gint x, y;
  gint x1, y1, x2, y2;

  CATCH_CANC (r);
  CATCH_F (r->active);
  if (r->rigidity_mask == NULL)
    {
      CATCH (lqr_carver_rigmask_init(r));
    }


  if (r->rigidity == 0)
    {
      return LQR_OK;
    }

  if (r->transposed)
    {
      CATCH (lqr_carver_transpose(r));
    }

  x1 = MAX (0, x_off);
  y1 = MAX (0, y_off);
  x2 = MIN (r->w, width + x_off);
  y2 = MIN (r->h, height + y_off);

  for (y = 0; y < y2 - y1; y++)
    {
      for (x = 0; x < x2 - x1; x++)
        {
          r->rigidity_mask[(y + y1) * r->w0 + (x + x1)] = (gfloat) buffer[y * width + x];
        }

    }

  return LQR_OK;
}


LQR_PUBLIC
LqrRetVal
lqr_carver_rigmask_add(LqrCarver *r, gdouble *buffer)
{
  return lqr_carver_rigmask_add_area(r, buffer, r->w0, r->h0, 0, 0);
}

LQR_PUBLIC
LqrRetVal
lqr_carver_rigmask_add_rgb_area(LqrCarver *r, guchar *rgb, gint channels, gint width, gint height, gint x_off, gint y_off)
{
  gint x, y, k, c_channels;
  gboolean has_alpha;
  gint x0, y0, x1, y1, x2, y2;
  gint transposed = 0;
  gint sum;
  gdouble rigmask;

  CATCH_CANC (r);
  CATCH_F (r->active);
  if (r->rigidity_mask == NULL)
    {
      CATCH (lqr_carver_rigmask_init(r));
    }

  if (r->rigidity == 0)
    {
      return TRUE;
    }

  CATCH (lqr_carver_flatten(r));
  if (r->transposed)
    {
      transposed = 1;
      CATCH (lqr_carver_transpose(r));
    }

  has_alpha = (channels == 2 || channels >= 4);
  c_channels = channels - (has_alpha ? 1 : 0);

  x0 = MIN (0, x_off);
  y0 = MIN (0, y_off);
  x1 = MAX (0, x_off);
  y1 = MAX (0, y_off);
  x2 = MIN (r->w0, width + x_off);
  y2 = MIN (r->h0, height + y_off);

  for (y = 0; y < y2 - y1; y++)
    {
      for (x = 0; x < x2 - x1; x++)
        {
          sum = 0;
          for (k = 0; k < c_channels; k++)
            {
              sum += rgb[((y - y0) * width + (x - x0)) * channels + k];
            }

          rigmask = (gdouble) sum / (255 * c_channels);
          if (has_alpha)
            {
	      rigmask *= (gdouble) rgb[((y - y0) * width + (x - x0) + 1) * channels - 1] / 255;
            }

          r->rigidity_mask[(y + y1) * r->w0 + (x + x1)] = (gfloat) rigmask;

        }

    }

  if (transposed)
    {
      CATCH (lqr_carver_transpose(r));
    }

  return LQR_OK;
}

LQR_PUBLIC
LqrRetVal
lqr_carver_rigmask_add_rgb(LqrCarver *r, guchar *rgb, gint channels)
{
  return lqr_carver_rigmask_add_rgb_area(r, rgb, channels, r->w0, r->h0, 0, 0);
}


/**** END OF LQR_CARVER_RIGMASK CLASS FUNCTIONS ****/

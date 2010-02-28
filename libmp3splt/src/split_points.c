/**********************************************************
 *
 * libmp3splt -- library based on mp3splt,
 *               for mp3/ogg splitting without decoding
 *
 * Copyright (c) 2002-2005 M. Trotta - <mtrotta@users.sourceforge.net>
 * Copyright (c) 2005-2010 Alexandru Munteanu - io_fx@yahoo.fr
 *
 * http://mp3splt.sourceforge.net
 *
 *********************************************************/

/**********************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307,
 * USA.
 *
 *********************************************************/

#include <string.h>

#include "splt.h"

int splt_sp_splitpoint_exists(splt_state *state, int index)
{
  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    return SPLT_TRUE;
  }
  else
  {
    return SPLT_FALSE;
  }
}

int splt_sp_append_splitpoint(splt_state *state, long split_value,
    const char *name, int type)
{
  int error = SPLT_OK;

  splt_d_print_debug(state,"Appending splitpoint _%s_ with value _%ld_\n",
      name, split_value);

  if (split_value >= 0)
  {
    state->split.real_splitnumber++;

    if (!state->split.points)
    {
      if ((state->split.points = malloc(sizeof(splt_point))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        return error;
      }
    }
    else
    {
      if ((state->split.points = realloc(state->split.points,
              state->split.real_splitnumber * sizeof(splt_point))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        return error;
      }
    }

    state->split.points[state->split.real_splitnumber-1].name = NULL;

    int value_error = SPLT_OK;
    value_error = splt_sp_set_splitpoint_value(state,
        state->split.real_splitnumber-1, split_value);
    if (value_error != SPLT_OK)
    {
      error = value_error;
      return error;
    }

    int name_error = SPLT_OK;
    name_error = splt_sp_set_splitpoint_name(state,
        state->split.real_splitnumber - 1, name);
    if (name_error != SPLT_OK)
    {
      error = name_error;
      return error;
    }

    splt_sp_set_splitpoint_type(state, state->split.real_splitnumber - 1, type);
  }
  else
  {
    splt_d_print_debug(state,"Negative splitpoint _%ld_\n", split_value);
    error = SPLT_ERROR_NEGATIVE_SPLITPOINT;
    return error;
  }

  return error;
}

splt_point *splt_sp_get_splitpoints(splt_state *state, int *splitpoints_number)
{
  *splitpoints_number = state->split.real_splitnumber;
  return state->split.points;
}

void splt_sp_free_splitpoints(splt_state *state)
{
  if (state->split.points)
  {
    int i = 0;
    for (i = 0; i < state->split.real_splitnumber; i++)
    {
      if (state->split.points[i].name)
      {
        free(state->split.points[i].name);
        state->split.points[i].name = NULL;
      }
    }
    free(state->split.points);
    state->split.points = NULL;
  }

  state->split.splitnumber = 0;
  state->split.real_splitnumber = 0;
}

int splt_sp_set_splitpoint_value(splt_state *state, int index, long split_value)
{
  splt_d_print_debug(state,"Splitpoint at _%d_ is %ld_\n", index, split_value);

  int error = SPLT_OK;

  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    state->split.points[index].value = split_value;
  }
  else
  {
    splt_e_error(SPLT_IERROR_INT,__func__, index, NULL);
  }

  return error;
}

int splt_sp_set_splitpoint_name(splt_state *state, int index, const char *name)
{
  splt_d_print_debug(state,"Splitpoint name at _%d_ is _%s_\n", index, name);

  int error = SPLT_OK;

  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    if (state->split.points[index].name)
    {
      free(state->split.points[index].name);
      state->split.points[index].name = NULL;
    }

    if (name != NULL)
    {
      if((state->split.points[index].name =
            malloc((strlen(name)+1)*sizeof(char))) == NULL)
      {
        error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        return error;
      }

      snprintf(state->split.points[index].name, strlen(name)+1, "%s",name);
    }
    else
    {
      state->split.points[index].name = NULL;
    }
  }
  else
  {
    splt_e_error(SPLT_IERROR_INT,__func__, index, NULL);
  }

  return error;
}

int splt_sp_set_splitpoint_type(splt_state *state, int index, int type)
{
  int error = SPLT_OK;

  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    state->split.points[index].type = type;
  }
  else
  {
    splt_e_error(SPLT_IERROR_INT,__func__, index, NULL);
  }

  return error;
}

long splt_sp_get_splitpoint_value(splt_state *state, int index, int *error)
{
  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    return state->split.points[index].value;
  }
  else
  {
    splt_e_error(SPLT_IERROR_INT,__func__, index, NULL);
    return -1;
  }
}

char *splt_sp_get_splitpoint_name(splt_state *state, int index, int *error)
{
  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    return state->split.points[index].name;
  }
  else
  {
    //splt_e_error(SPLT_IERROR_INT,__func__, index, NULL);
    return NULL;
  }
}

int splt_sp_get_splitpoint_type(splt_state *state, int index, int *error)
{
  if ((index >= 0) &&
      (index < state->split.real_splitnumber))
  {
    return state->split.points[index].type;
  }
  else
  {
    //splt_e_error(SPLT_IERROR_INT,__func__, index, NULL);
    //wtf ?
    return 1;
  }
}

void splt_sp_get_mins_secs_hundr_from_splitpoint(long splitpoint,
    long *mins, long *secs, long *hundr)
{
  *hundr = splitpoint % 100;
  splitpoint /= 100;
  *secs = splitpoint % 60;
  *mins = splitpoint / 60;
}

int splt_sp_cut_splitpoint_extension(splt_state *state, int index)
{
  int change_error = SPLT_OK;

  if (splt_sp_splitpoint_exists(state,index))
  {
    int get_error = SPLT_OK;
    char *temp_name = splt_sp_get_splitpoint_name(state, index, &get_error);

    if (get_error != SPLT_OK)
    {
      return get_error;
    }
    else
    {
      if (temp_name)
      {
        char *new_name = strdup(temp_name);
        if (new_name == NULL)
        {
          return SPLT_ERROR_CANNOT_ALLOCATE_MEMORY;
        }
        else
        {
          splt_su_cut_extension(new_name);
          change_error = splt_sp_set_splitpoint_name(state,index, new_name);
          free(new_name);
          new_name = NULL;
        }
      }
    }
  }

  return change_error;
}

void splt_sp_order_splitpoints(splt_state *state, int len)
{
  long temp = 0;

  int err = SPLT_OK;

  int i, j;
  float key;
  for (j=1; j < len; j++)
  {
    key = splt_sp_get_splitpoint_value(state,j,&err);
    i = j -1;
    while ((i >= 0) && 
        (splt_sp_get_splitpoint_value(state,i,&err) > key))
    {
      temp = splt_sp_get_splitpoint_value(state,i,&err);
      splt_sp_set_splitpoint_value(state,i+1,temp);
      i--;
    }
    splt_sp_set_splitpoint_value(state,i+1,key);
  }
}

long splt_sp_overlap_time(splt_state *state, int splitpoint_index)
{
  int error = SPLT_OK;
  long split_value = splt_sp_get_splitpoint_value(state, splitpoint_index, &error);
  long overlap_time = splt_o_get_long_option(state, SPLT_OPT_OVERLAP_TIME);
  if ((overlap_time > 0) && (split_value != LONG_MAX))
  {
    long total_time = splt_t_get_total_time(state);
    long overlapped_split_value = split_value + overlap_time;
    if (total_time > 0 && overlapped_split_value > total_time)
    {
      overlapped_split_value = total_time;
    }
    splt_sp_set_splitpoint_value(state, splitpoint_index, overlapped_split_value);

    return overlapped_split_value;
  }

  return split_value;
}


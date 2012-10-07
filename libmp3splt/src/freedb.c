/**********************************************************
 * libmp3splt -- library based on mp3splt,
 *               for mp3/ogg splitting without decoding
 *
 * Copyright (c) 2002-2005 M. Trotta - <mtrotta@users.sourceforge.net>
 * Copyright (c) 2005-2012 Alexandru Munteanu - m@ioalex.net
 *
 *********************************************************/

/**********************************************************
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
 * 02111-1307, USA.
 *********************************************************/

/*! \file 

All functions that are needed in order to do a Freedb search

Don't use these functions directly. The version of these functions
that is meant to be used directly are all in mp3splt.c.
*/

#include <string.h>
#include <unistd.h>

#ifdef __WIN32__
#include <conio.h>
#include <winsock.h>
#else
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#endif

#include "splt.h"
#include "socket_manager.h"
#include "freedb.h"

#define DONT_SKIP_LINES 0
#define SKIP_ONE_LINE 1

#if defined(__BEOS__) && !defined (HAS_GETPASS)
//used for proxy (proxy not implemented)
//#warning Faking getpass() !!!
//char *getpass(char *p)
//{
//      char *ret;
//      ret = malloc(30);
//      if (!ret)
//              return NULL;
//      puts(p);
//      fgets(ret,30,stdin);
//      return ret;
//}
#endif

// The alphabet fpr the base64 algorithm - for proxy (proxy not implemented)
//
// Base64 Algorithm: Base64.java v. 1.3.6 by Robert Harder
// Ported and optimized for C by Matteo Trotta
//
//const char alphabet [] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
//char *encode3to4 (unsigned char *source, int srcoffset, int num, char *destination, int destoffset)
//{
//
//    int inbuff=(num>0?(source[srcoffset]<<16):0)|(num>1?(source[srcoffset+1]<<8):0)|(num>2?(source[srcoffset+2]):0);
//    switch(num)
//    {
//      case 3:
//          destination[destoffset] = alphabet[(inbuff>>18)];
//          destination[destoffset+1] = alphabet[(inbuff>>12)&0x3f];
//          destination[destoffset+2] = alphabet[(inbuff>>6)&0x3f];
//          destination[destoffset+3] = alphabet[(inbuff)&0x3f];
//          return destination;
//
//      case 2:
//          destination[destoffset] = alphabet[(inbuff>>18)];
//          destination[destoffset+1] = alphabet[(inbuff>>12)&0x3f];
//          destination[destoffset+2] = alphabet[(inbuff>>6)&0x3f];
//          destination[destoffset+3] = '=';
//          return destination;
//
//      case 1:
//          destination[destoffset] = alphabet[(inbuff>>18)];
//          destination[destoffset+1] = alphabet[(inbuff>>12)&0x3f];
//          destination[destoffset+2] = '=';
//          destination[destoffset+3] = '=';
//          return destination;
//      default:
//          return destination;
//    }
//}

//used for proxy (proxy not implemented)
//char *b64 (unsigned char *source, int len)
//{
//      char *out;
//      int d, e=0;
//      d = ((len*4/3)+((len%3)>0?4:0));
//      
//      out = malloc(d+1);
//      
//      memset(out, 0x00, d+1);
//      for(d=0;d<(len-2);d+=3,e+=4)
//              out = encode3to4(source, d, 3, out, e);
//      if(d<len)
//              out = encode3to4(source, d, len-d, out, e);
//
//      return out;
//}
// End of Base64 Algorithm

//char *login (char *s)
//{
//      char *pass, junk[130];
//      fprintf (stdout, "Username: ");
//      fgets(junk, 128, stdin);
//      junk[strlen(junk)-1]='\0';
//      pass = getpass("Password: ");
//      sprintf (s, "%s:%s", junk, pass);
//      memset (pass, 0x00, strlen(pass));
//      free(pass);
//      return s;
//}

/*static splt_addr splt_freedb_useproxy(splt_proxy *proxy, splt_addr dest,
    const char search_server[256], int port)
{
  dest.proxy=0;
  memset(dest.hostname, 0, 256);
  //memset(line, 0, 270);

  //if (proxy->use_proxy)
  if (proxy)
  {
    //TODO
    strncpy(dest.hostname, proxy->hostname, 255);
    dest.port = proxy->port;
    dest.proxy = proxy->use_proxy;

    fprintf(stderr, "Using Proxy: %s on Port %d\n", dest.hostname, dest.port);

    dest.auth = malloc(strlen(line)+1);
    if (dest.auth==NULL)
    {
      perror("malloc");
      exit(1);
    }
    memset(dest.auth, 0x0, strlen(line)+1);
    strncpy(dest.auth, line, strlen(line));
    //dest.auth = b64(line, strlen(line));
  }

  if (!dest.proxy) 
  {
    if (strlen(search_server) == 0)
    {
      strncpy(dest.hostname, SPLT_FREEDB2_SITE, 255);
    }
    else
    {
      strncpy(dest.hostname, search_server, 255);
    }

    if (port == -1)
    {
      dest.port = SPLT_FREEDB_CDDB_CGI_PORT;
    }
    else
    {
      dest.port = port;
    }      
  }

  return dest;
}*/

char *get_cgi_path_and_cut_server(int type, const char *search_server)
{
  char *cgi_path = NULL;

  if (strlen(search_server) == 0)
  {
    splt_su_copy("/~cddb/cddb.cgi", &cgi_path);
    return cgi_path;
  }

  if (type == SPLT_FREEDB_SEARCH_TYPE_CDDB_CGI ||
      type == SPLT_FREEDB_GET_FILE_TYPE_CDDB_CGI)
  {
    char *path = strchr(search_server,'/');
    if (path)
    {
      splt_su_copy(path, &cgi_path);
      *path = '\0';
    }
  }

  return cgi_path;
}

static char *splt_freedb_get_server(const char search_server[256])
{
  char *server = NULL;

  if (strlen(search_server) == 0)
  {
    splt_su_copy(SPLT_FREEDB2_SITE, &server);
  }
  else
  {
    splt_su_copy(search_server, &server);
  }

  return server;
}

static int splt_freedb_get_port(int port_number)
{
  if (port_number == -1)
  {
    return SPLT_FREEDB_CDDB_CGI_PORT;
  }

  return port_number;
}

//line ="category discid artist / album"
static int splt_freedb_search_result_processor(const char *line, 
    int line_number, void *user_data)
{
  char *category = NULL;
  char *discid = NULL;

  splt_state *state = (splt_state *) user_data;

  const char *category_begin = splt_su_skip_spaces(line);
  const char *category_end = strchr(category_begin, ' ');
  if (category_end == NULL) { goto end; }
  splt_su_append(&category, category_begin, category_end - category_begin + 1, NULL);

  const char *discid_begin = splt_su_skip_spaces(category_end);
  const char *discid_end = strchr(discid_begin, ' ');
  if (discid_end == NULL) { goto end; }
  splt_su_append(&discid, discid_begin, discid_end - discid_begin + 1, NULL);

  splt_fu_freedb_set_disc(state, splt_fu_freedb_get_found_cds(state), 
      discid, category, strlen(category));

  splt_fu_freedb_append_result(state, splt_su_skip_spaces(discid_end), 0);

  splt_fu_freedb_found_cds_next(state);

end:
  if (category)
  {
    free(category);
    category = NULL;
  }
  if (discid)
  {
    free(discid);
    discid = NULL;
  }

  return SPLT_TRUE;
}

/*! search the freedb according to "search"

\param state The central structure this library keeps all its data in
\param search_string The string that is to be searched for
\param error The error code this action returns in
\param search_type the type of the search. Can be set to
SPLT_FREEDB_SEARCH_TYPE_CDDB_CGI
\param search_server The URL of the search server or NULL to select
the default which currently means freedb2.org
\param port The port on the server. -1 means default (Which should be
80). 
*/
int splt_freedb_process_search(splt_state *state, char *search,
    int search_type, const char search_server[256],
    int port_number)
{
  int error = SPLT_FREEDB_OK;
  int err = SPLT_OK;
  char *message = NULL;

  splt_socket_handler *sh = splt_sm_socket_handler_new(&error);
  if (error < 0) { return error; }

  char *cgi_path = get_cgi_path_and_cut_server(search_type, search_server);
  char *server = splt_freedb_get_server(search_server);
  int port = splt_freedb_get_port(port_number);

  splt_sm_connect(sh, server, port, state);
  if (sh->error < 0) { error = sh->error; goto end; }

  if (search_type == SPLT_FREEDB_SEARCH_TYPE_CDDB_CGI)
  {
    splt_su_replace_all_char(search, ' ', '+');
    err = splt_su_append_str(&message, 
        "GET ", cgi_path, "?cmd=cddb+album+", search, SPLT_FREEDB_HELLO_PROTO, NULL);
    if (err < 0) { error = err; goto disconnect; }

    splt_sm_send_http_message(sh, message, state);
    if (sh->error < 0) { error = sh->error; goto disconnect; }

    splt_fu_freedb_free_search(state);
    err = splt_fu_freedb_init_search(state);
    if (err < 0) { error = err; goto disconnect; }

    splt_sm_receive_and_process_without_headers(sh, state,
        splt_freedb_search_result_processor, state, SKIP_ONE_LINE);
    if (sh->error < 0) { error = sh->error; goto disconnect; }
  }
  else if (search_type == SPLT_FREEDB_SEARCH_TYPE_CDDB)
  {
    //TODO: new freedb web search
  }

  int found_cds = splt_fu_freedb_get_found_cds(state);
  if (found_cds == 0) 
  {
    error = SPLT_FREEDB_NO_CD_FOUND;
  }
  else if (found_cds == -1) 
  {
    splt_e_set_error_data(state, server);
    error = SPLT_FREEDB_ERROR_GETTING_INFOS;
  }
  else if (found_cds == SPLT_MAXCD) 
  {
    error = SPLT_FREEDB_MAX_CD_REACHED;
  }

disconnect:
  splt_sm_close(sh, state); 
  if (sh->error < 0) { error = sh->error; goto end; }

end:
  splt_sm_socket_handler_free(&sh);

  if (cgi_path)
  {
    free(cgi_path);
    cgi_path = NULL;
  }
  if (server)
  {
    free(server);
    server = NULL;
  }
  if (message)
  {
    free(message);
    message = NULL;
  }

  return error;
}

static int splt_freedb_process_hello_response(const char *line, 
    int line_number, void *user_data)
{
  int *error = (int *) user_data;

  if ((strncmp(line,"50",2) == 0) ||
      (strncmp(line,"40",2) == 0))
  {
    if (strncmp(line,"401",3) == 0)
    {
      *error = SPLT_FREEDB_NO_SUCH_CD_IN_DATABASE;
    }
    else
    {
      *error = SPLT_FREEDB_ERROR_SITE;
    }
  }

  return SPLT_FALSE;
}

char *test = NULL;

static int splt_freedb_process_get_file(const char *line, 
    int line_number, void *user_data)
{
  splt_get_file *get_file = (splt_get_file *) user_data;

  if (line_number == 1)
  {
    if ((strncmp(line,"50",2) == 0) ||
        (strncmp(line,"40",2) == 0))
    {
      if (strncmp(line,"401",3) == 0)
      {
        get_file->err= SPLT_FREEDB_NO_SUCH_CD_IN_DATABASE;
      }
      else
      {
        get_file->err = SPLT_FREEDB_ERROR_SITE;
      }

      return SPLT_FALSE;
    }

    return SPLT_TRUE;
  }

  if (get_file->stop_on_dot && strcmp(line, ".") == 0)
  {
    return SPLT_FALSE;
  }

  int err = splt_su_append_str(&get_file->file, line, "\n", NULL);
  if (err < 0)
  {
    get_file->err = err;
    return SPLT_FALSE;
  }

  return SPLT_TRUE;
}


/*! must only be called after process_freedb_search

returns the cddb file content corresponding to the last search, for
the disc_id (parameter of the function)

\param state The central structure that keeps all data this library
uses 
\param error Is set to the error code this action results in
\param disc_id The freedb disc ID.
\param cddb_get_type specifies the type of the get:
  it can be SPLT_FREEDB_GET_FILE_TYPE_CDDB_CGI (that works for both
  freedb and freedb2 at the moment - 18_10_06)
  or SPLT_FREEDB_GET_FILE_TYPE_CDDB (that only work for freedb at
  the moment - 18_10_06)

\todo see when we don't have a valid port or get_type
*/
char *splt_freedb_get_file(splt_state *state, int disc_id, int *error,
    int get_type, const char cddb_get_server[256], int port_number)
{
  int err = SPLT_FREEDB_FILE_OK;
  *error = err;
  char *message = NULL;

  splt_socket_handler *sh = splt_sm_socket_handler_new(&err);
  if (err < 0) { *error = err; return NULL; }

  splt_get_file *get_file = malloc(sizeof(splt_get_file));
  if (!get_file) { *error = SPLT_ERROR_CANNOT_ALLOCATE_MEMORY; return NULL; }

  get_file->err = SPLT_FREEDB_FILE_OK;
  get_file->file = NULL;
  get_file->stop_on_dot = SPLT_FALSE;

  char *cgi_path = get_cgi_path_and_cut_server(get_type, cddb_get_server);
  char *server = splt_freedb_get_server(cddb_get_server);
  int port = splt_freedb_get_port(port_number);

  const char *cd_category = splt_fu_freedb_get_disc_category(state, disc_id);
  const char *cd_id = splt_fu_freedb_get_disc_id(state, disc_id);

  splt_sm_connect(sh, server, port, state);
  if (sh->error < 0) { *error = sh->error; goto end; }

  if (get_type == SPLT_FREEDB_GET_FILE_TYPE_CDDB_CGI)
  {
    message = splt_su_get_formatted_message(state, 
        SPLT_FREEDB_CDDB_CGI_GET_FILE, cgi_path, cd_category, cd_id, NULL);

    splt_sm_send_http_message(sh, message, state);
    if (sh->error < 0) { *error = sh->error; goto disconnect; }

    splt_sm_receive_and_process_without_headers(sh, state,
        splt_freedb_process_get_file, get_file, DONT_SKIP_LINES);
    if (get_file->err < 0) { *error = get_file->err; goto disconnect; }
    if (sh->error < 0) { *error = sh->error; goto disconnect; }
  }
  else if (get_type == SPLT_FREEDB_GET_FILE_TYPE_CDDB)
  {
    get_file->stop_on_dot = SPLT_TRUE;

    splt_sm_send_http_message(sh, SPLT_FREEDB_HELLO, state);
    if (sh->error < 0) { *error = sh->error; goto disconnect; }

    splt_sm_receive_and_process(sh, state, splt_freedb_process_hello_response, &err);
    if (err < 0) { *error = err; goto disconnect; }
    if (sh->error < 0) { *error = sh->error; goto disconnect; }

    message = splt_su_get_formatted_message(state, SPLT_FREEDB_GET_FILE,
        cd_category, cd_id, NULL);

    splt_sm_send_http_message(sh, message, state);
    if (sh->error < 0) { *error = sh->error; goto disconnect; }

    splt_sm_receive_and_process(sh, state, splt_freedb_process_get_file, get_file);
    if (get_file->err < 0) { *error = get_file->err; goto disconnect; }
    if (sh->error < 0) { *error = sh->error; goto disconnect; }
  }

disconnect:
  splt_sm_close(sh, state); 
  if (sh->error < 0) { *error = sh->error; goto end; }

end:
  splt_sm_socket_handler_free(&sh);

  if (cgi_path)
  {
    free(cgi_path);
    cgi_path = NULL;
  }
  if (server)
  {
    free(server);
    server = NULL;
  }
  if (message)
  {
    free(message);
    message = NULL;
  }

  if (get_file)
  {
    char *file = get_file->file;

    free(get_file);
    get_file = NULL;

    return file;
  }

  return NULL;
}

  //deprecated, and not in use
  //but may useful for the implementation of the proxy
  /*int search_freedb (splt_state *state)
    {
    char *c, *e=NULL;
    FILE *output = NULL;
    struct sockaddr_in host;
    struct hostent *h;
    struct splt_addr dest;

    if ((c=getenv("HOME"))!=NULL) sprintf(message, "%s/"PROXYCONFIG, c);
    else strncpy(message, PROXYCONFIG, strlen(PROXYCONFIG));

    if (!(output=splt_io_fopen(message, "r"))) {
    if (!(output=splt_io_fopen(message, "w+"))) {
    fprintf(stderr, "\nWARNING Can't open config file ");
    perror(message);
    }
    else {
    fprintf (stderr, "Will you use a proxy? (y/n): ");
    fgets(junk, 200, stdin);
    if (junk[0]=='y') {
    fprintf (stderr, "Proxy Address: ");
    fgets(junk, 200, stdin);
    fprintf (output, "PROXYADDR=%s", junk);
    fprintf (stderr, "Proxy Port: ");
    fgets(junk, 200, stdin);
    fprintf (output, "PROXYPORT=%s", junk);
    fprintf (stderr, "Need authentication? (y/n): ");
    fgets(junk, 200, stdin);
    if (junk[0]=='y') {
    fprintf (output, "PROXYAUTH=1\n");
    fprintf (stderr, "Would you like to save password (insecure)? (y/n): ");
    fgets(junk, 200, stdin);
    if (junk[0]=='y') {
    login (message);
    e = b64(message, strlen(message));
    fprintf (output, "%s\n", e);
    memset(message, 0x00, strlen(message));
    memset(e, 0x00, strlen(e));
    free(e);
    }
    }
    }
    }
    }
        
    if (splt_fu_freedb_get_found_cds(state)<=0) {
    if (dest.proxy) {
    if (strstr(buffer, "HTTP/1.0")!=NULL) {
    if ((c = strchr (buffer, '\n'))!=NULL)
    buffer[c-buffer]='\0';
    fprintf (stderr, "Proxy Reply: %s\n", buffer);
    }
    }
    }
    return 0;
    }*/

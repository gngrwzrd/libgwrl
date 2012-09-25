
#include "gwrl/event.h"

#ifndef GWPR_PROACTOR_H
#define GWPR_PROACTOR_H

//on unix the maximum number of clients to accept in one reator event loop
//iteration. on windows the max amount of pre-allocated accept sockets
//made available to IOCP with AcceptEx.
#ifndef GWPR_MAX_ACCEPT
#	define GWPR_MAX_ACCEPT 256
#endif

//unix only, the maximum amount of struct gwprwrq to cache internally. when
//gwpr_write, or gwpr_send* methods are called it sticks a gwprwrq on a queue
//for writing. after that write happens the gwprwrq is then either cached for
//re-use later or freed depending on how many are already cached.
#ifndef GWPR_WRQUEUE_CACHE_MAX
#	define GWPR_WRQUEUE_CACHE_MAX 256
#endif

//windows only, the maximum amount of struct gwpr_ovlp to cache interanlly.
//when any IO operation happens a gwpr_ovlp structure is used for the IOCP
//function call. when that call completes the gwpr_ovlp is then cached for
//re-use later, or freed depending on how many are already cached.
#ifndef GWPR_IOCP_OVLP_CACHE_MAX
#	define GWPR_IOCP_OVLP_CACHE_MAX 256
#endif

//the maximum amount of filters allowed for a file input source.
#ifndef GWPR_FILTERS_MAX
#	define GWPR_FILTERS_MAX 0
#endif

//some forwards
struct gwpr;
struct gwprwrq;
struct gwpr_error_info;
struct gwpr_io_info;

//callback type for the gwpr_error_cb_id.
typedef void (gwpr_error_cb)(struct gwpr * pr, struct gwpr_error_info * info);

//callback type for any IO operation.
typedef void (gwpr_io_cb)(struct gwpr * pr, struct gwpr_io_info * info);

//default runtime options
#define GWPR_DEFAULT_OPTIONS {\
GWPR_MAX_ACCEPT,\
GWPR_WRQUEUE_CACHE_MAX,\
GWPR_IOCP_OVLP_CACHE_MAX,\
GWPR_SYNCHRONOUS_WRITE_MAX_BYTES}

//runtime options that can override some compile time #defines. useful for
//setting different options on a proactor that may need to be different
//than the default #define amounts.
typedef struct gwpr_options {
	int gwpr_max_accept;
	int gwpr_wrqueue_cache_max;
	int gwpr_iocp_ovlp_cache_max;
	int gwpr_synchronous_write_max_bytes;
} gwpr_options;

//ids for the "cbid" parameter to the gwpr_set_cb function.
typedef enum gwpr_cb_id {
	gwpr_error_cb_id,
	gwpr_accept_cb_id,
	gwpr_connect_cb_id,
	gwpr_closed_cb_id,
	gwpr_did_read_cb_id,
	gwpr_did_write_cb_id,
} gwpr_cb_id;

//IO operation ids. Because there are only one did_read and one
//did_write callback, you can use ioinfo->op to inspect which
//function it was that actually performed the read/write.
typedef enum gwpr_io_op_id {
	gwpr_read_op_id,
	gwpr_recv_op_id,
	gwpr_recvfrom_op_id,
	gwpr_write_op_id,
	gwpr_send_op_id,
	gwpr_sendto_op_id,
} gwpr_io_op_id;

//filter ids for gwpr_filter_add. when you add a filter these
//are used to control whether what you're adding is a read or
//write filter.
typedef enum gwpr_filter_id {
	gwpr_rdfilter_id,
	gwpr_wrfilter_id
} gwpr_filter_id;

//buffer for IO operations
typedef struct gwprbuf {
	int tag;
	size_t len;
	size_t bufsize;
	char * buf;
} gwprbuf;

//IO info structure passed to callbacks.
typedef struct gwpr_io_info {
	//the function id that initiated the IO.
	gwpr_io_op_id op;
	
	//general use peer, applies to accept/sendto/recvfrom/connect.
	struct sockaddr_storage peer;
	socklen_t peerlen;
	
	//data buffer that contains either newly read data, or the data
	//that was written in a write operation.
	gwprbuf * buf;
	
	//input source the IO operation completed on.
	gwrlsrc_file * src;
	
	//new peer input source. applies to accept/connect. this input source
	//has not been registered with the reactor runloop when you get it. It's
	//up to you to register it with a runloop. It's setup this way so you
	//can register the input source with a different runloop on another thread
	//if that's what you'd like.
	gwrlsrc_file * peersrc;
} gwpr_io_info;

//error info structure passed back to error callbacks.
typedef struct gwpr_error_info {
	int errnm;          //system error number
	char fnc[16];       //function that triggered the error
	gwpr_io_op_id op;   //io operation that triggered the error
	gwprbuf * buf;      //data buffer that didn't succeed
	gwrlsrc_file * src; //input source that generated the error
} gwpr_error_info;

//include some private and implementation specific headers.
#include "gwrl/proactor_private.h"
#ifdef PLATFORM_WINDOWS
#	include "gwrl/proactor_imp_iocp.h"
#else
#	include "gwrl/proactor_imp.h"
#endif

//buffers
gwprbuf * gwpr_buf_get(gwpr * pr, size_t bufsize);
gwprbuf * gwpr_buf_get_tagged(gwpr * pr, size_t bufsize, int tag);
gwprbuf * gwpr_buf_getp(gwpr * pr, size_t bufsize);
gwprbuf * gwpr_buf_get_with_data(gwpr * pr, size_t size, char * data, size_t datasize);
void gwpr_buf_free(gwpr * pr, gwprbuf * buf);

//create a new proactor
gwpr * gwpr_create(gwrl * rl);
void gwpr_free(gwpr * pr);

//shortcut for gwpr_src_add
gwrlsrc_file * gwpr_set_fd(gwpr * pr, fileid_t fd, void * userdata);

//set runtime override options
void gwpr_set_options(gwpr * pr, gwpr_options * opts);

//set callbacks for supported IO operations
void gwpr_set_cb(gwpr * pr, gwrlsrc_file * fsrc, gwpr_cb_id cbid, void * cb);

//add a file input source
void gwpr_src_add(gwpr * pr, gwrlsrc_file * fsrc);

//thread safe version of gwpr_src_add.
void gwpr_src_add_safely(gwpr * pr, gwrlsrc_file * fsrc);

//remove a file input source but don't free it
void gwpr_src_remove(gwpr * pr, gwrlsrc_file * fsrc);

//delete a file input source and free it
void gwpr_src_del(gwpr * pr, gwrlsrc_file * fsrc);

//initiate an asychronous connect
int gwpr_connect(gwpr * pr, gwrlsrc_file * fsrc, struct sockaddr_storage * addr);

//mark an input source as willing to accept incoming connections. The input
//source must be a socket that had already been listening and bound.
int gwpr_accept(gwpr * pr, gwrlsrc_file * fsrc);

//read data asynchronously into the provided buffer. Calling this once marks
//the input sources as willing to read whenever data is available. You have to
//specifically shut off reading when you don't want the proactor to read data
//on your behalf. Use the gwpr_stop_read method to stop read events at any time.
int gwpr_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf);
int gwpr_recv(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf);
int gwpr_recvfrom(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf);
void gwpr_stop_read(gwpr * pr, gwrlsrc_file * fsrc);

//write data asynchronously from the provided buffer. Unlike the read functions
//above, writes will only happen while there's data to write. After all the write data
//is written write events will be shut off for the input source until you initiate
//more writes.
int gwpr_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf);
int gwpr_send(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf);
int gwpr_sendto(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf, struct sockaddr_storage * peer, socklen_t peerlen);

//add filter at the end of the filter list for an input source.
void gwpr_filter_add(gwpr * pr, gwrlsrc_file * fsrc, gwpr_filter_id fid, gwpr_io_cb * fnc);

//remove all filters associated with an input source.
void gwpr_filter_reset(gwpr * pr, gwrlsrc_file * fsrc, gwpr_filter_id fid);

#endif

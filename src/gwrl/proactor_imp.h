
//proactor
typedef struct gwpr {
	size_t nwrqcache;
	struct gwpr_options options;
	gwrl * rl;
	gwprbufctl * bufctl;
	struct gwprwrq * wrqcache;
} gwpr;

//write queue
typedef struct gwprwrq {
	socklen_t peerlen;
	gwpr_io_op_id wrop;
	struct sockaddr_storage peer;
	gwprbuf * buf;
	struct gwprwrq * next;
} gwprwrq;

//proactor data for each gwrlsrc_file
typedef struct gwprdata {
	size_t rdbufsize;
	gwpr_io_op_id rdop;
	gwprbuf * rdbuf;
	struct gwprwrq * wrq;
	struct gwprwrq * wrqlast;
	gwpr_io_cb ** rdfilters;
	gwpr_io_cb ** wrfilters;
	gwpr_io_cb * acceptcb;
	gwpr_io_cb * connectcb;
	gwpr_io_cb * closedcb;
	gwpr_io_cb * didreadcb;
	gwpr_io_cb * didwritecb;
	gwpr_error_cb * errorcb;
} gwprdata;

gwprwrq * gwprwrq_get(gwpr * pr, gwrlsrc_file * fsrc);
void gwprwrq_free(gwpr * pr, gwrlsrc_file * fsrc, gwprwrq * wrq);
void gwprwrq_add(gwpr * pr, gwrlsrc_file * fsrc, gwprwrq * wrq);
void gwprwrq_putback(gwpr * pr, gwrlsrc_file * fsrc, gwprwrq * q);
void io_activity(gwpr * pr, gwpr_io_info * info);
void gwpr_src_activity(gwrl * rl, gwrlevt * evt);

void gwpr_write_buffer(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
     gwpr_io_op_id op, struct sockaddr_storage * peer, socklen_t peerlen,
     size_t * written, int * errnm);

bool gwpr_synchronous_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
	 gwpr_io_op_id op, struct sockaddr_storage * peer, socklen_t peerlen);

void gwpr_asynchronous_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
	 gwpr_io_op_id op, struct sockaddr_storage * peer, socklen_t peerlen);

int gwpr_asynchronous_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf,
	gwpr_io_op_id op);

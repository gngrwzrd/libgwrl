
//forwards
struct gwpr_ovlp;

//op codes for overlap structures with iocp. only used internally.
typedef enum gwpr_ovlp_op {
	gwpr_ovlp_op_wake,
	gwpr_ovlp_op_accept,
	gwpr_ovlp_op_connect,
	gwpr_ovlp_op_read,
	gwpr_ovlp_op_recv,
	gwpr_ovlp_op_recvfrom,
	gwpr_ovlp_op_write,
	gwpr_ovlp_op_send,
	gwpr_ovlp_op_sendto
} gwpr_ovlp_op;

//proactor
typedef struct gwpr {
	size_t novlpcache;
	gwrl * rl;
	gwprbufctl * bufctl;
	struct gwpr_ovlp * ovlpcache;
	struct gwpr_options options;
} gwpr;

//overlapped io for completion port
typedef struct gwpr_ovlp {
	OVERLAPPED _;
	uint8_t op;
	gwpr * pr;
	gwrlsrc_file * src;
	gwprbuf * buf;
	WSABUF wsabuf;
	socklen_t peerlen;
	struct sockaddr_storage peer;
	fileid_t acceptsock;
	struct gwpr_ovlp * next;
} gwpr_ovlp;

//proactor data for each gwrlsrc_file
typedef struct gwprdata {
	int accept_count;
	size_t rdbufsize;
	gwprbuf * rdbuf;
	gwpr_io_cb ** rdfilters;
	gwpr_io_cb ** wrfilters;
	gwpr_ovlp * rdovlp;
	gwpr_io_cb * acceptcb;
	gwpr_io_cb * connectcb;
	gwpr_io_cb * closedcb;
	gwpr_io_cb * didreadcb;
	gwpr_io_cb * didwritecb;
	gwpr_error_cb * errorcb;
} gwprdata;

gwpr_ovlp * gwpr_ovlp_get(gwpr * pr);
void gwpr_ovlp_free(gwpr * pr, gwpr_ovlp * ovlp);
int gwpr_asynchronous_read(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf, gwpr_ovlp_op op);
int gwpr_asynchronous_write(gwpr * pr, gwrlsrc_file * fsrc, gwprbuf * buf, struct sockaddr_storage * peer, socklen_t peerlen, gwpr_ovlp_op op);
#ifndef _SCOUTFS_NET_H_
#define _SCOUTFS_NET_H_

#include <linux/in.h>
#include "endian_swap.h"

#define SIN_FMT		"%pIS:%u"
#define SIN_ARG(sin)	sin, be16_to_cpu((sin)->sin_port)

static inline void scoutfs_addr_to_sin(struct sockaddr_in *sin,
				       struct scoutfs_inet_addr *addr)
{
	sin->sin_family = AF_INET;
	sin->sin_addr.s_addr = cpu_to_be32(le32_to_cpu(addr->addr));
	sin->sin_port = cpu_to_be16(le16_to_cpu(addr->port));
}

static inline void scoutfs_addr_from_sin(struct scoutfs_inet_addr *addr,
					 struct sockaddr_in *sin)
{
	addr->addr = be32_to_le32(sin->sin_addr.s_addr);
	addr->port = be16_to_le16(sin->sin_port);
}

struct scoutfs_net_connection;

/* These are called in their own blocking context */
typedef int (*scoutfs_net_request_t)(struct super_block *sb,
				     struct scoutfs_net_connection *conn,
				     u8 cmd, u64 id, void *arg, u16 arg_len);

/* These are called in their own blocking context */
typedef int (*scoutfs_net_response_t)(struct super_block *sb,
				      struct scoutfs_net_connection *conn,
				      void *resp, unsigned int resp_len,
				      int error, void *data);

typedef void (*scoutfs_net_notify_t)(struct super_block *sb,
				     struct scoutfs_net_connection *conn,
				     void *info, u64 node_id);

struct scoutfs_net_connection *
scoutfs_net_alloc_conn(struct super_block *sb,
		       scoutfs_net_notify_t notify_up,
		       scoutfs_net_notify_t notify_down, size_t info_size,
		       scoutfs_net_request_t *req_funcs, char *name_suffix);
u64 scoutfs_net_client_node_id(struct scoutfs_net_connection *conn);
int scoutfs_net_connect(struct super_block *sb,
			struct scoutfs_net_connection *conn,
			struct sockaddr_in *sin, unsigned long timeout_ms);
int scoutfs_net_bind(struct super_block *sb,
		     struct scoutfs_net_connection *conn,
		     struct sockaddr_in *sin);
void scoutfs_net_listen(struct super_block *sb,
			struct scoutfs_net_connection *conn);
int scoutfs_net_submit_request(struct super_block *sb,
			       struct scoutfs_net_connection *conn,
			       u8 cmd, void *arg, u16 arg_len,
			       scoutfs_net_response_t resp_func,
			       void *resp_data, u64 *id_ret);
int scoutfs_net_submit_request_node(struct super_block *sb,
				    struct scoutfs_net_connection *conn,
				    u64 node_id, u8 cmd,
				    void *arg, u16 arg_len,
				    scoutfs_net_response_t resp_func,
				    void *resp_data, u64 *id_ret);
void scoutfs_net_cancel_request(struct super_block *sb,
				struct scoutfs_net_connection *conn,
				u8 cmd, u64 id);
int scoutfs_net_sync_request(struct super_block *sb,
			     struct scoutfs_net_connection *conn,
			     u8 cmd, void *arg, unsigned arg_len,
			     void *resp, size_t resp_len);
int scoutfs_net_response(struct super_block *sb,
			 struct scoutfs_net_connection *conn,
			 u8 cmd, u64 id, int error, void *resp, u16 resp_len);
int scoutfs_net_response_node(struct super_block *sb,
			      struct scoutfs_net_connection *conn,
			      u64 node_id, u8 cmd, u64 id, int error,
			      void *resp, u16 resp_len);
void scoutfs_net_shutdown(struct super_block *sb,
			  struct scoutfs_net_connection *conn);
void scoutfs_net_free_conn(struct super_block *sb,
			   struct scoutfs_net_connection *conn);

void scoutfs_net_client_greeting(struct super_block *sb,
				 struct scoutfs_net_connection *conn,
				 bool new_server);
void scoutfs_net_server_greeting(struct super_block *sb,
				 struct scoutfs_net_connection *conn,
				 u64 node_id, u64 greeting_id,
				 bool sent_node_id, bool first_contact,
				 bool farewell);
void scoutfs_net_farewell(struct super_block *sb,
			  struct scoutfs_net_connection *conn);

int scoutfs_net_setup(struct super_block *sb);
void scoutfs_net_destroy(struct super_block *sb);

#endif

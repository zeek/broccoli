#include <stdio.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <broccoli.h>

void bro_addr_cb(BroConn* bc, void* user_data, BroAddr* a)
	{
	char addr[INET6_ADDRSTRLEN];
	inet_ntop(a->size == 1 ? AF_INET : AF_INET6, a->addr, addr,
			INET6_ADDRSTRLEN);
	printf("Received bro_addr(%s)\n", addr);

	BroEvent* event;
	event = bro_event_new("broccoli_addr");
	bro_event_add_val(event, BRO_TYPE_IPADDR, 0, a);
	bro_event_send(bc, event);
	bro_event_free(event);
	}

void bro_subnet_cb(BroConn* bc, void* user_data, BroSubnet* s)
	{
	char addr[INET6_ADDRSTRLEN];
	inet_ntop(s->sn_net.size == 1 ? AF_INET : AF_INET6, s->sn_net.addr, addr,
			INET6_ADDRSTRLEN);
	printf("Received bro_subnet(%s/%"PRIu32")\n", addr, s->sn_width);

	BroEvent* event;
	event = bro_event_new("broccoli_subnet");
	bro_event_add_val(event, BRO_TYPE_SUBNET, 0, s);
	bro_event_send(bc, event);
	bro_event_free(event);
	}

int main(int argc, char** argv)
	{
	BroConn* bc;
	char* host = "localhost:47757";

	bro_init(0);

	if ( ! (bc = bro_conn_new_str(host, BRO_CFLAG_NONE)) )
		{
		fprintf(stderr, "failed to get connection handle for %s\n", host);
		return 1;
		}

	bro_event_registry_add(bc, "bro_addr", (BroEventFunc)bro_addr_cb, 0);
	bro_event_registry_add(bc, "bro_subnet", (BroEventFunc)bro_subnet_cb, 0);

	if ( ! bro_conn_connect(bc) )
		{
		fprintf(stderr, "failed to connect to %s\n", host);
		return 1;
		}

	printf("Connected to Bro instance at: %s\n", host);

	int fd = bro_conn_get_fd(bc);
	fd_set fds;
	FD_ZERO(&fds);
	FD_SET(fd, &fds);

	while ( select(fd+1, &fds, 0, 0, 0) > 0 )
		bro_conn_process_input(bc);

	printf("Terminating\n");
	bro_conn_delete(bc);
	return 0;
	}

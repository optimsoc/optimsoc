#include <optimsoc-baremetal.h>
#include <optimsoc-mp.h>

#include <assert.h>


#define DATA_SIZE 1024*16

void sender(uint32_t receiver_tile) {
  uint32_t size, rsize;
  int rv;

  optimsoc_mp_endpoint_handle ep_send_local;
  optimsoc_mp_endpoint_handle ep_send_remote;
  optimsoc_mp_endpoint_handle ep_recv_local;
  optimsoc_mp_endpoint_handle ep_recv_remote;
  
  printf("Sender\n");
  printf("Create local endpoint for sending\n");
  rv = optimsoc_mp_endpoint_create(&ep_send_local, 0, 0,
				   OPTIMSOC_MP_EP_CHANNEL, 1, DATA_SIZE);
  assert(rv==0);

  printf("Create local endpoint for receiving\n");
  rv = optimsoc_mp_endpoint_create(&ep_recv_local, 0, 1,
				   OPTIMSOC_MP_EP_CHANNEL, 1, DATA_SIZE);
  assert(rv==0);

  printf("Get remote endpoint for sending\n");
  rv = optimsoc_mp_endpoint_get(&ep_send_remote, receiver_tile, 0, 0);
  assert(rv==0);
  
  printf("Get remote endpoint for receiving\n");
  rv = optimsoc_mp_endpoint_get(&ep_recv_remote, receiver_tile, 0, 1);
  assert(rv==0);

  printf("Connect channel\n");
  rv = optimsoc_mp_channel_connect(ep_send_local, ep_send_remote);
  assert(rv==0);

  uint8_t *data = malloc(DATA_SIZE);
  assert(data);

  for (int size = 4; size <= DATA_SIZE; size = size << 1) {  
    printf("Start %u\n", size);
    optimsoc_mp_channel_send(ep_send_local, ep_send_remote, data, size);
    printf("Send complete\n");
    optimsoc_mp_channel_recv(ep_recv_local, data, 512*DATA_SIZE, &rsize);
    assert(rsize == size);
    printf("Finished %u\n", size);
  }
}

void receiver() {
  int rv;
  uint32_t size, rsize;

  optimsoc_mp_endpoint_handle ep_send_local;
  optimsoc_mp_endpoint_handle ep_send_remote;
  optimsoc_mp_endpoint_handle ep_recv_local;
  optimsoc_mp_endpoint_handle ep_recv_remote;

  rv = optimsoc_mp_endpoint_create(&ep_send_local, 0, 1,
				   OPTIMSOC_MP_EP_CHANNEL, 2, DATA_SIZE);
  assert(rv==0);

  rv = optimsoc_mp_endpoint_create(&ep_recv_local, 0, 0,
				   OPTIMSOC_MP_EP_CHANNEL, 2, DATA_SIZE);
  assert(rv==0);

  rv = optimsoc_mp_endpoint_get(&ep_send_remote, 0, 0, 1);
  assert(rv==0);
  
  rv = optimsoc_mp_endpoint_get(&ep_recv_remote, 0, 0, 0);
  assert(rv==0);

  rv = optimsoc_mp_channel_connect(ep_send_local, ep_send_remote);
  assert(rv==0);

  uint8_t *data = malloc(DATA_SIZE);

  while (1) {
    optimsoc_mp_channel_recv(ep_recv_local, data, DATA_SIZE, &rsize);
    printf("Received %d\n", rsize);
    optimsoc_mp_channel_send(ep_send_local, ep_send_remote, data, rsize);
  }
}

int main(int argc, char* argv[]) {
  optimsoc_init(0);

  unsigned int rank = optimsoc_ctrank();
  unsigned int ranktotal = optimsoc_ctnum();

  optimsoc_mp_initialize(0);
  or1k_interrupts_enable();

  if (rank == 0) {
    sender(ranktotal-1);
  } else if (rank == (ranktotal-1)) {
    receiver();
  }
  
  return 0;
}

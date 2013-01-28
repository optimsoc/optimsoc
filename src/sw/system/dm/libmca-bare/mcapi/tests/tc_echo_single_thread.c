#include <mcapi.h>

char status_buff[MCAPI_MAX_STATUS_SIZE];
#include <mcapi_impl_spec.h>
#include <stdio.h>
#include <stdlib.h> /* for malloc */
#include <string.h>
#include <mca_config.h>

#ifndef NODE
#define NODE 1
#endif

#ifndef DOMAIN
#define DOMAIN 1
#endif
#define WRONG wrong(__LINE__,status);
void wrong(unsigned line,mca_status_t status)
{
  mca_status_t mcapi_status;
  mcapi_finalize(&mcapi_status);
  fprintf(stderr,"WRONG: line=%u\n", line);
  fprintf(stderr,"status=%s",mcapi_display_status(status,status_buff,sizeof(status_buff)));

  fflush(stdout);
  exit(1);
}


#define PORT_NUM1 100
#define PORT_NUM2 200

#define BUFF_SIZE 256

void scl_send (mcapi_sclchan_send_hndl_t send_handle, mcapi_endpoint_t recv,int data,uint32_t size) {

  mca_status_t status;
  switch (size) {
  case (8): mcapi_sclchan_send_uint8(send_handle,data,&status); break;
  case (16): mcapi_sclchan_send_uint16(send_handle,data,&status); break;
  case (32): mcapi_sclchan_send_uint32(send_handle,data,&status); break;
  case (64): mcapi_sclchan_send_uint64(send_handle,data,&status); break;
  default: fprintf (stderr,"ERROR: bad data size in call to send\n");
  };
  if (status == MCAPI_SUCCESS) {
    printf ("send %d\n",data);
  } else { WRONG }
}

void scl_recv (mcapi_sclchan_recv_hndl_t recv_handle,uint32_t size,unsigned long long exp_data) {
  int data = 0;

  uint64_t size_mask = 0;
  mca_status_t status;
  switch (size) {
  case (8): size_mask = 0xff;data=mcapi_sclchan_recv_uint8(recv_handle,&status); break;
  case (16): size_mask = 0xffff;data=mcapi_sclchan_recv_uint16(recv_handle,&status); break;
  case (32): size_mask = 0xffffffff;data=mcapi_sclchan_recv_uint32(recv_handle,&status); break;
  case (64): size_mask = 0xffffffffffffffffULL;data=mcapi_sclchan_recv_uint64(recv_handle,&status); break;
  default: fprintf (stderr,"ERROR: bad data size in call to send\n");
  };

  
  if (status == MCAPI_SUCCESS) {
    printf ("recv %d\n",data);
  } else { WRONG }
  
}

void msg_send (mcapi_endpoint_t send, mcapi_endpoint_t recv,char* msg) {
  mca_status_t status;
  int size = strlen(msg);
  int priority = 1;
  mcapi_msg_send(send,recv,msg,size,priority,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  printf ("send %s\n",msg);
}


void msg_recv (mcapi_endpoint_t recv,char* exp_msg,size_t exp_size) {
  mca_status_t status;
  size_t recv_size;
  char buffer[BUFF_SIZE];

  memset(buffer,0,BUFF_SIZE); 
  mcapi_msg_recv(recv,buffer,BUFF_SIZE,&recv_size,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  if (strcmp(buffer,exp_msg) != 0) { printf("error: exp[%s] received[%s]\n",exp_msg,buffer); WRONG  }
  if (recv_size != exp_size) { WRONG }
  printf("received %s\n",buffer);
}



int main () {
  mca_status_t status;
  mcapi_param_t parms;
  mcapi_info_t version;
  mcapi_endpoint_t ep1,ep2;    // message endpoints
  mcapi_endpoint_t ep3,ep4,ep5,ep6;     // scalar endpoints

  mcapi_sclchan_recv_hndl_t r4_recv,r6_recv;
  mcapi_sclchan_send_hndl_t r3_send,r5_send;
  mca_request_t request;
  //  mca_boolean_t msgs = MCA_FALSE;
  //mca_boolean_t scalars = MCA_TRUE;

  int i;
  int num_sends = 1000;
  char msg[BUFF_SIZE];
  size_t size = 0;

  /* create a node */
  mcapi_initialize(DOMAIN,NODE,NULL,&parms,&version,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  mcapi_set_debug_level(6);

  ep1 = mcapi_endpoint_create(1,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep2 = mcapi_endpoint_create(2,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep3 = mcapi_endpoint_create(3,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep4 = mcapi_endpoint_create(4,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep5 = mcapi_endpoint_create(5,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  ep6 = mcapi_endpoint_create(6,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  // messages - 4 nodes
  for (i = 0; i < num_sends; i++) {
    sprintf(msg ,"%d",i);
    printf ("********* %d **********\n",i);
    // 1 --> 2
    msg_send(ep1,ep2,msg);
    msg_recv(ep2,msg,strlen(msg));
    
    // 2 --> 3
    msg_send(ep2,ep3,msg);
    msg_recv(ep3,msg,strlen(msg)); 
    
    // 3 --> 4
    msg_send(ep3,ep4,msg);
    msg_recv(ep4,msg,strlen(msg));
    
    // 4 --> 1
    msg_send(ep4,ep1,msg);
    msg_recv(ep1,msg,strlen(msg));

  } 
  
  // scalars   

  if (0) {
  /*  3 ----> 4 */
  mcapi_sclchan_connect_i(ep3,ep4,&request, &status);
  if (status != MCAPI_SUCCESS) { WRONG }
  mcapi_wait(&request,&size,TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  mcapi_sclchan_send_open_i(&r3_send /*send_handle*/,ep3, &request, &status);
  if (status != MCAPI_SUCCESS) { WRONG }
  mcapi_wait(&request,&size,TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  mcapi_sclchan_recv_open_i(&r4_recv /*recv_handle*/,ep4, &request, &status);
  if (status != MCAPI_SUCCESS) { WRONG }
  mcapi_wait(&request,&size,TIMEOUT,&status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  /* 5 ------> 6 */
  mcapi_sclchan_connect_i(ep5,ep6,&request, &status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  mcapi_sclchan_send_open_i(&r5_send /*recv_handle*/,ep5, &request, &status);
  if (status != MCAPI_SUCCESS) { WRONG }
  
  mcapi_sclchan_recv_open_i(&r6_recv /*recv_handle*/,ep6, &request, &status);
  if (status != MCAPI_SUCCESS) { WRONG }

  for (i = 0; i < num_sends; i++) { 
    int size = 32;
    sprintf(msg ,"%d",i);
    scl_send(r3_send,ep4,i,size);
    scl_recv(r4_recv,size,i);
    scl_send(r5_send,ep6,i,size);
    scl_recv(r6_recv,size,i);    
  } 
  }
  
  mcapi_finalize(&status);
  if (status != MCAPI_SUCCESS) { WRONG }

  printf("   Test PASSED\n");
  return 0;
}
  


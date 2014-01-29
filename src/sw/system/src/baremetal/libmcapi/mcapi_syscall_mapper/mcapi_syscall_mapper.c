
#include <mcapi_syscall_mapper.h>
#include <mcapi_syscalls.h>
#include <mcapi_trans.h>
#include <or1k-support.h>

#include <assert.h>
#include <stdio.h>

#include <list.h>
#include <thread.h>
#include <scheduler.h>
#include <trace.h>
#include <optimsoc-baremetal.h>

extern void* vmm_virt2phys(thread_t thread, void* vaddr, size_t size);

#if 0
#define DBG(...) printf(__VA_ARGS__)
#else
#define DBG(...)
#endif

struct list_t* mcapi_requests;

struct mcapi_req_entry_t {
	unsigned int req_id;
	void *arg;
	thread_t thread;
};


void mcapi_syscall(unsigned int subid, void *arg);
void mcapi_sys_func(void *arg);

void mcapi_syscall_mapper_init(){

	thread_t mcapi_syscall_service;

	printf("Initialize MCAPI syscall mapper\n");

	mcapi_requests = list_init(NULL);

	thread_attr_t *thread_attr = malloc(sizeof(thread_attr_t));

	thread_attr_init(thread_attr);
	thread_attr->identifier = "MCAPI syscall service";

	thread_create(&mcapi_syscall_service,
		      &mcapi_sys_func,
		      thread_attr);

	or1k_syscall_handler_add(0x1, mcapi_syscall);
}

void mcapi_sys_func(void *arg){
	struct mcapi_req_entry_t *req;
	void *mcapi_param;
	void *arg1, *arg2;
	while(1){
		req = (struct mcapi_req_entry_t*)list_remove_head(mcapi_requests);
		if(req){

			switch(req->req_id){
			case MCAPI_TRANS_ENDPOINT_GET:
				mcapi_param = vmm_virt2phys(req->thread, req->arg, sizeof(struct mcapi_trans_endpoint_get));
				assert(mcapi_param != 0);

				arg1 = vmm_virt2phys(req->thread,
					((struct mcapi_trans_endpoint_get*)mcapi_param)->endpoint,
					sizeof(mcapi_endpoint_t));

				mcapi_trans_endpoint_get(
					(mcapi_endpoint_t*)arg1,
					((struct mcapi_trans_endpoint_get*)mcapi_param)->domain_id,
					((struct mcapi_trans_endpoint_get*)mcapi_param)->node_num,
					((struct mcapi_trans_endpoint_get*)mcapi_param)->port_num);
				DBG("mcapi_trans_endpoint_get(%p -> %d, %d, %d, %d)\n",
				    (mcapi_endpoint_t*)arg1,
				    *((mcapi_endpoint_t*)arg1),
				    ((struct mcapi_trans_endpoint_get*)mcapi_param)->domain_id,
				    ((struct mcapi_trans_endpoint_get*)mcapi_param)->node_num,
				    ((struct mcapi_trans_endpoint_get*)mcapi_param)->port_num);
				break;
			case MCAPI_TRANS_MSG_RECV:
				mcapi_param = vmm_virt2phys(req->thread, req->arg, sizeof(struct mcapi_trans_msg_recv));
				assert(mcapi_param != 0);

				arg1 = vmm_virt2phys(req->thread,
						     ((struct mcapi_trans_msg_recv*)mcapi_param)->buffer,
						     ((struct mcapi_trans_msg_recv*)mcapi_param)->buffer_size);

				arg2 = vmm_virt2phys(req->thread,
						     ((struct mcapi_trans_msg_recv*)mcapi_param)->received_size,
						     sizeof(size_t));

				((struct mcapi_trans_msg_recv*)mcapi_param)->ret =
					mcapi_trans_msg_recv(
						((struct mcapi_trans_msg_recv*)mcapi_param)->receive_endpoint,
						(char*)arg1,
						((struct mcapi_trans_msg_recv*)mcapi_param)->buffer_size,
						(size_t*)arg2);
				DBG("mcapi_trans_msg_recv(%d, %p, %d, %p -> %d) = %d\n",
				    ((struct mcapi_trans_msg_recv*)mcapi_param)->receive_endpoint,
				    (char*)arg1,
				    ((struct mcapi_trans_msg_recv*)mcapi_param)->buffer_size,
				    (size_t*)arg2,
				    *((size_t*)arg2),
				    ((struct mcapi_trans_msg_recv*)mcapi_param)->ret);
				break;
			case MCAPI_TRANS_MSG_SEND:
				mcapi_param = vmm_virt2phys(req->thread, req->arg, sizeof(struct mcapi_trans_msg_send));
				assert(mcapi_param != 0);

				arg1 = vmm_virt2phys(req->thread,
					((struct mcapi_trans_msg_send*)mcapi_param)->buffer,
					((struct mcapi_trans_msg_send*)mcapi_param)->buffer_size);

				((struct mcapi_trans_msg_send*)mcapi_param)->ret =
					mcapi_trans_msg_send(
						((struct mcapi_trans_msg_send*)mcapi_param)->send_endpoint,
						((struct mcapi_trans_msg_send*)mcapi_param)->receive_endpoint,
						(char*)arg1,
						((struct mcapi_trans_msg_send*)mcapi_param)->buffer_size);
				DBG("mcapi_trans_msg_send(%d, %d, %p, %d) = %d\n",
				    ((struct mcapi_trans_msg_send*)mcapi_param)->send_endpoint,
				    ((struct mcapi_trans_msg_send*)mcapi_param)->receive_endpoint,
				    (char*)arg1,
				    ((struct mcapi_trans_msg_send*)mcapi_param)->buffer_size,
				    ((struct mcapi_trans_msg_send*)mcapi_param)->ret);
				break;
			default:
				printf("MCAPI syscall service: req_id %d not implemented\n", req->req_id);
			}
			thread_resume(req->thread);
			free(req);
		}
		thread_yield();
	}
}


void mcapi_syscall(unsigned int subid, void *arg){
    struct optimsoc_scheduler_core *core_ctx;
    core_ctx = &optimsoc_scheduler_core[optimsoc_get_domain_coreid()];

	assert(arg != 0);

	void *mcapi_param;
	void *arg1, *arg2;
	struct mcapi_req_entry_t *req;

	switch(subid){

		// Blocking MCAPI calls
	case MCAPI_TRANS_ENDPOINT_GET:
	case MCAPI_TRANS_MSG_RECV:
	case MCAPI_TRANS_MSG_SEND:
		req = malloc(sizeof(struct mcapi_req_entry_t));
		assert(req != 0);
		req->req_id = subid;
		req->arg = arg;
		req->thread = core_ctx->active_thread;
		list_add_tail(mcapi_requests, (void*)req);

		memcpy(core_ctx->active_thread->ctx, exception_ctx, sizeof(struct arch_thread_ctx_t));
		core_ctx->active_thread->state = THREAD_SUSPENDED;
		scheduler_add(core_ctx->active_thread, wait_q);
		runtime_trace_suspend(core_ctx->active_thread->id);
		schedule();

		break;

		// Non-blocking MCAPI calls
	case MCAPI_TRANS_ENDPOINT_CREATE:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_endpoint_create));
		assert(mcapi_param != 0);

		arg1 = vmm_virt2phys(core_ctx->active_thread,
			((struct mcapi_trans_endpoint_create*)mcapi_param)->endpoint,
			sizeof(mcapi_endpoint_t));

		((struct mcapi_trans_endpoint_create*)mcapi_param)->ret =
			mcapi_trans_endpoint_create(
				(mcapi_endpoint_t*)arg1,
				((struct mcapi_trans_endpoint_create*)mcapi_param)->port_num,
				((struct mcapi_trans_endpoint_create*)mcapi_param)->anonymous
				);
		DBG("mcapi_trans_endpoint_create(%p -> %d, %d, %d) = %d\n",
		    (mcapi_endpoint_t*)arg1,
		    *((mcapi_endpoint_t*)arg1),
		    ((struct mcapi_trans_endpoint_create*)mcapi_param)->port_num,
		    ((struct mcapi_trans_endpoint_create*)mcapi_param)->anonymous,
		    ((struct mcapi_trans_endpoint_create*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_ENDPOINT_EXISTS:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_endpoint_exists));
		assert(mcapi_param != 0);

		((struct mcapi_trans_endpoint_exists*)mcapi_param)->ret =
			mcapi_trans_endpoint_exists(
				((struct mcapi_trans_endpoint_exists*)mcapi_param)->domain_id,
				((struct mcapi_trans_endpoint_exists*)mcapi_param)->port_num);
		DBG("mcapi_trans_endpoint_exists(%d, %d) = %d\n",
		    ((struct mcapi_trans_endpoint_exists*)mcapi_param)->domain_id,
		    ((struct mcapi_trans_endpoint_exists*)mcapi_param)->port_num,
		    ((struct mcapi_trans_endpoint_exists*)mcapi_param)->ret);
		break;

	case MCAPI_TRANS_GET_DOMAIN_NUM:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_get_domain_num));
		assert(mcapi_param != 0);

		arg1 = vmm_virt2phys(core_ctx->active_thread,
			((struct mcapi_trans_get_domain_num*)mcapi_param)->domain_num,
			sizeof(mcapi_node_t));

		((struct mcapi_trans_get_domain_num*)mcapi_param)->ret =
			mcapi_trans_get_domain_num((mcapi_node_t*)arg1);
		DBG("mcapi_trans_get_domain_num(%p -> %d) = %d\n",
		    (mcapi_node_t*)arg1,
		    *((mcapi_node_t*)arg1),
		    ((struct mcapi_trans_get_domain_num*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_GET_NODE_NUM:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_get_node_num));
		assert(mcapi_param != 0);

		arg1 = vmm_virt2phys(core_ctx->active_thread,
			((struct mcapi_trans_get_node_num*)mcapi_param)->node_num,
			sizeof(mcapi_node_t));

		((struct mcapi_trans_get_node_num*)mcapi_param)->ret =
			mcapi_trans_get_node_num((mcapi_node_t*)arg1);
		DBG("mcapi_trans_get_node_num(%p -> %d) = %d\n",
		    (mcapi_node_t*)arg1,
		    *((mcapi_node_t*)arg1),
		    ((struct mcapi_trans_get_node_num*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_INITIALIZE:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_initialize));
		assert(mcapi_param != 0);
		/* TODO handling of node_attrs parameter */
		((struct mcapi_trans_initialize*)mcapi_param)->ret =
			mcapi_trans_initialize(
				((struct mcapi_trans_initialize*)mcapi_param)->domain_id,
				((struct mcapi_trans_initialize*)mcapi_param)->node_num,
				0);
		DBG("mcapi_trans_initialize(%d, %d, %d) = %d\n",
		    ((struct mcapi_trans_initialize*)mcapi_param)->domain_id,
		    ((struct mcapi_trans_initialize*)mcapi_param)->node_num,
		    0,
		    ((struct mcapi_trans_initialize*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_INITIALIZED:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_initialized));
		assert(mcapi_param != 0);
		((struct mcapi_trans_initialized*)mcapi_param)->ret =
			mcapi_trans_initialized(
				((struct mcapi_trans_initialized*)mcapi_param)->domain_id,
				((struct mcapi_trans_initialized*)mcapi_param)->node_id);
		DBG("mcapi_trans_initialized(%d, %d) = %d\n",
		    ((struct mcapi_trans_initialized*)mcapi_param)->domain_id,
		    ((struct mcapi_trans_initialized*)mcapi_param)->node_id,
		    ((struct mcapi_trans_initialized*)mcapi_param)->ret);
		break;

	case MCAPI_TRANS_NUM_ENDPOINTS:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_num_endpoints));
		assert(mcapi_param != 0);

		((struct mcapi_trans_num_endpoints*)mcapi_param)->ret =
			mcapi_trans_num_endpoints();
		DBG("mcapi_trans_num_endpoints() = %d\n",
		    ((struct mcapi_trans_num_endpoints*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_MSG_AVAILABLE:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_msg_available));
		assert(mcapi_param != 0);

		((struct mcapi_trans_msg_available*)mcapi_param)->ret =
			mcapi_trans_msg_available(
				((struct mcapi_trans_msg_available*)mcapi_param)->receive_endpoint);
		DBG("mcapi_trans_msg_available(%d) = %d\n",
		    ((struct mcapi_trans_msg_available*)mcapi_param)->receive_endpoint,
		    ((struct mcapi_trans_msg_available*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_SET_NODE_NUM:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_set_node_num));
		assert(mcapi_param != 0);
		((struct mcapi_trans_set_node_num*)mcapi_param)->ret =
			mcapi_trans_set_node_num(
				((struct mcapi_trans_set_node_num*)mcapi_param)->node_num);
		DBG("mcapi_trans_set_node_num(%d) = %d\n",
		    ((struct mcapi_trans_set_node_num*)mcapi_param)->node_num,
		    ((struct mcapi_trans_set_node_num*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_VALID_ENDPOINT:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_valid_endpoint));
		assert(mcapi_param != 0);
		((struct mcapi_trans_valid_endpoint*)mcapi_param)->ret =
			mcapi_trans_valid_endpoint(
				((struct mcapi_trans_valid_endpoint*)mcapi_param)->endpoint);
		DBG("mcapi_trans_valid_endpoint(%d) = %d\n",
		    ((struct mcapi_trans_valid_endpoint*)mcapi_param)->endpoint,
		    ((struct mcapi_trans_valid_endpoint*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_VALID_ENDPOINTS:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_valid_endpoints));
		assert(mcapi_param != 0);
		((struct mcapi_trans_valid_endpoints*)mcapi_param)->ret =
			mcapi_trans_valid_endpoints(
				((struct mcapi_trans_valid_endpoints*)mcapi_param)->endpoint1,
				((struct mcapi_trans_valid_endpoints*)mcapi_param)->endpoint2);
		DBG("mcapi_trans_valid_endpoints(%d, %d) = %d\n",
		    ((struct mcapi_trans_valid_endpoints*)mcapi_param)->endpoint1,
		    ((struct mcapi_trans_valid_endpoints*)mcapi_param)->endpoint2,
		    ((struct mcapi_trans_valid_endpoints*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_VALID_NODE:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_valid_node));
		assert(mcapi_param != 0);
		((struct mcapi_trans_valid_node*)mcapi_param)->ret =
			mcapi_trans_valid_node(
				((struct mcapi_trans_valid_node*)mcapi_param)->node_num);
		DBG("mcapi_trans_valid_node(%d) = %d\n",
		    ((struct mcapi_trans_valid_node*)mcapi_param)->node_num,
		    ((struct mcapi_trans_valid_node*)mcapi_param)->ret);
		break;
	case MCAPI_TRANS_VALID_PORT:
		mcapi_param = vmm_virt2phys(core_ctx->active_thread, arg, sizeof(struct mcapi_trans_valid_port));
		assert(mcapi_param != 0);
		((struct mcapi_trans_valid_port*)mcapi_param)->ret =
			mcapi_trans_valid_port(
				((struct mcapi_trans_valid_port*)mcapi_param)->port_num);
		DBG("mcapi_trans_valid_port(%d) = %d\n",
		    ((struct mcapi_trans_valid_port*)mcapi_param)->port_num,
		    ((struct mcapi_trans_valid_port*)mcapi_param)->ret);
		break;

	default:
		printf("Unimplemented MCAPI syscall\n");
		break;
	}

}



# Copyright 2017-2019 The Open SoC Debug Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

from cutil cimport va_list
from libc.stdint cimport uint8_t, uint16_t, uint64_t
from libc.stdio cimport FILE
from posix.time cimport timespec


cdef extern from "osd/osd.h" nogil:
    ctypedef int osd_result

    struct osd_log_ctx:
        pass

    cdef struct osd_version:
        const uint16_t major
        const uint16_t minor
        const uint16_t micro
        const char *suffix

    ctypedef void (*osd_log_fn)(osd_log_ctx *ctx,
                                int priority, const char *file,
                                int line, const char *fn,
                                const char *format, va_list args)

    osd_result osd_log_new(osd_log_ctx **ctx, int log_priority,
                           osd_log_fn log_fn)

    void osd_log_free(osd_log_ctx **ctx)

    void osd_log_set_fn(osd_log_ctx *ctx, osd_log_fn log_fn)

    void osd_log_set_caller_ctx(osd_log_ctx *ctx, void *caller_ctx)

    void* osd_log_get_caller_ctx(osd_log_ctx *ctx)

    const osd_version* osd_version_get()



cdef extern from "osd/packet.h" nogil:
    # helper struct to avoid anonymous in-line structs, which are not well
    # supported by Cython
    struct _osd_packet_data_struct:
        uint16_t dest
        uint16_t src
        uint16_t flags
        uint16_t *payload

    struct osd_packet:
        uint16_t data_size_words
        # Note that the two fields below are actually an union referencing the
        # same memory. Cython doesn't support anonymous unions as of v0.28,
        # therefore we use this workaround. It works because Cython doesn't
        # make assumptions about the memory layout, it only calls into the C
        # code with the given member names.
        _osd_packet_data_struct data
        uint16_t *data_raw

    cdef enum osd_packet_type:
        OSD_PACKET_TYPE_REG = 0
        OSD_PACKET_TYPE_RES1 = 1
        OSD_PACKET_TYPE_EVENT = 2
        OSD_PACKET_TYPE_RES2 = 3

    osd_result osd_packet_new(osd_packet **packet, size_t size_data_words)

    void osd_packet_free(osd_packet **packet)

    unsigned int osd_packet_get_dest(const osd_packet *packet)

    unsigned int osd_packet_get_src(const osd_packet *packet)

    unsigned int osd_packet_get_type(const osd_packet *packet)

    unsigned int osd_packet_get_type_sub(const osd_packet *packet)

    osd_result osd_packet_set_header(osd_packet * packet,
                                     const unsigned int dest,
                                     const unsigned int src,
                                     const osd_packet_type type,
                                     const unsigned int type_sub)

    void osd_packet_to_string(const osd_packet *packet, char** str)

    unsigned int osd_packet_sizeconv_payload2data(unsigned int payload_words)

    unsigned int osd_packet_sizeconv_data2payload(unsigned int data_words)

    size_t osd_packet_sizeof(const osd_packet *packet)

    osd_result osd_packet_realloc(osd_packet **packet_p,
                                  size_t data_size_words_new)

    bint osd_packet_equal(const osd_packet *p1, const osd_packet *p2)

cdef extern from "osd/hostmod.h" nogil:
    struct osd_hostmod_ctx:
        pass

    ctypedef osd_result (*osd_hostmod_event_handler_fn)(void*, osd_packet*)

    osd_result osd_hostmod_new(osd_hostmod_ctx **ctx, osd_log_ctx *log_ctx,
                               const char *host_controller_address,
                               osd_hostmod_event_handler_fn event_handler,
                               void *event_handler_arg)

    void osd_hostmod_free(osd_hostmod_ctx **ctx)

    osd_result osd_hostmod_connect(osd_hostmod_ctx *ctx)

    osd_result osd_hostmod_disconnect(osd_hostmod_ctx *ctx)

    osd_result osd_hostmod_reg_read(osd_hostmod_ctx *ctx,
                                    void *result,
                                    uint16_t diaddr,
                                    uint16_t reg_addr,
                                    int reg_size_bit,
                                    int flags)

    osd_result osd_hostmod_reg_write(osd_hostmod_ctx *ctx,
                                     const void *data,
                                     uint16_t diaddr, uint16_t reg_addr,
                                     int reg_size_bit, int flags)

    uint16_t osd_hostmod_get_diaddr(osd_hostmod_ctx *ctx)

    bint osd_hostmod_is_connected(osd_hostmod_ctx *ctx)

    osd_result osd_hostmod_get_modules(osd_hostmod_ctx *ctx,
                                       unsigned int subnet_addr,
                                       osd_module_desc **modules,
                                       size_t *modules_len)

    unsigned int osd_hostmod_get_max_event_words(osd_hostmod_ctx *ctx,
                                                 unsigned int di_addr_target)

    osd_result osd_hostmod_event_send(osd_hostmod_ctx *ctx,
                                      const osd_packet* event_pkg)

    osd_result osd_hostmod_event_receive(osd_hostmod_ctx *ctx,
                                         osd_packet **event_pkg, int flags)

    osd_result osd_hostmod_mod_describe(osd_hostmod_ctx *ctx,
                                        uint16_t di_addr,
                                        osd_module_desc *desc)

    osd_result osd_hostmod_mod_set_event_dest(osd_hostmod_ctx *ctx,
                                              uint16_t di_addr, int flags)

    osd_result osd_hostmod_mod_set_event_active(osd_hostmod_ctx *ctx,
                                                uint16_t di_addr, int enabled,
                                                int flags)


cdef extern from "osd/hostctrl.h" nogil:
    struct osd_hostctrl_ctx:
        pass

    osd_result osd_hostctrl_new(osd_hostctrl_ctx ** ctx,
                                osd_log_ctx * log_ctx,
                                const char * router_address);

    osd_result osd_hostctrl_start(osd_hostctrl_ctx * ctx)

    osd_result osd_hostctrl_stop(osd_hostctrl_ctx *ctx)

    void osd_hostctrl_free(osd_hostctrl_ctx **ctx_p)

    bint osd_hostctrl_is_running(osd_hostctrl_ctx *ctx)

# XXX: move this into cglip
cdef extern from "libglip.h" nogil:
    cdef struct glip_option:
        char *name
        char *value

cdef extern from "osd/gateway.h" nogil:
    struct osd_gateway_transfer_stats:
        timespec connect_time
        uint64_t bytes_from_device
        uint64_t bytes_to_device

cdef extern from "osd/gateway_glip.h" nogil:
    struct osd_gateway_glip_ctx:
        pass

    osd_result osd_gateway_glip_new(osd_gateway_glip_ctx **ctx,
                                    osd_log_ctx *log_ctx,
                                    const char *host_controller_address,
                                    uint16_t device_subnet_addr,
                                    const char* glip_backend_name,
                                    const glip_option* glip_backend_options,
                                    size_t glip_backend_options_len)

    void osd_gateway_glip_free(osd_gateway_glip_ctx **ctx_p)

    osd_result osd_gateway_glip_connect(osd_gateway_glip_ctx *ctx)

    osd_result osd_gateway_glip_disconnect(osd_gateway_glip_ctx *ctx)

    bint osd_gateway_glip_is_connected(osd_gateway_glip_ctx *ctx)

    osd_gateway_transfer_stats* osd_gateway_glip_get_transfer_stats(osd_gateway_glip_ctx *ctx)

cdef extern from "osd/cl_mam.h" nogil:
    cdef struct osd_mem_desc_region:
        uint64_t baseaddr
        uint64_t memsize

    cdef struct osd_mem_desc:
        unsigned int di_addr
        uint16_t data_width_bit
        uint16_t addr_width_bit
        uint8_t num_regions
        osd_mem_desc_region regions[8]

    osd_result osd_cl_mam_get_mem_desc(osd_hostmod_ctx *hostmod_ctx,
                                       unsigned int mam_di_addr,
                                       osd_mem_desc *mem_desc)

    osd_result osd_cl_mam_read(const osd_mem_desc *mem_desc,
                               osd_hostmod_ctx *hostmod_ctx,
                               void *data, size_t nbyte, uint64_t start_addr)

    osd_result osd_cl_mam_write(const osd_mem_desc *mem_desc,
                                osd_hostmod_ctx *hostmod_ctx,
                                const void *data, size_t nbyte,
                                uint64_t start_addr)

cdef extern from "osd/module.h" nogil:
    cdef struct osd_module_desc:
        uint16_t addr
        uint16_t vendor
        uint16_t type
        uint16_t version

    const char* osd_module_get_type_short_name(unsigned int vendor_id,
                                               unsigned int type_id)

    const char* osd_module_get_type_long_name(unsigned int vendor_id,
                                              unsigned int type_id)

cdef extern from "osd/memaccess.h" nogil:
    struct osd_memaccess_ctx:
        pass

    osd_result osd_memaccess_new(osd_memaccess_ctx **ctx,
                             osd_log_ctx *log_ctx,
                             const char *host_controller_address)

    osd_result osd_memaccess_connect(osd_memaccess_ctx *ctx)

    osd_result osd_memaccess_disconnect(osd_memaccess_ctx *ctx)

    bint osd_memaccess_is_connected(osd_memaccess_ctx *ctx)

    void osd_memaccess_free(osd_memaccess_ctx **ctx_p)

    osd_result osd_memaccess_cpus_start(osd_memaccess_ctx *ctx,
                                        unsigned int subnet_addr)

    osd_result osd_memaccess_cpus_stop(osd_memaccess_ctx *ctx,
                                       unsigned int subnet_addr)


    osd_result osd_memaccess_find_memories(osd_memaccess_ctx *ctx,
                                           unsigned int subnet_addr,
                                           osd_mem_desc **memories,
                                           size_t *num_memories)

    osd_result osd_memaccess_loadelf(osd_memaccess_ctx *ctx,
                                     const osd_mem_desc* mem_desc,
                                     const char* elf_file_path, int verify)

cdef extern from "osd/systracelogger.h" nogil:
    struct osd_systracelogger_ctx:
        pass

    osd_result osd_systracelogger_new(osd_systracelogger_ctx **ctx,
                                      osd_log_ctx *log_ctx,
                                      const char *host_controller_address,
                                      uint16_t stm_di_addr)

    osd_result osd_systracelogger_connect(osd_systracelogger_ctx *ctx)

    osd_result osd_systracelogger_disconnect(osd_systracelogger_ctx *ctx)

    bint osd_systracelogger_is_connected(osd_systracelogger_ctx *ctx)

    void osd_systracelogger_free(osd_systracelogger_ctx **ctx_p)

    osd_result osd_systracelogger_start(osd_systracelogger_ctx *ctx)

    osd_result osd_systracelogger_stop(osd_systracelogger_ctx *ctx)

    osd_result osd_systracelogger_set_sysprint_log(osd_systracelogger_ctx *ctx,
                                                   FILE *fp)

    osd_result osd_systracelogger_set_event_log(osd_systracelogger_ctx *ctx,
                                                FILE *fp)


cdef extern from "osd/coretracelogger.h" nogil:
    struct osd_coretracelogger_ctx:
        pass

    osd_result osd_coretracelogger_new(osd_coretracelogger_ctx **ctx,
                                      osd_log_ctx *log_ctx,
                                      const char *host_controller_address,
                                      uint16_t stm_di_addr)

    osd_result osd_coretracelogger_connect(osd_coretracelogger_ctx *ctx)

    osd_result osd_coretracelogger_disconnect(osd_coretracelogger_ctx *ctx)

    bint osd_coretracelogger_is_connected(osd_coretracelogger_ctx *ctx)

    void osd_coretracelogger_free(osd_coretracelogger_ctx **ctx_p)

    osd_result osd_coretracelogger_start(osd_coretracelogger_ctx *ctx)

    osd_result osd_coretracelogger_stop(osd_coretracelogger_ctx *ctx)

    osd_result osd_coretracelogger_set_log(osd_coretracelogger_ctx *ctx,
                                           FILE *fp)

    osd_result osd_coretracelogger_set_elf(osd_coretracelogger_ctx *ctx,
                                           const char* elf_filename)


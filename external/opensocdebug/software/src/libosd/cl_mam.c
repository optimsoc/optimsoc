/* Copyright 2017 The Open SoC Debug Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <osd/cl_mam.h>

#include <assert.h>
#include <errno.h>
#include <osd/osd.h>
#include <osd/packet.h>
#include <osd/reg.h>
#include <stdio.h>
#include <string.h>
#include "osd-private.h"

/**
 * Maximum number of words in a burst write transfer.
 *
 * Limited by the size of HDR1.SELSIZE, which is a 8 bit field.
 */
#define MAM_MAX_BURST_WORDS 255

/**
 * Get a DI packet in a transfer
 */
static osd_result pkg_in_transfer(const struct osd_mem_desc *mem_desc,
                                  struct osd_hostmod_ctx *hostmod_ctx,
                                  const uint8_t *transfer,
                                  size_t transfer_size,
                                  unsigned int pkg_idx,
                                  struct osd_packet **pkg)
{
    osd_result rv;

    unsigned int max_words_per_pkg =
            osd_hostmod_get_max_event_words(hostmod_ctx, mem_desc->di_addr);
    unsigned int max_bytes_per_pkg = max_words_per_pkg * sizeof(uint16_t);

    unsigned int pos_start = pkg_idx * max_bytes_per_pkg;
    unsigned int pos_end = pos_start + max_bytes_per_pkg;
    if (pos_end > transfer_size) {
        pos_end = transfer_size;
    }
    unsigned int pkg_payload_size_bytes = pos_end - pos_start;
    unsigned int pkg_payload_size_words =
            pkg_payload_size_bytes / sizeof(uint16_t);
    unsigned int pkg_data_size_bytes =
            osd_packet_sizeconv_payload2data(pkg_payload_size_words);

    struct osd_packet *p;
    rv = osd_packet_new(&p, pkg_data_size_bytes);
    if (OSD_FAILED(rv)) {
        return rv;
    }
    assert(p);

    rv = osd_packet_set_header(p, mem_desc->di_addr,
                               osd_hostmod_get_diaddr(hostmod_ctx),
                               OSD_PACKET_TYPE_EVENT, 0);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    // copy data endianness-aware (could use memcpy() on big-endian machines)
    unsigned int bpos = pos_start;
    for (unsigned int w = 0; w < pkg_payload_size_words; w++) {
        p->data.payload[w] = transfer[bpos] << 8 | transfer[bpos + 1];
        bpos += 2;
    }

    *pkg = p;

    return OSD_OK;
}

/**
 * Determine the number of DI packets required to send a transfer to the MAM
 */
static unsigned int num_pkgs_for_transfer(const struct osd_mem_desc *mem_desc,
                                          struct osd_hostmod_ctx *hostmod_ctx,
                                          size_t transfer_size)
{
    unsigned int words_per_pkg =
            osd_hostmod_get_max_event_words(hostmod_ctx, mem_desc->di_addr);
    unsigned int bytes_per_pkg = words_per_pkg * sizeof(uint16_t);

    return INT_DIV_CEIL(transfer_size, bytes_per_pkg);
}

/**
 * Create a MAM Transfer Request
 */
static osd_result create_mam_transfer(const struct osd_mem_desc *mem_desc,
                                      struct osd_hostmod_ctx *hostmod_ctx,
                                      const void *data, size_t nbyte,
                                      uint64_t start_addr, bool we, bool burst,
                                      bool sync, uint16_t selsize,
                                      uint8_t **transfer, size_t *transfer_size)
{
    uint8_t *trans;
    ssize_t trans_size;

    unsigned int aw_b = mem_desc->addr_width_bit / 8;
    unsigned int dw_b = mem_desc->data_width_bit / 8;

    assert((we && nbyte) || (!we && !nbyte));
    assert(nbyte % dw_b == 0 && "Only transfers of full words are possible.");
    assert(start_addr % aw_b == 0 && "Addresses must be word-aligned.");
    if (burst) {
        assert(selsize != 0);
        if (we) {
            assert(selsize == nbyte / dw_b);
        }
    }

    trans_size = 2 + aw_b + nbyte;
    trans = malloc(trans_size * sizeof(uint8_t));
    assert(trans);

    // HDR0
    trans[0] = 0;
    trans[0] = we << 7 | burst << 6 | sync << 5;

    // HDR1
    trans[1] = selsize;

    // ADDR
    for (unsigned int i = 0; i < aw_b; i++) {
        trans[2+i] = (start_addr >> ((aw_b - i - 1) * 8)) & 0xFF;
    }

    // write data
    if (we) {
        memcpy(&trans[2+aw_b], data, nbyte);
    }

    *transfer = trans;
    *transfer_size = trans_size;

    return OSD_OK;
}

/**
 * Send a MAM Transfer Request as series of DI packets to the MAM module
 */
static osd_result send_mam_transfer(const struct osd_mem_desc *mem_desc,
                                    struct osd_hostmod_ctx *hostmod_ctx,
                                    const uint8_t *transfer,
                                    size_t transfer_size)
{
    osd_result rv;

    unsigned int numpkgs = num_pkgs_for_transfer(mem_desc, hostmod_ctx,
                                                 transfer_size);
    struct osd_packet *pkg;
    for (unsigned int i = 0; i < numpkgs; i++) {
        pkg = NULL;
        rv = pkg_in_transfer(mem_desc, hostmod_ctx, transfer, transfer_size, i,
                             &pkg);
        if (OSD_FAILED(rv)) {
            free(pkg);
            return rv;
        }

        rv = osd_hostmod_event_send(hostmod_ctx, pkg);
        if (OSD_FAILED(rv)) {
            free(pkg);
            return rv;
        }
        free(pkg);
    }

    return OSD_OK;
}

/**
 * Issue a write transfer to the Memory Access Module (MAM)
 */
static osd_result mam_write(const struct osd_mem_desc *mem_desc,
                            struct osd_hostmod_ctx *hostmod_ctx,
                            const void *data, size_t nbyte,
                            uint64_t start_addr, bool burst, bool sync,
                            uint8_t selsize)
{
    assert(mem_desc);
    assert(hostmod_ctx);
    assert(data);

    uint8_t *transfer;
    size_t transfer_size;
    osd_result rv, retval;

    rv = create_mam_transfer(mem_desc, hostmod_ctx, data, nbyte, start_addr,
                             true, burst, sync, selsize,
                             &transfer, &transfer_size);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    rv = send_mam_transfer(mem_desc, hostmod_ctx, transfer, transfer_size);
    if (OSD_FAILED(rv)) {
        retval = rv;
        goto free_return;
    }

    if (sync) {
        struct osd_packet *rx_pkg = NULL;
        rv = osd_hostmod_event_receive(hostmod_ctx, &rx_pkg,
                                       OSD_HOSTMOD_BLOCKING);
        free(rx_pkg);
        if (OSD_FAILED(rv)) {
            retval = rv;
            goto free_return;
        }
    }

    retval = OSD_OK;
free_return:
    free(transfer);
    return retval;
}
/**
 * Determine the size of the transfer parts in bytes
 *
 * A memory transfer consists of three parts: a prolog, the bulk part, and an
 * epilog. The bulk part is always aligned to the block size. The unaligned
 * data before and after the bulk part are contained in the prolog and epilog,
 * respectively.
 */
static void calculate_parts(uint64_t start_addr, size_t nbyte,
                            size_t dw_b, size_t *prolog, size_t *bulk,
                            size_t *epilog)
{
    if (nbyte < dw_b) {
        *prolog = nbyte;
        *epilog = 0;
        *bulk = 0;
    } else {
        *prolog = (dw_b - start_addr) % dw_b;
        *epilog = ((start_addr + nbyte) % dw_b);
        *bulk = nbyte - *prolog - *epilog;
    }
}

static void align_data_to_word(uint8_t baddr, const void* data, size_t nbyte,
                               size_t dw_b, uint8_t** data_word,
                               uint8_t *byte_select)
{
    assert(baddr + nbyte <= dw_b
           && "Single-word transfers cannot cross a word boundary.");

    // calculate byte select mask
    uint8_t bs = 0;

    for (size_t i = 0; i < dw_b; i++) {
        if ((i >= baddr) && (i < baddr + nbyte)) {
            bs |= 1 << i;
        }
    }
    assert(bs != 0);

    // create transfer word (aligned to word boundary)
    uint8_t *word = calloc(dw_b, sizeof(uint8_t));
    assert(word);
    memcpy(word + baddr, data, nbyte);

    *byte_select = bs;
    *data_word = word;
}

/**
 * Write multiple bytes, up to a single word, to the memory
 *
 * Perform a single-word write with byte select to write un-aligned bytes.
 *
 * @param mem_desc descriptor of the target memory
 * @param hostmod_desc the host module handling the communication
 * @param data the data to be written
 * @param nbyte the number of bytes to write
 * @param start_addr first byte address to write data to. Must *not* be word-
 *                   aligned.
 * @param sync synchronous behavior: only return if the last write has been
 *             acknowledged by the memory.
 * @return OSD_OK if the write was successful
 *         any other value indicates an error
 */
static osd_result write_single(const struct osd_mem_desc *mem_desc,
                               struct osd_hostmod_ctx *hostmod_ctx,
                               const void *data, size_t nbyte,
                               uint64_t start_addr, bool sync)
{
    osd_result rv;
    uint8_t *data_word;
    uint8_t byte_select;

    size_t dw_b = mem_desc->data_width_bit / 8;
    uint8_t baddr = start_addr % dw_b;

    align_data_to_word(baddr, data, nbyte, dw_b, &data_word, &byte_select);

    rv = mam_write(mem_desc, hostmod_ctx, data_word, dw_b, start_addr - baddr,
                   false, sync, byte_select);

    free(data_word);

    return rv;
}

/**
 * Write a (linear) burst of data words to the memory
 *
 * Writes must be aligned to word boundaries.
 *
 * @param mem_desc descriptor of the target memory
 * @param hostmod_ctx the host module handling the communication
 * @param data the data to be written
 * @param nbyte the number of bytes to write
 * @param start_addr first byte address to write data to. All subsequent words
 *                   are written to consecutive addresses. Must be word-aligned.
 * @param sync synchronous behavior: only return if the last write has been
 *             acknowledged by the memory.
 * @return OSD_OK if the write was successful
 *         any other value indicates an error
 */
static osd_result write_burst(const struct osd_mem_desc *mem_desc,
                              struct osd_hostmod_ctx *hostmod_ctx,
                              const void *data, size_t nbyte,
                              uint64_t start_addr, bool sync)
{
    osd_result rv;

    unsigned int dw_b = (mem_desc->data_width_bit / 8);
    unsigned int aw_b = (mem_desc->addr_width_bit / 8);
    assert(nbyte % dw_b == 0);
    assert(start_addr % aw_b == 0);

    unsigned int max_bulk_transfer_size_byte = MAM_MAX_BURST_WORDS * dw_b;
    unsigned int num_transfers = INT_DIV_CEIL(nbyte,
                                              max_bulk_transfer_size_byte);

    size_t transfer_size_byte, transfer_size_words;
    size_t tpos_start, tpos_end;
    for (unsigned int t = 0; t < num_transfers; t++) {
        tpos_start = t * max_bulk_transfer_size_byte;
        tpos_end = tpos_start + max_bulk_transfer_size_byte;
        if (tpos_end > nbyte) {
            tpos_end = nbyte;
        }

        transfer_size_byte = tpos_end - tpos_start;
        transfer_size_words = transfer_size_byte / dw_b;

        // only the last transfer is done synchronously to improve performance
        bool sync_last = sync && (t == num_transfers - 1);

        rv = mam_write(mem_desc, hostmod_ctx, (uint8_t*)data + tpos_start,
                       transfer_size_byte, start_addr + tpos_start, true,
                       sync_last, transfer_size_words);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    return OSD_OK;
}

static osd_result mam_read(const struct osd_mem_desc *mem_desc,
                           struct osd_hostmod_ctx *hostmod_ctx,
                           void *data, size_t nbyte,
                           uint64_t start_addr, bool burst, uint8_t selsize)
{
    osd_result rv;
    uint8_t *transfer;
    size_t transfer_size;

    rv = create_mam_transfer(mem_desc, hostmod_ctx,
                             NULL, // data
                             0, // nbyte
                             start_addr, // addr
                             false, // we
                             burst, // burst
                             false, // sync (not applicable for reads)
                             selsize, // selsize
                             &transfer, &transfer_size);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    rv = send_mam_transfer(mem_desc, hostmod_ctx, transfer, transfer_size);
    free(transfer);
    if (OSD_FAILED(rv)) {
        return rv;
    }

    struct osd_packet *rx_pkg = NULL;
    size_t rx_nbyte = 0;

    do {
        rv = osd_hostmod_event_receive(hostmod_ctx, &rx_pkg,
                                       OSD_HOSTMOD_BLOCKING);
        if (OSD_FAILED(rv)) {
            return rv;
        }
        size_t payload_size_words =
            osd_packet_sizeconv_data2payload(rx_pkg->data_size_words);

        // copy data endianness-aware (could use memcpy() on big-endian machines)
        for (unsigned int w = 0; w < payload_size_words; w++) {
            *((uint8_t*)data + rx_nbyte) = (rx_pkg->data.payload[w] >> 8) & 0xFF;
            *((uint8_t*)data + rx_nbyte + 1) = rx_pkg->data.payload[w] & 0xFF;
            rx_nbyte += 2;
        }

        free(rx_pkg);
    } while (rx_nbyte < nbyte);

    return rv;
}

static osd_result read_single(const struct osd_mem_desc *mem_desc,
                              struct osd_hostmod_ctx *hostmod_ctx,
                              void *data, size_t nbyte,
                              uint64_t start_addr)
{
    osd_result rv;
    uint8_t *data_word;
    uint8_t byte_select;

    size_t dw_b = mem_desc->data_width_bit / 8;
    uint8_t baddr = start_addr % dw_b;

    align_data_to_word(baddr, data, nbyte, dw_b, &data_word, &byte_select);

    rv = mam_read(mem_desc, hostmod_ctx, data_word, dw_b, start_addr - baddr,
                  false, byte_select);

    memcpy(data, data_word + baddr, nbyte);

    free(data_word);

    return rv;
}

static osd_result read_burst(const struct osd_mem_desc *mem_desc,
                             struct osd_hostmod_ctx *hostmod_ctx,
                             void *data, size_t nbyte,
                             uint64_t start_addr)
{
    osd_result rv;

    unsigned int dw_b = (mem_desc->data_width_bit / 8);
    unsigned int aw_b = (mem_desc->addr_width_bit / 8);
    assert(nbyte % dw_b == 0);
    assert(start_addr % aw_b == 0);

    unsigned int max_bulk_transfer_size_byte = MAM_MAX_BURST_WORDS * dw_b;
    unsigned int num_transfers = INT_DIV_CEIL(nbyte,
                                              max_bulk_transfer_size_byte);

    size_t transfer_size_byte, transfer_size_words;
    size_t tpos_start, tpos_end;
    for (unsigned int t = 0; t < num_transfers; t++) {
        tpos_start = t * max_bulk_transfer_size_byte;
        tpos_end = tpos_start + max_bulk_transfer_size_byte;
        if (tpos_end > nbyte) {
            tpos_end = nbyte;
        }

        transfer_size_byte = tpos_end - tpos_start;
        transfer_size_words = transfer_size_byte / dw_b;

        rv = mam_read(mem_desc, hostmod_ctx, (uint8_t*)data + tpos_start,
                      transfer_size_byte, start_addr + tpos_start,
                      true, transfer_size_words);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    return rv;
}

API_EXPORT
osd_result osd_cl_mam_write(const struct osd_mem_desc *mem_desc,
                            struct osd_hostmod_ctx *hostmod_ctx,
                            const void *data, size_t nbyte, uint64_t start_addr)
{
    assert(mem_desc);
    assert(data);
    assert(hostmod_ctx);

    osd_result rv;
    unsigned int dw_b = (mem_desc->data_width_bit / 8);
    assert(dw_b);

    // TODO: insert checks if the write is within a single region

    size_t prolog, bulk, epilog;
    calculate_parts(start_addr, nbyte, dw_b, &prolog, &bulk, &epilog);

    if (prolog) {
        bool sync = (!bulk && !epilog);
        rv = write_single(mem_desc, hostmod_ctx, data, prolog, start_addr,
                          sync);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    if (bulk) {
        bool sync = !epilog;
        rv = write_burst(mem_desc, hostmod_ctx, (uint8_t*)data + prolog, bulk,
                         start_addr + prolog, sync);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    if (epilog) {
        rv = write_single(mem_desc, hostmod_ctx, (uint8_t*)data + prolog + bulk,
                          epilog, start_addr + prolog + bulk, true);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    return OSD_OK;
}

API_EXPORT
osd_result osd_cl_mam_read(const struct osd_mem_desc *mem_desc,
                           struct osd_hostmod_ctx *hostmod_ctx,
                           void *data, size_t nbyte, uint64_t start_addr)
{
    assert(mem_desc);
    assert(data);
    assert(hostmod_ctx);

    osd_result rv;
    unsigned int dw_b = (mem_desc->data_width_bit / 8);

    // TODO: insert checks if the write is within a single region

    size_t prolog, bulk, epilog;
    calculate_parts(start_addr, nbyte, dw_b, &prolog, &bulk, &epilog);

    if (prolog) {
        rv = read_single(mem_desc, hostmod_ctx, data, prolog, start_addr);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    if (bulk) {
        rv = read_burst(mem_desc, hostmod_ctx, (uint8_t*)data + prolog, bulk,
                        start_addr + prolog);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    if (epilog) {
        rv = read_single(mem_desc, hostmod_ctx, (uint8_t*)data + prolog + bulk,
                         epilog, start_addr + prolog + bulk);
        if (OSD_FAILED(rv)) {
            return rv;
        }
    }

    return OSD_OK;
}

API_EXPORT
osd_result osd_cl_mam_get_mem_desc(struct osd_hostmod_ctx *hostmod_ctx,
                                   unsigned int mam_di_addr,
                                   struct osd_mem_desc *mem_desc)
{
    assert(hostmod_ctx);
    assert(mem_desc);

    osd_result rv;

    mem_desc->di_addr = mam_di_addr;

    uint16_t regvalue;
    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, mam_di_addr,
                              OSD_REG_MAM_AW, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    mem_desc->addr_width_bit = regvalue;

    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, mam_di_addr,
                              OSD_REG_MAM_DW, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    mem_desc->data_width_bit = regvalue;

    rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, mam_di_addr,
                              OSD_REG_MAM_REGIONS, 16, 0);
    if (OSD_FAILED(rv)) return rv;
    mem_desc->num_regions = regvalue;

    assert(mem_desc->num_regions <= 8);

    // clear regions
    for (int i = 0; i < 8; i++) {
        mem_desc->regions[i].baseaddr = 0;
        mem_desc->regions[i].memsize = 0;
    }

    for (int region = 0; region < mem_desc->num_regions; region++) {
        uint64_t baseaddr = 0;
        for (int w = 0; w < 4; w++) {
            rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, mam_di_addr,
                                      OSD_REG_MAM_REGION_BASEADDR(region, w),
                                      16, 0);
            if (OSD_FAILED(rv)) return rv;
            uint8_t regvalue_msb = (regvalue >> 8) & 0xFF;
            uint8_t regvalue_lsb = regvalue & 0xFF;
            baseaddr |= (uint64_t)regvalue_msb << (w * 16 + 8);
            baseaddr |= (uint64_t)regvalue_lsb << (w* 16);
        }
        mem_desc->regions[region].baseaddr = baseaddr;

        uint64_t memsize = 0;
        for (int w = 0; w < 4; w++) {
            rv = osd_hostmod_reg_read(hostmod_ctx, &regvalue, mam_di_addr,
                                      OSD_REG_MAM_REGION_MEMSIZE(region, w),
                                      16, 0);
            if (OSD_FAILED(rv)) return rv;
            uint8_t regvalue_msb = (regvalue >> 8) & 0xFF;
            uint8_t regvalue_lsb = regvalue & 0xFF;
            memsize |= (uint64_t)regvalue_msb << (w * 16 + 8);
            memsize |= (uint64_t)regvalue_lsb << (w * 16);
        }
        mem_desc->regions[region].memsize = memsize;
    }
    return OSD_OK;
}

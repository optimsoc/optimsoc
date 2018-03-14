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

#ifndef OSD_PACKET_H
#define OSD_PACKET_H

#include <czmq.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup libosd-packet Packet
 * @ingroup libosd
 *
 * @{
 */

/**
 * A packet in the Open SoC Debug system
 */
// We must use zero-length data members (a GCC extension) instead of standard C
// flexible arrays (uint16_t payload[]) as flexible array members are not
// allowed in unions by the C standard.
struct osd_packet {
    uint16_t data_size_words;  //!< size of data/data_raw in uint16_t words
    union {
        struct {
            uint16_t dest;        //!< packet destination address
            uint16_t src;         //!< packet source address
            uint16_t flags;       //!< packet flags
            uint16_t payload[0];  //!< (size_data_words - 3) words of payload
        } data;

        uint16_t data_raw[0];  //!< data_size_words 16 bit words of data
    };
};

/**
 * Packet types
 */
enum osd_packet_type {
    OSD_PACKET_TYPE_REG = 0,    //< Register access
    OSD_PACKET_TYPE_RES1 = 1,   //< Reserved (will be discarded)
    OSD_PACKET_TYPE_EVENT = 2,  //< Debug Event
    OSD_PACKET_TYPE_RES2 = 3    //< Reserved (will be discarded)
};

/**
 * Values of the TYPE_SUB field in if TYPE == OSD_PACKET_TYPE_REG
 */
enum osd_packet_type_reg_subtype {
    REQ_READ_REG_16 = 0b0000,            //< 16 bit register read request
    REQ_READ_REG_32 = 0b0001,            //< 32 bit register read request
    REQ_READ_REG_64 = 0b0010,            //< 64 bit register read request
    REQ_READ_REG_128 = 0b0011,           //< 128 bit register read request
    RESP_READ_REG_SUCCESS_16 = 0b1000,   //< 16 bit register read response
    RESP_READ_REG_SUCCESS_32 = 0b1001,   //< 32 bit register read response
    RESP_READ_REG_SUCCESS_64 = 0b1010,   //< 64 bit register read response
    RESP_READ_REG_SUCCESS_128 = 0b1011,  //< 128 bit register read response
    RESP_READ_REG_ERROR = 0b1100,        //< register read failure
    REQ_WRITE_REG_16 = 0b0100,           //< 16 bit register write request
    REQ_WRITE_REG_32 = 0b0101,           //< 32 bit register write request
    REQ_WRITE_REG_64 = 0b0110,           //< 64 bit register write request
    REQ_WRITE_REG_128 = 0b0111,          //< 128 bit register write request
    RESP_WRITE_REG_SUCCESS = 0b1110,     //< the preceding write request was
                                         //  successful
    RESP_WRITE_REG_ERROR = 0b1111        //< the preceding write request failed
};

/**
 * Values of the TYPE_SUB field in if TYPE == OSD_PACKET_TYPE_EVENT
 */
enum osd_packet_type_event_subtype {
    EV_LAST = 0,                         //< last (only) event packet
    EV_CONT = 1,                         //< intermediate event packet
    EV_OVERFLOW = 5                      //< overflow event packet
};

// debug packet structure
#define DP_HEADER_TYPE_SHIFT 14
#define DP_HEADER_TYPE_MASK 0b11

#define DP_HEADER_TYPE_SUB_SHIFT 10
#define DP_HEADER_TYPE_SUB_MASK 0b1111

#define DP_HEADER_SRC_SHIFT 0
#define DP_HEADER_SRC_MASK ((1 << 16) - 1)

#define DP_HEADER_DEST_SHIFT 0
#define DP_HEADER_DEST_MASK ((1 << 16) - 1)

/**
 * Allocate memory for a packet with given data size and zero all data fields
 *
 * The osd_packet.size field is set to the allocated size.
 *
 * @param[out] packet the packet to be allocated
 * @param[in]  size_data_words number of uint16_t words in the packet, including
 *             the header words.
 * @return     OSD_OK if successful, any other value indicates an error
 *
 * @see osd_packet_new_from_zframe()
 * @see osd_packet_realloc()
 * @see osd_packet_free()
 */
osd_result osd_packet_new(struct osd_packet **packet, size_t size_data_words);

/**
 * Reallocate memory for a packet to increase or decrease its size
 *
 * @param      packet_p A pointer to an existing packet, which will be
 *                      reallocated. The resulting pointer will potentially
 *                      differ from the the passed pointer!
 * @param[in]  size_data_words number of uint16_t words in the packet, including
 *             the header words. May be larger or smaller than the size of the
 *             existing packet.
 * @return     OSD_OK if successful, any other value indicates an error
 *
 * @see osd_packet_new()
 * @see osd_packet_free()
 */
osd_result osd_packet_realloc(struct osd_packet **packet_p,
                              size_t data_size_words_new);
/**
 * Create a new packet from a zframe
 *
 * @see osd_packet_new()
 */
osd_result osd_packet_new_from_zframe(struct osd_packet **packet,
                                      const zframe_t *frame);

/**
 * Free the memory associated with the packet and NULL the object
 */
void osd_packet_free(struct osd_packet **packet);

/**
 * Append the payload of the second packet into the first packet
 *
 * The header data of the second packet is ignored.
 *
 * @param   first_p a pointer to an existing packet, which will be reallocated
 *                 the resulting pointer will be *different* than the
 *                 the passed pointer!
 * @param   second the packet of which the payload is appended to @p first
 * @return  OSD_OK if successful, any other value indicates an error
 */
osd_result osd_packet_combine(struct osd_packet** first_p,
                              const struct osd_packet *second);

/**
 * Extract the DEST field out of a packet
 */
unsigned int osd_packet_get_dest(const struct osd_packet *packet);

/**
 * Extract the SRC field out of a packet
 */
unsigned int osd_packet_get_src(const struct osd_packet *packet);

/**
 * Extract the TYPE field out of a packet
 */
unsigned int osd_packet_get_type(const struct osd_packet *packet);

/**
 * Extract the TYPE_SUB field out of a packet
 */
unsigned int osd_packet_get_type_sub(const struct osd_packet *packet);

/**
 * Set the TYPE_SUB field in a packet
 */
osd_result osd_packet_set_type_sub(struct osd_packet *packet,
                                   const unsigned int type_sub);

/**
 * Populate the header of a osd_packet
 *
 * @param packet
 * @param dest     packet destination
 * @param src      packet source
 * @param type     packet type
 * @param type_sub packet subtype
 *
 * @return OSD_OK on success, any other value indicates an error
 */
osd_result osd_packet_set_header(struct osd_packet *packet,
                                 const unsigned int dest,
                                 const unsigned int src,
                                 const enum osd_packet_type type,
                                 const unsigned int type_sub);

/**
 * Size in bytes of a packet
 */
size_t osd_packet_sizeof(const struct osd_packet *packet);

/**
 * Get the number of data words required for the given payload words
 *
 * Data words are the number of 16 bit words in a DI packet, including the
 * DI header. Payload words are the number of 16 bit words in a DI packet
 * excluding the header.
 */
unsigned int osd_packet_sizeconv_payload2data(unsigned int payload_words);

/**
 * Get the number of paylaod words required for the given data words
 *
 * Data words are the number of 16 bit words in a DI packet, including the
 * DI header. Payload words are the number of 16 bit words in a DI packet
 * excluding the header.
 */
unsigned int osd_packet_sizeconv_data2payload(unsigned int data_words);

/**
 * Log a debug message with the packet in human-readable form
 *
 * Use the @p msg parameter to prefix the dumped packet in the log entry with,
 * for example, the type of packet being logged. This is preferrable over
 * writing two log entries to keep the information together.
 *
 * @param packet packet to log
 * @param log_ctx the log context to write to
 * @param msg message to be prepended to the dumped packet
 */
void osd_packet_log(const struct osd_packet *packet,
                    struct osd_log_ctx *log_ctx, const char* msg);

/**
 * Dump a packet in human-readable (debugging) form to a file stream
 *
 * @param packet packet to dump
 * @param fd stream to dump packet to. You can use stdout and stderr here.
 *
 * @see osd_packet_to_string()
 */
void osd_packet_dump(const struct osd_packet *packet, FILE *fd);

/**
 * Dump the packet to a string (for human consumption)
 *
 * The string representation of a packet is for debugging purposes only and may
 * change at any time, do not rely on it for automated parsing.
 *
 * @see osd_packet_dump()
 */
void osd_packet_to_string(const struct osd_packet *packet, char **str);

/**
 * Write a packet from a file descriptor
 *
 * In its current implementation this function memory-dumps osd_packet structs
 * to a file, without any encoding. This has some consequences:
 *
 * - The resulting file is in native endianness, i.e. not portable between
 *   little and big endian machines.
 * - No file header is present, or any for of file magic number to identify
 *   the file type.
 * - No file integrity checks, such as checksums.
 *
 * @param packet the packet to write
 * @param fd the open file descriptor to write to
 * @return bool operation successful?
 *
 * @see osd_packet_fread()
 */
bool osd_packet_fwrite(const struct osd_packet *packet, FILE *fd);

/**
 * Read a packet from an open file descriptor
 *
 * See the discussion in osd_packet_fwrite() for known limitations.
 *
 * @param fd an open file descriptor to read from
 * @return the read packet, or NULL if reading failed
 *
 * @see osd_packet_fwrite()
 */
struct osd_packet* osd_packet_fread(FILE *fd);

/**
 * Check if two packets are equal
 */
bool osd_packet_equal(const struct osd_packet *p1, const struct osd_packet *p2);

/**@}*/ /* end of doxygen group libosd-packet */

#ifdef __cplusplus
}
#endif

#endif  // OSD_PACKET_H

/* Copyright (c) 2012-2013 by the author(s)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef TRACEPACKET_H_
#define TRACEPACKET_H_

#include <stdint.h>
#include <stddef.h>
#include <cstdlib>
#include <cstring>

class TracePacket
{
public:
    TracePacket();
    virtual ~TracePacket();

    /**
     * Get a raw trace packet for sending over TCP
     */
    const uint8_t* getRawPacket();

    /**
     * Get the size of the raw packet, as returned by getRawPacket()
     */
    virtual size_t getRawPacketSize() const = 0;

protected:
    /**
     * Update the data inside m_rawPacket
     */
    virtual void refreshRawPacketData() = 0;

    /**
     * The raw packet data.
     *
     * You need to allocate enough memory inside your constructor to make this
     * variable hold getRawPacketSize() items.
     */
    uint8_t* m_rawPacket;
};

class STMTracePacket : public TracePacket
{
public:
    uint32_t coreid;
    uint32_t timestamp;
    uint16_t id;
    uint32_t value;

    STMTracePacket() : coreid(0), timestamp(0), id(0), value(0)
    {
        m_rawPacket = (uint8_t*)calloc(getRawPacketSize(), sizeof(uint8_t));
    }

    size_t getRawPacketSize() const
    {
        return sizeof(coreid) + sizeof(timestamp) + sizeof(id) +
               sizeof(value);
    }

protected:
    void refreshRawPacketData()
    {
        memcpy(&m_rawPacket[0], (void*) &coreid, 4);
        memcpy(&m_rawPacket[4], (void*) &timestamp, 4);
        memcpy(&m_rawPacket[8], (void*) &id, 2);
        memcpy(&m_rawPacket[10], (void*) &value, 4);
    }
};


#endif /* TRACEPACKET_H_ */

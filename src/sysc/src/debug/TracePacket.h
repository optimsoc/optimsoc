/*
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * ============================================================================
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Philipp Wagner, philipp.wagner@tum.de
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
 */

#ifndef TRACEPACKET_H_
#define TRACEPACKET_H_

#include <stdint.h>
#include <stddef.h>

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

#endif /* TRACEPACKET_H_ */

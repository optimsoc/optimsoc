/* Copyright (c) 2017 by the author(s)
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
 *   Stefan Wallentowitz <stefan@wallentowitz.de>
 */

#pragma once

#include <string>
#include <vector>
#include <cstdint>
#include <cstdio>

/**
 * The tracer class
 *
 * The tracer class is used to emit traces compatible with the <a
 * href="http://diamon.org/ctf">Common Trace Format</a>.
 */
class Tracer {
public:
    /**
     * Singleton: Get instance
     *
     * We are using a singleton to enforce we have exactly one instance.
     *
     * @return Instance of the Tracer
     */
    static Tracer& instance() {
        static Tracer inst;
        return inst;
    }

    /**
     * Default constructor
     *
     * By default the tracer is disabled.
     */
    Tracer();

    /**
     * Initialize the tracer
     *
     * This automatically enables the tracer. The nocfull parameter is
     * used to switch between tracing NoC packets when entering and
     * leaving the NoC (full NoC trace) or only when leaving the NoC
     * (default).
     *
     * It creates a directory in the form "ctf-20170421-153500" to not
     * overwrite old experiments.
     *
     * @param nocfull Get a full trace of the NoC
     */
    void init(bool nocfull);

    void initNoC(int numLinks);
    void traceNoCPacket(int link, uint32_t flit, int last,
                        uint64_t timestamp);
private:
    bool mEnabled;
    bool mNoCFull;
    std::string mFolder;

    FILE *mNoCTrace;
    size_t mNoCNumLinks;
    std::vector<std::vector<uint32_t> > mNoCBuffer;
    static const std::string mNoCMetadata;

    void emitNoCEventContext(int link, uint16_t id, uint64_t timestamp,
                             uint32_t header);
};

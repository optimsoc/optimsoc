/* Copyright (c) 2015 by the author(s)
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
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#ifndef _VERILATEDCONTROL_H_
#define _VERILATEDCONTROL_H_

#include <verilated.h>
#include <vltstd/svdpi.h>
#include "VerilatedToplevel.h"
#include "OptionsParser.h"

#include <vector>

namespace simutilVerilator {

typedef void (*readmemh_func)();
typedef void (*readmemh_file_func)(const char* file);

class VerilatedControl {
public:
    static VerilatedControl& instance() {
        static VerilatedControl inst;
        return inst;
    }
    void init(VerilatedToplevel &top, int argc, char* argv[]);
    ~VerilatedControl();
    void run();
    void setMemoryFuncs(readmemh_func, readmemh_file_func);
    void addMemory(const char* scopename);
    uint64_t getTime();
private:
    OptionsParser *m_opt;
    VerilatedToplevel *m_top;

    std::vector<const char*> m_Memories;
    readmemh_func m_readmemh;
    readmemh_file_func m_readmemh_file;

    uint64_t m_time;

    // Singleton
    VerilatedControl() { }
    VerilatedControl(const VerilatedControl&);
    VerilatedControl& operator = (const VerilatedControl &);
};

}

#endif

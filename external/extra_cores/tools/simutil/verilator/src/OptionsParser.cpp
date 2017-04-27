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

#include "OptionsParser.h"

#include <getopt.h>
#include <sstream>
#include <stdexcept>

namespace simutilVerilator {

OptionsParser::OptionsParser() {
    mStandalone = false;
    mVcd = false;
    mVcdFrom = 0;
    mVcdTo = 0;
    mMemInit = "";
    mLimit = 0;
}

OptionsParser::~OptionsParser() {
    // TODO Auto-generated destructor stub
}

unsigned long long OptionsParser::str2ull(const std::string &str) {
    std::stringstream stream(str);
    unsigned long long result;
    if((stream >> result).fail()) {
        throw std::runtime_error("Conversion failed");
    }
    return result;
}

void OptionsParser::parse(int argc, char **argv) {
    int c;

    // disable error handling of getopt
    opterr = 0;

    while (1) {
        static struct option long_options[] = {
                {"standalone", no_argument,       0, 'a'},
                {"meminit",    required_argument, 0, 'b'},
                {"vcd", optional_argument, 0, 'c' },
                {"vcd-from",   required_argument, 0, 'd'},
                {"vcd-to",     required_argument, 0, 'e'},
                {"limit", required_argument, 0, 'f'},
                {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long(argc, argv, "ab:cd:e:", long_options, &option_index);
        if (c == -1) {
            break;
        }

        switch(c) {
        case 0:
            break;
        case 'a':
            mStandalone = true;
            break;
        case 'b':
            mMemInit = optarg;
            break;
        case 'c':
            mVcd = true;
            break;
        case 'd':
            try {
                mVcdFrom = str2ull(optarg);
            } catch (std::runtime_error &) {
                // Do nothing
            }
            break;
        case 'e':
            try {
                mVcdTo = str2ull(optarg);
            } catch (std::runtime_error &) {
                // Do nothing
            }
            break;
        case 'f':
            try {
                mLimit = str2ull(optarg);
            } catch (std::runtime_error &) {
                // Do nothing
            }
            break;
        }
    }
}

}

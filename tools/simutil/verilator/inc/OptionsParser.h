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

#ifndef _OPTIONSPARSER_H_
#define _OPTIONSPARSER_H_

#include <string>

namespace simutilVerilator {

/**
 * Command line option parser
 *
 * Takes the command line arguments (argc, argv) and extracts the
 * parameters for OpTiMSoC simulations. The actual handling of the
 * parameters is up to the calling simulation, but this gives
 * convenient access to the parameters.
 * 
 * Currently supported parameters:
 *
 * --meminit=<file>: Initialize memories with given vmem file
 *
 * --vcd-from=<long>, --vcd-to=<long>: Trace VCD from timestamp to
 *   timestamp. Only useful for VCDed simulations.
 */
class OptionsParser {
private:
    /*! --standalone set */
    bool mStandalone;
    /*! --meminit vmem file */
    std::string mMemInit;
    bool mVcd;
    /*! --vcd-from timestamp */
    unsigned long long mVcdFrom;
    /*! --vcd-to timestamp */
    unsigned long long mVcdTo;
    /*! maximum number of cycles */
    unsigned long long mLimit;
public:
    /*! Default constructor */
    OptionsParser();
    /*! Default destructor */
    virtual ~OptionsParser();

    /**
     * Parse command line options
     *
     * Parses the command line options and populates the internal data
     * from it. All parameters not known to this parser are ignored and
     * the argument vector remains unchanged.
     *
     * @param argc Number of arguments
     * @param argv Array of argument strings
     */
    void parse(int argc, char** argv);

    /**
     * Helper function: Convert string to unsigned long long
     *
     * @param str String with the unsigned long long (decimal)
     * @return The value if correct decimal string
     * @throw std::runtime_exception if the string cannot be converted
     */
    unsigned long long str2ull(const std::string &str);

    /**
     * Return if --standalone was set
     *
     * @return true/false whether --standalone was set
     */
    bool getStandalone() {
        return mStandalone;
    }

    bool isVcd() {
        return mVcd;
    }

    /**
     * Get --vcd-from value
     *
     * Get the --vcd-from value, if the option was not present it
     * returns 0.
     *
     * @return The vcd-from value if set, else 0
     */
    unsigned long long getVcdFrom() {
        return mVcdFrom;
    }

    /**
     * Get --vcd-to value
     *
     * Get the --vcd-to value, if the option was not present it returns
     * 0.
     *
     * @return The vcd-from value if set, else 0
     */
    unsigned long long getVcdTo() {
        return mVcdTo;
    }

    /**
     * Was --meminit set?
     *
     * Returns whether --meminit was set or if it was empty (cannot be
     * distinguished currently).
     *
     * @return true/false if --meminit was set
     */
    bool hasMemInit() {
        return (mMemInit.length() > 0);
    }

    /**
     * Get --meminit filename
     *
     * @return --meminit filename
     */
    const char* getMemInit() {
        return mMemInit.c_str();
    }

    unsigned long long getLimit() {
        return mLimit;
    }
};

}

#endif

/* Copyright (c) 2013 by the author(s)
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

#ifndef DEBUGMODULE_H_
#define DEBUGMODULE_H_

// Forward declaration
class DebugConnector;

#define DBGTYPE_ITM 2
#define DBGTYPE_STM 5

/**
 * Abstract debug module class
 *
 * This is the abstract debug module class defining all functions a debug
 * module has to implement and providing some convenience functions.
 *
 * \ingroup debug
 */
class DebugModule : public sc_module {
public:
    /**
     * Constructor
     *
     * The constructor initializes the SystemC module and sets a backlink to
     * the debug connection which can be accessed via m_dbgconn by the derived
     * classes.
     */
    DebugModule(sc_module_name nm, DebugConnector *dbgconn)
        : sc_module(nm), m_dbgconn(dbgconn) {}

    /**
     * Virtual destructor
     *
     * Is empty at the moment.
     */
    virtual ~DebugModule() {};

    /**
     * Get the type of this debug module
     */
    virtual uint16_t getType() = 0;

    /**
     * Get the implementation version of this debug module
     */
    virtual uint16_t getVersion() = 0;

    /**
     * Write to memory-mapped register
     *
     * Write to a memory-mapped register of the debug module.
     *
     * \param address Address of the register
     * \param size    Size of the write
     * \param data    Data to write
     * \return Result code
     */
    virtual uint16_t write(uint16_t address, uint16_t size, char* data) = 0;

    /**
     * Read from a memory-mapped register
     *
     * Read data from a memory-mapped register
     *
     * \param address Address of the register
     * \param[out] size The size of the data read
     * \param[out] data The data read
     * \return Result code
     */
    virtual uint16_t read(uint16_t address, uint16_t *size, char** data) = 0;

    /** Set the address of this module */
    void setAddress(uint16_t addr) { m_address = addr; }
    /** Get the address of this module */
    uint16_t getAddress() { return m_address; }

protected:
    DebugConnector *m_dbgconn; /*!< Pointer to the debug connector */
    uint16_t       m_address;  /*!< Address of this module */
};

#endif /* DEBUGMODULE_H_ */

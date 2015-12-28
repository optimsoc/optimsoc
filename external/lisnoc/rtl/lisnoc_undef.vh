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
 * =============================================================================
 *
 * This file should be included at the end of every file that
 * includes lisnoc_def.vh. It avoids any conflicts with your system.
 *
 * Author(s):
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 *   Andreas Lankes <andreas.lankes@tum.de>
 *   Michael Tempelmeier <michael.tempelmeier@tum.de>
 */

`undef FLIT_TYPE_PAYLOAD
`undef FLIT_TYPE_HEADER
`undef FLIT_TYPE_LAST
`undef FLIT_TYPE_SINGLE

`undef SELECT_NORTH
`undef SELECT_EAST
`undef SELECT_SOUTH
`undef SELECT_WEST
`undef SELECT_LOCAL

`undef NORTH
`undef EAST
`undef SOUTH
`undef WEST
`undef LOCAL

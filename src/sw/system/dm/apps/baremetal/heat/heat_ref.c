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
 * =================================================================
 *
 * Reference code for heat distribution simulation.
 *
 * Author(s):
 *   Philipp Wagner <philipp.wagner@tum.de>
 *   Stefan Wallentowitz <stefan.wallentowitz@tum.de>
 */

#include <stdio.h>  // For printf

#define XSIZE 10
#define YSIZE 10

#define ITERATIONS 10

float matrix[2][(XSIZE+2)*(YSIZE+2)];
int curmatrix;
int other;

#define POS(x,y) ((y)*(XSIZE+2)+(x))

int main() {

    for (int x=0;x<XSIZE+2;x++) {
        for (int y=0;y<YSIZE+2;y++) {
            matrix[0][POS(x,y)] = 0.0;
            matrix[1][POS(x,y)] = 0.0;
        }
    }

    matrix[0][POS(0,0)] = 9.99;
    matrix[0][POS(1,0)] = 9.99;
    matrix[0][POS(0,1)] = 9.99;
    matrix[0][POS(0,2)] = 9.99;
    matrix[1][POS(0,0)] = 9.99;
    matrix[1][POS(1,0)] = 9.99;
    matrix[1][POS(0,1)] = 9.99;
    matrix[1][POS(0,2)] = 9.99;

    int curmatrix = 1;
    int other = 0;

    for (int n=1;n<=ITERATIONS;n++) {
        for (int x=1;x<XSIZE+1;x++) {
            for (int y=1;y<YSIZE+1;y++) {
                matrix[curmatrix][POS(x,y)] = 0.25 * (matrix[other][POS(x-1,y)] +
                        matrix[other][POS(x+1,y)] +
                        matrix[other][POS(x,y-1)] +
                        matrix[other][POS(x,y+1)]);
            }
        }
        curmatrix = 1 - curmatrix;
        other = 1 - other;
    }

    for (int y=1;y<YSIZE+1;y++) {
        for (int x=1;x<XSIZE+1;x++) {
            printf("%.2f ",matrix[1-curmatrix][POS(x,y)]);
        }
        printf("\n");
    }
}

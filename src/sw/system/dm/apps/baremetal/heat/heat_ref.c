/**
 * This file is part of OpTiMSoC.
 *
 * OpTiMSoC is free hardware: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * As the LGPL in general applies to software, the meaning of
 * "linking" is defined as using the OpTiMSoC in your projects at
 * the external interfaces.
 *
 * OpTiMSoC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with OpTiMSoC. If not, see <http://www.gnu.org/licenses/>.
 *
 * =================================================================
 *
 * Reference code for heat distribution simulation.
 *
 * (c) 2013 by the author(s)
 *
 * Author(s):
 *    Stefan Wallentowitz, stefan.wallentowitz@tum.de
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

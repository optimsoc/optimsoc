#include <mp_simple.h>
#include <optimsoc.h>
#include <uart.h>

#define SOCCERBOARD_TILE 5
#define ROBOT_TILE 3

//#define DONTMOVE 1

void enable_sensor() {
  unsigned int buffer;
  buffer = SOCCERBOARD_TILE << 27 | 1;
  optimsoc_mp_simple_send(1,&buffer);
}

void disable_sensor() {
  unsigned int buffer;
  buffer = SOCCERBOARD_TILE << 27;
  optimsoc_mp_simple_send(1,&buffer);
}

void robot_putc(unsigned char c) {
    unsigned int buffer;
    buffer = ROBOT_TILE << 27 | c;
    optimsoc_mp_simple_send(1,&buffer);
}

volatile unsigned char IR[8];
volatile int count=0;

void recv_sensor(unsigned int* buffer,int size) {
  //  OPTIMSOC_REPORT(0x20,buffer[0])
  int id = (buffer[0] >> 8) & 0x7;
  //  OPTIMSOC_REPORT(0x21,id)
  IR[id] = buffer[0] & 0xff;
}


#define THRESHOLD 50

volatile unsigned int curspeed = 0;

void setDriveSpeed(unsigned int speed) {
    if (speed != curspeed) {
        curspeed = speed;
        OPTIMSOC_REPORT(0x10,speed)
#ifndef DONTMOVE
        robot_putc(137);
        robot_putc(0);
        robot_putc(speed&0xff);
        robot_putc(0x7f);
        robot_putc(0xff);
#endif
    }
}

void turnAroundLeft() {
#ifndef DONTMOVE
  OPTIMSOC_REPORT(0x11,0)
    robot_putc(137);
    robot_putc(0);
    robot_putc(100);
    robot_putc(0x00);
    robot_putc(0x01);
#endif
}

void turnAroundRight() {
#ifndef DONTMOVE
  OPTIMSOC_REPORT(0x11,1)
    robot_putc(137);
    robot_putc(0);
    robot_putc(100);
    robot_putc(0xff);
    robot_putc(0xff);
#endif
}

void wait(unsigned int us) {
    for (int i=0;i<10*us;i++) {
        asm("l.nop");
    }
}

//#define FRONT 1
//#define LEFT  0
//#define RIGHT 2
#define FRONT 3
#define LEFT  2
#define RIGHT 4


void control() {
    // Start driving forward
    setDriveSpeed(100);

    while(1) {
        // Hit wall?
      if ((IR[FRONT] >= THRESHOLD) ||
	  (IR[LEFT] >= THRESHOLD) ||
	  (IR[RIGHT] >= THRESHOLD)) {
            // Stop
            setDriveSpeed(0);
	    if ((IR[FRONT]<IR[LEFT]) && (IR[FRONT]<IR[RIGHT])) {
	      turnAroundLeft();
	    } else if (IR[LEFT]>IR[RIGHT]) {
	      turnAroundRight();
	    } else {
	      turnAroundLeft();
	    }
        } else {
            setDriveSpeed(100);
        }
        // Wait
        wait(1000);
    }
}

int main() {
    robot_putc(128);
    robot_putc(132);

    optimsoc_mp_simple_init();
    optimsoc_mp_simple_addhandler(0,&recv_sensor);
    enable_sensor();
//  disable_sensor();


  control();
}

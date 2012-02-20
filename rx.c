#include <Mega32.h>
#include <delay.h>
#include <stdio.h>
#include <./txrx.c>
void init(void);
void task1(void);

#define motSpd 313
char i;

char a;
unsigned int time1;
unsigned char data,pos=0, halfStep = 0;
unsigned char  dataready=0, packetcount=0, packetready=0, datareporting=0;
unsigned char x,y,jx,jy,ix,iy,sol;
signed char xmov,ymov,packet[3],count2;
unsigned char mot_y[5] = {1,2,4,8,0};
unsigned char mot_x[5] = {128,64,32,16,0};
unsigned char thing;
void task1(void);

//Receive data from mouse triggered by mouse starting clock
interrupt[TIM0_OVF] void motors(void){
   if(time1 > 0) time1--;
}

void init() {
  DDRC=0xff;
  PORTC=0xff;
  DDRD=0x8c;
  PORTD.2 = 1;
  TIMSK = 1;
  TCCR0 = 1;
  time1 = motSpd;
  txrx_init(0,1,233,0);
  rx_reset(30);
  init_getrx();
}

void main(void){
  init();
  while(1) {
     if(time1 == 0) task1();
     if(rxdone()){
        PORTD.2 = 0;
        sol = rx_data[1];
        PORTD.3 = !(sol & 0x01);
        xmov += rx_data[2];
        ymov += rx_data[3];
        rx_reset(30);
        init_getrx();
     }
     //if(xmov != 0) PORTD.2 = 0;
     //else PORTD.2 = 1;
        //printf("Status: %x\n\rX Mov:  %x\n\rY Mov:  %x\n\n\r",packet[0],packet[1],packet[2]);
  }                            //SANDMAN!
}

void task1(void){
     time1 = motSpd;
     if(xmov < 0){
        jx = ix;
        ix = (ix - 1) % 4;
        xmov++;
     }
     else if(xmov > 0){
        jx = ix;
        ix = (ix + 1) % 4;
        xmov--;
     }
     else if(xmov == 0) jx=4;
     if(ymov < 0){
        jy = iy;
        iy = (iy - 1) % 4;
        ymov++;
     }
     else if(ymov > 0){  
        jy = iy;
        iy = (iy + 1) % 4;
        ymov--;                 
     }
     else if(ymov == 0) jy=4;
     
     halfStep = !halfStep & 0x01;
     if (halfStep == 1) PORTC = mot_x[ix] + mot_y[iy] + mot_x[jx] + mot_y[jy]; 
     if (halfStep == 0) PORTC = mot_x[ix] + mot_y[iy];
     PORTD.2=sol & 0x01;
  }

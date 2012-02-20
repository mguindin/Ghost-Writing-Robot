#include <Mega32.h>
#include <delay.h>
#include <stdio.h>
#include <./txrx.c>
void send(unsigned char);
void init(void);

char i;

char a;
unsigned char data,pos=0;
unsigned char  dataready=0, packetcount=0, packetready=0, datareporting=0;
unsigned char x,y,ix,iy;
signed char xmov,ymov,packet[3];
unsigned char mot_x[4] = {1,2,4,8};
unsigned char mot_y[4] = {16,32,64,128};
unsigned char thing;

//Receive data from mouse triggered by mouse starting clock
interrupt [EXT_INT0] void receive(void) {

  unsigned char shamt=0, count=0, parity=0;
  data = 0;
  //PORTB.6 = 0;
  while(PIND & 0x08);		//wait for clock low
  //Read 8 data bits
  for(count=0;count<8;count++){
    while(!(PIND & 0x08));   	// wait for clock high
    while(PIND & 0x08);  	// wait for clock low
    data=data | (((PIND & 0x04)>>2) << count);
  }

  while(!(PIND & 0x08));   	// wait for clock high
  while(PIND & 0x08);  	// wait for clock low
  parity = (PIND & 0x04) >> 2;
  while(!(PIND & 0x08));   // wait for high
  while(PIND & 0x08);  // wait for low clock
  while(!(PIND & 0x08));   // wait for clock high
  if(datareporting) packet[packetcount++]=data;
  if(packetcount > 2) {
     packetcount = 0;
     packetready = 1;
  }
}

void init_receive() {

  MCUCR = 0b00000000; // INT0 interrupt enable
  GICR =  0b01000000;
  #asm
    sei
  #endasm
}

void disable_receive() {

  GICR = 0;		//Disable interrupts

}

void init() {
  UCSRB = 0x10 + 0x08 ;
  UBRRL = 103; //UART yo
  DDRA = 0x00;	//PORTA as input
  DDRB = 0xff;	//PORTB as output
  PORTB = 0x00;
  DDRC = 0xff;	//PORTC as output, test stepper motor
  PORTC = 0;
  DDRD = 0b00110010;	//PORTD as output
  xmov=0;
  ymov=0;
  send(0xff);	//Reset mouse
  #define motSpd 2200
  init_receive();
   //wait for codes
  while(data != 0xfa) {}	//Acknowledge
  while(data != 0xaa) {}	//Self test passed
  while(data != 0x00) {}	//Mouse ID
  disable_receive();
  delay_ms(500);
  send(0xff);
  init_receive();
  while(data != 0xfa) {}	//Acknowledge
  while(data != 0xaa) {}	//Self test passed
  while(data != 0x00) {}	//Mouse ID
  PORTC.7=1;
  disable_receive();
  send(0xf3);             //set sample rate
  init_receive();
  while(data != 0xfa) {}	//Acknowledge
  PORTC.6=1;
  disable_receive();
  send(100);
  init_receive();
  while(data != 0xfa) {}	//Acknowledge
  PORTC.5=1;
  disable_receive();
  delay_ms(400);
  send(0xF4); 		// enable data reporting
  init_receive();
  while(data != 0xfa);
  //txrx_init(1,1,249,1);
  //rx_reset(30);
  //init_getrx();
  packetcount--;    //for some reason packet count is off, this corrects that
  datareporting=1;	//Ready to receive movement data
  //pos = 0;*/




}

void main(void){
  init();
  //PORTB.7=0;
  while(1) {
     if(packetready){
        xmov += packet[1];
        ymov += packet[2];
        //printf("Status: %x\n\rX Mov:  %x\n\rY Mov:  %x\n\n\r",packet[0],packet[1],packet[2]);
        packetready = 0;
        //UDR = 0xff;
        //UDR = 0x00;
        //tx_me(packet,3,5);
        //thing = decode(rx_data[5],rx_data[6]);
        //rx_reset(30);
        //init_getrx();
     }
     if(xmov != 0){
        if(xmov < 0){
           ix = (ix - 1) % 4;
           xmov++;
        }
        else if(xmov > 0){
           ix = (ix + 1) % 4;
           xmov--;
        }
     }
     if(ymov != 0){
        if(ymov < 0){
           iy = (iy - 1) % 4;
           ymov++;
        }
        if(ymov > 0){
           iy = (iy + 1) % 4;
           ymov--;
        }
     }

     PORTB = mot_x[ix] + mot_y[iy];
     delay_us(motSpd);
     PORTC = ~packet[0];
  }

}

//Send a command to the mouse
void send(unsigned char command) {

  char parity=0;
  char mask = 0x01;
  parity =  !((command & 0x01) ^		//Calculate odd parity
            ((command & 0x02) >> 1) ^
            ((command & 0x04) >> 2) ^
            ((command & 0x08) >> 3) ^
            ((command & 0x10) >> 4) ^
            ((command & 0x20) >> 5) ^
            ((command & 0x40) >> 6) ^
            ((command & 0x80) >> 7));
  PORTD.4 = 1;  	// Pull clock low
  //while(PIND & 0x08); //Wait for clock low
  delay_us(120);
  PORTD.5 = 1;  	// data low
  delay_us(30);
  PORTD.4 = 0; 	// Release clock
  while(!(PIND & 0x08)); // wait for clock high
  for (i=0; i<8; i++) {
       while(PIND & 0x08);  // wait for low clock
       PORTD.5 = !(command & mask); // send inverted data bit to data line
       while(!(PIND & 0x08));   // wait for clock high
       mask = mask<<1;   // update mask
  }
  //PORTB = ~command;
  while(PIND & 0x08);  // wait for low clock
  PORTD.5 = !parity;	//send inverted parity bit to data line
  while(!(PIND & 0x08));   // wait for high
  while(PIND & 0x08);  // wait for low clock
  PORTD.5 = 0;
  while(!(PIND & 0x08));   // wait for clock high

  PORTD.5 = 0;	//release the data line

  while((PIND & 0x04)); // wait for data go low
  while(!(PIND & 0x04)); // wait for data go hi
  //PORTB.7 = 0;
  }

//Includes
#include <mega32.h>
#include <./txrx.c>
#include <delay.h>
unsigned char a[6]={4,8,15,16,23,42};
unsigned char b[7]={1,2,4,8,16,32,64};
int mcswitch,i;
unsigned char trigger;

void main(void){
   DDRA = 0x00;
   DDRB = 0xff;
   DDRC = 0xff;
   PORTB = 0xff;
   PORTC = 0x00;
   TIMSK = 0;
   delay_ms(500);
   PORTC = 0xff;
   delay_ms(500);
   mcswitch = 0;
   txrx_init(1,1,249,1);
   delay_ms(10);
   delay_ms(10);
   rx_reset(30);
   tx_me(a,6,5);
   init_getrx();
   while(1){
      while(!rx_empty()) {PORTC = ~get_next_rx_data(); delay_ms(20);}
      rx_reset(30);
      init_getrx();
      if(trigger == 1) tx_me(b,7,5);
      if(trigger == 0) tx_me(a,6,5);
      trigger = ~trigger & 0x01;

     //PORTC.5=0;
      //if(~PINA.0) PORTC = ~get_next_rx_data();
      if(~PINA.0 && (mcswitch == 0)){
         tx_me(a,6,5);
         mcswitch++;
      }
      if(~PINA.1 && ((mcswitch == 1) || (mcswitch == 7))){
         PORTC=~get_next_rx_data();
         mcswitch++;
      }
      if(~PINA.2 && ((mcswitch == 2) || (mcswitch == 8))){
         PORTC=~get_next_rx_data();
         mcswitch++;
      }
      if(~PINA.3 && ((mcswitch == 3) || (mcswitch == 9))){
         PORTC=~get_next_rx_data();
         mcswitch++;
      }
      if(~PINA.4 && ((mcswitch == 4) || (mcswitch == 10))){
         PORTC=~get_next_rx_data();
         mcswitch++;
      }
      if(~PINA.5 && ((mcswitch == 5) || (mcswitch == 11))){
         PORTC=~get_next_rx_data();
         mcswitch++;
      }
      if(~PINA.6 && ((mcswitch == 6) || (mcswitch == 12))){
         if(mcswitch == 12){
            rx_reset(30);
            mcswitch = 0;
            txdone();
         }
         else{
            PORTC=~get_next_rx_data();
            mcswitch++;
         }
      }

      //if(b[0] == 4) PORTB.7 = 0;*/
   }
}

#include <Mega32.h>
#include <delay.h>

void main(void){
   DDRD = 0xff;
   PORTD = 0; 
   PORTD = 0;
   while(1){
      PORTD.3 = 1;
      delay_ms(1500);
   }
}
 

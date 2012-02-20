//Includes
#include <mega32.h>

//definitions
#define MAX_LENGTH_RX 32
#define MAX_LENGTH_TX_PKT 32
#define MAX_LENGTH_TX_DATA 16
#define synch_char 0b10101010  //170
#define start_char 0b10010000  //202
#define end_char 0b11010100    //212

//declarations - general
unsigned char txrx_i;
unsigned char counter, drop, count;
//declarations - RX
unsigned char rx_, decoded_byte, rx_byte, rx_done, rx_length, buffer, rx_started, max_rx_length, rx_led;
char rx_data[MAX_LENGTH_RX];

//declarations - TX
unsigned char tx_data_length, tx_pkt_length, tx_id, tx_byte, tx_ing, tx_pkt_byte, tx_data_byte, tx_led;
char tx_data[MAX_LENGTH_TX_PKT];
char in_data[MAX_LENGTH_TX_DATA];

//the encoding...
flash char code[16] = {0b10001011,0b10001101,0b10010011,0b10010101,0b10010110,
0b10011001,0b10011010,0b10011100,0b10100011,0b10100101,0b10100110,0b10101001,
0b10101100,0b10110001,0b10110010,0b10110100};


/****************************************************************************************
** GENERAL FUNCTIONS ********************************************************************
****************************************************************************************/

//**********************************************************
void txrx_init(int tx, int rx, int baud_num, char led) //00-none, 10-tx only, 01-rx only, 11-both
{
	//setup the transmission...
	UCSRB.0 = 0;  //Bit 0 - TXB8
	UCSRB.1 = 0;  //Bit 1 - RXB8
	UCSRB.2 = 0;  //Bit 2 - UCSZ2
	UCSRB.3 = tx; //Bit 3 - TXEN
	UCSRB.4 = rx; //Bit 4 - RXEN
	UCSRB.5 = tx; //Bit 5 - UDRIE
	UCSRB.6 = 0;  //Bit 6 - TXCIE
	UCSRB.7 = rx; //Bit 7 - RXCIE
	UBRRL = baud_num;
   counter = 0;
	tx_byte=MAX_LENGTH_TX_PKT;
	tx_ing=0;
        tx_led=led;
	rx_done=1;
        rx_led=led;
   #asm
   sei
   #endasm
 }

//**********************************************************
char decodeOne(char msbyte)
{
	txrx_i=0;
        buffer=start_char;
	while(txrx_i<16) {
		if(msbyte==code[txrx_i])
			buffer=txrx_i;
		txrx_i++;
        }
        return buffer;
}

//**********************************************************
char decode(char msbyte, char lsbyte)
{
	txrx_i=0;
	while(txrx_i<16) {
		if(msbyte==code[txrx_i])
			buffer=txrx_i;
		txrx_i++;
	}
	txrx_i=0;
	while(txrx_i<16) {
		if(lsbyte==code[txrx_i]) {
			buffer = (buffer<<4);
			buffer = buffer | txrx_i;
		}
		txrx_i++;
	}
	return buffer;
}


/****************************************************************************************
** RECEIVE SIDE FUNCTIONS ***************************************************************
****************************************************************************************/

//**********************************************************
//RX Complete ISR
interrupt [14] void RX_complete(void){
   if(rx_done==0){
	   if (rx_byte==0){
        	rx_data[rx_byte]=UDR;

		   if(rx_data[0]==synch_char) rx_started=1;
        	else rx_started=0;

         if(rx_data[0]==synch_char & rx_started){
	         rx_byte++;
            if(rx_led==1) PORTC.6=0;
        	}
      }
      else if (rx_byte==1){
        	rx_data[rx_byte]=UDR;
	      if(rx_data[0]==synch_char && (rx_data[1]&0xf0)==(start_char & 0xf0)){
            rx_byte++;
         }
      }
      else{
        if(rx_byte == 3) drop = 1;
	      rx_data[rx_byte]=UDR;
        // if (rx_byte==4){
        //    rx_length = decode(rx_data[3],rx_data[4]);
		   //}
        	if(drop == 1){
		      if(rx_led==1) PORTC.6=1;
			   rx_done=1;
			   count++;
            //rx_byte--;
            drop = 0;
        	}
	      rx_byte++;
      }
   }
}

//**********************************************************
void rx_reset(char max_rx)
{
	rx_done=0;
	rx_started=0;
	rx_length=0;
   rx_byte=0;
	max_rx_length=max_rx;
}

//**********************************************************
char rxdone(void) { return rx_done; }

//**********************************************************
//RECEIVE SIDE ITERATOR
void init_getrx(void) { rx_ = 0; }

char get_next_rx_data(void)
{
        if(rx_ <= 2) return rx_data[rx_++];
        else {
                counter++;
                decoded_byte = decode(rx_data[rx_],rx_data[(char)(rx_+1)]);
                rx_ = rx_ + 2;
                return decoded_byte;
        }
}

char rx_empty(void) { return (rx_ < rx_byte) ? 0 : 1; }


/****************************************************************************************
** TRANSMIT SIDE FUNCTIONS **************************************************************
****************************************************************************************/

//**********************************************************
//UDR Empty ISR
interrupt [15] void udr_empty(void)
{
	if(tx_byte< tx_pkt_length && tx_ing)
	{
		UDR=tx_data[tx_byte];
		tx_byte++;
        	if(tx_led==1) PORTC.0=0;
	}
	if(tx_byte==tx_pkt_length)
	{
        	if(tx_led==1) PORTC.0=1;
        	UDR=0x00;
                tx_ing=0;
	}
	PORTC.1 = 1;
}

//**********************************************************
void encode(void)
{
        //sunchronization.
	tx_data[0] = synch_char; //calibration
	//tx_data[1] = synch_char; //calibration
	//tx_data[1] = synch_char; //synchronization
	//tx_data[3] = synch_char; //synchronization
        //start character
	tx_data[1] = (start_char & 0xf0) | ((in_data[0] & 0b00110000) >> 2) | (in_data[0] & 0b00000011);

        //1-byte of ID
 	//tx_data[5] = code[tx_id]; //Channel Id, static 0 for this unit

        //2-bytes of length
	//tx_data[6] = code[tx_data_length>>4]; //length of data to expect.
	//tx_data[7] = code[(tx_data_length<<4)>>4]; //length of data to expect.

        //Payload
        tx_pkt_byte=2; tx_data_byte=0;
        while(tx_data_byte<tx_data_length-1)
        {
        //   tx_data[tx_pkt_byte] = code[in_data[tx_data_byte]>>4];
	      //  tx_data[(char)(tx_pkt_byte+1)] = code[(in_data[tx_data_byte]<<4)>>4];
	      tx_data[tx_pkt_byte] = in_data[tx_data_byte+1];
	      //  tx_pkt_byte=tx_pkt_byte+2;
	      tx_pkt_byte++;
	        tx_data_byte++;
        }

        //End character
	//tx_data[tx_pkt_length-1] = end_char;
}

//**********************************************************
void tx_me(char tx_data[], int length, int id)
{
        if(tx_ing==0)
        {       
                txrx_i=0;
                while(txrx_i < length)
                {
                        in_data[txrx_i]=tx_data[txrx_i];
                        txrx_i++;
                }
                tx_id=id;
                tx_data_length=length;
                //tx_pkt_length=length*2+4;
                tx_pkt_length=length+1;
                encode();
                tx_byte=0;
                tx_ing=1;
        }
}

//**********************************************************
char txdone(void) { return !tx_ing; }

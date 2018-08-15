//PA0-PA3 connected to columns
//PA4-PA7 connected to rows

#define key_prt PORTA //keyboard port
#define key_ddr DDRA  //keyboard ddr
#define key_pin PINA  //keyboard pin

#define E 5
#define RS 6

void send_a_command(unsigned char command);
void send_a_character(unsigned char character);
void send_a_string(char *string_of_characters);
void ask_password();
void get_password();
void start_timer();
void TIMER0_OVF() org 0x016;
void stop_timer();
int check_password();
void clear_lcd();
void lcd_screen_on_cursor_blinking();
int cnt=0;
unsigned int key,i,ovf,timer_running,ic=0,theft_protection_mode=0;
unsigned char colloc,rowloc;
unsigned char password[4]={'1','2','3','4'};
unsigned char entered_password[4]={'0','0','0','0'};

void reg_init()
{
    TIMSK = (1<<TOIE0);
    SREG.B7=1;
    /*TIMSK.B2 = 0x1;
    GICR=(1<<INT0);
    MCUCR=0x01;*/

}

void timer_init(){
      ovf=0;
      timer_running=1;
      TCNT0=0x00;
      TCCR0=0x05;
}

/*void INT0_vect() org 0x002
{

 theft_protection_mode=1-theft_protection_mode;
 clear_lcd();
 if(theft_protection_mode==0)
 {
   send_a_string("Theft Protection");
   send_a_command(0xc0);
   send_a_string("Mode OFF");
 }
 else
 {
   send_a_string("Theft Protection");
   send_a_command(0xc0);
   send_a_string("Mode ON");
 }
 delay_ms(500);
 clear_lcd(); //Clear Screen 0x01 = 00000001
  delay_ms(50);
  send_a_command(0x38);//telling lcd we are using 8bit command /data mode
  delay_ms(50);
  lcd_screen_on_cursor_blinking();//LCD SCREEN ON and courser blinking
  send_a_string("ACJ Safety Soln.");//displaying a string
  send_a_command(0x80 + 0x40 +0);// moving courser to second line of LCD
  delay_ms(1);

}*/

unsigned char keypad[4][4]={'A','3','2','1',
                            'B','6','5','4',
                            'C','9','8','7',
                            'D','#','0','*'};

int main(void)
{
        key_ddr=0xf0;
        key_prt=0xff;

        DDRB = 0xFF;
        DDRD = 0xF0;
        DDRC = 0x0C;
        
        //taking column pins as input and row pins as output
        DDRA = 0xF0;

        delay_ms(50);//giving delay of 50ms

        delay_ms(500);
        clear_lcd(); //Clear Screen 0x01 = 00000001
        delay_ms(50);
        send_a_command(0x38);//telling lcd we are using 8bit command /data mode
        delay_ms(50);
        lcd_screen_on_cursor_blinking();//LCD SCREEN ON and courser blinking
        send_a_string("ACJ Safety Soln.");//displaying a string
        send_a_command(0x80 + 0x40 +0);// moving courser to second line of LCD
        delay_ms(1);
        PORTA=0xF0;// powering the row ins
        delay_ms(1);
        PORTC.B2=0;

        reg_init();


        while(1)
        {
          if(PINC.B0==1 || PINC.B4==1)//pir
          {
           //clear_lcd();

           if(PINC.B5==1)//theft protection mode is ON
           {

            send_a_string("Motion Detected");
             if(PINC.B1==0) //if darkness
             {
               PORTC.B2=1;
             }
             delay_ms(5000);
             clear_lcd(); //clear Screen 0x01 = 00000001

             timer_init();

             ask_password();

             while(timer_running==1)
             {
               get_password();
               if(timer_running==1)
               {
                 clear_lcd(); //Clear Screen 0x01 = 00000001
                 send_a_string("You entered: ");

                 send_a_command(0xC0);
                 send_a_character(entered_password[0]);
                 send_a_character(entered_password[1]);
                 send_a_character(entered_password[2]);
                 send_a_character(entered_password[3]);

                 delay_ms(500);

                 if(check_password()==1)
                 {
                  stop_timer();
                  clear_lcd(); //Clear Screen 0x01 = 00000001
                  send_a_string("Authenticated!");
                  send_a_command(0xC0);
                  send_a_string("Welcome!");
                  break;
                 }
                 else if(check_password()==0)
                 {
                  clear_lcd(); //Clear Screen 0x01 = 00000001
                  send_a_string("Wrong password");
                  send_a_command(0xC0);
                  send_a_string("Try again: ");
                  lcd_screen_on_cursor_blinking();//LCD SCREEN ON and courser blinking
                 }
               }
             }
             
              delay_ms(5000);
              clear_lcd(); //Clear Screen 0x01 = 00000001
              delay_ms(50);
              send_a_command(0x38);//telling lcd we are using 8bit command /data mode
              delay_ms(50);
              lcd_screen_on_cursor_blinking();//LCD SCREEN ON and courser blinking
              send_a_string("ACJ Safety Soln.");//displaying a string
              send_a_command(0x80 + 0x40 +0);// moving courser to second line of LCD
              delay_ms(1);
              PORTC.B2=0;
           }
           else //theft protection mode is OFF
           {
            if(PINC.B1==0)
            {
             PORTC.B2=1;

            }
            else
            {
             PORTC.B2=0;

            }
            send_a_string("Motion Detected");
            delay_ms(5000);
            clear_lcd(); //Clear Screen 0x01 = 00000001
            delay_ms(50);
            send_a_command(0x38);//telling lcd we are using 8bit command /data mode
            delay_ms(50);
            lcd_screen_on_cursor_blinking();//LCD SCREEN ON and courser blinking
            send_a_string("ACJ Safety Soln.");//displaying a string
            send_a_command(0x80 + 0x40 +0);// moving courser to second line of LCD
            delay_ms(1);
            PORTC.B2=0;

           }
          }
          else
          {
           //delay_ms(10000);
           PORTC.B2=0;
          }
        }
}


void send_a_command(unsigned char command)
{
    PORTB = command;
    PORTD &= ~ (1<<RS); //putting 0 in RS to tell lcd we are sending command
    PORTD |= 1<<E; //telling lcd to receive command /data at the port
    delay_ms(50);
    PORTD &= ~1<<E;//telling lcd we completed sending data
    PORTB= 0;
    //PORTD.B2=0;
}



void send_a_character(unsigned char character)
{
    PORTB= character;
    PORTD |= 1<<RS;//telling LCD we are sending data not commands
    PORTD |= 1<<E;//telling LCD to start receiving command/data
    delay_ms(50);
    PORTD &= ~1<<E;//telling lcd we completed sending data/command
    PORTB = 0;
}

void send_a_string(char *string_of_characters)
{
    while(*string_of_characters > 0)
    {
     send_a_character(*string_of_characters++);
    }
}

void ask_password()
{
     send_a_command(0x01); //Clear Screen 0x01 = 00000001
     send_a_string("Enter password:");
     send_a_command(0xC0);
     send_a_command(0b00001111);//LCD SCREEN ON and courser blinking
}

void get_password()
{
 int j=0;
 while(1)
 {
   do
    {
      key_prt&=0x0f;
      colloc=(key_pin&0x0f);
    }while(colloc!=0x0f);   //checks unpressed

    do
    {
      do
      {
              delay_ms(20);
              colloc=(key_pin&0x0f);
              if(timer_running==0)
                  return ;
      }while(colloc==0x0f);//waits for input

      delay_ms(20);
      colloc=(key_pin&0x0f);
    }while(colloc==0x0f);//checks for spikes or false inputs

    while(1)
    {
      key_prt=0xef;//for checking r3 pressed
      delay_ms(20);
      colloc=(key_pin&0x0f);
      delay_ms(20);

      if(colloc!=0x0f)
      {
           rowloc=0;
           break;
      }

      key_prt=0xdf;
      delay_ms(20);
      colloc=(key_pin&0x0f);
      delay_ms(20);

      if(colloc!=0x0f)
      {
              rowloc=1;
              break;
      }

      key_prt=0xbf;
      delay_ms(20);
      colloc=(key_pin&0x0f);
      delay_ms(20);
      if(colloc!=0x0f)
      {
              rowloc=2;
              break;
      }

      key_prt=0x7f;
      delay_ms(20);
      colloc=(key_pin&0x0f);
      delay_ms(20);
      rowloc=3;
      break;
     }


    if(colloc==0x0e)
    {
     send_a_character(keypad[rowloc][0]);
     entered_password[j]=keypad[rowloc][0];
     j++;
    }
    else if(colloc==0x0d)
    {
      send_a_character(keypad[rowloc][1]);
      entered_password[j]=keypad[rowloc][1];
      j++;
    }
    else if(colloc==0x0b)
    {
       send_a_character(keypad[rowloc][2]);
       entered_password[j]=keypad[rowloc][2];
       j++;
    }
    else if(colloc==0x07)
    {
     send_a_character(keypad[rowloc][3]);
     entered_password[j]=keypad[rowloc][3];
     j++;
    }

    if(j==4)
    {
     return ;
    }
 }
}

void start_timer()
{
 ovf=0;
 timer_running=1;
 TCCR0=0x05;
 TCNT0=0x00;
}

void TIMER0_OVF() org 0x016
{

 //TIMSK.B0=0;

 if(ovf>916)
 {
  delay_ms(500);
  stop_timer();
  send_a_command(0x01); //Clear Screen 0x01 = 00000001
  send_a_string("Burglar alert!");
  PORTC.B3=1;
  
  delay_ms(5000);
  PORTC.B3=0;
 }
 else
 {
  ovf++;
 }
TIFR.B0=1;
//TIMSK.B0=1;
}

void stop_timer()
{
 timer_running=0;
 TCCR0=0x00;
}

void clear_lcd()
{
send_a_command(0x01); //Clear Screen 0x01 = 00000001
}

void lcd_screen_on_cursor_blinking()
{
 send_a_command(0x0F);//LCD SCREEN ON and courser blinking
}

int check_password()
{
 for(i=0;i<4;i++)
 {
  if(password[i]!=entered_password[i])
  {
   return 0;

  }
 }
 return 1;
}


/*while(1)
        {
                //PORTC=0x00;    //just for check
                do
                {
                        key_prt&=0x0f;
                        colloc=(key_pin&0x0f);
                        //PORTC^=0xff;
                        //delay_ms(500);
                }while(colloc!=0x0f);

//delay_ms(5000);
                do
                {
                        do
                        {
                                delay_ms(20);
                                colloc=(key_pin&0x0f);
                                //PORTC^=0xff;
                        //delay_ms(500);
                        }while(colloc==0x0f);

                        delay_ms(20);
                        colloc=(key_pin&0x0f);
                }while(colloc==0x0f);


                while(1)
                {       //if 7-4: R0-R3 and 3-0: C0-C3

                        key_prt=0xef;//for checking r3 pressed
                        delay_ms(20);
                        colloc=(key_pin&0x0f);
                        delay_ms(20);

                        if(colloc!=0x0f)
                        {
                             //PORTC=0x02;
                             //delay_ms(500);
                                rowloc=0;
                                break;
                        }

                        key_prt=0xdf;
                        delay_ms(20);
                        colloc=(key_pin&0x0f);
                        delay_ms(20);

                        if(colloc!=0x0f)
                        {
                        //PORTC=0x01;
                        //delay_ms(500);
                                rowloc=1;
                                break;
                        }

                        key_prt=0xbf;
                        delay_ms(20);
                        colloc=(key_pin&0x0f);
                        delay_ms(20);
                        if(colloc!=0x0f)
                        {
                        //PORTC=0x03;
                        //delay_ms(500);
                                rowloc=2;
                                break;
                        }

                        key_prt=0x7f;
                        delay_ms(20);
                        colloc=(key_pin&0x0f);
                        delay_ms(20);
                        //if(colloc!=0x0f)
                        //{
                        rowloc=3;
                        break;
                  }
                          //}
                        //check column and send result to port B

                        //PORTC=0x0C;
                        //delay_ms(500);
                        //PORTC=0x00;
                        //delay_ms(500);
                        if(colloc==0x0e)
                        {
                         send_a_character(keypad[rowloc][0]);
                                //PORTC=keypad[rowloc][0];

                        }
                        else if(colloc==0x0d)
                        {
                          send_a_character(keypad[rowloc][1]);
                        //        PORTC=keypad[rowloc][1];

                        }
                        else if(colloc==0x0b)
                        {
                           send_a_character(keypad[rowloc][2]);
                                //PORTC=keypad[rowloc][2];
                        }
                        else
                        {
                         send_a_character(keypad[rowloc][3]);
                                //PORTC=keypad[rowloc][3];
                        }
                //PORTC=0x00;
                delay_ms(500);
          }*/
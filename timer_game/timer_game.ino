#include <avr/interrupt.h>

uint16_t int_count = 0;
uint16_t int_count2 = 0;
uint8_t magic_number;
uint8_t i = 0;
int16_t count = 3000;
uint8_t maximum;
uint8_t value;


uint8_t table [10] = {
  0x3F,
  0x06,
  0x5B,
  0x4F,
  0x66,
  0x6D,
  0x7D,
  0x07,
  0x7F,
  0x6F
};


void setup() {
  Serial.begin(9600);
  DDRD = B11111100;
  DDRB = B100011;
  DDRC = B001111;
  set_timers();
  print_intro();
}


void print_intro() {
  Serial.println("Welcome to the timer game! In this game, you will have to guess the magic number befere the time runs out");
  Serial.println("Please, enter the difficulty level");
  Serial.println("1 --> 0 - 99");
  Serial.println("10 --> 0 - 999");
  Serial.println("100 --> 0 - 9999"); 
  while (!Serial.available()) {}
  String comm = Serial.readStringUntil('\n');
  if (comm == "1" || comm == "10" || comm == "100") {
    maximum = comm.toInt();
  } else {
    Serial.println("Bad number. Try again");
    print_intro();
  }
  Serial.print("You have selected ");
  Serial.println(maximum);
  Serial.println("-----------------------------------------");
  Serial.println("Instructions are easy:");
  Serial.println("Write \"start\" to start the counter");
  Serial.println("When you want it to stop, just write \"stop\"");
  Serial.println("After that, you just need to guess the magic number. Good luck");
  Serial.println("-----------------------------------------");

  while (!Serial.available()) {}
  while (Serial.readStringUntil('\n') != "start") {}
  start_game();
  Serial.println("The game has started!");

  while(Serial.readStringUntil('\n') != "stop") {}
  stop_game();
  Serial.println("The magic number is now set. Try to guess it!");

  start_timer2();

  get_res();
}

void get_res() {
  uint16_t res;
  while (!Serial.available()) {}
  String input = Serial.readStringUntil('\n');
  res = input.toInt();
  if (res == 0) {
    Serial.println("Please, enter a number");
  }
  if ( res < magic_number) {
    Serial.println("Higher");
    get_res();
  } else if (res > magic_number) {
    Serial.println("Lower");
    get_res();
  } else if (res == magic_number) {
    Serial.println("Magic!! You have won! Congratulations!");
    Serial.println("-----------------------------------------");
    stop_timer2();
    PORTC = 0x0F;
    print_intro();
  }
}
void loop() {
  uint16_t res;
  
}

void start_game() {
  magic_number = 0;
  TCNT1L = 0;
  TCNT1H = 0xFF;
  count = 6000;
  TIMSK1 = 1;
}

void stop_game() {
  TIMSK1 = 0;
}

void start_timer2() {
  TCNT2 = value;
  TIMSK2 = 1;
}

void stop_timer2() {
  TIMSK2 = 0;
}

void magic_counter() {
  magic_number++;
  if (magic_number == 100*maximum) { magic_number = 0; }
}

void set_timers() {
  //Timer 1
  TCCR1A = 0;
  TCCR1B = 2;



  //Timer 2 
  TCCR2A &= ~((1<<WGM21) | (1<<WGM20));
  TCCR2B &= ~((1<<CS21) | (1<<CS20) | (1<<WGM22));
  TCCR2B |= (1<<CS22);
  ASSR &= ~(1<<AS2);
  value = 256 - (int)((float)F_CPU*0.001/64.0);
  TCNT2 = value;
  //TIMSK2 |= (1<<TOIE2);
}

void counter() {
  count--;
  PINB=(1<<5);
  if (count == 0) {
    stop_game();
    stop_timer2();
    Serial.println("The time is out. You've lost");
    print_intro();
  }
}

void showNumber() {
    uint16_t x = count/(1*pow(10,i));
    x = x%10;

    PORTC = (PORTC = 0x0F) & ~(1<<i);
    PORTD = (PORTD &= 0x03) | (table[x]<<2);
    PORTB = (PORTB &= 0x7C) | (table[x]>>6);    
    
}

void is_overflow() {
  int_count += 1;
  int_count2 += 1;

  if (int_count >= 5) {
    int_count -= 5;
    showNumber();

    i++;
    if (i == 4) { i = 0; }
  }

  if (int_count2 >= 10) {
    int_count2 -= 10;
    counter();
  }
}

ISR(TIMER1_OVF_vect) {
  TCNT1L = 0;
  TCNT1H = 0xFF;
  magic_counter();
}

ISR(TIMER2_OVF_vect) {
  TCNT2 = value;
  is_overflow();
}



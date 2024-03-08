#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <time.h>

#define POWER 21 // BCM-GPIO 5
#define START 13 // BCM-GPIO 9
#define ADC 14 // BCM-GPIO 11
#define ADL 12 // BCM-GPIO 10
#define DATA0 9 // BCM-GPIO 3
#define DATA1 3 // BCM-GPIO 22
#define DATA2 7 // BCM-GPIO 4
#define DATA3 2 // BCM-GPIO 27
#define DATA4 0 // BCM-GPIO 17
#define DATA5 8 // BCM-GPIO 2

#define NODATA 63
#define MAX_LOOPS 2   // if target is not found / invalid, max loops till display stops

#define LOOPTIME 0.1
#define ADC_PERIOD 4  // symmetric, ADC_ON == ADC_PERIOD/2
#define ADL_PERIOD 200
#define ADL_ON 20

int main ( int argc, char *argv[] )
{
  int target = NODATA;
  if (argc != 2)
  {
    time_t rawtime;
    struct tm * timeinfo;
    time (&rawtime);
    timeinfo = localtime (&rawtime);
    target = ((timeinfo->tm_hour % 12 * 60 + timeinfo->tm_min) + 7) / 15;
    printf("Using Time as argument (%2d:%2d = %d)\n", timeinfo->tm_hour % 12, timeinfo->tm_min, target);
  }
  else {
    target = atoi(argv[1]);
  }
  printf ("Raspberry Pi FlipClock\n") ;
 
  if (wiringPiSetup () == -1)
    return 1 ;
 
  pinMode (POWER, OUTPUT) ;
  pinMode (START, OUTPUT) ;
  pinMode (ADC, OUTPUT) ;
  pinMode (ADL, OUTPUT) ;
  pinMode (DATA0, INPUT) ;
  pinMode (DATA1, INPUT) ;
  pinMode (DATA2, INPUT) ;
  pinMode (DATA3, INPUT) ;
  pinMode (DATA4, INPUT) ;
  pinMode (DATA5, INPUT) ;
 
  digitalWrite (POWER, 1) ;
  digitalWrite (START, 1) ;
  digitalWrite (ADL, 1) ;
  digitalWrite (ADC, 1) ;
  
  int adc_count = 0, adl_count = 0; // counter for adc/adl timing
  int prev_position = NODATA, start_position = NODATA;
  int loop_count = 0;
  for (;;)
  {
    adc_count++;
    adl_count++;

    if (adl_count == 1 && digitalRead(ADC) == 1) {
      int position = digitalRead(DATA0) + 2*digitalRead(DATA1) + 4*digitalRead(DATA2) + 8*digitalRead(DATA3) + 16*digitalRead(DATA4) + 32*digitalRead(DATA5);
      if (position != NODATA) {
        if (start_position == NODATA) {
          start_position = position;
        }
        if (prev_position == start_position && position != start_position) {
          loop_count += 1;
        }
        if (position != prev_position) {
          printf ("\nstart %d, loop %d, Data read: %d%d%d%d%d%d = %d", start_position, loop_count, digitalRead(DATA0), digitalRead(DATA1), digitalRead(DATA2), digitalRead(DATA3), digitalRead(DATA4), digitalRead(DATA5), position);
        }
        else {
          printf (".");
        }
        if ((loop_count > 0 && target == position && target == prev_position) || (loop_count == MAX_LOOPS+1)) {
          printf("\nstop display\n");
          delay(1);
          digitalWrite(START, 0);
        }
        prev_position = position;
      }
    }

    // ADC timer
    if (adc_count == ADC_PERIOD/2) {
      adc_count = 0;
      digitalWrite (ADC, abs(digitalRead(ADC) - 1)) ;       // toggle ADC
    }

    // ADL timer
    if (adl_count == ADL_PERIOD-ADL_ON) {
      digitalWrite (ADL, 0) ;
    }
    if (adl_count == ADL_PERIOD) {
      if (digitalRead(START) == 0) {
        return 0;
      }
      digitalWrite (ADL, 1) ;
      adl_count = 0;
    }
    
    // loop delay
    delay (LOOPTIME) ;
  }

  return 0 ;
}
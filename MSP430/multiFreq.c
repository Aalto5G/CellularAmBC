#include  <msp430.h>

unsigned int testSeq[] = {
	0, 0, 0, 0, 1, 1, 0,
	1, 0, 0, 0, 1, 1, 0,
	1, 1, 1, 1, 0, 0, 1,
	0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
	0, 0, 1, 1, 1, 1, 1, 1, 0, 0,
	0, 1, 1, 0, 1, 1, 1, 0, 1, 0,
	1, 1, 0, 0, 1, 1, 0, 0, 1, 1,
	1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
	0, 0, 1, 0, 0, 0, 0, 1, 0, 0,
	0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
	1, 0, 0, 0, 0, 0, 0, 0, 0, 1};
unsigned int FSK_flag=0, ptr=0;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT
  
  //Calibrate DCO for 1MHz operation
  BCSCTL1 = CALBC1_1MHZ;
  DCOCTL = CALDCO_1MHZ;

  P1SEL |= 0x02;                            // P1.1 option select Timer_A output
  P1DIR |= 0x03;                            // P1.0 - P1.1 outputs
  
  TACCTL0 = OUTMOD_4 + CCIE;                // TACCR0 toggle, interrupt enabled
  TACCTL1 = OUTMOD_4 + CCIE;                // TACCR1 toggle, interrupt enabled
  TACTL = TASSEL_2 + ID_3 + MC_2 + TAIE;          // SMCLK, Contmode, int enabled

  _BIS_SR(LPM0_bits + GIE);                 // Enter LPM0 w/ interrup
  ptr=0;
  FSK_flag=testSeq[ptr++];
}

// Timer A0 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A0 (void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER0_A0_VECTOR))) Timer_A0 (void)
#else
#error Compiler not supported!
#endif
{
  TACCR0 += FSK_flag?250:625;                           // reload period
}

// Timer A1 Interrupt Vector (TA0IV) handler
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void Timer_A1(void)
#elif defined(__GNUC__)
void __attribute__((interrupt(TIMER0_A1_VECTOR))) Timer_A1 (void)
#else
#error Compiler not supported!
#endif
{
  switch( TA0IV )
  {
  case  2:  TACCR1 += 5000;                 // reload period
			FSK_flag = testSeq[ptr++];
			if(ptr>=101) ptr=0;
			P1OUT ^= FSK_flag;
           break;
  case 10:                   // Timer overflow
           break;
  default: break;
  }
}


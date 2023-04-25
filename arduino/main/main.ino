// #define DEBUG 1

#define CLOCK 16000000           //16 MHz for MEGA and NANO.
#define MAX_SAMPLE_RATE 20000    //Max 20kHz 

float v_factor[2]; 			// volts per unit of the analog-to-digital converter
uint8_t v_i=0;					// index
uint8_t v_state[2]; 		// 0 == initial undefined state, 1 == first positive zone, 2 == ndgative zone
uint8_t v_pins[2];			// pin number for each channel
int16_t v_now[2]; 			// previous measured value from analog read
int16_t v_mem[2]; 			// temporary values
int16_t v_off[2];  			// offset value (raw scale) for voltage measurements
int16_t v_max[2];				// max value of the curve
int16_t v_min[2];				// min value of the curve
int16_t v_noise[2];
long v_p[2];						// sine curve period
long v_t[2];						// time in microseconds from the begining of the sine curve
long v_mem_pos[2];

unsigned long t = 0, tc = 0, t_init=0;

String msgString;
char inChar;   
int16_t period = 0;


ISR(TIMER1_COMPA_vect) {
	// for each variable
	for (v_i=0; v_i<2; v_i++){
		// read AD convertion
		v_now[v_i] = analogRead(v_pins[v_i]);
		if (v_now[v_i] >= v_off[v_i]){
			// zero-cross, positive slope
			if (v_state[v_i] == 1){
				v_min[v_i] = v_mem[v_i];
				v_mem[v_i] = v_now[v_i];
				v_state[v_i] = 0;
			}
			// checkout maximum
			if (v_now[v_i] >= v_mem[v_i]){
				v_mem[v_i] = v_now[v_i];
        v_mem_pos[v_i] = micros();
			}
		}
		else if (v_now[v_i] < v_off[v_i]){
			// zero-cross, negative slope
			if (v_state[v_i] == 0){
				v_max[v_i] = v_mem[v_i];
				v_mem[v_i] = v_now[v_i];
				v_state[v_i] = 1;
        // time elapsed
        v_p[v_i] = v_mem_pos[v_i] - v_t[v_i];
        v_t[v_i] = v_mem_pos[v_i];
			}
			// checkout minimum
			if (v_now[v_i] <= v_mem[v_i]){
				v_mem[v_i] = v_now[v_i];
			}
		}
	}
}

float get_rms(uint8_t i){
	return (v_max[i] - v_min[i]) * v_factor[i];  // v_peak = (v_max - v_min) / 2; sqrt(1.414213562) = 1.414213562
}

float get_freq(uint8_t i){
	return 1000000.0 / (float)(v_p[i]);
}

float get_power(uint8_t idx_v, uint8_t idx_i){
	return get_rms(idx_v) * get_rms(idx_i);
}

float get_power_factor(int idx_v, int idx_i){
  return (v_max[idx_i] - v_off[idx_i] > v_noise[idx_i]) ? cos(((float)((v_t[idx_v] - v_t[idx_i]) % v_p[idx_v])) / ((float)(v_p[idx_v])) * 6.283185307) : 1.0; // (t_voltage - t_current) / T * 2 * pi
}


/**
 * Serial interruption routine.
 */
void serialEvent() {
  while (Serial.available()) {
    inChar = (char)Serial.read();
    msgString += inChar;
    if (inChar == '\n'){
      t = 0;
      t_init = millis();
      if (sscanf(msgString.c_str(), "%d\n", &period) != 1){
        //err = ERR_PARAMETERS;
      }
      else if (period <= 20){
        //err = ERR_TSAMPLE_PARAMETER;
      }
      msgString = "";
    }
  }
}


#ifdef DEBUG

void setup() {
  Serial.begin(115200);
}

void loop() {
  Serial.print((analogRead(A0)));
  Serial.print(",");
  Serial.println((analogRead(A1)));
}

#else

void setup() {
  // stop interrupts for till we make the settings 
  cli();

  // voltage sensor initial values
  v_pins[0] = A0;
  v_off[0] = 508; // EXPECTED: 1024 / 2 = 512
  v_factor[0] = 0.493847019; // EXPECTED: 250 / 1023 * 2 = 0.488758553
  v_noise[0] = 5;
  
  // current sensor initial values
  v_pins[1] = A1;
  v_off[1] = 512; // EXPECTED: 1024 / 2 = 512
  v_factor[1] = 0.00828583; // EXPECTED: (5 * 2.5 / sqrt(2)) [Irms] / 1023 = 0.008640112
  v_noise[1] = 3;
  
  // common initial values
  for (int i=0; i<2; i++){
    v_state[i] = 0;
    v_max[i] = v_min[i] = v_mem[i] = 0;
    v_t[i] = 0;
    v_mem_pos[i] = 0;
  }

  //set timer1 interrupt (20Khz)
  TCCR1A = 0;
  //turn on CTC mode and set CS12, CS11 and CS10 bits prescaler
  TCCR1B = (1 << WGM12) | (0 << CS12) | (1 << CS11) | (0 << CS10); //prescaler 8
  //set compare match register (clock/(frequency*prescaler)-1) <= 65535
  OCR1A = CLOCK / (8 * MAX_SAMPLE_RATE) - 1;
  //enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);
  //initialize counter value to 0
  TCNT1  = 0;

  // enable back the interrupts
  sei();

  //Serial.begin(9600);
  Serial.begin(38400);
}

float voltage = 0;
float current = 0;
float power_factor = 0;
float frequency = 0;
float voltage_acum = 0;
float current_acum = 0;
float power_factor_acum = 0;
float frequency_acum = 0;
int count = 0;
  
void loop() {
	tc = millis() - t_init;

  voltage_acum += get_rms(0);
  current_acum += get_rms(1);
  power_factor_acum += get_power_factor(0, 1);
  frequency_acum += get_freq(0);
  count ++;
 
	if (tc >= t + period){
		t += period;
		if (period > 0){
			voltage = voltage_acum / count;
			current = current_acum / count;
			power_factor = power_factor_acum / count;
			frequency = frequency_acum / count;

      voltage_acum = 0;
      current_acum = 0;
      power_factor_acum = 0;
      frequency_acum = 0;
      count = 0;
  
			Serial.print(tc);
			Serial.print(",");
			Serial.print(voltage, 1);
			Serial.print(",");
			Serial.print(current, 2);
			Serial.print(",");
			Serial.print(voltage * current, 1);
			Serial.print(",");
			Serial.print(power_factor, 2);
			Serial.print(",");
			Serial.print(frequency, 1);
			/*
      Serial.print(",");
      Serial.print(v_max[1]);
      Serial.print(",");
      Serial.print(v_min[1]);
      Serial.print(",");
      Serial.print((v_max[1] + v_min[1]) / 2);
			*/
			Serial.print("\n");
		}
	}
}

#endif

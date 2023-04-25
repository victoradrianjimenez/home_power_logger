// #define DEBUG 1

#define CLOCK 16000000           //16 MHz for MEGA and NANO.
#define MAX_SAMPLE_RATE 20000    //Max 20kHz 

float v_factor[2]; 			// volts per unit of the analog-to-digital converter
uint8_t v_state[2]; 		// 0 == initial undefined state, 1 == first positive zone, 2 == ndgative zone
uint8_t v_pins[2];			// pin number for each channel
int16_t v_pre[2]; 			// last measured value from analog read
int16_t v_now[2]; 			// previous measured value from analog read
int16_t v_pre_fixed[2]; 			// 
int16_t v_now_fixed[2]; 			// 
int16_t v_tmp[2]; 			// temporary values
int16_t v_off[2];  			// offset value (raw scale) for voltage measurements
int16_t v_plus[2];  		// offset value for final value
int16_t v_max[2];			// max value of the curve
int16_t v_min[2];			// min value of the curve
long v_t_pre[2];			// time in microseconds from the begining of the sine curve in previous cycle
long v_t_now[2];			// time in microseconds from the begining of the sine curve
int v_i=0;

unsigned long t = 0, tc = 0, t_init=0;

String msgString;
char inChar;   
int16_t period = 0;


ISR(TIMER1_COMPA_vect) {
	for (v_i=0; v_i<2; v_i++){
		// read AD convertion
		v_pre[v_i] = v_now[v_i];
		v_now[v_i] = analogRead(v_pins[v_i]);
		v_pre_fixed[v_i] = v_pre[v_i] - v_off[v_i]
		v_now_fixed[v_i] = v_now[v_i] - v_off[v_i]

		// zero-cross, positive slope
		if (v_now_fixed[v_i] >= 0 && v_pre_fixed[v_i] < 0){
			v_state[v_i] = 1;
			// keep minimum value and reset auxiliary variable
			v_t_pre[v_i] = v_t_now[v_i];
			v_t_now[v_i] = micros();
			v_min[v_i] = v_tmp[v_i];
			v_tmp[v_i] = v_now[v_i];
		}
		// zero-cross, negative slope
		else if ((v_now[v_i] - v_off[v_i]) <= 0 && (v_pre[v_i] - v_off[v_i]) > 0){
			v_state[v_i] = 2;
			// keep maximum value and reset auxiliary variable
			v_max[v_i] = v_tmp[v_i];
			v_tmp[v_i] = v_now[v_i];
		}
		// positive zone of the sine curve
		if (v_state[v_i] == 1){
			// check if max
			if (v_now[v_i] > v_tmp[v_i]){
				v_tmp[v_i] = v_now[v_i];
			}
		}
		// negative zone of the sine curve
		else if (v_state[v_i] == 2){
			// check if max
			if (v_now[v_i] < v_tmp[v_i]){
				v_tmp[v_i] = v_now[v_i];
			}
		}
	}
}

float get_rms(int i){
	return v_plus[i] + (v_max[i] - v_min[i]) * v_factor[i] / 2.0;  // v_peak = (v_max - v_min) / 2; sqrt(1.414213562) = 1.414213562
}

float get_freq(int i){
	return 1000000.0 / (float)((v_t_now[i] - v_t_pre[i]));
}

float get_power(int idx_v, int idx_i){
	return get_rms(idx_v) * get_rms(idx_i);
}

float get_power_factor(int idx_v, int idx_i){
	return cos(((float)(v_t_now[idx_v] - v_t_pre[idx_i])) / ((float)(v_t_now[idx_v] - v_t_pre[idx_v])) * 3.141592654 * 2.0);  // (t_voltage - t_current) / T * 2 * pi
}

void setup() {
	// stop interrupts for till we make the settings 
	cli();

	// voltage sensor initial values
	v_pins[0] = A0;
	v_off[0] = 510;  // 512
	//b = (off/((max-min)/2) * sample - 250) / (off / ((max- min)/2) -1)
	v_plus[0] = 198.129032258;  // IDEAL = 195
	//m = (250-b) / off
	v_factor[0] = 0.10170778; // IDEAL = (250 - 195) / 512 = 0.107421875
	
	// current sensor initial values
	v_pins[1] = A1;
	v_off[1] = 512;
	//b = 0
	v_plus[1] = 0;
	//m = (5.1 * sqrt(2)) / 512
	v_factor[1] = 0.015329854; // (5A / 512)
	
	// common initial values
	for (int i=0; i<2; i++){
		v_state[i] = 0;
		v_pre[i] = v_now[i] = 0;
		v_max[i] = v_min[i] = v_tmp[i] = 0;
		v_t_pre[i] = v_t_now[i] = 0;
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

void loop() {
	tc = millis() - t_init;
	if (tc >= t + period){
		t += period;
		if (period > 0){
			float voltage = get_rms(0);
			float current = get_rms(1);
			float power_factor = get_power_factor(0, 1);
			float frequency = get_freq(0);

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
			Serial.print(frequency, 2);
			#ifdef DEBUG
			Serial.print(",");
			Serial.print(v_max[1]);
			Serial.print(",");
			Serial.print(v_min[1]);
			#endif
			Serial.print("\n");
		}
	}
}

void loop_debug() {
	Serial.println((analogRead(A1)));
}

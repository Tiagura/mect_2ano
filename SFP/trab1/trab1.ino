// board is esp8266

#define MAX_LEVEL_BUTTON 4  //push button
#define MIN_LEVEL_BUTTON 0  //flash button
#define GREEN_LED 12        //green led
#define LIGHT_SENSOR A0     //light sensor

void setup(){
    pinMode(MAX_LEVEL_BUTTON, INPUT);
    pinMode(MIN_LEVEL_BUTTON, INPUT);
    pinMode(LIGHT_SENSOR, INPUT);
    pinMode(GREEN_LED, OUTPUT);
    Serial.begin(9600);
}

int max_level = 0;
int min_level = 0;
int light_level = 0;

void loop(){
    //if push button is pressed, water level is MAX, turn off green led
    max_level = digitalRead(MAX_LEVEL_BUTTON);
    min_level = digitalRead(MIN_LEVEL_BUTTON);
    light_level = analogRead(LIGHT_SENSOR);

    if(max_level == LOW){
        digitalWrite(GREEN_LED, LOW);
        Serial.println("Water level is MAX");
    }
    //if flash button is pressed, water level is MIN, turn on green led
    else if(min_level== LOW){
        digitalWrite(GREEN_LED, HIGH);
        Serial.println("Water level is MIN");
    }

    Serial.println("SENSOR MAX: "+ String(max_level) +" | SENSOR MIN "+ String(min_level) +" | SENSOR LIGHT "+ String(light_level));
    delay(1000);
}
#define BLYNK_TEMPLATE_ID "TMPL4kdG28Htv"
#define BLYNK_TEMPLATE_NAME "Кмітлива Зарядка"
#define BLYNK_AUTH_TOKEN "dV9rFi5yWXA6G6bGlONDxl9bCSuxLT1l"
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
char ssid[] = "Keenetic-9112-2G";
char pass[] = "7WfS4kDaK";
int redLed=2;
int greenLed=4;
int pwmPin=6;
byte batPin=A0;
float bat_volt=0;
float real_bat_volt=0;
float start_bat_volt=0;
byte curPWM=0;
int charged_percent =0;
volatile boolean state = LOW;
byte mode=0;
byte tmpCount=0;
char outMess[16];
#define batDev 3.42
#define adc2volt 0.0047
#define bat_full_volt 14.3
#define bat_balance_volt 13.8
#define bat_disch_volt 11.4
#define bat_storage_volt 12.7
#define bat_refresh_volt 9 
#define pwm_charge 255
#define pwm_off 0
#define deltaPWM 10
#define mode_charge 1
#define mode_balance 2
#define mode_storage 3
#define mode_sleep 4
#define mode_refresh 5
#define mode_wait 6
WidgetLCD lcd(V9);
BLYNK_CONNECTED(){
  lcd.clear();
  read_U();
  lcd.print(0, 0, bat_volt);
  lcd.print(0, 1, mode);
}
BLYNK_WRITE(V1){
  int pinValue = param.asInt();
  if(pinValue == 1){
    mode=mode_sleep;
  }else{
    mode=mode_wait;
  }
}
float readVolts(byte pin){
  float tmpRead=0;
  for (int i=0;i<250;i++){
    tmpRead+=analogRead(pin);
  }
  tmpRead=tmpRead/250;
  return(tmpRead*adc2volt);
}
void read_U()
{
  bat_volt=(readVolts(batPin))*batDev;
}
void read_U_bat_real()
{
  analogWrite(pwmPin,0);
  real_bat_volt=(readVolts(batPin))*batDev;
  analogWrite(pwmPin,curPWM);
 }

void setup()
{
  byte TCCR0B;
  TCCR0B = TCCR0B & 0b11111000 | 0x05;
  Serial.begin(115200);
  pinMode(pwmPin,OUTPUT);
  pinMode(redLed,OUTPUT);
  pinMode(greenLed,OUTPUT);
  
  digitalWrite(pwmPin,LOW);
  digitalWrite(redLed,LOW);
  digitalWrite(greenLed,LOW);
  read_U();
  mode=mode_wait;
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

}


void loop()
{
Blynk.run();
switch (mode){
 
  case (mode_charge):
    curPWM=pwm_charge;
    digitalWrite(redLed, HIGH);
    digitalWrite(greenLed,state);
    state = !state;
    if (bat_volt >= bat_full_volt) 
      {
        mode=mode_balance;
        tmpCount=3;
      }
  BLYNK_CONNECTED();
  break;
  
  case (mode_balance):
      if (tmpCount==0)
    {
      if (bat_volt >= bat_balance_volt)
        {
          if (curPWM < deltaPWM) curPWM=pwm_off; 
          if (curPWM != pwm_off) curPWM=curPWM-deltaPWM;
          BLYNK_CONNECTED();
        }
      
      if (bat_volt < bat_balance_volt)
        {
          if(curPWM > (pwm_charge-deltaPWM)) curPWM=pwm_charge;
          if(curPWM != pwm_charge) curPWM=curPWM+deltaPWM;
          BLYNK_CONNECTED();
         }
        tmpCount=3;
    }

    digitalWrite(redLed, state);
    digitalWrite(greenLed,!state);
    state = !state;
    tmpCount=tmpCount-1;
    if (curPWM < 10) mode=mode_storage; 
  break;
  
  case (mode_storage):
    curPWM=pwm_off;
    if (bat_volt < bat_storage_volt) mode=mode_charge;
    digitalWrite(greenLed, HIGH);
    digitalWrite(redLed,state);
    state = !state;
    BLYNK_CONNECTED();
  break;
  
  case (mode_sleep):
    curPWM=pwm_off;
    digitalWrite(redLed, LOW);
    digitalWrite(greenLed,LOW);
    if (bat_volt >= bat_disch_volt) mode=mode_charge;
    if ((bat_volt < bat_disch_volt) && (bat_volt >= bat_refresh_volt))
     { 
       mode=mode_refresh; 
       start_bat_volt=bat_volt;
       tmpCount=5;
     }
     BLYNK_CONNECTED();
  break;
  
  case (mode_refresh):
  curPWM=pwm_charge;
    if (tmpCount==0)
  {
    tmpCount=5;
    curPWM=pwm_off;
  }
  tmpCount=tmpCount-1;
  if (real_bat_volt >= bat_disch_volt) mode=mode_charge;
 
    digitalWrite(redLed, state);
    digitalWrite(greenLed,state);
    state = !state;
    BLYNK_CONNECTED();
  break;
  case (mode_wait):
    BLYNK_WRITE();
  break;
}
 read_U();
  if (bat_full_volt <= real_bat_volt) mode=mode_sleep;
  read_U_bat_real();
  if (real_bat_volt<bat_refresh_volt) 
  {
    sprintf(outMess, "!Low Battery! :(");  
    mode=mode_sleep;
  }
}
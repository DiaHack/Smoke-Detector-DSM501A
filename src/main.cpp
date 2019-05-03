#define BLYNK_PRINT Serial
#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
//Your Blynk auth
char auth[] = "*****";
// Your WiFi credentials.
char ssid[] = "Wifi SSID";
char pass[] = "Wifi Pass";
#define EspSerial Serial3
// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200
ESP8266 wifi(&Serial);
int pin = 3; //DSM501A input Arduino Digital Pin 3
unsigned long duration;
unsigned long starttime;
unsigned long endtime;
unsigned long sampletime_ms = 30000; //30 seconds sample time.It's from datasheet of sensor.
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float concentration = 0;
float particle = 0;
WidgetTerminal Terminal(V3); //Activate Blynk terminal widget for Virtual Pin 3
void setup()
{
  // Debug console
  Serial.begin(9600);
  delay(10);
  // Set ESP8266 baud rate
  Serial.begin(ESP8266_BAUD);
  delay(10);

  pinMode(pin, INPUT);
  starttime = millis();
  Serial.println("Sensör Isınıyor..."); // Sensor is heating...
  Terminal.clear();
  Terminal.println("Sensör Isınıyor..."); // for Blynk terminal

  Terminal.flush();
  delay(60000); // It is for heating resistor for air flow through sensor.Don't change
  Blynk.begin(auth, wifi, ssid, pass);
  Terminal.println("Sensör Hazır. 30 saniye sonra ilk ölçüm alınacaktır!");
}
void loop()
{
  Blynk.run();
  duration = pulseIn(pin, LOW);
  lowpulseoccupancy += duration;
  endtime = millis();
  if ((endtime - starttime) > sampletime_ms)
  {
    ratio = (lowpulseoccupancy - endtime + starttime + sampletime_ms) / (sampletime_ms * 10.0); // Integer percentage 0=>100
    concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) + 520 * ratio + 0.62;             // using spec sheet curve
    //ratio = lowpulseoccupancy/(sampletime_ms*10.0);  // Integer percentage 0=>100
    //concentration =  (0.0026 * pow(ratio, 2)) + (0.0859 * ratio) - 0.0184; //   // Equations based on Samyoung DSM501A spec sheet
    particle = (0.2069 * pow(ratio, 4)) - (9.7081 * pow(ratio, 3)) + (156.86 * pow(ratio, 2)) - (389.72 * ratio) + 2169.9;

    if (concentration > 150000)//For preventing overflowed concentration value.It causes very big value on Blynk superchart.
    {
      concentration = 0;
    }

    Blynk.virtualWrite(V1, concentration); //Super chart data for V1
    Blynk.virtualWrite(V2, concentration); //Super chart data for V2

    Serial.print("lowpulseoccupancy:");
    Serial.print(lowpulseoccupancy);

    Serial.print("    ratio:");
    Serial.print(ratio);

    Serial.print("    DSM501A:");
    Serial.print(concentration);

    Serial.print("    Particle:");
    Serial.println(particle);

    lowpulseoccupancy = 0;
    starttime = millis();

    Terminal.print("DSM501A: ");
    Terminal.println(concentration);
    Terminal.flush();
  }
}

#include <DHT.h>

const int PinDHT = 2;               // D4 auf ESP 8266
const int PinRelayTemperature = 5;  // D1 auf ESP 8266
const int PinRelayHumid = 4;        // D2 auf ESP 8266

DHT dht(PinDHT, DHT22);

bool relayStateTemperature = false;
bool relayStateHumid = false;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);

  pinMode(PinRelayTemperature, OUTPUT);
  digitalWrite(PinRelayTemperature, HIGH);

  pinMode(PinRelayHumid, OUTPUT);
  digitalWrite(PinRelayHumid, HIGH); 

  dht.begin();



}

void loop() {
  // put your main code here, to run repeatedly:

  float temperature = dht.readTemperature();
  float humid = dht.readHumidity();
 

  // Relay Temperature

    if (relayStateTemperature == false)
    {
      if (temperature > +7)
      {
        digitalWrite(PinRelayTemperature, LOW);
        relayStateTemperature = true;
      }

    }
  else
  {
    if ( temperature < +4)
    {
      digitalWrite(PinRelayTemperature, HIGH);
      relayStateTemperature = false;
    }


  }

// Relay Feuchtigkeit

 

    if (relayStateHumid == false)
    {
      if (humid > +85)
      {
        digitalWrite(PinRelayHumid, LOW);
        relayStateHumid = true;
      }

    }
  else
  {
    if ( humid < +78)
    {
      digitalWrite(PinRelayHumid, HIGH);
      relayStateHumid = false;
    }


  }

  

  Serial.print(temperature);
  Serial.print("°C ");
  Serial.print("   -->  Relay Temperatur ");
  Serial.println(relayStateTemperature);

  Serial.print(humid);
  Serial.print("% ");
  Serial.print("    -->  Relay Luftfeuchtigkeit ");
  Serial.println(relayStateHumid);

  delay(2000);
}

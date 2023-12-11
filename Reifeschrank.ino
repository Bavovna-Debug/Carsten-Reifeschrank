#include <stdint.h>
#include <stdlib.h>
#include <DHT.h>
#include <oled.h>

// Switch temperature relay on/off when temperature crosses the bellow edge:
//
const float TemperatureEdgeTop    = +7.0;
const float TemperatureEdgeBottom = +4.0;

// Switch humidity relay on/off when humidity crosses the bellow edge:
//
const float HumidityEdgeTop       = +85.0;
const float HumidityEdgeBottom    = +78.0;

// Pinout
//
const uint8_t PinDHT              = D4;
const uint8_t PinRelayTemperature = D7;
const uint8_t PinRelayHumidity    = D6;
const uint8_t PinSCL              = D1;
const uint8_t PinSDA              = D2;

// Recommended contrast for SH1106 display is 128.
// Values between 0 (black screen) and 255 (highest brightness) are allowed.
//
const uint8_t DisplayContrast = 128;

DHT dht(PinDHT, DHT22);
OLED display = OLED(PinSDA, PinSCL, 16);

// Global variables to track the current state of relays.
//
static bool relayStateTemperature = false;
static bool relayStateHumidity    = false;

// Global variables for current and the last known measurement values.
//
static float currentTemperature   = NAN;
static float currentHumidity      = NAN;
static float lastTemperature      = +999.0;
static float lastHumidity         =  999.0;

// Global variables to keep the min/max values.
//
static float maxKnownTemperature  = -999.0;
static float maxKnownHumidity     =    0.0;
static float minKnownTemperature  = +999.0;
static float minKnownHumidity     =  999.0;

void setup()
{
  Serial.begin(9600);

  pinMode(PinRelayTemperature, OUTPUT);
  digitalWrite(PinRelayTemperature, HIGH);

  pinMode(PinRelayHumidity, OUTPUT);
  digitalWrite(PinRelayHumidity, HIGH);

  dht.begin();

  display.begin();
  display.set_contrast(DisplayContrast);

  PrintLogo();
}

void loop()
{
  currentTemperature = dht.readTemperature();
  currentHumidity = dht.readHumidity();

  PrintValuesToConsole();

  // If something is wrong with the hardware, just skip processing and wait some time
  // before continuing.
  //
  if (ValidateValues())
  {
    delay(5000);
    return;
  }

  if ((currentTemperature != lastTemperature) || (currentHumidity != lastHumidity))
  {
    if (currentTemperature < minKnownTemperature)
      minKnownTemperature = currentTemperature;

    if (currentTemperature > maxKnownTemperature)
      maxKnownTemperature = currentTemperature;

    if (currentHumidity < minKnownHumidity)
      minKnownHumidity = currentHumidity;

    if (currentHumidity > maxKnownHumidity)
      maxKnownHumidity = currentHumidity;

    lastTemperature = currentTemperature;
    lastHumidity = currentHumidity;

    HandleRelais();

    RerfeshDisplay();
  }

  delay(2000);
}

bool
ValidateValues(void)
{
  // At least one of the read values is NAN, which means that DHT library
  // was not able to fetch it from DHT unit.
  //
  if (isnan(currentTemperature))
    return false;
  if (isnan(currentHumidity))
    return false;

  // Temperature value is not NAN but is in some range, which is unusual for planet earth.
  // We assume then that the value is kind of broken.
  //
  if ((currentTemperature < -100.0) || (currentTemperature > +100.0))
    return false;

  // Humidity value is not NAN but is not in the valid range of 0% to 100%.
  // Keeping in mind that 100% is actually unnatural for a room as well.
  //
  if ((currentHumidity < 0.0) || (currentHumidity > 100.0))
    return false;

  // Otherwise, we assume that the values are valid.
  //
  return true;
}

void
HandleRelais(void)
{
  // Relay Temperature
  if (relayStateTemperature == false)
  {
    if (currentTemperature > TemperatureEdgeTop)
    {
      digitalWrite(PinRelayTemperature, LOW);
      relayStateTemperature = true;
    }
  }
  else
  {
    if (currentTemperature < TemperatureEdgeBottom)
    {
      digitalWrite(PinRelayTemperature, HIGH);
      relayStateTemperature = false;
    }
  }

  // Relay Feuchtigkeit
  if (relayStateHumidity == false)
  {
    if (currentHumidity > HumidityEdgeTop)
    {
      digitalWrite(PinRelayHumidity, LOW);
      relayStateHumidity = true;
    }
  }
  else
  {
    if (currentHumidity < HumidityEdgeBottom)
    {
      digitalWrite(PinRelayHumidity, HIGH);
      relayStateHumidity = false;
    }
  }
}

void
PrintValuesToConsole(void)
{
  Serial.print(currentTemperature);
  Serial.print("°C ");
  Serial.print(" --> Relay Temperatur ");
  Serial.println(relayStateTemperature);

  Serial.print(currentHumidity);
  Serial.print("% ");
  Serial.print(" --> Relay Luftfeuchtigkeit ");
  Serial.println(relayStateHumidity);
}

void
RerfeshDisplay(void)
{
  String buffer;

  display.clear();

  display.set_invert(true);
  delay(50);
  display.set_invert(false);

  buffer = FloatToString(currentTemperature, true);
  display.draw_string(0, 0, buffer.c_str(), OLED::DOUBLE_SIZE);

  buffer = FloatToString(currentHumidity, false) + '\%';
  display.draw_string(0, 16, buffer.c_str(), OLED::DOUBLE_SIZE);

  display.draw_string(72, 0, "Max");
  buffer = FloatToString(maxKnownTemperature, true);
  display.draw_string(96, 0, buffer.c_str());

  display.draw_string(72, 8, "Min");
  buffer = FloatToString(minKnownTemperature, true);
  display.draw_string(96, 8, buffer.c_str());

  display.draw_string(72, 16, "Max");
  buffer = FloatToString(maxKnownHumidity, false);
  display.draw_string(96, 16, buffer.c_str());

  display.draw_string(72, 24, "Min");
  buffer = FloatToString(minKnownHumidity, false);
  display.draw_string(96, 24, buffer.c_str());

  display.display();
}

void
PrintLogo(void)
{
  display.draw_string(0, 16, "Reifeschrank");
  display.draw_string(0, 24, "Build: 231210");
  display.display();

  delay(1500);

  display.set_scrolling(OLED::DIAGONAL_RIGHT);
  delay(450);
  display.set_scrolling(OLED::NO_SCROLLING);

  display.clear();
}

String
FloatToString(float value, bool withSign)
{
  int x = value * 10.0f;

  char buffer[20];

  if (withSign == false)
  {
    sprintf(buffer, "%d.%d", x / 10, x % 10);
  }
  else
  {
    if (value < 0.0f)
    {
      x = -x;
      sprintf(buffer, "-%d.%d", x / 10, x % 10);
    }
    else
    {
      sprintf(buffer, "+%d.%d", x / 10, x % 10);
    }
  }

  return String(buffer);
}

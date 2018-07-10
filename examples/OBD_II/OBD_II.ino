#include "OBD.h"
#include "U8glib.h"

//U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);	// I2C / TWI 
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_NO_ACK|U8G_I2C_OPT_FAST);	// Fast I2C / TWI 

/*
Engine
    PID_RPM – Engine RPM (rpm)
    PID_ENGINE_LOAD – Calculated engine load (%)
    PID_COOLANT_TEMP – Engine coolant temperature (°C)
    PID_ENGINE_LOAD – Calculated Engine load (%)
    PID_ABSOLUTE_ENGINE_LOAD – Absolute Engine load (%)
    PID_TIMING_ADVANCE – Ignition timing advance (°)
    PID_ENGINE_OIL_TEMP – Engine oil temperature (°C)
    PID_ENGINE_TORQUE_PERCENTAGE – Engine torque percentage (%)
    PID_ENGINE_REF_TORQUE – Engine reference torque (Nm)

Intake/Exhaust
    PID_INTAKE_TEMP – Intake temperature (°C)
    PID_INTAKE_PRESSURE – Intake manifold absolute pressure (kPa)
    PID_MAF_FLOW – MAF flow pressure (grams/s)
    PID_BAROMETRIC – Barometric pressure (kPa)

Speed/Time
    PID_SPEED – Vehicle speed (km/h)
    PID_RUNTIME – Engine running time (second)
    PID_DISTANCE – Vehicle running distance (km)

Driver
    PID_THROTTLE – Throttle position (%)
    PID_AMBIENT_TEMP – Ambient temperature (°C)

Electric Systems
    PID_CONTROL_MODULE_VOLTAGE – vehicle control module voltage (V)
    PID_HYBRID_BATTERY_PERCENTAGE – Hybrid battery pack remaining life (%)
*/

const int TEST_LED_PIN = 5;
const int MODE_BTN_PIN = 4;
COBD obd;
int mode = 0;
bool modeBtnWasPushed = false;
int framesCount = 0;
int requestsCount = 0;
int lastTime = 0;
int frameTime = 0;

struct Data
{
  const int pid;
  int value;
  const char *title;
  const char *unit;
};

Data data[] = 
{
  {PID_RPM,                       -1, "RPM: ", "r/m"},
  {PID_ENGINE_LOAD,               -1, "Rel. load: ", "%"},
  //{PID_ABSOLUTE_ENGINE_LOAD,      -1, "Abs. load: ", "%"},
  //{PID_ENGINE_TORQUE_PERCENTAGE,  -1, "Torque: ", "%"},
  {PID_THROTTLE,                  -1, "Throttle open: ", "%"},
  {PID_AMBIENT_TEMP,              -1, "Ambient t: ", "*C"},
  {PID_INTAKE_TEMP,               -1, "Intake t: ", "*C"},
  //{PID_ENGINE_OIL_TEMP,           -1, "Oil t: ", "*C"},
  {PID_COOLANT_TEMP,              -1, "Coolant t: ", "*C"},
  //{PID_CONTROL_MODULE_VOLTAGE,    -1, "Voltage: ", "V"},
  {PID_SPEED,                     -1, "Speed: ", "km/h"},
  {PID_FUEL_LEVEL,                -1, "Fuel in tank: ", "%"},
  //{PID_DISTANCE,                  -1, "Distance: ", "km"},
  //{PID_RUNTIME,                   -1, "Runtime: ", "s"}

  {PID_DISTANCE_WITH_MIL,         -1, "Distance *: ", ""},
  {PID_TIME_WITH_MIL,             -1, "Time *: ", ""},
  {PID_ENGINE_FUEL_RATE,          -1, "Fuel rate: ", ""},
};

void setup()
{
  //Serial.begin(9600);
  //Serial.println("Start setup");

  pinMode(TEST_LED_PIN, OUTPUT);
  pinMode(MODE_BTN_PIN, INPUT);

  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_6x12);
    u8g.setPrintPos(0, 32); 
    u8g.print("Initializing OBD...");
  } while(u8g.nextPage());

  digitalWrite(TEST_LED_PIN, HIGH);
  obd.begin();  // start communication with OBD-II UART adapter
  while(!obd.init())  // initiate OBD-II connection until success
    ;
  digitalWrite(TEST_LED_PIN, LOW);
  
  //Serial.println("Setup complete");
}

void printStat(int y, const char *text, int value, const char *suffix)
{
  u8g.setPrintPos(0, y);
  String str = text;
  str += value;
  if (suffix != 0)
    str += suffix;
  u8g.print(str);
}

void draw()
{
  u8g.setFont(u8g_font_6x12);
  printStat(20, data[mode].title, data[mode].value, data[mode].unit);

  printStat(60, "Frame duration ", frameTime, "ms");

  u8g.drawPixel(abs(128 - (framesCount % 256)), 62);
  u8g.drawPixel(abs(128 - (requestsCount % 256)), 63);
  digitalWrite(TEST_LED_PIN, (requestsCount % 2 > 0) ? HIGH : LOW);
}

void loop()
{
  //Serial.println("Loop...");
  
  bool modeBtnPushed = (digitalRead(MODE_BTN_PIN) == HIGH);
  if (modeBtnPushed != modeBtnWasPushed)
  {
    if (modeBtnPushed)
    {
      mode = (mode + 1) % (sizeof(data) / sizeof(data[0]));
      //Serial.print("Change mode to ");
      //Serial.println(mode);
    }
    modeBtnWasPushed = modeBtnPushed;
  }
  
  //obd.read(data[mode].pid, data[mode].value);
  if (!obd.asyncInProgress())
      obd.asyncRequest(data[mode].pid);
  int responce = 0;
  if (obd.asyncGet(responce))
  {
      data[mode].value = responce;
      requestsCount++;
      digitalWrite(TEST_LED_PIN, HIGH);
  }
  else
      digitalWrite(TEST_LED_PIN, LOW);


  u8g.firstPage();  
  do {
    draw();
  } while(u8g.nextPage());

  framesCount++;
  int freshTime = millis();
  frameTime = freshTime - lastTime;
  lastTime = freshTime;
}

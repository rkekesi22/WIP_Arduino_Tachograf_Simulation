#include <LiquidCrystal_I2C.h>
#include <TM1637TinyDisplay.h>
#include <RTClib.h>

#define DIO 3
#define CLK 2

TM1637TinyDisplay display(CLK,DIO);
LiquidCrystal_I2C lcd(0x27,16,2);
RTC_DS1307 rtc;


int potPin = A0;

const uint8_t btnUp = 10;
const uint8_t btnDown = 9;
const uint8_t btnSelect = 8;

bool work = false;

enum Status {
  DRIVING,
  REST
};

const char* statusText[] =
{
  "Driving",
  "Rest   "
};

struct DriverInformation
{
  Status status;
};

DriverInformation driver1 = {REST};

int menu = 1;
const int maxMenu = 2;
float currentlyTruckSpeed = 0;
unsigned long startDateTime = 0;
unsigned long debounceTime = 5000;

void setup() {
  Serial.begin(9600);

  pinMode(btnUp, INPUT_PULLUP);
  pinMode(btnDown, INPUT_PULLUP);
  pinMode(btnSelect, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.print("Welcome!");

  delay(2000);

  display.begin();
  display.setBrightness(7);

  lcd.clear();
  if( !rtc.begin())
  {
    lcd.print("RTC is not found");
    while(1);
  }

  if( !rtc.isrunning())
  {
    lcd.print("RTC settings up");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  DateTime now = rtc.now();
  standardmenu(now, 0);
}

void loop() {
  DateTime now = rtc.now();

  if( btnUp == LOW )
  {
    menu++;
    if( menu > maxMenu)
    {
      menu = 1;
    }

    //updatemenu();
  }

  if( btnDown == LOW )
  {
    menu--;
    if( menu < 1)
    {
      menu = maxMenu;
    }

    //updatemenu();
  }

  int value = analogRead(potPin);

  float truckSpeed = map(value,0,1023,0,90); //km/h

  display.showNumberDec(truckSpeed);

  if(truckSpeed > 0 && driver1.status == REST)
  {
    startDateTime = millis();

    driver1.status = DRIVING;
    lcd.clear();
    
    lcd.setCursor(0,0);
    lcd.print("Status:");
  
    lcd.setCursor(0,1);
    lcd.print(statusText[driver1.status]);

    //delay(2000);
  }

  if( truckSpeed != currentlyTruckSpeed)
  {
    if( (millis() - startDateTime) > debounceTime )
    {
       startDateTime = millis();
       currentlyTruckSpeed = truckSpeed;
       standardmenu(now, currentlyTruckSpeed);
    }
  }
}

void standardmenu(DateTime now, float truckspeed)
{
  lcd.clear();

  int hour = now.hour();
  int minute = now.minute();

  lcd.setCursor(0,0);

  if( hour < 10)
    lcd.print("0");
  lcd.print(hour);

  lcd.print(":");
  
  if( minute < 10)
    lcd.print("0");
  lcd.print(minute);

  lcd.setCursor(12,0);
  lcd.print(truckspeed);


  lcd.setCursor(0,1);
  lcd.print("Status: ");
  lcd.print(statusText[driver1.status]);
}


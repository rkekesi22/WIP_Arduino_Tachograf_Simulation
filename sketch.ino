#include <LiquidCrystal_I2C.h>
#include <TM1637TinyDisplay.h>
#include <RTClib.h>

#define DIO 3
#define CLK 2

TM1637TinyDisplay display(CLK,DIO);
LiquidCrystal_I2C lcd(0x27,16,2);
RTC_DS1307 rtc;

struct Button {
  uint8_t pin;
  bool lastRead;
  bool stableState;
  unsigned long lastChange;
};


int potPin = A0;


Button btnUp = {10, HIGH, HIGH, 0};
Button btnDown = {9, HIGH, HIGH, 0};
Button btnSelect = {8, HIGH, HIGH, 0};

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
int basemenu = 1;
const int maxMenu = 2;
float currentlyTruckSpeed = 0;
unsigned long startDateTime = 0;
unsigned long debounceTime = 5000;
unsigned long debounce = 50;

void setup() {
  Serial.begin(9600);

  pinMode(btnUp.pin, INPUT_PULLUP);
  pinMode(btnDown.pin, INPUT_PULLUP);
  pinMode(btnSelect.pin, INPUT_PULLUP);

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

  if( checkButton(btnUp, debounce) )
  {
    if(basemenu == 1)
    {
      menu++;
      if( menu > maxMenu)
      {
        menu = 1;
      }

      updateMenu();
    }
  }

  if( checkButton(btnDown, debounce) )
  {
    if(basemenu == 1)
    {
       menu--;
      if( menu < 1)
      {
        menu = maxMenu;
      }

      updateMenu();
    }
   
  }
  

  if( checkButton(btnSelect, debounce) )
  {
    menu = 1;

    if( basemenu == 1)
    {
      updateMenu();
    }

    
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

  
  if( truckSpeed == 0 && driver1.status == DRIVING)
  {
    if( (millis() - startDateTime) > debounceTime )
    {
      driver1.status = REST;
      lcd.clear();
      
      lcd.setCursor(0,0);
      lcd.print("Status:");
      
      lcd.setCursor(0,1);
      lcd.print(statusText[driver1.status]);
  }
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

void updateMenu()
{
  lcd.clear();

  switch(menu)
  {
    case 1:
      lcd.print("Change Status");
      break;
    case 2:
      lcd.print("Driving Time");
      break;
  }

}

bool checkButton( Button &button,
unsigned long debounceTime)
{
  bool reading = digitalRead(button.pin);

   if( reading !=  button.lastRead)
   {
    button.lastChange = millis();
   }

   if ( (millis()-button.lastChange) > debounceTime)
   {
    if( reading != button.stableState)
    {
      button.stableState = reading;

      if( button.stableState == LOW )
      {
        button.lastRead = reading;
        return true;
      }
    }
   }

   button.lastRead = reading;
   return false;

}

/*
 * Express Box build by Arduino Uno(DFRobot I/O Expansion Shield V7.1 used).
 * 
 * The circuit:
 * - I2C LCD1602 VCC attached to +5V
 * - I2C LCD1602 GND attached to GND
 * - I2C LCD1602 SCL attached to SCL or A5
 * - I2C LCD1602 SDA attached to SDA or A4
 * - 4×4 Keypad rows attached to D8,D9,D10,D11
 * - 4×4 Keypad cols attached to D4,D5,D6,D7
 * - Servo attached to D3,+5V,GND
 * - Digital Infrared Sensor attached to D12,+5V,GND
 * 
 * created 2018
 * modified 8 Nov 2019
 * by Xiao Yang 杨霄
 */

#include <EEPROM.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <Servo.h>

//#define INIT
//Initialize Express Box

LiquidCrystal_I2C lcd(0x27,16,2);
//Choose LiquidCrystal_I2C address:0x20,0x27

Servo myservo;
int pos=0;

#define INFRARED 12

const byte ROWS=4;
const byte COLS=4;
const char hexaKeys[ROWS][COLS]=
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS]={11,10,9,8};
byte colPins[COLS]={7,6,5,4};
Keypad customKeypad=Keypad(makeKeymap(hexaKeys),rowPins,colPins,ROWS,COLS);

String oldPassword="";//保存老密码
String newPassword="";//保存输入的修改密码
int error=0;//错误计数
String password="";//保存输入密码时的字符串
const unsigned int CloseTime=60*1000L;//关门等待时间
const unsigned long ErrDelayTime=5*60*1000L;//锁定时间

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
#ifdef INIT
  for(int i=0;i<=3;i++)
  {
    EEPROM.write(i,'0');//将EEPROM中0-3地址写入0，初始默认密码四个零
  }
  Serial.println("Express Box initialized.");
#endif
  for(int i=0;i<=3;i++)
  {
    char a=EEPROM.read(i);
    oldPassword+=a;
  }
  Serial.print("Old Password is:");
  Serial.println(oldPassword);
  delay(2000);//读取密码
  myservo.attach(3);//舵机
  lcd.init();
  lcd.backlight();
  lcd.print("Press any key.");//液晶屏
  pinMode(INFRARED,INPUT);//红外传感器
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(INFRARED)==LOW)
  {
    lcd.backlight();
    lcd.setCursor(0,1);
    lcd.print("Express arrived.");
    lcd.setCursor(9,0);
  }
  else if(digitalRead(INFRARED)==HIGH)
  {
    lcd.noBacklight();
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(9,0);
  }//红外检测
  char customKey=customKeypad.getKey();
  if(customKey!=0)
  {
    lcd.backlight();
    lcd.clear();
    lcd.home();
    lcd.print("Password:");
    bool flag1=true;
    int cursorPos=9;
    while(flag1)
    {
      customKey=customKeypad.getKey();
      if((customKey!=0)&&(customKey!='#'))
      {
        password+=customKey;
        lcd.print('*');
        cursorPos++;
        if((cursorPos>=14)||(customKey=='*'))
        {
          cursorPos=9;
          lcd.setCursor(cursorPos,0);
          lcd.print("     ");
          lcd.setCursor(cursorPos,0);
          password="";
        }
      }
      else if((password.length()==4)&&(customKey=='#'))
      {
        flag1=false;
      }
    }//输入和读取密码
    if(password==oldPassword)
    {
      password="";
      error=0;
      lcd.clear();
      lcd.setCursor(4,0);
      lcd.print("Bingo!");
      for(pos=0;pos<=90;pos+=1)
      {
        myservo.write(pos);
        delay(7.5);
      }
      delay(2000);
      lcd.home();
      lcd.print("Input'*'to exit.");
      bool flag2=true;
      unsigned long start=millis();//开始时间
      unsigned long now=millis();//当前时间
      while(flag2)
      {
        customKey=customKeypad.getKey();
        now=millis();
        Serial.print("Remaining time:");
        Serial.print(CloseTime-(now-start));
        Serial.println("ms");
        if((customKey=='*')||(now-start>=CloseTime))
        {
          flag2=false;
          for(pos=90;pos>=0;pos-=1)
          {
            myservo.write(pos);
            delay(7.5);
          }
          lcd.clear();
          delay(500);
          lcd.home();
          lcd.print("Change Password?");
          lcd.setCursor(0,1);
          lcd.print("A.YES       B.NO");
          bool flag3=true;
          while(flag3)
          {
            customKey=customKeypad.getKey();
            if(customKey=='A')
            {
              flag3=false;
              lcd.clear();
              lcd.home();
              lcd.print("The new:");
              bool flag4=true;
              cursorPos=8;
              while(flag4)
              {
                customKey=customKeypad.getKey();
                if((customKey!=0)&&(customKey!='#'))
                {
                  lcd.print(customKey);
                  newPassword+=customKey;
                  cursorPos++;
                  if((cursorPos>=13)||(customKey=='*'))
                  {
                    cursorPos=8;
                    lcd.setCursor(cursorPos,0);
                    lcd.print("     ");
                    lcd.setCursor(cursorPos,0);
                    newPassword="";
                  }
                }
                else if((newPassword.length()==4)&&(customKey=='#'))
                {
                  flag4=false;
                  Serial.print("New password is:");
                  Serial.println(newPassword);
                  lcd.clear();
                  lcd.home();
                  lcd.print("Save the new!");
                  delay(2000);
                  oldPassword=newPassword;
                  for(int i=0;i<=3;i++)
                  {
                      EEPROM.write(i,newPassword[i]);
                  }
                }
              }
              for(int i=0;i<=3;i++)
              {
                char a=EEPROM.read(i);
                Serial.print("EEPROM ");
                Serial.print(i);
                Serial.print(" is:");
                Serial.println(a);
              }
              lcd.clear();
              lcd.home();
              lcd.print("Press any key.");
            }//修改密码
            else if(customKey=='B')
            {
              flag3=false;
              lcd.clear();
              lcd.home();
              lcd.print("Press any key.");
            }//不修改
          }
        }
      }
    }//密码正确
    else if(password!=oldPassword)
    {
      password="";
      lcd.clear();
      lcd.setCursor(5,0);
      lcd.print("ERROR!");
      error++;
      delay(1500);
      lcd.clear();
      lcd.home();
      lcd.print("Press any key.");
    }//密码错误
    if(error>=3)
    {
      lcd.clear();
      lcd.home();
      lcd.print("Wait for 300s...");
      delay(1000);
      unsigned long start=millis();//开始时间
      unsigned long now=millis();//当前时间
      while(now-start<ErrDelayTime)
      {
        lcd.home();
        lcd.print("Wait for ");
        lcd.setCursor(9,0);
        lcd.print((300000-(now-start))/1000);
        lcd.print("s...  ");
        now=millis();
        char key=customKeypad.getKey();
        if(key!=0)
        {
          start=millis();
          now=millis();
        }
      }//延时
      lcd.clear();
      lcd.home();
      lcd.print("Press any key.");
    }//错到第三次
  }
}

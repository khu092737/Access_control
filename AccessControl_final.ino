#include <LiquidCrystal.h>
#include<MFRC522.h>
#include<SPI.h>
#include<Wire.h>
#include<Keypad.h>

#define RST_PIN 5
#define SS_PIN 53

//카드 데이터 저장
#define PICC_0 0xC2
#define PICC_1 0xAA
#define PICC_2 0x55
#define PICC_3 0x1B

MFRC522 rfid(SS_PIN, RST_PIN);
bool Rfid_match = false;
bool Next_acc = false;
bool emergency = false;

LiquidCrystal lcd(21,19,17,15,13,11);//rs,e,d4,d5,d6,d7

const byte ROWS = 4; //4행
const byte COLS = 3; //3열

byte rowPins[ROWS] = {31, 29, 27, 25}; //R1, R2, R3, R4
byte colPins[COLS] = {30, 28, 26}; //C1, C2, C3 (C4:사용X)


char hexaKeys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

Keypad customKeypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

char password[4] = {'1','2','3','4'}; //저장된 비번
char mastercode[8] = {'1','2','3','4','5','6','7','8'};//마스터코드
char input_pw[4]; // 숫자 입력 배열
char input_mc[8]; // 마스터 코드 입력
char input_mode; //모드 입력 변수


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(A1,OUTPUT); //문열림 LED
  pinMode(A0,OUTPUT); //문잠김 LED
  pinMode(A2,INPUT_PULLUP);
  pinMode(24,INPUT_PULLUP);

  SPI.begin(); //SPI초기화

  rfid.PCD_Init();
  lcd.begin(16,2); //
  lcd.clear();
  
  //digitalWrite(A0, HIGH);//문잠김 led상시 켜짐
  
}

void loop() {
  input_mode = customKeypad.getKey();
  // put your main code here, to run repeatedly:
  lcd.setCursor(0, 0);
  // print the number of seconds since reset:
  lcd.print("Lock");
  if(emergency == true)
  {
    digitalWrite(A0, LOW);//문잠김 led상시 꺼짐
    digitalWrite(A1, HIGH);//문열림 led상시 켜짐
    lcd.clear();
    lcd.print("Emergency!");
  }
  else
  {
    digitalWrite(A0, HIGH);//문잠김 led상시 켜짐
    digitalWrite(A1, LOW);//문열림 led상시 꺼짐
  }
    
  if(input_mode == '#')
  {
    lcd.clear();
    delay(500);
    Master_mode();
  }
  
  
  if(input_mode == '*')
  {
    lcd.clear();
    delay(500);
    Password_Check();
  }

  if(Next_acc == true)
  {
    lcd.clear();
    delay(500);
    lcd.print("Tag a card");
    delay(3000);
    Rfid_Check();
    
  }

  if(Rfid_match == true)
  {
    lcd.clear();
    delay(500);
    Door_Open();
  }

  if(digitalRead(24) == LOW || digitalRead(A2) == LOW)
  {
    if(emergency == true)
    {
      lcd.clear();
      lcd.print("Emergency clear!");
      delay(3000);
      lcd.clear();
      emergency = false;
      return emergency;
    }
    else
    {
      lcd.clear();
      delay(500);
      Door_Open();
    }
    
  }


}

void Password_Check()
{
  int i;
  char InputKey;

  lcd.print("enter password");
  lcd.setCursor(0, 1);
  for(i = 0; i < 4; i++)
  {
    InputKey = customKeypad.getKey();
    while(!InputKey)//InputKey가 false이면 while문 계속 실행, true이면 wile문 빠져나와 위에 for문 실행
    {
      InputKey = customKeypad.getKey();//키패드 값을 customKey에 저장
      if(InputKey)
      {
        input_pw[i] = InputKey;//입력한 비번을 비번 검증배열에 저장
      }
    }
    lcd.print("*");
  }


  if(input_pw[0] == password[0] && input_pw[1] == password[1] && input_pw[2] == password[2] && input_pw[3] == password[3])
  {
    delay(500);
    lcd.clear();
    lcd.print("good");
    delay(5000);
    Next_acc = true;
    return Next_acc;
  }
  
  else
  {
    delay(500);
    lcd.clear();
    lcd.print("wrong password");
    delay(5000);
    lcd.clear();
  }

}


void Door_Open()
{
  lcd.clear();
  lcd.print("door open!");
  digitalWrite(A0, LOW);
  digitalWrite(A1, HIGH);
  Serial.println("open!");

  delay(5000);
  input_mode = 0;
  Next_acc = false;
  Rfid_match = false;
  digitalWrite(A1, LOW);
  lcd.clear();

}

void Moving_Check()
{

}

void Rfid_Check()
{
  
  if (!rfid.PICC_IsNewCardPresent())
    return;

  //RF 카드의 ID가 인식 안되었다면 더 이상 진행하지 말고 빠져나감
  if (!rfid.PICC_ReadCardSerial())
    return;

  //PICC 타입 읽어오기
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  //MIFARE 방식이 아닐경우
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    lcd.clear();
    lcd.print("not this type");
  } 

  //카드 데이터가 동일할 경우
  if (rfid.uid.uidByte[0] == PICC_0 || rfid.uid.uidByte[1] == PICC_1 || rfid.uid.uidByte[2] == PICC_2 || rfid.uid.uidByte[3] == PICC_3 )
  {
    delay(500);
    lcd.clear();
    lcd.print("good");
    delay(5000);
    Rfid_match = true;
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    return Rfid_match;
  }
  
  //맞는 카드가 아니라면
  else
  {
    delay(500);
    lcd.clear();
    lcd.print("unmatched card");
    delay(5000);
  }

}

void Master_mode()
{
  int i;//반복문용 변수 선언
  char InputKey;

  lcd.print("master code:");
  lcd.setCursor(0, 1);
  for(i = 0; i < 8; i++)
  {
    InputKey = customKeypad.getKey();
    while(!InputKey)//InputKey가 false이면 while문 계속 실행, true이면 wile문 빠져나와 위에 for문 실행
    {
      InputKey = customKeypad.getKey();//키패드 값을 InputKey에 저장
      if(InputKey)
      {
        input_mc[i] = InputKey;//입력한 비번을 비번 검증배열에 저장
      }
    }
    lcd.print("*");
  }


  if(input_mc[0] == mastercode[0] && input_mc[1] == mastercode[1] && input_mc[2] == mastercode[2] && input_mc[3] == mastercode[3] && input_mc[4] == mastercode[4] && input_mc[5] == mastercode[5] && input_mc[6] == mastercode[6] && input_mc[7] == mastercode[7])
  {
    delay(500);
    lcd.clear();
    lcd.print("Emergency!");
    delay(5000);
    emergency = true;
    return emergency;
  }
  
  else
  {
    delay(500);
    lcd.clear();
    lcd.print("unmatched");
    delay(5000);
    lcd.clear();
  }

}
//-------------------------------------------------------------------
/*  Arduino LED Matrix 4x MAX7219 8x8 display - modules              *
 *  Digital Clock - Thermometer - Humidity                         * 
 *  Time, Date and brightness level can be controlled by button    * 
 *  has been modified and its work, trust me.                      *
 *  dibuat berdasarkan hasil 5C (COPAS, COMOT, COLEK, CUIL, CURI)  *
 *  Kemudian di ATM_BRI (Amati, Tiru, Modifikasi Buat Rangkaian Ini)
 */
//-------------------------------------------------------------------

//Libraries (Perpustakaan Arsip)
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Max72xxPanel.h>
#include <DHT.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Sodaq_DS3231.h>
#define SW0   A2  //tombol seting 1 menu
#define SW1   A1  //tombol seting 2 plus
#define SW2   A3  //tombol seting 3 minus
#define DHTPIN 5  //Data pin of DHT 11 
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
const int Led       = 9;   // for second indicator
const int Buzer     = 6;   // for buzzer
const int Ledalarm  = 2;   // led alarm indikator
const int photoCell = A0;  // LDR
DateTime now;
//char daysOfTheWeek[7][12] = {"Sunday","Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};  
char daysOfTheWeek[9][12] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jumat", "Sabtu","Minggu"}; 
char *pasar[] ={"Wage","Kliwon","Legi","Pahing","Pon"};              
const int pinCS = 10; //Attach CS to this pin, DIN to MOSI(11) and CLK to SCK(13) (cf http://arduino.cc/en/Reference/SPI )
const int numberOfHorizontalDisplays = 4; //jumlah panel matrix
const int numberOfVerticalDisplays = 1;
int kecepatan; // In milliseconds (text speed)
const int spacer = 1;
const int width = 5 + spacer; // The font width is 5 pixels
Max72xxPanel matrix = Max72xxPanel(pinCS, numberOfHorizontalDisplays, numberOfVerticalDisplays);
int length, photoCellValue, count = 0;
int hum, temp;
char suhu[4];
String msg, msg2, msg3, msg4;             //bukan micin
unsigned long previousMillis = 0;        //will store last time LED was updated
int alarmOnOff, bright, geser = 0;
int AH, AM, buz, jowo, mode, lama;
int alarm_state, alarm_state2;
boolean presentInput1; 
boolean presentInput2; 
boolean presentInput3; 
boolean alarmON = false;
boolean turnItOn = false;
void setup() {
  Serial.begin(9600); 
  pinMode(Buzer,OUTPUT);   
  pinMode(Led,OUTPUT);         
  pinMode(Ledalarm,OUTPUT);
  pinMode(SW0,INPUT_PULLUP);
  pinMode(SW1,INPUT_PULLUP);
  pinMode(SW2,INPUT_PULLUP);  
  dht.begin();  //baca sensor dht11  
  bright      = EEPROM.read(0);  //set kecerahan tampilan
  geser       = EEPROM.read(1);  //set kecepatan teks bergeser (scrolling)
  AH          = EEPROM.read(2);  //set alarm jam
  AM          = EEPROM.read(3);  //set alarm menit
  alarmOnOff  = EEPROM.read(4);  //set status alarm
  buz         = EEPROM.read(5);  //set nada tombol on/off
  jowo        = EEPROM.read(6);  //set tampil/tidak pasaran jawa
  mode        = EEPROM.read(7);  //set mode 24/12H
  lama        = EEPROM.read(8);  //set lama tampil jam (15,30,45,60 atau 75 detik)
  delay(10);  
  rtc.begin();  
  Wire.begin();
  Wire.beginTransmission(0x68);
  Wire.write(0x07); 
  Wire.write(0x10); 
  Wire.endTransmission();  
  matrix.setIntensity(bright); 
  matrix.setPosition(3, 0, 0); 
  matrix.setPosition(2, 1, 0); 
  matrix.setPosition(1, 2, 0); 
  matrix.setPosition(0, 3, 0); 
  matrix.setRotation(0, 3);  
  matrix.setRotation(1, 3);
  matrix.setRotation(2, 3);
  matrix.setRotation(3, 3);   
  Tiiit(); //(.)(.)\\
  Tiiit();//  )  (  \\
         //  (    )          
}//end setup
void loop() { 
  unsigned long currentMillis = millis();
  kedip_kedip(); 
  cek_tombol();  
  cek_tombol2(); 
  Tombol_Alarm();
  if(alarmON){Call_Alarm();}
  if(alarmOnOff==1){digitalWrite(Ledalarm,HIGH);}else{digitalWrite(Ledalarm,LOW);}     
  if(bright==0){
    photoCellValue = analogRead(photoCell);
    photoCellValue = map(photoCellValue,1023,0,0,15); 
    matrix.setIntensity(photoCellValue);} 
  else{matrix.setIntensity(bright);} 
  hum = dht.readHumidity();
  temp = dht.readTemperature()-2;        
  if (currentMillis - previousMillis >= 1000) {
    previousMillis = currentMillis;
    count++; 
    }
  if(lama==0){  
    if (count >=0 && count <15){
        waktu();
        cek_tombol(); 
        } 
    else if (count >= 15){
        scroll();      
        count=0;       
        }  
    }
  else if(lama==1){  
    if (count >=0 && count <30){
        waktu();
        cek_tombol(); 
        } 
    else if (count >= 30){
        scroll();      
        count=0;       
        }  
    }
  else if(lama==2){
    if (count >=0 && count <45){
        waktu();
        cek_tombol(); 
        } 
    else if (count >= 45){
        scroll();      
        count=0;       
        }  
    }
  else if(lama==3){
    if (count >=0 && count <60){
        waktu();
        cek_tombol(); 
        } 
    else if (count >= 60){
        scroll();      
        count=0;       
        }      
    } 
  else if(lama==4){
    if (count >=0 && count <75){
        waktu();
        cek_tombol(); 
        } 
    else if (count >= 75){
        scroll();      
        count=0;       
        }      
    }         
}//end loop
void waktu(){
   DateTime now = rtc.now();
   int HH = now.hour();  
   int H12 = now.hour()%12;   
   int MM = now.minute();
   static uint16_t   Pewaktu;
   uint16_t          Temer = millis();
   if(mode==1){ 
      if (H12<10){msg = "0" + String(H12);}
      else{msg = String(H12);}
      if (H12==0){msg = "12";}
   }
   else if(mode==0){    
      if (HH<10){msg = "0" + String(HH);}
      else{msg = String(HH);}
   }
   if(Temer-Pewaktu<500){msg += ":" ;}    
   else{msg += " ";}
   if(Temer-Pewaktu>1000){Pewaktu = Temer;}      
   if (MM<10){msg += "0" + String(MM);}
   else{msg += String(MM);}   
   length = msg.length() * width;
   for (int i = numberOfHorizontalDisplays * 8 - length; i>0; i--){msg += " ";}
   matrix.setCursor((numberOfHorizontalDisplays * 8 - length)/2,0); //Center text 
   matrix.fillScreen(LOW);
   matrix.print(msg);
   matrix.write();
}//end waktu
void scroll(){
  DateTime now = rtc.now();  
  int dd = now.date();
  int mm = now.month();
  int yyyy = now.year();
  String duadigit, teks;
  String msg9, msg10, msg11;
  msg9 = " ";
  msg10 = ". ";    
  if(geser==0)     {kecepatan=20;}
  else if(geser==1){kecepatan=30;}
  else if(geser==2){kecepatan=40;}
  else if(geser==3){kecepatan=50;}
  else if(geser==4){kecepatan=60;}
  else if(geser==5){kecepatan=70;}
  else if(geser==6){kecepatan=80;}
  else if(geser==7){kecepatan=90;}
  else if(geser==8){kecepatan=100;}    
  if (dd<10){msg2 = "0" + String(dd) + " ";}  
  else      {msg2 = String(dd) + " ";}  
  TwoDigit(now.month(),duadigit);
  if(now.month()==1)      {teks="Januari";}
  else if(now.month()==2) {teks="Februari";}
  else if(now.month()==3) {teks="Maret";}
  else if(now.month()==4) {teks="April";}
  else if(now.month()==5) {teks="Mei";}
  else if(now.month()==6) {teks="Juni";}
  else if(now.month()==7) {teks="Juli";}
  else if(now.month()==8) {teks="Agustus";}
  else if(now.month()==9) {teks="September";}  
  else if(now.month()==10){teks="Oktober";}
  else if(now.month()==11){teks="November";}
  else if(now.month()==12){teks="Desember";}    
  msg2 += teks;
  msg2 += " ";  
  msg2 += String(yyyy);           
  msg2 += ". ";       
  msg3 = "Suhu:";
  msg3 += temp;  
  msg3 += "*C";  
  msg3 += ". ";
  msg4 = "Hum:";
  msg4 += hum;  
  msg4 += "%";      
  if(jowo==1){msg = daysOfTheWeek[now.dayOfWeek()] + msg9 + String(pasar[jumlahhari()%5]) + msg10 + msg2 + msg3 + msg4;}
  else{msg = daysOfTheWeek[now.dayOfWeek()] + msg10 + msg2 + msg3 + msg4;}                
  for ( int i = 0 ; i < width * msg.length() + matrix.width() - 1 - spacer; i++ ) { 
    matrix.fillScreen(LOW);
    int letter = i / width;
    int x = (matrix.width() - 1) - i % width;
    int y = (matrix.height() - 8) / 2; 
    while ( x + width - spacer >= 0 && letter >= 0 ) {
      if ( letter < msg.length() ) {
         matrix.drawChar(x, y, msg[letter], HIGH, LOW, 1);
         }
      letter--;
      x -= width;
    }
    matrix.write(); //kirim gambar untuk ditampilkan    
    delay(kecepatan);
    }
}
void fullOn(){
  matrix.fillScreen(HIGH);
  matrix.write();}
void fullOff(){
  matrix.fillScreen(LOW);
  matrix.write();}
void TwoDigit(int digit, String &hasilDigit){
  hasilDigit = "";
  if (digit < 10){
      hasilDigit += "0";
      hasilDigit += digit;      }
  else{hasilDigit += digit;}}
void kedip_kedip(){
  DateTime now = rtc.now();
  int detik = now.second()%2;
  if(detik!=0){digitalWrite(Led,HIGH);}
  else        {digitalWrite(Led, LOW);}}
void Tiiit(){
  digitalWrite(Buzer,HIGH);
  delay(50);
  digitalWrite(Buzer,LOW);
  delay(50);}
void Tombol_Alarm(){
  presentInput2 = digitalRead(SW2); 
  alarmOnOff = EEPROM.read(4);
  if (presentInput2==LOW){
      if(alarmON){
        alarmON = false;
        turnItOn = false; 
        digitalWrite(Ledalarm,LOW);
        for(int z=0;z<300;z++){notifikasi_alarm_off();}
      }
      else{
        alarmON = true;
        digitalWrite(Ledalarm,HIGH);
        for(int z=0;z<300;z++){notifikasi_alarm_on();}
      }
      delay(300);
    }    
}
void Call_Alarm(){  
  alarm_state2 = digitalRead(SW2);  
  DateTime now = rtc.now();
  int jam = now.hour();
  int j12 = now.hour()%12;
  int mnt = now.minute();
  int det = now.second();    
  if(mnt==(AM+2)){turnItOn = false;}   
  if(mode==0){if(jam==AH && mnt==AM && alarmOnOff==1){turnItOn = true; waktu();}}
  else if(mode==1){
     if(j12>11){j12=12;}
     if(j12>12){j12=1;}
     if(j12==AH && mnt==AM && alarmOnOff==1){turnItOn = true; waktu();}}        
     if (turnItOn==true){
         DateTime now = rtc.now();
         waktu(); 
         digitalWrite(Buzer,HIGH); digitalWrite(Ledalarm,HIGH);
         delay(50);
         digitalWrite(Buzer,LOW);  digitalWrite(Ledalarm,LOW);
         delay(50);
         digitalWrite(Buzer,HIGH); digitalWrite(Ledalarm,HIGH);
         delay(50);
         digitalWrite(Buzer,LOW);  digitalWrite(Ledalarm,LOW);
         delay(50);
         digitalWrite(Buzer,HIGH); digitalWrite(Ledalarm,HIGH);
         delay(50);
         digitalWrite(Buzer,LOW);  digitalWrite(Ledalarm,LOW);
         delay(50);
         digitalWrite(Buzer,HIGH); digitalWrite(Ledalarm,HIGH);
         delay(250);
         digitalWrite(Buzer,LOW);  digitalWrite(Ledalarm,LOW);
         delay(250);         
         }
     else{digitalWrite(Buzer,LOW); digitalWrite(Ledalarm,LOW);}           
}
void cek_tombol(){
  byte menu = 0;
  int jam, j12, mnt, dtk, tgl, bln, thn, hr;
  if(digitalRead(SW0)==LOW){
    delay(300);  
    DateTime now = rtc.now();
    jam = now.hour();
    j12 = now.hour()%12;
    mnt = now.minute();
    dtk = now.second();
    tgl = now.date();  
    bln = now.month();
    thn = now.year()-2000;
    hr = now.dayOfWeek();
    menu = 1;
    if(buz==1){Tiiit();} else{}}  
  while(menu == 1){
    DateTime now = rtc.now();
    if(digitalRead(SW1)==LOW){
      delay(300);
      if(buz==1){Tiiit();} else{}
      if(mode==0){
        jam++;
        if(jam>23){jam=0;}
        }
      else if(mode==1){
        j12++;
        if(j12>12){j12=1;}
        }
      }
    if(digitalRead(SW2)==LOW){
      delay(300);
      if(buz==1){Tiiit();} else{}      
      if(mode==0){
        jam--;
        if(jam<0){jam=23;}
        }
      else if(mode==1){
        j12--;
        if(j12<1){j12=12;}
        }
      }
    if(digitalRead(SW0)==LOW){
      delay(300);
      if(buz==1){Tiiit();} else{}
      menu = 2;
      }      
    if(jam<10){msg = "J:0" + String(jam);} else{msg = "J:" + String(jam);}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 1
  while(menu == 2){
    DateTime now = rtc.now();
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      mnt++;
      if(mnt>59){mnt=0;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      mnt--;
      if(mnt<0){mnt=59;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 3;
      }    
    if(mnt<10){msg = "M:0" + String(mnt);} else{msg = "M:" + String(mnt);}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 2
  while(menu == 3){
    DateTime now = rtc.now();
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      tgl++;
      if(tgl>31){tgl=1;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      tgl--;
      if(tgl<1){tgl=31;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 4;
      }
    if(tgl<10){msg = "Tg:0" + String(tgl);} else{msg = "Tg:" + String(tgl);}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 3
  while(menu == 4){
    DateTime now = rtc.now();
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      bln++;
      if(bln>12){bln=1;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      bln--;
      if(bln<1){bln=12;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 5;
      }
    if(bln<10){msg = "Bl:0" + String(bln);} else{msg = "Bl:" + String(bln);}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 4      
   while(menu == 5){
    DateTime now = rtc.now();
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      thn++;
      if(thn>99){thn=19;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      thn--;
      if(thn<19){thn=99;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 6;
      }
    if(thn<10){msg = "Th:0" + String(thn);} else{msg = "Th:" + String(thn);}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 5 
    while(menu == 6){
    DateTime now = rtc.now();
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      hr++;
      if(hr>6){hr=0;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      hr--;
      if(hr<0){hr=6;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      DateTime dt(thn, bln, tgl, jam, mnt, 30, hr);
      rtc.setDateTime(dt);           // Adjust date-time as defined 'dt' above  
      for(int z=0;z<500;z++){notifikasi_simpan();}
      menu = 0;
      }
    msg = String(daysOfTheWeek[hr]);
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays * 8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 6     
}//end cek_tombol
void notifikasi_simpan(){
  msg = "Save";
  length = msg.length() * width;
  for (int i = 8; i>0; i--){msg += " ";}
  matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
  matrix.fillScreen(LOW);
  matrix.print(msg);
  matrix.write();}
void notifikasi_alarm_off(){
  msg = "$ OFF";
  length = msg.length() * width;
  for (int i = 8; i>0; i--){msg += " ";}
  matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
  matrix.fillScreen(LOW);
  matrix.print(msg);
  matrix.write();}
void notifikasi_alarm_on(){
  msg = "$ ON";
  length = msg.length() * width;
  for (int i = 8; i>0; i--){msg += " ";}
  matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
  matrix.fillScreen(LOW);
  matrix.print(msg);
  matrix.write();}
void cek_tombol2(){
  byte menu = 0;
  presentInput1 = digitalRead(SW1); 
  presentInput2 = digitalRead(SW2); 
  bright      = EEPROM.read(0);
  geser       = EEPROM.read(1);
  AH          = EEPROM.read(2);
  AM          = EEPROM.read(3);
  alarmOnOff  = EEPROM.read(4);
  buz         = EEPROM.read(5);
  jowo        = EEPROM.read(6);
  mode        = EEPROM.read(7);
  lama        = EEPROM.read(8); 
  if(presentInput1==LOW){ 
    delay(300); if(buz==1){Tiiit();} else{} 
    menu = 1;
    } 
  while(menu==1){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      lama++;
      if(lama>4){lama=0;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      lama--;
      if(lama<0){lama=4;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 2;
      }    
    if(lama==0)     {msg = "LT:15";} 
    else if(lama==1){msg = "LT:30";}
    else if(lama==2){msg = "LT:45";}
    else if(lama==3){msg = "LT:60";}
    else if(lama==4){msg = "LT:75";}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    }//end menu 1
  while(menu==2){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      bright++;
      if(bright>15){bright=0;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      bright--;
      if(bright<0){bright=15;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 3;
      }    
    if((bright>0)&&(bright<10)){msg = "Br:0" + String(bright);} else{msg = "Br:" + String(bright);}
    if(bright==0){msg = "Auto";}
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    matrix.setIntensity(bright); //set kecerahan
    }//end menu 2  
  while(menu==3){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      geser++;
      if(geser>8){geser=0;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      geser--;
      if(geser<0){geser=8;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 4;
      }
    msg = "Scr:" + String(geser);
    length = msg.length() * width;
    for (int i = 8; i>0; i--){msg += " ";}
    matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
    matrix.fillScreen(LOW);
    matrix.print(msg);
    matrix.write();
    if(geser==0){kecepatan=20;}
    else if(geser==1){kecepatan=30;}
    else if(geser==2){kecepatan=40;}
    else if(geser==3){kecepatan=50;}
    else if(geser==4){kecepatan=60;}
    else if(geser==5){kecepatan=70;}
    else if(geser==6){kecepatan=80;}
    else if(geser==7){kecepatan=90;}
    else if(geser==8){kecepatan=100;}
    delay(kecepatan);
    }//end menu 3   
    while(menu==4){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      buz = 1;
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      buz = 0;
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 5;
      }    
      if(buz==1){msg = "b:ON";} else if(buz==0){msg = "b:OFF";}
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
    }//end menu 4
    while(menu==5){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      jowo = 0;
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      jowo = 1;
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 6;
      }    
      if(jowo==1){msg = "Pj:ON";} else{msg = "Pj:OF";}
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
    }//end menu 5
    while(menu==6){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      mode = 0;
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      mode = 1;
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 7;
      }    
      if(mode==1){msg = "12Hr";} else{msg = "24Hr";}
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
    }//end menu 6
    while(menu==7){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      alarmOnOff=1;
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      alarmOnOff=0;
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      if(alarmOnOff==1){        
        alarmON = true;
        menu = 8;
        }
      else if(alarmOnOff==0){
        EEPROM.write(0,bright);
        EEPROM.write(1,geser);
        EEPROM.write(4,alarmOnOff);
        EEPROM.write(5,buz);
        EEPROM.write(6,jowo);
        EEPROM.write(7,mode);
        EEPROM.write(8,lama);
        for(int z=0;z<500;z++){notifikasi_simpan();}
        menu = 0;
        }
      }          
    if(alarmOnOff==1){  
      msg = "$ ON";
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
      digitalWrite(Ledalarm,HIGH);
      }
    else if(alarmOnOff==0){  
      msg = "$ OFF";
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
      digitalWrite(Ledalarm,LOW);
      }  
    }  
  while(menu==8){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      AH++;
      if(mode==0) {if(AH>23){AH=0;}}
      else if(mode==1){if(AH>12){AH=1;}}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      AH--;
      if(mode==0){if(AH<0){AH=23;}}
      else if(mode==1){if(AH<1){AH=12;}}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      menu = 9;
      }    
      if(AH<10){msg = "AJ:0" + String(AH);} else{msg = "AJ:" + String(AH);}
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
    }//end menu 8  
  while(menu==9){
    if(digitalRead(SW1)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      AM++;
      if(AM>59){AM=0;}
      }
    if(digitalRead(SW2)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      AM--;
      if(AM<0){AM=59;}
      }
    if(digitalRead(SW0)==LOW){
      delay(300);if(buz==1){Tiiit();} else{}
      EEPROM.write(0,bright);
      EEPROM.write(1,geser);
      EEPROM.write(2,AH);
      EEPROM.write(3,AM);
      EEPROM.write(4,alarmOnOff);
      EEPROM.write(5,buz);
      EEPROM.write(6,jowo);
      EEPROM.write(7,mode);
      EEPROM.write(8,lama);
      for(int z=0;z<500;z++){notifikasi_simpan();}
      menu = 0;
      }    
      if(AM<10){msg = "AM:0" + String(AM);} else{msg = "AM:" + String(AM);}
      length = msg.length() * width;
      for (int i = 8; i>0; i--){msg += " ";}
      matrix.setCursor((numberOfHorizontalDisplays*8-length)/2,0); //Center text 
      matrix.fillScreen(LOW);
      matrix.print(msg);
      matrix.write();
    }//end menu 9       
}
//digunakan untuk menghitung hari (lagu Krisdayanti) pasaran jawa
  int jumlahhari(){ 
  DateTime  now = rtc.now(); 
  int d = now.date();
  int m = now.month();
  int y = now.year();
  int hb[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365};
  int ht = (y - 1970) * 365 - 1;
  int hs = hb[m - 1] + d;
  int kab = 0;
  int i;
  if(y % 4 == 0) {
    if(m > 2) {
    hs++;
    }
  }
  for(i = 1970; i < y; i++) {
    if(i % 4 == 0) {
    kab++;
    }
  }
  return (ht + hs + kab); 
}
//===================================end/selesai=====================================//

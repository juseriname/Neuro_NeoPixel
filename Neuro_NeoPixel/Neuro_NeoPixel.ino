#include <SoftwareSerial.h>
#include <Adafruit_NeoPixel.h>

SoftwareSerial mySerial(2, 3); // TX, RX

///neopixel_setup
// Which pin on the Arduino is connected to the NeoPixels?
#define NEO1 11
#define NEO2 10
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS 64

Adafruit_NeoPixel neos1(NUMPIXELS, NEO1, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel neos2(NUMPIXELS, NEO2, NEO_GRB + NEO_KHZ800);

#define BAUDRATE 57600
#define DEBUGOUTPUT 0

// checksum variables
byte generatedChecksum = 0;
byte checksum = 0;
int payloadLength = 0;
byte payloadData[64] = {0};
byte poorQuality = 0;
byte attention = 0;
byte meditation = 0;
byte etc = 0;

// system variables
long lastReceivedPacket = 0;
boolean bigPacket = false;
boolean brainwave = false;

void setup()
{
  ///neo1
  neos1.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  neos1.show();            // 네오픽셀에 빛을 출력하기 위한 것인데 여기서는 모든 네오픽셀을 OFF하기 위해서 사용한다.
  neos1.setBrightness(50); // 네오픽셀의 밝기 설정(최대 255까지 가능)
  ///neo2
  neos2.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  neos2.show();            // 네오픽셀에 빛을 출력하기 위한 것인데 여기서는 모든 네오픽셀을 OFF하기 위해서 사용한다.
  neos2.setBrightness(50); // 네오픽셀의 밝기 설정(최대 255까지 가능)

  Serial.begin(BAUDRATE);           // USB
  mySerial.begin(BAUDRATE);

}

////////////////////////////////
// Read data from Serial UART //
////////////////////////////////
byte ReadOneByte()

{
  int ByteRead;
  while (!mySerial.available());
  ByteRead = mySerial.read();

#if DEBUGOUTPUT
  mySerial.print((char)ByteRead);   // echo the same byte out the USB serial (for debug purposes)
#endif

  return ByteRead;
}

unsigned int delta_wave = 0;
unsigned int theta_wave = 0;
unsigned int low_alpha_wave = 0;
unsigned int high_alpha_wave = 0;
unsigned int low_beta_wave = 0;
unsigned int high_beta_wave = 0;
unsigned int low_gamma_wave = 0;
unsigned int mid_gamma_wave = 0;

void read_waves(int i) {
  delta_wave = read_3byte_int(i);
  i += 3;
  theta_wave = read_3byte_int(i);
  i += 3;
  low_alpha_wave = read_3byte_int(i);
  i += 3;
  high_alpha_wave = read_3byte_int(i);
  i += 3;
  low_beta_wave = read_3byte_int(i);
  i += 3;
  high_beta_wave = read_3byte_int(i);
  i += 3;
  low_gamma_wave = read_3byte_int(i);
  i += 3;
  mid_gamma_wave = read_3byte_int(i);
}

int read_3byte_int(int i) {
  return ((payloadData[i] << 16) + (payloadData[i + 1] << 8) + payloadData[i + 2]);
}


/////////////
//MAIN LOOP//
/////////////
void loop()
{
  unsigned long currentMillis = millis();
  // Look for sync bytes
  if (ReadOneByte() == 170)
  {
    if (ReadOneByte() == 170)
    {
      payloadLength = ReadOneByte();

      if (payloadLength > 169)                     //Payload length can not be greater than 169
        return;
      generatedChecksum = 0;
      for (int i = 0; i < payloadLength; i++)
      {
        payloadData[i] = ReadOneByte();            //Read payload into memory
        generatedChecksum += payloadData[i];
      }

      checksum = ReadOneByte();                      //Read checksum byte from stream
      generatedChecksum = 255 - generatedChecksum;   //Take one's compliment of generated checksum

      if (checksum == generatedChecksum)
      {
        poorQuality = 200;
        attention = 0;
        meditation = 0;

        for (int i = 0; i < payloadLength; i++)
        { // Parse the payload
          switch (payloadData[i])
          {
            case 0x02:
              i++;
              poorQuality = payloadData[i];
              bigPacket = true;
              break;
            case 0x04:
              i++;
              attention = payloadData[i];
              break;
            case 0x05:
              i++;
              meditation = payloadData[i];
              break;
            case 0x16:
              i++;
              etc = payloadData[i];
              break;
            case 0x80:
              i = i + 3;
              break;
            case 0x83:                         // ASIC EEG POWER INT
              i++;
              brainwave = true;
              byte vlen = payloadData[i];
              //mySerial.print(vlen, DEC);
              //mySerial.println();
              read_waves(i + 1);
              i += vlen; // i = i + vlen
              break;
          } // switch
        } // for loop

#if !DEBUGOUTPUT

        // *** Add your code here ***
        neos1.clear();
        neos2.clear();
        if (bigPacket)
        {
          Serial.print("===========================\n");
          //Serial.print("here\n");
          if (poorQuality == 0) {
            ///neo1
            SetColor(neos1.Color(169,0,0),int(attention/1.5625),true);
            ///neo2
            SetColor(neos2.Color(29,117,183),int(meditation/1.5625),false);
            Serial.print("neo1 ");
            Serial.print(int(attention/1.5625));
            Serial.print(" turn ON.\n");
            Serial.print("neo2 ");
            Serial.print(int(meditation/1.5625));
            Serial.print(" turn ON.\n");
          }
          else 
          {
              SetColor(neos1.Color(int(poorQuality*1.275),int(poorQuality*1.275),int(poorQuality*1.275)),int(attention/1.5625),true);
              SetColor(neos2.Color(int(poorQuality*1.275),int(poorQuality*1.275),int(poorQuality*1.275)),int(meditation/1.5625),false);
          }

          Serial.print("===========================\n");
          Serial.print("PoorQuality: ");
          Serial.print(poorQuality, DEC);
          Serial.println();
          Serial.print("Attention: "); ///here attention
          Serial.print(attention, DEC);
          Serial.println();
          Serial.print("Meditation: ");
          Serial.print(meditation, DEC);
          Serial.println();
//          Serial.print("Blink eye : ");
//          Serial.print(etc, DEC);
//          Serial.println();
//          Serial.print("Delta value is: ");
//          Serial.print(delta_wave, DEC);
//          Serial.println();
//          Serial.print("Theta value is: ");
//          Serial.print(theta_wave, DEC);
//          Serial.println();
//          Serial.print("Low Alpha value is: ");
//          Serial.print(low_alpha_wave, DEC);
//          Serial.println();
//          Serial.print("High Alpha value is: ");
//          Serial.print(high_alpha_wave, DEC);
//          Serial.println();
//          Serial.print("Alertness value1 is: ");
//          Serial.print(low_beta_wave, DEC);
//          Serial.println();
//          Serial.print("Alertness value2 is: ");
//          Serial.print(high_beta_wave, DEC);
//          Serial.println();
//          Serial.print("Low gamma wave is: ");
//          Serial.print(low_gamma_wave, DEC);
//          Serial.println();
//          Serial.print("Mid gamma wave is: ");
//          Serial.print(mid_gamma_wave, DEC);
//          Serial.println();
//          Serial.print("Time since last packet: ");
//          Serial.print(millis() - lastReceivedPacket, DEC);
//          lastReceivedPacket = millis();
//          Serial.print("\n"); Serial.print("\n");




        }
#endif
        bigPacket = false;
      }
      else {
        // Checksum Error
      }  // end if else for checksum
    } // end if read 0xAA byte
  } // end if read 0xAA byte
}

void SetColor(uint32_t color, int amount, bool isneo1) {
  if(isneo1)
  {
    for (int i = 0; i < amount; i++) { // For each pixel in strip...
      neos1.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      neos1.show();                          //  Update strip to match
      //Serial.print("neo1 show!!\n");
    }
  }
  else
  {
    for (int i = 0; i < amount; i++) { // For each pixel in strip...
      neos2.setPixelColor(i, color);         //  Set pixel's color (in RAM)
      neos2.show();                          //  Update strip to match
      //Serial.print("neo2 show!!\n");
    }
  }
  
}

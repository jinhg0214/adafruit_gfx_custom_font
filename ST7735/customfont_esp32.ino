#include <Adafruit_GFX.h>    // Core graphics library
#include <XTronical_ST7735.h> // Hardware-specific library
#include <SPI.h>

#include <Fonts/FreeMono18pt7b.h> // adafruit gfx 제공 폰트
#include "SymbolMono18pt7b.h" // adafruit  예제 제공 커스텀 폰트 

// set up pins we are going to use to talk to the screen
#define TFT_DC     2       // register select (stands for Data Control perhaps!)
#define TFT_RST   4         // Display reset pin, you can also connect this to the ESP32 reset
                            // in which case, set this #define pin to -1!
#define TFT_CS   5       // Display enable (Chip select), if not enabled will not talk on SPI bus

#define DWIDTH 128 
#define DHEIGHT 128

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);  

uint8_t  Magnifier;
uint16_t First_Glyph;

void setup() {
  Serial.begin(115200);
  tft.init();   // initialize a ST7735S chip
  tft.setRotation(2);

  delay (1000);
  Magnifier=1;
  First_Glyph=0;

  Show ();
}

void loop() {
  int16_t val;
  if(Serial.available()) {
    val= Serial.parseInt();
    if (val<0) {
      Magnifier=-val;
    } else {
      First_Glyph=val;
    }
    while(Serial.available())  {Serial.read();};
    Show ();
  }
  delay(1000);
}

/*
 * Use this function instead of display.drawChar to draw the symbol or to use
 * the default font if it's not in the symbol range.
 */
void drawSymbol(uint16_t x, uint16_t y, uint8_t c, uint16_t color, uint16_t bg, uint8_t Size){
  if( (c>=32) && (c<=126) ){ //If it's 33-126 then use standard mono 18 font
      tft.setFont(&FreeMono18pt7b);
  } else {
    tft.setFont(&SymbolMono18pt7b);//Otherwise use special symbol font
    if (c>126) {      //Remap anything above 126 to be in the range 32 and upwards
      c-=(127-32);
    }
  }
  tft.drawChar(x,y,c,color,bg,Size);
}



/*
 * Display characters on the screen starting with First_Glyph until you hit
 * last character of the font or run out of space on the screen.
 */
void Show(void)  {
  /*
   * Although our characters are at most 16 pixels wide, we're going to space them
   * an arbitrary horizontal amount DELTA_C for readability. Similarly the vertical 
   * spacing is set at DELTA_R for readability. The BASE_R is a vertical adjustment 
   * that is the distance from the top of the cell to the baseline of the character.
   */
  #define DELTA_C 21
  #define DELTA_R 30
  #define BASE_R 22
  uint8_t Max_C = DWIDTH / (DELTA_C * Magnifier);
  uint8_t Max_R = DHEIGHT / (DELTA_R * Magnifier);
  uint8_t NumChar = Max_C * Max_R;
  uint16_t Last_Glyph = min(First_Glyph+NumChar-1,SymbolMono18pt7b.last+128-32);
  Serial.print("Displaying "); Serial.print(NumChar,DEC);
  Serial.print(" glyphs in "); Serial.print(Max_R,DEC);
  Serial.print(" rows by "); Serial.print(Max_C,DEC);
  Serial.println(" columns.");
  Serial.print("Magnifier:"); Serial.println(Magnifier,DEC);
  Serial.print("Displaying characters:"), 
  Serial.print(First_Glyph,DEC); 
  Serial.print(" through ");
  Serial.println(Last_Glyph,DEC);
  Serial.println();
  tft.fillScreen(ST7735_BLACK);  //Clear the screen
  if(Magnifier>1){    //If magnified, draw a grid for reference
    for (uint16_t C=0; C<DWIDTH; C=C+(DELTA_C*Magnifier)) {
      tft.drawFastVLine(C,0,DHEIGHT,ST7735_YELLOW); 
    }
    for (uint16_t R=0; R<DHEIGHT; R=R+(DELTA_R*Magnifier)) {
      tft.drawFastHLine(0,R,DWIDTH,ST7735_YELLOW); 
    }
  }
  uint8_t I=0;
  for (uint8_t Glyph = First_Glyph; Glyph <= Last_Glyph; Glyph++) {
    drawSymbol((I % Max_C) * DELTA_C * Magnifier,
        (BASE_R*Magnifier) + (I / Max_C) * DELTA_R * Magnifier,
        Glyph,ST7735_WHITE,ST7735_BLACK,Magnifier);
    I++;
  }
}

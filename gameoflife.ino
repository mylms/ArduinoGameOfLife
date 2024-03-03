/*
 Name:    GameOfLife_1.ino
 Created: 3/7/2017 7:56:21 PM
 Author:  mylms.cz
*/

#define BLACK 0x0000
#define YELLOW 0xFFE0
#define WHITE 0xFFFF
#define MYLMSCYAN 0x659F
#define MYLMSGREEN 0x4DFA

#define BUTTON1 2
#define BUTTON2 3
#define LCDLIGHT 9

#define WORLDSIZE 30  //velikost hracího pole
#define TIMETODARK 60 //čas do zhasnutí displeje

#include <SPI.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_GFX.h>


Adafruit_ILI9341 tft = Adafruit_ILI9341(5, 6, 11, 13, 4, 12);

bool oldArray[WORLDSIZE][WORLDSIZE];
bool newArray[WORLDSIZE][WORLDSIZE];

byte rndNumber; //náhodné číslo pro generování
byte neighbors; //počet sousedů okolo buňky

unsigned int generation;  //číslo aktuální generace
byte population;  //počet buněk
byte lastPopulation;  //počet předchozích buněk

byte graphPossition;  //pozice vykreslování grafu

int ax; //pomocná proměnná pro vykreslování (musí být int)
int ay; //pomocná proměnná pro vykreslování (musí být int)
int bx; //pomocná proměnná pro vykreslování (musí být int)
int by; //pomocná proměnná pro vykreslování (musí být int)

byte lightingTime;  //aktální čas svícení

long time_setDisplay;
long time_grafika;




void setup(void){
  //Serial.begin(9600);

  pinMode(LCDLIGHT, OUTPUT);  //podsvícení LCD
  pinMode(BUTTON1, INPUT);  //tlačítko LCD
  pinMode(BUTTON2, INPUT);  //podsvícení LCD

  randomSeed(analogRead(A0)); //generátor náhodných čísel

  tft.begin();  //inicializace LCD
  tft.fillScreen(ILI9341_BLACK);  //vymazání LCD

  //digitalWrite(LCDLIGHT, HIGH); //rozsvícení displeje
  intro();
  
  naplneni(); //počáteční naplnení náhodnými buňkami
}

void loop(void){
  //stisk tlacitka
  if (millis() >= time_setDisplay + 50) {
    time_setDisplay = millis();
    setDisplay();
  }

  //vykreslení grafiky
  if (millis() >= time_grafika + 500) {
    time_grafika = millis();
    grafika();
  }
}

void intro() {
  tft.setCursor(25, 20);
  tft.setTextColor(MYLMSCYAN);  tft.setTextSize(4);
  tft.print("mylms.cz");
  tft.setTextColor(MYLMSGREEN);  tft.setTextSize(2);
  tft.setCursor(40, 80);
  tft.print("Game of Life");

  digitalWrite(LCDLIGHT, HIGH); //rozsvícení displeje

  delay(2000);   //nojo, nojo... tady to ale nevadí :)

  tft.fillScreen(ILI9341_BLACK);  //vymazání LCD

  tft.setCursor(5, 242);
  tft.setTextColor(RED);  tft.setTextSize(1);
  tft.print("Generace: ");
  tft.setCursor(155, 242);
  tft.print("Populace: ");
}

void grafika() {
  //kompletní obsluha grafiky

  vypocet();  //vypočet grafiky

  if (lightingTime < TIMETODARK) {
    // svítí displej
    vykresleni(); //pokud displej svítí, tak vykresluj
    lightingTime++; //přičtění lighting time (pro zhasnutí displeje)
  }
  
}

void setDisplay() {
  //obsluha podsvětlení displeje
  if (digitalRead(BUTTON1) && digitalRead(BUTTON2)) {
    digitalWrite(LCDLIGHT, HIGH); //rozsvícení displeje
    lightingTime = 0;
  }

  if (lightingTime >= TIMETODARK) {
    //zhasnutí displeje po nastaveném čase
    digitalWrite(LCDLIGHT, LOW);  //zhasnutí displeje
  }

}

void naplneni() {
  //naplnění pole náhodnými hodnotami

  for (byte x = 0; x < WORLDSIZE; x++) {
    for (byte y = 0; y < WORLDSIZE; y++) {
      
      rndNumber = random(255);

      if(rndNumber >= 220) {
        oldArray[x][y] = true;
      }
      else {
        oldArray[x][y] = false;
      }
    }
  }
}

void vykresleni() {
  byte cellSize = 240 / WORLDSIZE;

    for (byte x = 0; x < WORLDSIZE; x++) {
      for (byte y = 0; y < WORLDSIZE; y++) {


        if (oldArray[x][y] != newArray[x][y]) {

          if (newArray[x][y]) {
            //živá buňka
            tft.fillRect(x * cellSize, y * cellSize, cellSize - 2, cellSize - 2, WHITE);

          }
          else {
            //mrtvá buňka
            tft.fillRect(x * cellSize, y * cellSize, cellSize - 2, cellSize - 2, BLACK);
          }
        }

        oldArray[x][y] = newArray[x][y];

      }
    }



  //vykreslení čísla generace

  graphPossition++;

  //Generace
  tft.fillRect(60, 242, 50, 8, BLACK);  //maskování
  tft.setCursor(60, 242);
  tft.print(generation);

  //Populace
  tft.fillRect(210, 242, 25, 8, BLACK); //maskování
  tft.setCursor(210, 242);
  tft.print(population);

  //Graf
  if (graphPossition == 240) {
    graphPossition = 0;
    tft.fillRect(0, 250, 240, 80, BLACK);
  }

  tft.drawLine(graphPossition, 319, graphPossition, 319 - map(population, 0, 255, 0, 75), CYAN);
}

void vypocet() {
  byte cellSize = 240 / WORLDSIZE;
  population = 0; //reset populace před sečtením
  generation++; //další generace buněk

  for (byte x = 0; x < WORLDSIZE; x++) {
    for (byte y = 0; y < WORLDSIZE; y++) {

    
      neighbors = pocetSousedu(x,y);

      
      //zobrazení počtu okolních buněk
      /*
      switch (neighbors) {
      case 0:
        tft.drawChar(x * cellSize, y * cellSize, '0', RED, YELLOW, 1);
        break;
      case 1:
        tft.drawChar(x * cellSize, y * cellSize, '1', RED, YELLOW, 1);
        break;
      case 2:
        tft.drawChar(x * cellSize, y * cellSize, '2', RED, YELLOW, 1);
        break;
      case 3:
        tft.drawChar(x * cellSize, y * cellSize, '3', RED, YELLOW, 1);
        break;
      case 4:
        tft.drawChar(x * cellSize, y * cellSize, '4', RED, YELLOW, 1);
        break;
      case 5:
        tft.drawChar(x * cellSize, y * cellSize, '5', RED, YELLOW, 1);
        break;
      case 6:
        tft.drawChar(x * cellSize, y * cellSize, '6', RED, YELLOW, 1);
        break;
      case 7:
        tft.drawChar(x * cellSize, y * cellSize, '7', RED, YELLOW, 1);
        break;
      case 8:
        tft.drawChar(x * cellSize, y * cellSize, '8', RED, YELLOW, 1);
        break;
      case 9:
        tft.drawChar(x * cellSize, y * cellSize, '9', RED, YELLOW, 1);
        break;
      case 10:
        tft.drawChar(x * cellSize, y * cellSize, '10', RED, YELLOW, 1);
        break;
      default:
        tft.drawChar(x * cellSize, y * cellSize, 'X', RED, YELLOW, 1);
        break;

      }
      */
      

      if (oldArray[x][y]) {
        //ziva bunka

        if ((neighbors == 2 )||(neighbors == 3)) {
          newArray[x][y] = true; //bunka zustane
          population++;
        }
        else {
          newArray[x][y] = false; //bunka umre
        }
      }
      else {
        //mrtva bunka
        if (neighbors == 3) {
          newArray[x][y] = true; //bunka ozije
          population++;
        }
        else {
          newArray[x][y] = false; //bunka neozije
        }
      }

      
    }
  }

  //přidání buněk při stejné populaci jako minule
  if (lastPopulation == population) {
    injectCells();
  }

  lastPopulation = population;
}

int pocetSousedu(byte x, byte y) {
  byte result = 0;

  //příprava pro přetečení z plochy
  ax = x + 1;
  ay = y + 1;

  bx = x - 1;
  by = y - 1;

  if (ax == WORLDSIZE) {
    ax = 0;
  }

  if (ay == WORLDSIZE) {
    ay = 0;
  }

  if (bx == -1) {
    bx = WORLDSIZE-1;
  }

  if (by == -1) {
    by = WORLDSIZE-1;
  }

  if (oldArray[bx][by]) { result++; }
  if (oldArray[x][by]) { result++; }
  if (oldArray[ax][by]) { result++; }
  if (oldArray[bx][y]) { result++; }
  if (oldArray[ax][y]) { result++; }
  if (oldArray[bx][ay]) { result++; }
  if (oldArray[x][ay]) { result++; }
  if (oldArray[ax][ay]) { result++; }

  //počet sousedů
  //return oldArray[bx][by] + oldArray[x][by] + oldArray[ax][by] + oldArray[bx][y] + oldArray[ax][y] + oldArray[bx][ay] + oldArray[x][ay] + oldArray[ax][ay];
  return result;
}

void injectCells() {
  byte rndx = random(WORLDSIZE);
  byte rndy = random(WORLDSIZE);

  //glider
  newArray[rndx + 1][rndy] = true;
  newArray[rndx + 2][rndy+1] = true;
  newArray[rndx][rndy+2] = true;
  newArray[rndx + 1][rndy+2] = true;
  newArray[rndx + 2][rndy+2] = true;
}

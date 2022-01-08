/*
 * 
 * BF ARDUINO
 * POLICIA CIENTIFICA/SC
 * 
 * BASED ON PROJECT ALPFINDER: https://github.com/lpinqn/alpfinder - 2018 - Sebastian Gomez - Gabinete de Pericias Informaticas - Neuquen - Argentina
 * 
 * 
 */


/*
 * v1.00 - first release -- 
 * dictionary attack improved - still limited to 250 passcodes due to arduino memory
 * 
 * v1.01 - 
 * add LCD to show last two patterns used. 
 * correct bug in arc 3-8. change the pattern to array of int. 
 * correct bug in repeatable point 5 in pattern
 * 
 * v1.02 -
 * correct bug in arc 1-3 if point 2 was used before
 * still missing correct bruteforce check for this bug
 * 
 * 
 * v2.00
 * add microsd card to store the wordlist
 * add menu to choose between wordlist 4,5,6,7,8,9 dots
 * showing the actual and last one pattern tryed was abandoned. actually showing the number of the tries- which match to line of the dictionary file
 * removed bruteforce (all possible dots) since will be too big to memory and too long to finish
 * add validate c and h to menu
 * still missing menu to alter c,h,h1 and test them
 * still missing menu to config x1 and y1 start dot
 * still missing optimizations to code
 * 
 * 
 * v2.01
 * corrected bug to not waking up for the first try
 * corrected bug to not executing movements of the candidates
 * 
 */

//Carrega biblioteca mouse e teclado
#include <Mouse.h>
#include <MouseTo.h>
#include <Keyboard.h>
#include "Arduino.h"
#include "SD.h"
#include <LiquidCrystal.h>

// Pin of cs of sd card module
#define SDFILE_PIN_CS  2
String sw_rev = "V 2.01";
/*
 * x1 e y1 - ponto 1. usar funcao valida
 * c - distancia entre os pontos. usar funcao validatec para verificar valor
 * 
 * h - distancia entre os pontos. como usa uma funcao diferente para mover, o valor provavelmente vai ser diferente de c. usar funcao checkh para verificar. nao pode ser maior que 90
 * 
 * h1 - se h precisar ser maior que 90, usar aqui o valor. recomendado colocar h como metade ou um quarto
 * 
 * 
 */



// SAMSUNG S9 - G9600
// Dot screen locations
// 5,8 inchs
// 1440x2960 - calibra ponto inicial (ponto 1)
  int x1 = 60; 
  int y1 = 275;
//Step to locate another absolute dot in screen - calibra distancia entre pontos - usar funcao test
  int c = 60;
// Relative step - distancia diagonal
  int h = 43;
// Relative step to long diagonal
  int h1 = 180;
// Waiting penalty time in seconds to restart cracking
  int penaltytime = 31;
// Number of rounds failed to apply the penalty time
  int penaltyrounds = 1;
  
// Motorola XT2095-1
// Dot screen locations
// 5,8 inchs
// 1440 * 2960 - calibra ponto inicial (ponto 1)
/*  int x1 = 60; 
  int y1 = 320;
//Step to locate another absolute dot in screen - calibra distancia entre pontos - usar funcao test
  int c = 60;
// Relative step
  int h = 60; //30
// Relative step to long diagonal - valorproximo a hipotenusa da diagonal maior
  int h1 = 180; //215
// Waiting penalty time in seconds to restart cracking
  int penaltytime = 16;
// Number of rounds failed to apply the penalty time
  int penaltyrounds = 1;*/

// Motorola XT2055
// Dot screen locations
// 5,8 inchs
// 760 * 1600 - calibra ponto inicial (ponto 1)
/*  int x1 = 55; 
  int y1 = 330;
//Step to locate another absolute dot in screen - calibra distancia entre pontos - usar funcao test
  int c = 65;
// Relative step
  int h = 50; //30
// Relative step to long diagonal - valorproximo a hipotenusa da diagonal maior
  int h1 = 200; //215
// Waiting penalty time in seconds to restart cracking
  int penaltytime = 31;
// Number of rounds failed to apply the penalty time
  int penaltyrounds = 1;*/


// Motorola MotoX Concept Proof Template
// Dot screen locations
// 4,7 inchs
// 720 * 1280
/*  int x1 = 55; 
  int y1 = 195;
//Step to locate another absolute dot in screen
  int c = 70;
// Relative step
  int h = 80;
// Relative step to long diagonal
  int h1 = 220;
// Waiting penalty time in seconds to restart cracking
  int penaltytime = 13;
// Number of rounds failed to apply the penalty time
  int penaltyrounds = 5;
*/
// Samsung S8 VirtualBox Concept Proof Template
// Dot screen locations
// 5,5 inchs
// 1440 * 2960
/*  int x1 = 120;
  int y1 = 550;
  int c = 210;
  int h = 75;
  int h1 = 380;
  int penaltytime = 15;
  int penaltyrounds = 1;
*/
/*
//Tablet Coby Concept Proof Template
// 7 inchs
  int x1 = 50;
  int y1 = 165;
  int c = 30;
  int h = 90;
  int h1 = 67; //It is not used when it is lower than ´h´
  int penaltytime = 15;
  int penaltyrounds = 5;
*/

//Sony Xperia Z1 Concept Proof Template
// 5,0 inchs
// 1080 x 1920
/*  int x1 = 80;
  int y1 = 260;
  int c = 70;
  int h = 75;
  int h1 = 300;
  int penaltytime = 15;
  int penaltyrounds = 5;
*/

// Pause frame
int pause = 150; //Delay to the next pattern in milliseconds

// Define location of dots in the x and y axis
int x2 = x1 + c;
int y2 = y1;
int x3 = x1 + 2 * c;
int y3 = y1;
int x4 = x1;
int y4 = y1 + c;
int x5 = x1 + c;
int y5 = y1 + c;
int x6 = x1 + 2 * c;
int y6 = y1 + c;
int x7 = x1;
int y7 = y1 + 2 * c;
int x8 = x1 + c;
int y8 = y1 + 2 * c;
int x9 = x1 + 2 * c;
int y9 = y1 + 2 * c;

// Counter to control waiting times
int contador = 0;
 
//Define os pinos que serão utilizados para ligação ao display
LiquidCrystal lcd(9, 5, 8, 7, 4, 3);


// variables related to sdcard
File sdFile;
//boolean to enable microsd card reading options
bool cardpresent;
boolean SDfound;


//variables related to menu
int buttons[] = {12,13,11};

const int nrButtons = 3; // change to 4 if additional button added
int menusize = 10;
String menu[] = {
  "Menu",                         //0
  "Menu>Wordlist",                //1
  "Menu>Wordlist>4p",             //2
  "Menu>Wordlist>5p",             //3
  "Menu>Wordlist>6p",             //4
  "Menu>Wordlist>7p",             //5
  "Menu>Wordlist>8p",             //6
  "Menu>Wordlist>9p",             //7
  "Menu>Validar c",               //8
  "Menu>Validar h"                //9
};

int t;
int pressedButton, currentPos,currentPosParent, possiblePos[20], possiblePosCount, possiblePosScroll = 0;
String parent = "";

bool execute;

//variables to show the actual candidate and penultimate candidate
String buffer;
//String part_buffer;
//int lcdbuffer; 
 


void printFileErrorDisplay(){
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Erro microSD");
}

void printEndTriesDisplay(){
  lcd.clear();
  lcd.setCursor(2, 0);
  lcd.print("Fim da execução");
}


void loop() {

  pressedButton = checkButtonPress();
  if (pressedButton !=0) {
    switch (pressedButton) {
      case 1:
          possiblePosScroll = (possiblePosScroll + 1) % possiblePosCount; // Scroll
      break;
      // If I wanted a 4 button controll of the menu
      //case 4:
        //  possiblePosScroll = (possiblePosScroll + possiblePosCount - 1) % possiblePosCount; // Scroll
      //break;
      case 2:
        currentPos = possiblePos[possiblePosScroll]; //Okay
      break;
      case 3:
        currentPos = currentPosParent; //Back
        possiblePosScroll = 0;
      break;
    }
    updateMenu();
  }
}



String mid(String str, int start, int len) {
   int t = 0;
   String u = ""; 
   for (t = 0; t < len;t++) {
    u = u + str.charAt(t+start-1);
   }
   return u;
}

int inStrRev(String str,String chr) {
  int t = str.length()-1;
  int u = 0;
   while (t>-1) {
    if (str.charAt(t)==chr.charAt(0)) {
      u = t+1;t = -1;
    }
    t = t - 1;
   }
  return u;
}

int len(String str) {
  return str.length();
}



void updateMenu () {
  possiblePosCount = 0;
  while (possiblePosCount == 0) {
    
    for (t = 1; t < menusize;t++) {
      if (mid(menu[t],1,inStrRev(menu[t],">")-1).equals(menu[currentPos])) {
        possiblePos[possiblePosCount]  =  t;
        possiblePosCount = possiblePosCount + 1;
      }
    }
    
    //find the current parent for the current menu
    parent = mid(menu[currentPos],1,inStrRev(menu[currentPos],">")-1);
    currentPosParent = 0;
    for (t = 0; t < menusize; t++) {
       if (parent == menu[t]) {currentPosParent = t;}
    }

    // reached the end of the Menu line
    if (possiblePosCount == 0) {
      //Menu Option Items
      switch (currentPos) {
      case 2:{
        lcd.clear();
        lcd.print("crack 4 pontos");
        MouseTo.home();
        delay(5000);
        sdFile = SD.open("pattern4.txt", FILE_READ);
          if (sdFile) {
            int i = 1;
            while(sdFile.available()){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("tentativa: ");
              lcd.print(i);
              //le nova linha*/
              buffer = sdFile.readStringUntil('\n');
              lcd.setCursor(0, 1);
              lcd.print("cand: ");
              lcd.print(buffer.substring(0, 4));
              //move o mouse
              int secuencia[4];
              int k;
              for(k=0;k<5;k++){
                secuencia[k]= buffer.substring(k, k+1).toInt();
              }    
              movesequence(secuencia, 4);
              i++;
            }
          printEndTriesDisplay();
          while(1);
        }
        else{
          printFileErrorDisplay();
          while(1);
        }
        break;
      }
      case 3:{
        lcd.clear();
        lcd.print("crack 5 pontos");
        MouseTo.home();
        delay(5000);
        sdFile = SD.open("pattern5.txt", FILE_READ);
          if (sdFile) {
            int i = 1;
            while(sdFile.available()){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("tentativa: ");
              lcd.print(i);
              //le nova linha*/
              buffer = sdFile.readStringUntil('\n');
              lcd.setCursor(0, 1);
              lcd.print("cand: ");
              lcd.print(buffer.substring(0, 5));
              //move o mouse
              int secuencia[5];
              int k;
              for(k=0;k<6;k++){
                secuencia[k]= buffer.substring(k, k+1).toInt();
              }
              movesequence(secuencia, 5);
              i++;
            }
          printEndTriesDisplay();
          while(1);
        }
        else{
          printFileErrorDisplay();
          while(1);
        }
        break;
      }
      case 4:{
        lcd.clear();
        lcd.print("crack 6 pontos");
        MouseTo.home();
        delay(5000);
        sdFile = SD.open("pattern6.txt", FILE_READ);
          if (sdFile) {
            int i = 1;
            while(sdFile.available()){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("tentativa: ");
              lcd.print(i);
              //le nova linha*/
              buffer = sdFile.readStringUntil('\n');
              lcd.setCursor(0, 1);
              lcd.print("cand: ");
              lcd.print(buffer.substring(0, 6));
              //move o mouse
              int secuencia[6];
              int k;
              for(k=0;k<7;k++){
                secuencia[k]= buffer.substring(k, k+1).toInt();
              }
              movesequence(secuencia, 6);
              i++;
            }
          printEndTriesDisplay();
          while(1);
        }
        else{
          printFileErrorDisplay();
          while(1);
        }
        break;
      }
      case 5:{
        lcd.clear();
        lcd.print("crack 7 pontos");
        MouseTo.home();
        delay(5000);
        sdFile = SD.open("pattern7.txt", FILE_READ);
          if (sdFile) {
            int i = 1;
            while(sdFile.available()){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("tentativa: ");
              lcd.print(i);
              //le nova linha*/
              buffer = sdFile.readStringUntil('\n');
              lcd.setCursor(0, 1);
              lcd.print("cand: ");
              lcd.print(buffer.substring(0, 7));
              //move o mouse
              int secuencia[7];
              int k;
              for(k=0;k<8;k++){
                secuencia[k]= buffer.substring(k, k+1).toInt();
              }
              movesequence(secuencia, 7);
              i++;
            }
          printEndTriesDisplay();
          while(1);
        }
        else{
          printFileErrorDisplay();
          while(1);
        }
        break;
      }
      case 6:{
        lcd.clear();
        lcd.print("crack 8 pontos");
        MouseTo.home();
        delay(5000);
        sdFile = SD.open("pattern8.txt", FILE_READ);
          if (sdFile) {
            int i = 1;
            while(sdFile.available()){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("tentativa: ");
              lcd.print(i);
              //le nova linha*/
              buffer = sdFile.readStringUntil('\n');
              lcd.setCursor(0, 1);
              lcd.print("cand: ");
              lcd.print(buffer.substring(0, 8));
              //move o mouse
              int secuencia[8];
              int k;
              for(k=0;k<9;k++){
                secuencia[k]= buffer.substring(k, k+1).toInt();
              }
              movesequence(secuencia, 8);
              i++;
            }
          printEndTriesDisplay();
          while(1);
        }
        else{
          printFileErrorDisplay();
          while(1);
        }
        break;
      }
      case 7:{
        lcd.clear();
        lcd.print("crack 9 pontos");
        MouseTo.home();
        delay(5000);
        sdFile = SD.open("pattern9.txt", FILE_READ);
          if (sdFile) {
            int i = 1;
            while(sdFile.available()){
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("tentativa: ");
              lcd.print(i);
              //le nova linha*/
              buffer = sdFile.readStringUntil('\n');
              lcd.setCursor(0, 1);
              lcd.print("cand: ");
              lcd.print(buffer.substring(0, 9));
              //move o mouse
              int secuencia[9];
              int k;
              for(k=0;k<10;k++){
                secuencia[k]= buffer.substring(k, k+1).toInt();
              }
              movesequence(secuencia, 9);
              i++;
            }
          printEndTriesDisplay();
          while(1);
        }
        else{
          printFileErrorDisplay();
          while(1);
        }
        break;
      }
      case 8:{
        lcd.clear();
        lcd.print("Validando c");
        validatec(); // To calibrate ´c´ paramenter (Mouse Absolute shift)
        break;
      }
      case 9:{
        lcd.clear();
        lcd.print("Validando h");
        checkh(); // To calibrate ´h´parameter (Mouse Relative shift) to conect two dots
        break;
      }
      
    }

      // go to the parent
      currentPos = currentPosParent;
    }
  }
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(mid(menu[currentPos],inStrRev(menu[currentPos],">")+1,len(menu[currentPos])-inStrRev(menu[currentPos],">")));
    lcd.setCursor(0,1); lcd.print(mid(menu[possiblePos[possiblePosScroll]],inStrRev(menu[possiblePos[possiblePosScroll]],">")+1,len(menu[possiblePos[possiblePosScroll]])-inStrRev(menu[possiblePos[possiblePosScroll]],">")));

}

// Look for a button press
int checkButtonPress() {
  int bP = 0;
  int rBp = 0;
  for (t = 0; t<nrButtons;t++) {
    if (digitalRead(buttons[t]) == 0) {bP = (t + 1);}
  }
  rBp = bP;
  while (bP != 0) { // wait while the button is still down
    bP = 0;
    for (t = 0; t<nrButtons;t++) {
      if (digitalRead(buttons[t]) == 0) {bP = (t + 1);}
    }
  }
  return rBp;
}




void mover(int num) {
  MouseTo.home();
  switch (num) {
    case 1:
      MouseTo.setTarget(x1, y1);
      break;
    case 2:
      MouseTo.setTarget(x2, y2);
      break;
    case 3:
      MouseTo.setTarget(x3, y3);
      break;
    case 4:
      MouseTo.setTarget(x4, y4);
      break;
    case 5:
      MouseTo.setTarget(x5, y5);
      break;
    case 6:
      MouseTo.setTarget(x6, y6);
      break;
    case 7:
      MouseTo.setTarget(x7, y7);
      break;
    case 8:
      MouseTo.setTarget(x8, y8);
      break;
    case 9:
      MouseTo.setTarget(x9, y9);
      break;
  }
 while (MouseTo.move() == false) {}
    delay(pause);
    Mouse.press();
}

void setup()
{
  Mouse.begin();
  Serial.begin(9600);
  
  
  //Define o número de colunas e linhas do LCD
  lcd.begin(16, 2);
  
  //escreve no lcd o candidato e o anterior
  //Limpa a tela
  lcd.clear();
  //Posiciona o cursor na coluna 0, linha 0
  lcd.setCursor(2, 0);
  //Envia o texto entre aspas para o LCD
  lcd.print("--BFarduino--");
  lcd.setCursor(5, 1);
  //Envia o texto entre aspas para o LCD
  lcd.print(sw_rev);

  delay(2000);

  pinMode(SDFILE_PIN_CS, OUTPUT);
    if (SDfound == 0) {
      if (!SD.begin(SDFILE_PIN_CS)) {
        printFileErrorDisplay();
      while(1);
      }
    }


  for (t=0;t<nrButtons;t++) {
    pinMode(buttons[t],INPUT_PULLUP); 
  }


  execute = false;
  
  updateMenu();

}



void validatec(){
for ( int walk = 1; walk <= 9; walk++ )
    {
      mover(walk);
      delay(2000);
      Mouse.release();
    }
}

//funcao para calibrar h
void checkh(){
for ( int centena = 1; centena <= 9; centena++ )
   {
    for ( int decena = 1; decena <= 9; decena++ )
       {           
        for ( int unidad = 1; unidad <= 9; unidad++ )
       {    
        mover(centena);
        delay(pause);
        moverrel(centena,decena);
        delay(pause);
        moverrel(decena,unidad);
        delay(pause);
        Mouse.release();
       }
      }
  }
}

//funcao que move o mouse entre os pontos
void moverrel(int origen, int destino) {
Mouse.begin();
if (origen == 1){
               if (destino == 2){
                Mouse.move(h,0);
               }
               if (destino == 3){//usado no caso de ponto 2 ja ter sido clicado
                Mouse.begin();
                Mouse.move(h,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h,0);
               }
              if (destino == 4){
                Mouse.move(0,h);
               }
              if (destino == 5){
                Mouse.move(h,h);
               }
              if (destino == 6){
                Mouse.begin();
                Mouse.move(h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,h/2);
               }
              if (destino == 7){//usado no caso de ponto 4 ja ter sido clicado
                Mouse.begin();
                Mouse.move(0,h);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h);
               }
              if (destino == 8){
                Mouse.begin();
                Mouse.move(h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,h/2);
               }
               if (destino == 9){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(h,h);
                delay(pause);
                Mouse.begin();
                Mouse.move(h,h);
               }
 }
if (origen == 2){     
               if (destino == 1){
                Mouse.move(-h,0);
              }
              if (destino == 3){
                Mouse.move(h,0);
              }
              if (destino == 4){
                Mouse.move(-h,h);
              }
              if (destino == 5){
                Mouse.move(0,h);
              }
              if (destino == 6){
                Mouse.move(h,h);
              }
              if (destino == 7){
                Mouse.begin();
                Mouse.move(-h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,h/2);
              }
              if (destino == 8){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(0,h);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h);
              }
              if (destino == 9){
                Mouse.begin();
                Mouse.move(h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,h/2);
              }
}
if (origen == 3){
              if (destino == 1){//usado no caso de ponto 2 ja ter sido clicado
                Mouse.begin();
                Mouse.move(-h,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h,0);
              }
              if (destino == 2){
                Mouse.move(-h,0);
              }
              if (destino == 4){
                Mouse.begin();
                Mouse.move(-h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,h/2);
              }
              if (destino == 5){
                Mouse.move(-h,h);
              }
              if (destino == 6){
                Mouse.move(0,h);
              }
              if (destino == 7){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(-h,h);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h,h);
              }
              if (destino == 8){
                Mouse.begin();
                Mouse.move(-h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,h/2);
              }
              if (destino == 9){//usado no caso de ponto 6 ja ter sido clicado
                Mouse.begin();
                Mouse.move(0,h);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,h);
              }
}
if (origen == 4){
              if (destino == 1){
                Mouse.move(0,-h);
              }
              if (destino == 2){
                Mouse.move(h,-h);
              }
              if (destino == 3){
                Mouse.begin();
                Mouse.move(h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,-h/2);
              }
              if (destino == 5){
                Mouse.move(h,0);
              }
              if (destino == 6){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(h,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h,0);
              }
              if (destino == 7){
                Mouse.move(0,h);
              }
              if (destino == 8){
                Mouse.move(h,h);
              }
              if (destino == 9){
                Mouse.begin();
                Mouse.move(h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,h/2);
              }
}
if (origen == 5){
              if (destino == 1){
                Mouse.move(-h,-h);
              }
              if (destino == 2){
                Mouse.move(0,-h);
              }
              if (destino == 3){
                Mouse.move(h,-h);
              }
              if (destino == 4){
                Mouse.move(-h,0);
              }
              if (destino == 6){
                Mouse.move(h,0);
              }
              if (destino == 7){
                Mouse.move(-h,h);
              }
              if (destino == 8){
                Mouse.move(0,h);
              }
              if (destino == 9){
                Mouse.move(h,h);
              }
}
if (origen == 6){
              if (destino == 1){
                Mouse.begin();
                Mouse.move(-h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,-h/2);
              }
              if (destino == 2){
                Mouse.move(-h,-h);
              }
              if (destino == 3){
                Mouse.move(0,-h);
              }
              if (destino == 4){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(-h,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h,0);
              }
              if (destino == 5){
                Mouse.move(-h,0);
              }
              if (destino == 7){
                Mouse.begin();
                Mouse.move(-h/2,h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0); 
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,h/2);
              }
              if (destino == 8){
                Mouse.move(-h,h);
              }
              if (destino == 9){
                Mouse.move(0,h);
              }
                
}
if (origen == 7){
              if (destino == 1){//usado no caso de ponto 4 ja ter sido clicado
                Mouse.begin();
                Mouse.move(0,-h);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h);
              }
              if (destino == 2){
                Mouse.begin();
                Mouse.move(h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,-h/2);
              }
              if (destino == 3){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(h,-h);
                delay(pause);
                Mouse.begin();
                Mouse.move(h,-h);
              }
              if (destino == 4){
                Mouse.move(0,-h);
              }
              if (destino == 5){
                Mouse.move(h,-h);
              }
              if (destino == 6){
                Mouse.begin();
                Mouse.move(h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,-h/2);
              }
              if (destino == 8){
                Mouse.move(h,0);
              }
              if (destino == 9){//usado no caso de ponto 8 ja ter sido clicado
                Mouse.begin();
                Mouse.move(h,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(h,0);
              }
}
if (origen == 8){
              if (destino == 1){
                Mouse.begin();
                Mouse.move(-h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,-h/2);              
              }
              if (destino == 2){//usado no caso de ponto 8 ja ter sido clicado
                Mouse.begin();
                Mouse.move(0,-h);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h);
              }
              if (destino == 3){
                Mouse.begin();
                Mouse.move(h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(h/2,-h/2);    
              }
              if (destino == 4){
                Mouse.move(-h,-h);
              }
              if (destino == 5){
                Mouse.move(0,-h);
              }
              if (destino == 6){
                Mouse.move(h,-h);
              }
              if (destino == 7){
                Mouse.move(-h,0);
              }
              if (destino == 9){
                Mouse.move(h,0);
              }
}              
if (origen == 9){
             if (destino == 1){//usado no caso de ponto 5 ja ter sido clicado
                Mouse.begin();
                Mouse.move(-h,-h);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h,-h);
              }
             if (destino == 2){
                Mouse.begin();
                Mouse.move(-h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,-h/2);    
              }
             if (destino == 3){//usado no caso de ponto 6 ja ter sido clicado
                Mouse.begin();
                Mouse.move(0,-h);
                delay(pause);
                Mouse.begin();
                Mouse.move(0,-h);
              }
             if (destino == 4){
                Mouse.begin();
                Mouse.move(-h/2,-h/2);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0); 
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,0); 
                delay(pause);
                Mouse.begin();
                Mouse.move(-h/2,-h/2);    
              }
             if (destino == 5){
                Mouse.move(-h,-h);
              }
             if (destino == 6){
                Mouse.move(0,-h);
              }
              if (destino == 7){//usado no caso de ponto 8 ja ter sido clicado
                Mouse.begin();
                Mouse.move(-h,0);
                delay(pause);
                Mouse.begin();
                Mouse.move(-h,0);
              }
             if (destino == 8){
                Mouse.move(-h,0);
              }
      }
Mouse.end();
delay(pause);
}



void waitingtime(){
                        Mouse.release();
                        delay(1000);
                        contador = contador + 1;
                        if((contador % penaltyrounds) == 0){
                          contador = 0; //Reset the counter when it reaches 5 rounds
                          Keyboard.write(0x20);
                          Keyboard.println();
                          Keyboard.write(0x20);
                          Keyboard.println(); 
                          for (int wait = 1; wait <= penaltytime; wait++ )
                          {
                           delay(1000);
                           
                           /*
                            * mouse fica se mexendo
                           Mouse.begin();
                           MouseTo.setTarget(10, y1);
                           while (MouseTo.move() == false) {}
                           Mouse.press();
                           for (int cycle = 1; cycle <= 8; cycle++ )
                              { 
                                Mouse.move(0,h);
                                delay(50);
                              }
                           Mouse.release();*/
                          };
                          /*
                           * acorda com uma mexida horizontal do mouse e tecla espaco
                           */
                          Mouse.begin();
                          MouseTo.setTarget(10, y1>2);
                          Mouse.move(10,y1+2*c);
                          delay(pause);
                          Keyboard.write(0x20);
                          delay(50);
                          Mouse.release();
                        }
}

void movesequence(int patron[], int longitud){

  int i;  
  int h3 = h1;
  int h4 = 0;
  // This conditional tries to approach the pointer with accuracy through the long diagonal. When h1 is lower than h, it is not used. h3 y h4 are used as auxiliary variables to retain values.
  /*
  * acorda com uma mexida horizontal do mouse e tecla espaco
  */
  Mouse.begin();
  MouseTo.setTarget(10, y1>2);
  Mouse.move(10,y1+2*c);
  Mouse.release();
  delay(pause);
  Keyboard.write(0x20);
  Keyboard.println();
  
  delay(pause);
  //posiciona o mouse para o primeiro ponto
  mover(patron[0]);
  delay(pause);
  //se h1 eh maior que h
  if (h1 > h) {
    for (i = 0; i < longitud - 1; i++){
      do
      {
        moverrel(patron[i],patron[i+1]);
        delay(pause);
        h1 = h1 - h;
      } while (h1 >= h);
      if (h1 < h) {
          h4 = h;
          h = h1;
          moverrel(patron[i],patron[i+1]);
          h = h4;
       }
       h1 = h3;
       delay(pause);
    }
    waitingtime();
  }
  //se h1 nao eh maior que h
  else
  {
    for (i = 0; i < longitud - 1; i++){
      moverrel(patron[i],patron[i+1]);
      delay(pause);
    }
    waitingtime();
  }
}

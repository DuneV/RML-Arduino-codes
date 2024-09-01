// Nano_GC9A01_voltage_meter
//
// grid voltage variation monitor (230V - 250V AC) 
// rolling averaged voltage (of 10 readings) is sent to display
// NOTE: here voltage is generated with random function - say this is a demo
// the real version gets data from a ZMPT101B sensor
//
// platform: Arduino Nano - works also with Uno
// display:  GC9A010 driven circular display 240*240 pixels
//
// 
// CG9A01    Arduino Nano
//  RST -------- NC
//  CST -------- 10
//  DC ---------  9
//  SDA -------- 11
//  SCL -------- 13
//  
// Floris Wouterlood
// July 11, 2023
// public domain

// made for a 240*240 pixel circular display
// all x-y-coordinates relative to center = x = 120 and y = 120
     
   #include "SPI.h"
   #include "Adafruit_GC9A01A.h"

   #define TFT_DC 2
   #define TFT_CS 4
 
   Adafruit_GC9A01A tft (TFT_CS, TFT_DC);
 
   #define BLACK      0x0000                                                               // some extra colors
   #define BLUE       0x001F
   #define RED        0xF800
   #define GREEN      0x07E0
   #define CYAN       0x07FF
   #define MAGENTA    0xF81F
   #define YELLOW     0xFFE0
   #define WHITE      0xFFFF
   #define ORANGE     0xFBE0
   #define GREY       0x84B5
   #define BORDEAUX   0xA000
   #define AFRICA     0xAB21                                                               // current dial color

   #define DEG2RAD 0.0174532925 

   int multiplier;
   int    frametime = 1000; 
   int    x_pos;
   int    y_pos; 
   int    center_x = 120;                                                                  // center x of dial on 240*240 TFT display
   int    center_y = 120;                                                                  // center y of dial on 240*240 TFT display
   float  pivot_x, pivot_y,pivot_x_old, pivot_y_old;
   float  p1_x,p1_y,p2_x,p2_y,p3_x, p3_y, p4_x, p4_y, p5_x, p5_y; 
   float  p1_x_old,p1_y_old, p2_x_old, p2_y_old, p3_x_old, p3_y_old;
   float  p4_x_old, p4_y_old, p5_x_old, p5_y_old;
   float  angleOffset = 3.14;
   float  arc_x;
   float  arc_y;
   int    radius = 120;                                                                    // center y of circular scale                                                   
   float  angle_circle = 0;
   float  needleAngle = 0;
   int    iteration = 0;
   int    j;                                                            
   float  volt = 220;
   int    needle_multiplier = 1;
   float  needle_setter;             
                                                                                           // voltage rolling averaging stuff 
   const byte nvalues = 10;                                                                // rolling average window size
   static byte current = 0;                                                                // index for current value
   static byte cvalues = 0;                                                                // count of values read (<= nvalues)
   static float sum = 0;                                                                   // rolling sum
   static float values[nvalues];
   float averagedVoltage = 235;                                                            // to start with
   
void setup() {

  randomSeed (analogRead(0));  

   tft.begin();    
   Serial.begin (9600); 
   Serial.println (""); 
   Serial.println (""); 
   tft.setRotation (0);  
   
   tft.fillScreen (BLACK);
   tft.drawCircle (center_x, center_y,120, BLACK);             
   pivot_x = center_x;
   pivot_y = center_y+50;

   p1_x_old = center_x; p1_y_old = center_y+50;
   p2_x_old = center_x; p2_y_old = center_y+50;
   p3_x_old = center_x; p3_y_old = center_y+50;
   p4_x_old = center_x; p4_y_old = center_y+50;
   p5_x_old = center_x; p5_y_old = center_y+50;

   volt = 240;                                                                             // initial value setting the needle
   create_dial ();
   needle_setter = volt;
   needleAngle = (((needle_setter)*DEG2RAD*1.8)-3.14);
   needle();  
   draw_pivot ();
}


void loop (){
  
   iteration++;
   Serial.println ();      
   Serial.print ("iteration ");
   Serial.println (iteration); 
   volt = random (230,250);                                                                // voltage simulator  
   Serial.print ("simulated volt out of ZMPT101B: ");
   Serial.println (volt);  
   averagedVoltage = movingAverage(volt);
   Serial.print ("averaged volt =      ");
   Serial.println (averagedVoltage);
   Serial.println ();  
   Serial.println ();   

   displayNumerical ();
   needle_setter = averagedVoltage;     
   needle();
   draw_pivot (); 
   
   delay (frametime);
} 

void needle (){                                                                            // dynamic needle management

   tft.drawLine (pivot_x, pivot_y, p1_x_old, p1_y_old, AFRICA);                            // remove old needle  
   tft.fillTriangle (p1_x_old, p1_y_old, p2_x_old, p2_y_old, p3_x_old, p3_y_old, AFRICA);  // remove old arrow head
   tft.fillTriangle (pivot_x, pivot_y, p4_x_old, p4_y_old, p5_x_old, p5_y_old, AFRICA);    // remove old arrow head
    
   needleAngle = (((needle_setter)*0.01745331*1.8)-3.14);
   p1_x = (pivot_x + ((radius)*cos(needleAngle)));                                         // needle tip
   p1_y = (pivot_y + ((radius)*sin(needleAngle))); 

   p2_x = (pivot_x + ((radius-15)*cos(needleAngle-0.05)));                                 // needle triange left
   p2_y = (pivot_y + ((radius-15)*sin(needleAngle-0.05))); 

   p3_x = (pivot_x + ((radius-15)*cos(needleAngle+0.05)));                                 // needle triange right
   p3_y = (pivot_y + ((radius-15)*sin(needleAngle+0.05))); 

   p4_x = (pivot_x + ((radius-90)*cos(angleOffset+(needleAngle-0.2))));                    // needle triange left
   p4_y = (pivot_y + ((radius-90)*sin(angleOffset+(needleAngle-0.2)))); 

   p5_x = (pivot_x + ((radius-90)*cos(angleOffset+(needleAngle+0.2))));                    // needle triange right
   p5_y = (pivot_y + ((radius-90)*sin(angleOffset+(needleAngle+0.2)))); 
  
   p1_x_old = p1_x; p1_y_old = p1_y;                                                       // remember previous needle position
   p2_x_old = p2_x; p2_y_old = p2_y;                                                                         
   p3_x_old = p3_x; p3_y_old = p3_y;                                                                      

   p4_x_old = p4_x; p4_y_old = p4_y;                                                       // remember previous needle counterweight position
   p5_x_old = p5_x; p5_y_old = p5_y;                                                                      

   tft.drawLine (pivot_x, pivot_y, p1_x, p1_y, BLACK);                                     // create needle 
   tft.fillTriangle (p1_x, p1_y, p2_x, p2_y, p3_x, p3_y, BLACK);                           // create needle tip pointer
   tft.drawLine (center_x-80, center_y+70, center_x+80,center_y+70, WHITE);                // repair floor 
   tft.fillTriangle (pivot_x, pivot_y, p4_x, p4_y, p5_x, p5_y, BLACK);                     // create needle counterweight
}


void create_dial (){

   tft.fillCircle (center_x, center_y,120, AFRICA);                                        // general dial field
   tft.drawCircle (center_x, center_y,118,GREY);  
   tft.drawCircle (center_x, center_y,117,BLACK);
   tft.drawCircle (center_x, center_y,116,BLACK);  
   tft.drawCircle (center_x, center_y,115,GREY);

   for (j= 30; j<75    ; j+=5)
       {
        needleAngle = ((j*DEG2RAD*1.8)-3.14);
        arc_x = (pivot_x + ((radius+15)*cos(needleAngle)));                                // needle tip
        arc_y = (pivot_y + ((radius+15)*sin(needleAngle))); 
        tft.drawPixel  (arc_x,arc_y,BLACK);
        tft.fillCircle (arc_x,arc_y,2, BLACK);
        }

   tft.setTextColor (BLACK,AFRICA);    
   tft.setTextSize (2);
   tft.setCursor (center_x+15, center_y+40);
   tft.print ("V - AC");                                                                                                                                                                          
   tft.drawLine (center_x-80, center_y+70, center_x+80,center_y+70, WHITE);                // create floor  
}


void draw_pivot (){
 
   tft.fillCircle (pivot_x, pivot_y,8,RED);               
   tft.drawCircle (pivot_x, pivot_y,8,BLACK);            
   tft.drawCircle (pivot_x, pivot_y,3,BLACK);      
}

 
void displayNumerical (){

   tft.fillRect (center_x-82, center_y+40, 62,16,AFRICA);
   tft.setTextColor (BLACK);    
   tft.setTextSize (2);
   tft.setCursor (center_x-80, center_y+40);
   tft.print (averagedVoltage,1);   
}


float movingAverage(float value) {

   sum += value;                    
   if (cvalues == nvalues)                                                                 // if the window is full, adjust the sum by deleting the oldest value
     sum -= values[current];

   values[current] = value;                                                                // replace the oldest with the latest

   if (++current >= nvalues)
     current = 0;

   if (cvalues < nvalues)
     cvalues += 1;

   return sum/cvalues;
}

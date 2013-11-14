#define usbMidi

const boolean hardwareAveraging = true;
const int numReadings = 10;
const int diffVal = 2;

const int pspStickCount = 4;
const int pspStickAveraging = 10;
const int pspStickDiffVal = 2;

int pspStickPins[pspStickCount];
int pspStickCurrentReadings[pspStickCount];
int pspStickReadings[pspStickCount][pspStickAveraging];
int pspStickAverages[pspStickCount];
int pspStickTotals[pspStickCount];
int pspStickLastValues[pspStickCount];
int pspStickBoundaries[pspStickCount][2];
int pspStickIndex = 0;

void setup()
{
  // initialize serial communication with computer:
  Serial.begin(9600);                   
  // initialize all the readings to 0: 
  /*
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readingsX[thisReading] = 0;    
  for (int thisReading = 0; thisReading < numReadings; thisReading++)
    readingsY[thisReading] = 0;       
  */
  pspStickPins[0] = 1;
  pspStickPins[1] = 2;
  pspStickPins[2] = 3;
  pspStickPins[3] = 4;
  
  if(hardwareAveraging) {
    analogReadAveraging(numReadings); 
  }
  
  for(int currentStick = 0; currentStick < pspStickCount; currentStick++) {
    if(!hardwareAveraging) {
      for(int currentReading = 0; currentReading < pspStickAveraging; currentReading++) {
        pspStickReadings[currentStick][currentReading] = 0;
      }
    }
    
    pspStickBoundaries[currentStick][0] = 1024;
    pspStickBoundaries[currentStick][1] = 0;
    pspStickLastValues[currentStick] = 0;
  }
}

void loop() {
  for(int currentStick = 0; currentStick < pspStickCount; currentStick++) {
    if(hardwareAveraging) {
      pspStickCurrentReadings[currentStick] = analogRead(pspStickPins[currentStick]);
      
    }else{
      //Subtract the very last reading
      pspStickTotals[currentStick] = pspStickTotals[currentStick] - pspStickReadings[currentStick][pspStickIndex];
      //read from the sensor
      pspStickReadings[currentStick][pspStickIndex] = analogRead(pspStickPins[currentStick]);
      //add the new reading to the total
      pspStickTotals[currentStick] = pspStickTotals[currentStick] + pspStickReadings[currentStick][pspStickIndex];
      //calculate the average
      pspStickAverages[currentStick] = pspStickTotals[currentStick] / pspStickAveraging;
    }
  }
  
  //Calculate the recorded boundaries for the sticks
  for(int currentStick = 0; currentStick < pspStickCount; currentStick++) {
    if(hardwareAveraging) {
      if(pspStickCurrentReadings[currentStick] < pspStickBoundaries[currentStick][0]) {
        //Serial.println("Changing low: " + String(pspStickReadings[currentStick][pspStickIndex]));
        pspStickBoundaries[currentStick][0] = pspStickCurrentReadings[currentStick];
      }
      
      if(pspStickCurrentReadings[currentStick] > pspStickBoundaries[currentStick][1]) {
        pspStickBoundaries[currentStick][1] = pspStickCurrentReadings[currentStick];
      }
      
    }else{
      if(pspStickReadings[currentStick][pspStickIndex] < pspStickBoundaries[currentStick][0]) {
        //Serial.println("Changing low: " + String(pspStickReadings[currentStick][pspStickIndex]));
        pspStickBoundaries[currentStick][0] = pspStickReadings[currentStick][pspStickIndex];
      }
      
      if(pspStickReadings[currentStick][pspStickIndex] > pspStickBoundaries[currentStick][1]) {
        pspStickBoundaries[currentStick][1] = pspStickReadings[currentStick][pspStickIndex];
      }
    }
  }
  
  //increment the stored values for the averaging function
  pspStickIndex++;
   
  //If we're at the end of the array...
  if(hardwareAveraging || pspStickIndex >= pspStickAveraging) {
    //wrap back to the beginning!
    pspStickIndex = 0;
    boolean changed = false;
    for(int currentStick = 0; currentStick < pspStickCount; currentStick++) {
      if(hardwareAveraging) {
        if(diff(pspStickLastValues[currentStick], pspStickCurrentReadings[currentStick])) {
          pspStickLastValues[currentStick] = pspStickCurrentReadings[currentStick];
          changed = true;
        }
      }else{
        if(diff(pspStickLastValues[currentStick], pspStickAverages[currentStick])) {
          pspStickLastValues[currentStick] = pspStickAverages[currentStick];
          changed = true;
        }
      }
    }
     
    if(changed) {
      String output = "sticks"; //"X1,Y1,X2,Y2 ";
      for(int currentStick = 0; currentStick < pspStickCount; currentStick++) {
        //output += String(map(pspStickAverages[currentStick], pspStickBoundaries[currentStick][0], pspStickBoundaries[currentStick][1], 0, 255));
        //output += "(" + String(pspStickBoundaries[currentStick][0]) + "," + String(pspStickBoundaries[currentStick][1]) + ") ";
        
        #ifdef usbMidi
          usbMIDI.sendControlChange(110+currentStick, map(pspStickLastValues[currentStick], pspStickBoundaries[currentStick][0], pspStickBoundaries[currentStick][1], 0, 127), 1);
        #endif
        output += "," + String(map(pspStickLastValues[currentStick], pspStickBoundaries[currentStick][0], pspStickBoundaries[currentStick][1], 0, 255));
      }

      Serial.println(output);
      //analogWrite(3, map(pspStickAverages[0], pspStickBoundaries[0][0], pspStickBoundaries[0][1], 0, 255));
      //analogWrite(4, map(pspStickAverages[1], pspStickBoundaries[1][0], pspStickBoundaries[1][1], 0, 255));
      //analogWrite(5, map(pspStickAverages[2], pspStickBoundaries[2][0], pspStickBoundaries[2][1], 0, 255));
    }
  }
  
  delay(1);        // delay in between reads for stability            
}

boolean diff(int lastVal, int averageVal) {
  if((lastVal+pspStickDiffVal) <= averageVal)
    return true;
  if((lastVal-pspStickDiffVal) >= averageVal)
    return true;
    
  return false; 
}

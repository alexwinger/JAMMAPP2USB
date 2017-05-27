/*
 * File:   jamma_buttons.c
 * Author: alexw
 *
 * Created on 20 de mayo de 2017, 12:02 AM
 */


#include <xc.h>
#include <stdint.h>
#include <fixed_address_memory.h>
#include "app_device_joystick.h"
#include "jamma_buttons.h"

#define CLKDIR   TRISBbits.RB4
#define SAVEDIR  TRISBbits.RB3

#define DATA0DIR TRISBbits.RB2
#define DATA1DIR TRISBbits.RB1
#define DATA2DIR TRISBbits.RB0

#define CLK   LATBbits.LATB4
#define SAVE  LATBbits.LATB3

#define DATA0 PORTBbits.RB2
#define DATA1 PORTBbits.RB1
#define DATA2 PORTBbits.RB0


/** TYPE DEFINITIONS ************************************************/
uint8_t updateBtns[3];


typedef union _PLAYER_CONTROL
{
    struct
    {
        uint8_t up:1;
        uint8_t down:1;
        uint8_t left:1;
        uint8_t right:1;
        uint8_t b1:1;
        uint8_t b2:1;
        uint8_t b3:1;
        uint8_t b4:1;//
        uint8_t b5:1;
        uint8_t b6:1;
        uint8_t select:1;
        uint8_t start:1;
        uint8_t :4;
    } buttons;
    uint16_t val;
} PLAYER_CONTROL;

typedef union _CABINET_BUTTONS
{
    struct
    {
        uint8_t coin:1;
        uint8_t service:1;
        uint8_t game_select:1;
        uint8_t generic1:1;
        uint8_t generic2:1;
        uint8_t generic3:1;
        uint8_t generic4:1;
        uint8_t :1;
    } buttons;
    uint16_t val;
} CABINET_BUTTONS;

static uint16_t buffer[3];
static PLAYER_CONTROL  joystick1;
static PLAYER_CONTROL  joystick2;
static CABINET_BUTTONS cabinet;
static uint8_t bitcounter;

enum JAMMASTATE {
    UNINITIALIZED = 0,
    INITIALIZED,
    BUTTONSSAVED,
    BUTTONREAD
}jammasm = UNINITIALIZED;

static void idleOutputs(){
    SAVE = 0;
    CLK = 0;
}

static void setupButtons(void) {
    DATA0DIR = 1;
    DATA1DIR = 1;
    DATA2DIR = 1;
    CLKDIR   = 0;
    SAVEDIR  = 0;
    idleOutputs();
    updateBtns[0]=updateBtns[1]=updateBtns[2];
    jammasm = INITIALIZED;
}

static void saveButtons(){
    buffer[0] = buffer[1] = buffer[2] = 0;
    SAVE = 1;
    CLK  = 0;
    bitcounter=0;
    jammasm = BUTTONSSAVED;
}

void clearUpdateButtons(uint8_t controlIdx){

    if(controlIdx>0 && controlIdx <3){
        updateBtns[controlIdx] = 0;
    }
}


uint16_t updateButtons(uint8_t controlIdx){
    uint16_t ret=0;
    
    if(controlIdx>0 && controlIdx <3){
        ret = updateBtns[controlIdx];
    }
    
    return ret;
}



uint16_t getButtons(uint8_t controlIdx){
    uint16_t ret=0;
    switch(controlIdx)
    {
        case 0:
            ret = joystick1.val;
            break;
        case 1:
            ret = joystick2.val;
            break;
        case 2:
            ret = cabinet.val;
            break;
    }
    
    return ret;
}

static void readData()
{

    idleOutputs();   
    bitcounter++;

    buffer[0] |= (DATA0==1 ? 0u:1u);
    buffer[1] |= (DATA2==1 ? 0u:1u);
    buffer[2] |= (DATA1==1 ? 0u:1u);
    
    if(bitcounter >= 16) {
        jammasm = INITIALIZED;
        
        if(buffer[0] != joystick1.val){
            updateBtns[0]=1;
            joystick1.val = buffer[0];
        }
        
        if(buffer[1] != joystick2.val){
            updateBtns[1]=1;
            joystick2.val = buffer[1];
        }
        
        if(buffer[2] != cabinet.val){
            updateBtns[2]=1;
            cabinet.val   = buffer[2];
        }
        
        APP_DeviceJoystickTasks(buffer);
        
    } else {
        jammasm = BUTTONREAD;
    }
}

static void shiftData(){
    CLK = 1;
    
    buffer[0] <<= 1;
    buffer[1] <<= 1;
    buffer[2] <<= 1;
    
    jammasm = BUTTONSSAVED;
}

static uint16_t counter=0;

void jammaTask(){
    
    switch(jammasm){
        case UNINITIALIZED:
             setupButtons();
            break;
        case INITIALIZED:
             saveButtons();
            break;
        case BUTTONSSAVED:
             readData();
            break;
        case BUTTONREAD:
             shiftData();
            break;
        default:
            jammasm = UNINITIALIZED;
            break;
    }
}
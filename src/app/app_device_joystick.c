/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#ifndef USBJOYSTICK_C
#define USBJOYSTICK_C

/** INCLUDES *******************************************************/
#include "usb.h"
#include "usb_device_hid.h"

#include "system.h"

#include "app_led_usb_status.h"

#include "stdint.h"

/** DECLARATIONS ***************************************************/
//http://www.microsoft.com/whdc/archive/hidgame.mspx
#define HAT_SWITCH_NORTH            0x0
#define HAT_SWITCH_NORTH_EAST       0x1
#define HAT_SWITCH_EAST             0x2
#define HAT_SWITCH_SOUTH_EAST       0x3
#define HAT_SWITCH_SOUTH            0x4
#define HAT_SWITCH_SOUTH_WEST       0x5
#define HAT_SWITCH_WEST             0x6
#define HAT_SWITCH_NORTH_WEST       0x7
#define HAT_SWITCH_NULL             0x8

/** TYPE DEFINITIONS ************************************************/
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
    uint8_t val[2];
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
    uint8_t val[2];
} CABINET_BUTTONS;


/** VARIABLES ******************************************************/
/* Some processors have a limited range of RAM addresses where the USB module
 * is able to access.  The following section is for those devices.  This section
 * assigns the buffers that need to be used by the USB module into those
 * specific areas.
 */
#if defined(FIXED_ADDRESS_MEMORY)
    #if defined(COMPILER_MPLAB_C18)
        #pragma udata JOYSTICK_DATA=JOYSTICK_DATA_ADDRESS
            INPUT_CONTROLS joystick_input;
        #pragma udata
    #elif defined(__XC8)
        INPUT_CONTROLS joystick_input @ JOYSTICK_DATA_ADDRESS;
    #endif
#else
typedef union _BUTTONS_STATE
{
    struct
    {
        PLAYER_CONTROL joystick1;
        PLAYER_CONTROL joystick2;
        CABINET_BUTTONS cabinet;
    } members;
    uint8_t report[6];
} BUTTONS_STATE;
#endif

volatile BUTTONS_STATE buttons_state;
USB_VOLATILE USB_HANDLE lastTransmission = 0;
volatile uint16_t contador;


/*********************************************************************
* Function: void APP_DeviceJoystickInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceJoystickInitialize(void)
{
    //initialize the variable holding the handle for the last
    // transmission
    lastTransmission = 0;
    buttons_state.members.joystick1.val[0] = 255;
    buttons_state.members.joystick2.val[0] = 1;
    buttons_state.members.cabinet.val[0] = 1;
    contador = 0;

    //enable the HID endpoint
    USBEnableEndpoint(JOYSTICK_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
}//end UserInit


void update_joysticks() {
    contador++;
    if( contador >= 2) {
        buttons_state.members.joystick1.val[0]++;
        buttons_state.members.joystick1.val[1]--;
        
        buttons_state.members.joystick2.val[0]++;
        buttons_state.members.joystick2.val[1]--;
        
        
        buttons_state.members.cabinet.val[0]++;
        
        contador = 0;
    }
}


/*********************************************************************
* Function: void APP_DeviceJoystickTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceJoystickInitialize() and APP_DeviceJoystickStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceJoystickTasks(void)
{  
    
    //update_joysticks();
    /* If the USB device isn't configured yet, we can't really do anything
     * else since we don't have a host to talk to.  So jump back to the
     * top of the while loop. */
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        /* Jump back to the top of the while loop. */
        return;
    }

    /* If we are currently suspended, then we need to see if we need to
     * issue a remote wakeup.  In either case, we shouldn't process any
     * keyboard commands since we aren't currently communicating to the host
     * thus just continue back to the start of the while loop. */
    if( USBIsDeviceSuspended() == true )
    {
        /* Jump back to the top of the while loop. */
        return;
    }

    //If the last transmission is complete
    if(!HIDTxHandleBusy(lastTransmission))
    {
        lastTransmission = HIDTxPacket(JOYSTICK_EP, (uint8_t*)&buttons_state, 6);
    }
    
}//end ProcessIO

#endif

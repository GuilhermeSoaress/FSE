#include <driver/gpio.h>
#include <driver/i2c.h>
#include "freertos/portmacro.h"
#include "hal/i2c_types.h"
#include <unistd.h>
#include <esp_log.h>
#define _LCD_I2C_H
#define CURSOR_SHIFT_LEFT 1
#define CURSOR_SHIFT_RIGHT 0
#define SLAVE_ADDRESS_LCD 0x26
/**
* GPIO pins used for 4 bit mode operations
* reserved for non i2c uses
*/
typedef union {

    struct  {
    int enable_pin;
    int register_select;
    int data_4;
    int data_5;
    int data_6;
    int data_7;
    } pins_t;

    int pin_array[6];

} lcd_pins_t;

esp_err_t i2c_master_init(void);
/**
*@brief sends command or data depending on the register select state 
*@param rs_state register select state 0 for command 1 for data
*@param comm command coded in hexadecimal (0x)
*/
void i2c_lcd_send_command(int rs_state, int comm, int address);
/**
*@brief initialization sequence for the LCD screen
*@param address i2c address of lcd to send instructions to
*/
void i2c_lcd_init_sequence(int address);
/**
*@brief writes message to lcd 
*@param message message to be written (duh)
*@param address i2c address of lcd to send instructions to
*/
void i2c_lcd_write_message(char* msg,int address);
/**
*@brief clears lcd screen 
*@param message message to be written (duh)
*@param address i2c address of lcd to send instructions to
*/
void i2c_lcd_clear_screen(int address);
/**
*@brief defines custom character from an 8x8 bit matrix
*@param pattern pattern to be drawn as custom character
*@param location location to store custom characters in (max 4 custom characters)
*/
void i2c_lcd_custom_char(int* pattern[], int location,int address);

/**
*@brief moves cursor to specified coordinates
*@param x x position (columns)
*@param y y position (rows)
*@param direction either CURSOR_SHIFT_LEFT or CURSOR_SHIFT_RIGHT
*@param address i2c address of lcd to send instructions to
*/
void i2c_lcd_shift_cursor(int x,int y, int direction, int address);
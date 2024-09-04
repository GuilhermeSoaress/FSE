/**
* consider the following 8 bit encoded hexadecimal bits[7] (bits[0] being the weakest bit)
* if we send the hexadecimal to the pcf8574 lcd1602 adapter board it will be mapped as follows
* bit[0]: RS
* bit[1]: RW
* bit[2] : E
* bit[3] : (None check schematics )
* bit[4] : D4
* bit[5] : D5
* bit[6] : D6
* bit[7] : D7
*/

#include "esp_err.h"
#include "esp_lcd.h"
#include <rom/ets_sys.h>

static char *TAGLCD ="LCD";

esp_err_t i2c_master_init(void){

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(I2C_NUM_0, &conf);

    return i2c_driver_install(I2C_NUM_0, conf.mode, 0, 0, 0);
}

void init_lcd_pins(lcd_pins_t* pins,gpio_config_t* pin_conf)
{
    for(int i =0;i<6;i++)   gpio_set_direction((pins->pin_array)[i],GPIO_MODE_OUTPUT);

}



void custom_char(int* pattern[], int location,int address)
{
  i2c_lcd_send_command(0,0x40 + location *8,address);
  for(int i=0; i<8;i++ )
       {
      i2c_lcd_send_command(1,pattern[i],address);         
       }       
}

void i2c_lcd_send_command(int rs_state, int comm,int address)
{
  //data_u contains the upper segment of the command (upper 4 bits)
  //data_l contains the lower segment of the command ( lower 4 bits )
	char data_u, data_l;

  //message to be sent to the pcf8574: 4 packets
	uint8_t data_t[4];
  // comm&0xf0 <=> comm & 11110000 (selects the first 4 bits)
	data_u = comm&0xf0;
  // (comm<<4)&0xf0 : shifts command 4 bits to the left then selects the first 4 bits of shifted command ie the last 4 bits of the original command
	data_l = comm<<4;
  
  esp_err_t err;
  switch(rs_state)
  {
    case 0:
 // example : if the command is 0xff data_t[0] will contain: 1111 0000 | 0000 0100 <=> 1111 0100 
 //                                                                                          ^ enable pin bit
	    data_t[0] = data_u|0x0c;  //en=1, rs=0 
	    data_t[1] = data_u|0x08;  //en=0, rs=0
	    data_t[2] = data_l|0x0c;  //en=1, rs=0
	    data_t[3] = data_l|0x08;  //en=0, rs=0 
      err=i2c_master_write_to_device(I2C_NUM_0,address ,data_t,4,1000);
      if (err != 0) ESP_LOGI(TAGLCD, "Error sending comm");
      break;
    case 1:
	    data_t[0] = data_u|0x0d;  //en=1, rs=1 
	    data_t[1] = data_u|0x09;  //en=0, rs=1
	    data_t[2] = data_l|0x0d;  //en=1, rs=1
	    data_t[3] = data_l|0x09;  //en=0, rs=1
      err=i2c_master_write_to_device(I2C_NUM_0,address,data_t,4,1000);
      if (err != 0) ESP_LOGI(TAGLCD, "Error sending comm");
      break;
  }
}



void i2c_lcd_write_message(char* msg,int address)
{
      while((*msg)!='\0')
      {
      i2c_lcd_send_command(1,*msg,address);
      msg++;
      }
}



void i2c_lcd_init_sequence(int address)
{
    i2c_lcd_send_command(0,0x03,address);
    ets_delay_us(4500); // wait min 4.1ms

    i2c_lcd_send_command(0,0x03,address);
    ets_delay_us(4500); // wait min 4.1ms
    
    i2c_lcd_send_command(0,0x03,address);
    ets_delay_us(45000);

    i2c_lcd_send_command(0,0x02,address); // interface in 4 bit mode
    ets_delay_us(200);
    
    i2c_lcd_send_command(0,0x28,address); //function set
    ets_delay_us(1000);
    
    i2c_lcd_send_command(0,0x0c,address); //display on, cursor on, blinking on
    ets_delay_us(50);
    
    i2c_lcd_send_command(0,0x01,address); //clear
    ets_delay_us(2000);
    
    i2c_lcd_send_command(0,0x06,address); //set write mode
    i2c_lcd_send_command(0,0x0c,address); //send cursor home
    ets_delay_us(2000);
}

void i2c_lcd_clear_screen(int address)
{
  i2c_lcd_send_command(0, 0x01, address);
}
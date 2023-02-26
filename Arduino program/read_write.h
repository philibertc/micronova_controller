#include <Arduino.h>

// Codes
#define STOVE_CODE_RAM_READ     0x00
#define STOVE_CODE_EEPROM_READ  0x20
#define STOVE_CODE_RAM_WRITE    0x80
#define STOVE_CODE_EEPROM_WRITE 0xA0

// 0 - OFF, 1 - Starting, 2 - Pellet loading, 3 - Ignition, 4 - Work, 5 - Brazier cleaning, 6 - Final cleaning, 7 - Standby, 8 - Pellet missing alarm, 9 - Ignition failure alarm, 10 - Alarms (to be investigated)
// Addresses
#define STOVE_ADDR_AMBIENT_TEMP   0x01
#define STOVE_ADDR_STATE          0x21
#define STOVE_ADDR_FUMES_TEMP     0x3E
#define STOVE_ADDR_FUMES_SPEED    0x37
#define STOVE_ADDR_POWER_RAM      0x34
#define STOVE_ADDR_POWER_EEPROM   0x7F
#define STOVE_ADDR_IRCOMMAND      0x58
#define STOVE_ADDR_THERMOSTAT     0x7D


// States
#define STOVE_STATE_TURN_ON       0x01
#define STOVE_STATE_TURN_OFF      0x06
#define STOVE_STATE_FORCE_OFF     0x00


// IR Commands ( thank you eni23 )
#define STOVE_IR_POWERUP          0x50
#define STOVE_IR_POWERDOWN        0x54
#define STOVE_IR_TEMPUP           0x52
#define STOVE_IR_TEMPDOWN         0x58
#define STOVE_IR_POWER            0x5A

// Checksum: Code+Address+Value on hexadecimal calculator


class read
{
	private:

		const int pin_rx;
		const int pin_rx_enable;

	public:

		char stove_rx_data[2];
		uint8_t last_read_value;
		uint8_t last_read_param;
		uint8_t last_read_checksum;

		read( int rx, int tx, int rx_enable );

		void enable_rx();
		void disable_rx();
		void flush_input();
		void read(uint8_t location, uint8_t addr);
		uint8_t read_ram( uint8_t addr );
		uint8_t read_eeprom( uint8_t addr );
}


class write
{
	private:

		const int pin_tx;

	public:

		void write( uint8_t location, uint8_t command, uint8_t data );
		void write_ram( uint8_t command, uint8_t data );
		void write_eeprom( uint8_t command, uint8_t data );
		void simulate_infrared(uint8_t data, uint8_t repetitions );
		byte calculate_checksum( uint8_t dest, uint8_t addr, uint8_t val );
		void read_answer();
}
#include "top_hci.h"
#include <string.h>
#include <stdbool.h>

#include "FreeRTOS.h"

#define HCI_RELIABLE_PACKET (0x1u << 7)
#define HCI_INTEGRITY_CHECK (0x1u << 6)
#define HCI_VENDOR_NORDIC_OPCODE 14u
#define HCI_HEADER_SIZE 4u
#define HCI_CRC_SIZE 2u
static struct{
	uint8_t sequence_number;
	uint8_t acknowledge_number;
}self;
#if 0
static void _inc_seq(void) {
	//increase sequence number
	self.sequence_number = (self.sequence_number + 1) & 0x7;
}
#endif
static uint8_t
_header_checksum_calculate(const uint8_t * hdr) {
	uint32_t checksum = hdr[0];
	checksum += hdr[1];
	checksum += hdr[2];
	checksum &= 0xFFu;
	checksum = (~checksum + 1u);
	return (uint8_t) checksum;
}

static uint16_t
_crc16_compute(const uint8_t * data, uint32_t size, const uint16_t * init_val) {
	uint32_t i;
	uint16_t crc = init_val?(*init_val):(0xFFFF);
	for (i = 0; i < size; i++) {
		crc = (uint8_t) (crc >> 8) | (crc << 8);
		crc ^= data[i];
		crc ^= (uint8_t) (crc & 0xff) >> 4;
		crc ^= (crc << 8) << 4;
		crc ^= ((crc & 0xff) << 4) << 1;
	}
	return crc;
}
uint16_t hci_crc16_compute_cont(uint8_t * raw, uint32_t length, const uint16_t * cont){
	return _crc16_compute(raw, length, cont);
}
uint16_t hci_crc16_compute(uint8_t * raw, uint32_t length){
	return _crc16_compute(raw, length, NULL);
}
uint32_t hci_decode(uint8_t * raw, uint32_t length, const hci_decode_handler_t * handler){
	bool has_checksum = false;

	if(length < HCI_HEADER_SIZE){
		//LOGI("header size fail %u \r\n", length);
		handler->on_decode_failed();
		return 0;
	}
	if( !(raw[0] & HCI_RELIABLE_PACKET) ){
		//Unreliable packet does not necessarily mean it fails, just throw a warning instead
		//LOGI("Not reliable packet\r\n");
	}
	if( !(raw[0] & HCI_INTEGRITY_CHECK) ){
		//LOGI("No Checksum\r\n");
	}else{
		has_checksum = true;
	}
	if((raw[1] & 0x0Fu) == HCI_VENDOR_NORDIC_OPCODE){
		//custom opcode is 14, we don tknow how to handle others
		//may need unknown message handler
		//is_custom = true;
	}
	//check header integrity
	const uint32_t expected_checksum = (raw[0] + raw[1] + raw[2] + raw[3]) & 0xFFu;
	if(expected_checksum != 0){
		//LOGI("Header Checksu fail \r\n");
		handler->on_decode_failed();
		return 0;
	}
	//check body integrity
	if(has_checksum){
		uint16_t crc_calculated = _crc16_compute(raw, (length - HCI_CRC_SIZE), NULL);
		uint16_t crc_received = *(uint16_t*) (raw + length - HCI_CRC_SIZE);
		if (crc_received != crc_calculated) {
			/*LOGI("Body Checksum fail cal%x rcvd %x\r\n", crc_calculated,
					crc_received);*/
			return 0;
		}
	}
	//update ack number
	{
		uint32_t decoded_length = length - HCI_HEADER_SIZE - (has_checksum?HCI_CRC_SIZE:0);
		uint8_t ack = (raw[0] & 0x38u) >> 3u;
		self.acknowledge_number = (raw[0] & 0x7u) + 1;
		//LOGI("ack:%x", ack);
		if (ack == self.sequence_number) {
			//retransmit
			handler->on_ack_failed();
		} else {
			self.sequence_number = ack;
			handler->on_ack_success();
			//message succeeded
		}
		if(decoded_length){
			handler->on_message(raw + 4, length);
		}
		return decoded_length;
	}
}

uint8_t * hci_encode(uint8_t * message_body, uint32_t body_length, uint32_t * out_encoded_len){
	uint32_t total_size = HCI_HEADER_SIZE + body_length + HCI_CRC_SIZE;
	void * ret = (void*)pvPortMalloc(total_size);
	if(ret){
		uint8_t * header = (uint8_t *) (ret);
		uint8_t * body = (uint8_t *) (header + HCI_HEADER_SIZE);
		uint16_t * checksum = (uint16_t*) (body + body_length);
		//pack header
		header[0] = (HCI_RELIABLE_PACKET | HCI_INTEGRITY_CHECK)
						+ self.sequence_number + (self.acknowledge_number << 3);
		*(uint16_t*) (header + 1) = (uint16_t) (((body_length + HCI_CRC_SIZE)
						<< 4) + HCI_VENDOR_NORDIC_OPCODE);
		header[3] = _header_checksum_calculate(header);
		memcpy(body, message_body, body_length);
		*checksum = _crc16_compute(header, body_length + HCI_HEADER_SIZE, NULL);
		if(out_encoded_len){
			*out_encoded_len = total_size;
		}

	}
	return ret;
}
void hci_free(uint8_t * encoded_message){
	vPortFree(encoded_message);
}
void hci_init(void){
	self.sequence_number = 1;
}

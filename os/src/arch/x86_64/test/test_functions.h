#ifndef _TEST_FUNCTIONS_H
#define _TEST_FUNCTIONS_H

void printBlock(void *params);
void readBlock(void *params);
void grabKeyboardInput();
void readBlock32();
void send_test_ethernet_frame();
void send_test_arp_request();
void send_test_arp_reply();
void send_test_ipv4();
void send_test_udp();
void send_ping();

#endif

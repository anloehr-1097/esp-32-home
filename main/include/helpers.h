#ifndef MAIN_INCLUDE_HELPERS_H_
#define MAIN_INCLUDE_HELPERS_H_

#include "esp_bit_defs.h"

const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_FAIL_BIT = BIT1;

/*
 * Maximum number of items that can be waiting in the queue at any given time.
 */
constexpr unsigned int MAX_QUEUE_SIZE = 20;

/*
 * How often wifi tasks tries to reconnect to wifi if connection is lost,
 * before giving up and setting WIFI_FAIL_BIT in event group permanentely.
 */
constexpr unsigned int MAX_WIFI_RETRY_NUM = 10;

#endif  // MAIN_INCLUDE_HELPERS_H_

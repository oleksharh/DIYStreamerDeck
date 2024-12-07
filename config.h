#ifndef CONFIG_H
#define CONFIG_H

// TFT Configuration
#define TFT_RST  20
#define TFT_DC   21
#define TFT_CS   19
#define TFT_WR   17
#define TFT_RD   16

#define TFT_D0   5
#define TFT_D1   4
#define TFT_D2   3
#define TFT_D3   2
#define TFT_D4   15
#define TFT_D5   14
#define TFT_D6   13
#define TFT_D7   12

#define SERIAL_BAUDRATE 115200

// Test configuratuions (if you happen to read it,
// just ignore current state of the repository, the whole project is under development)
// @author: https://github.com/alejandrosnz

// HTTP configuration
#define HTTP_INSECURE 1

// Spotify configuration
#define SPOTIFY_MARKET     "GB"      // 2-letter country code https://www.iban.com/country-codes
#define SPOTIFY_API_DELAY  1 * 1000  // 1 second delay between API calls

// Time and weather configuration
#define TIME_ZONE          "Europe/London"  // Timezone code https://timezonedb.com/time-zones
#define WEATHER_QUERY      "London,EN,city" // Weather location https://openweathermap.org/current#name
#define WEATHER_API_DELAY  5 * 60 * 1000    // 5 minutes delay between API calls

// Display configuration
#define MAX_CHAR_TITLE_PER_LINE  11
#define MAX_CHAR_ARTIST_PER_LINE 21


#endif 
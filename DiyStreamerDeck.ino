#include "config.h"
#include "secrets.h"
#include <TFT_eSPI.h> // Hardware-specific library for TFT
#include <ArduinoJson.h>

TFT_eSPI tft = TFT_eSPI();

// ------------------
// Standard Libraries
// ------------------
#if defined(ESP8266)
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined(ESP32)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#endif

// ----------------------------
// Spotify API lib
// github.com/witnessmenow/spotify-api-arduino
// ----------------------------
#include <SpotifyArduino.h>
#include <SpotifyArduinoCert.h>

// ----------------------------
// EzTime lib
// https://github.com/ropg/ezTime
// ----------------------------

#include <ezTime.h>
Timezone timezone;

WiFiClientSecure client;
SpotifyArduino spotify(client, SPOTIFY_CLIENT_ID, SPOTIFY_CLIENT_SECRET, SPOTIFY_REFRESH_TOKEN);
CurrentlyPlaying currentlyPlaying;
unsigned long spotify_latest_request = 0;
unsigned long weather_latest_request = 0;


void setup()
{
  Serial.begin(SERIAL_BAUDRATE);

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Text color and background
  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.println("Connecting...");
  delay(10000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("Connected to WiFi! IP: ");
  Serial.println(WiFi.localIP());

  tft.setTextSize(3);
  tft.setCursor(10, 10);
  tft.println("Connected to WiFi!");

  // Client Secure configuration
  #if defined(ESP8266)
  client.setFingerprint(SPOTIFY_FINGERPRINT);  // These expire every few months
  #elif defined(ESP32)
  client.setCACert(spotify_server_cert);
  #endif
  #if defined(HTTP_INSECURE)
  client.setInsecure();
  #endif

  // Time sync
  waitForSync();
  timezone.setLocation(TIME_ZONE);

  // Get Spotify auth token
  Serial.println("Refreshing Access Tokens");
  if (!spotify.refreshAccessToken()) {
    Serial.println("Failed to get access tokens");
  }

}

void updateSpotifyData(CurrentlyPlaying _currentlyPlaying) {
  currentlyPlaying = _currentlyPlaying;
}


void splitString(String input, int threshold) {
    int start = 0;

    while (start < input.length())
    {
        // Check if the remaining part is within the threshold
        if (input.length() - start <= threshold) {
            tft.setCursor(10,10);
            tft.println(input.substring(start));
            break;
        }

        // Find the last space within the threshold
        int end = start + threshold;
        int lastSpace = input.lastIndexOf(' ', end);

        if (lastSpace == -1 || lastSpace < start) {
            // If no space is found, split directly at the threshold
            tft.println(input.substring(start, end));
            start = end;
        } else {
            // Split at the last space
            tft.println(input.substring(start, lastSpace));
            start = lastSpace + 1; // Move past the space
        }
    }
}


long prevprogress = 0;



void printCurrentlyPlayingToDisplay() {
  Serial.println("--------- Currently Playing ---------");

  if (prevprogress == 0)
  {
    tft.fillScreen(TFT_BLACK);
  }

  // Print TRACK name
  String trackName = String(currentlyPlaying.trackName);
  Serial.println("Track: " + trackName);
  // replaceSpecialCharacters(trackName);
  splitString(trackName, MAX_CHAR_TITLE_PER_LINE);
  // if (trackName.length() <= MAX_CHAR_TITLE_PER_LINE) {
  //   tft.setCursor(10, 10);
  //   tft.print(trackName);
  // } else {
  //   tft.print(trackName.substring(0, MAX_CHAR_TITLE_PER_LINE));
  //   tft.setCursor(0, 24);
  //   if (trackName.charAt(MAX_CHAR_TITLE_PER_LINE) == ' ') {
  //     trackName.remove(MAX_CHAR_TITLE_PER_LINE, 1);
  //   }
  //   tft.print(trackName.substring(MAX_CHAR_TITLE_PER_LINE, MAX_CHAR_TITLE_PER_LINE * 2));
  // }

  // Print ARTIST list
  // String allArtistNames = "";
  // for (int i = 0; i < currentlyPlaying.numArtists; i++) {
  //   allArtistNames += currentlyPlaying.artists[i].artistName;
  //   if (i < currentlyPlaying.numArtists - 1) {
  //     allArtistNames += ", ";
  //   }
  // }
  // Serial.println("Artists: " + allArtistNames);
  // tft.setCursor(0, 36);
  // tft.setTextSize(1);
  // replaceSpecialCharacters(allArtistNames);
  // tft.println(allArtistNames.substring(0, MAX_CHAR_ARTIST_PER_LINE));
  // if (allArtistNames.charAt(MAX_CHAR_ARTIST_PER_LINE) == ' ') {
  //   allArtistNames.remove(MAX_CHAR_ARTIST_PER_LINE, 1);
  // }
  // tft.print(allArtistNames.substring(MAX_CHAR_ARTIST_PER_LINE, MAX_CHAR_ARTIST_PER_LINE * 2));

  // Print progress bar
  long progress = currentlyPlaying.progressMs;
  long duration = currentlyPlaying.durationMs;
  int progress_pixel = map(progress, 0, duration, 0, 126);

  if (prevprogress < progress)
    {
      prevprogress = progress;
    } else
    {
      prevprogress = progress;
      tft.fillScreen(TFT_BLACK);
    }

  tft.drawRect(0, 80, 128, 6, TFT_WHITE);
  tft.fillRect(0, 80, progress_pixel, 6, TFT_WHITE);

  // Print play or stop button
  tft.setCursor(10, 150);
  if (currentlyPlaying.isPlaying) {
    tft.write(0x10);  // Play
  } else {
    tft.write(0xFD);  // Stop
  }

  // Print elapsed and total time
  char buffer[15];
  sprintf(buffer, "%02lu:%02lu/%02lu:%02lu", progress / 1000 / 60, progress / 1000 % 60, duration / 1000 / 60, duration / 1000 % 60);
  tft.setCursor(200, 157);
  tft.print(buffer);
}

void loop()
{
  if (millis() > spotify_latest_request + SPOTIFY_API_DELAY) {
    int status = spotify.getCurrentlyPlaying(updateSpotifyData, SPOTIFY_MARKET);

    if (status == 200) {
      printCurrentlyPlayingToDisplay();
      Serial.println("Successfully got currently playing");
    } else if (status == 204) {
      Serial.println("Doesn't seem to be anything playing");
      printLocalTime();
    } else {
      Serial.print("ERROR! Status " + status);
    }
    spotify_latest_request = millis();
  }


}

void replaceSpecialCharacters(String &str) {
  // Replace lower-case spanish chars
  str.replace("á", "a");
  str.replace("é", "e");
  str.replace("í", "i");
  str.replace("ó", "o");
  str.replace("ú", "u");
  str.replace("ñ", "n");
  str.replace("ü", "u");

  // Replace upper-case spanish chars
  str.replace("Á", "A");
  str.replace("É", "E");
  str.replace("Í", "I");
  str.replace("Ó", "O");
  str.replace("Ú", "U");
  str.replace("Ñ", "N");
  str.replace("Ü", "U");

  // Replace other special chars
  str.replace("Ø", "O");
}


void printLocalTime() {
  // Format and print current time
  // tft.fillScreen(TFT_BLACK);
  tft.setCursor(20, 5);
  tft.setTextSize(3);
  tft.print(timezone.dateTime("H:i"));

}





// void displayBMP(const uint16_t *bmpData, int16_t startX, int16_t startY, int16_t width, int16_t height)
// {
//   Serial.print("Width: ");
//   Serial.println(width);
//   Serial.print("Height: ");
//   Serial.println(height);

//   Serial.println("Starting to draw BMP file...");

//   // // BMP data is stored bottom-to-top, so we need to process it in reverse
//   // for (int y = 0; y < height; y++) {
//   //     for (int x = 0; x < width; x++) {
//   //         // Calculate the index for bmpData array
//   //         uint16_t color = bmpData[(height - 1 - y) * width + x]; // Bottom-to-top handling
//   //         // Draw the pixel at the corresponding location
//   //         tft.drawPixel(startX + x, startY + y, color);
//   //     }

//   for (int i = 0; i < 1024; i++)
//   {
//     uint16_t color = bmpData[i];
//     int x = i % 32;
//     int y = i / 32;
//     tft.drawPixel(startX + x, startY + y, color);
//   }

//   Serial.println("BMP file drawing completed.");
// }


// void setup_IGNORED()
// {
//   Serial.begin(115200);

//   // Initialize TFT
//   tft.begin();
//   tft.setRotation(3);
//   tft.fillScreen(TFT_BLACK);
//   tft.fillScreen(TFT_BLACK);
//   tft.setTextColor(TFT_WHITE);
//   tft.setTextSize(2);
//   tft.setCursor(10, 10);
//   tft.println("TFT Display");

//   displayBMP(smileyBitmap, 0, 0, 32, 32);
// }

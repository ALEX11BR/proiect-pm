#include <SPI.h>

#include <TFT_eSPI.h>

#include <duktape.h>


TFT_eSPI tft = TFT_eSPI();

duk_context *ctx = duk_create_heap_default();

void setup() {
    duk_eval_string(ctx, "console.log('Hello, world!');");
}

void loop() {

}

#include <lvgl.h>
#include <TFT_eSPI.h>
#include <ui.h>
#include <ArduinoJson.h>
/*Don't forget to set Sketchbook location in File/Preferences to the path of your UI project (the parent foder of this INO file)*/

/*Change to your screen resolution*/
static const uint16_t screenWidth  = 160;
static const uint16_t screenHeight = 128;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[ screenWidth * screenHeight / 10 ];

TFT_eSPI tft = TFT_eSPI(screenWidth, screenHeight); /* TFT instance */

JsonDocument doc;

#if LV_USE_LOG != 0
/* Serial debugging */
void my_print(const char * buf)
{
    Serial.printf(buf);
    Serial.flush();
}
#endif

/* Display flushing */
void my_disp_flush( lv_disp_drv_t *disp, const lv_area_t *area, lv_color_t *color_p )
{
    uint32_t w = ( area->x2 - area->x1 + 1 );
    uint32_t h = ( area->y2 - area->y1 + 1 );

    tft.startWrite();
    tft.setAddrWindow( area->x1, area->y1, w, h );
    tft.pushColors( ( uint16_t * )&color_p->full, w * h, true );
    tft.endWrite();

    lv_disp_flush_ready( disp );
}

/*Read the touchpad*/
void my_touchpad_read( lv_indev_drv_t * indev_driver, lv_indev_data_t * data )
{
    uint16_t touchX = 0, touchY = 0;

    bool touched = false;//tft.getTouch( &touchX, &touchY, 600 );

    if( !touched )
    {
        data->state = LV_INDEV_STATE_REL;
    }
    else
    {
        data->state = LV_INDEV_STATE_PR;

        /*Set the coordinates*/
        data->point.x = touchX;
        data->point.y = touchY;

        Serial.print( "Data x " );
        Serial.println( touchX );

        Serial.print( "Data y " );
        Serial.println( touchY );
    }
}

void setup()
{
    Serial.begin( 115200 ); /* prepare for possible serial debug */
    Serial1.begin(115200);
    pinMode(7, OUTPUT);
    digitalWrite(7, HIGH);

    //String LVGL_Arduino = "";
    //LVGL_Arduino += String('V') + lv_version_major() + "." + lv_version_minor() + "." + lv_version_patch();
    //Serial.println( LVGL_Arduino );

    lv_init();

#if LV_USE_LOG != 0
    lv_log_register_print_cb( my_print ); /* register print function for debugging */
#endif

    tft.begin();          /* TFT init */
    tft.setRotation( 3 ); /* Landscape orientation, flipped */

    lv_disp_draw_buf_init( &draw_buf, buf, NULL, screenWidth * screenHeight / 10 );

    /*Initialize the display*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init( &disp_drv );
    /*Change the following line to your display resolution*/
    disp_drv.hor_res = screenWidth;
    disp_drv.ver_res = screenHeight;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register( &disp_drv );

    /*Initialize the (dummy) input device driver*/
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init( &indev_drv );
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register( &indev_drv );


    ui_init();

    Serial.println( "Setup done" );
}

void loop()
{
    lv_timer_handler(); /* let the GUI do its work */
    while (Serial1.available()) {  // Kiểm tra có dữ liệu từ Serial không
      String jsonString = Serial1.readStringUntil('\n');  // Đọc dòng JSON
      delay(500);
      DeserializationError error = deserializeJson(doc, jsonString);
      serializeJson(doc, Serial);
      Serial.println();
      const char* hour = doc["hour"];
      lv_label_set_text(ui_hourLabel, hour);
      const char* minute = doc["minute"];
      lv_label_set_text(ui_MinuteLabel, minute);
      const char*soil = doc["soil"];
      //const char* soiltxt = soil + "%";
      //Serial.println(soil);
      lv_label_set_text(ui_SoilMoisteurPercent, soil);
      long soilNum = atoi(soil);
      Serial.println(soilNum);
      lv_bar_set_value(ui_SoilMonsteurBar, soilNum, LV_ANIM_OFF);
      const char* ldr = doc["ldr"];
      //const char* ldrtxt = ldr + "%";
      //Serial.println(ldrtxt);
      lv_label_set_text(ui_LightPercent, ldr);
      int ldrNum = atoi(ldr);
      Serial.println(ldrNum);
      lv_bar_set_value(ui_HumidityPercent, ldrNum, LV_ANIM_OFF);
    }

    //delay(5);
}

/* Includes ---------------------------------------------------------------- */
#include <Vision_Sorting_V1_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"

// --- WI-FI & WEB SERVER HEADERS ---
#include <WiFi.h>
#include <WebServer.h>
#include <base64.h>

WebServer server(80);
String global_base64 = "";

// Select camera model
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#if defined(CAMERA_MODEL_ESP_EYE)
#define PWDN_GPIO_NUM    -1
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    4
#define SIOD_GPIO_NUM    18
#define SIOC_GPIO_NUM    23

#define Y9_GPIO_NUM      36
#define Y8_GPIO_NUM      37
#define Y7_GPIO_NUM      38
#define Y6_GPIO_NUM      39
#define Y5_GPIO_NUM      35
#define Y4_GPIO_NUM      14
#define Y3_GPIO_NUM      13
#define Y2_GPIO_NUM      34
#define VSYNC_GPIO_NUM   5
#define HREF_GPIO_NUM    27
#define PCLK_GPIO_NUM    25

#elif defined(CAMERA_MODEL_AI_THINKER)
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#else
#error "Camera model not selected"
#endif

/* Constant defines -------------------------------------------------------- */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS           320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS           240
#define EI_CAMERA_FRAME_BYTE_SIZE                 3

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; 
static bool is_initialised = false;
uint8_t *snapshot_buf; 

static camera_config_t camera_config = {
    .pin_pwdn = PWDN_GPIO_NUM,
    .pin_reset = RESET_GPIO_NUM,
    .pin_xclk = XCLK_GPIO_NUM,
    .pin_sscb_sda = SIOD_GPIO_NUM,
    .pin_sscb_scl = SIOC_GPIO_NUM,

    .pin_d7 = Y9_GPIO_NUM,
    .pin_d6 = Y8_GPIO_NUM,
    .pin_d5 = Y7_GPIO_NUM,
    .pin_d4 = Y6_GPIO_NUM,
    .pin_d3 = Y5_GPIO_NUM,
    .pin_d2 = Y4_GPIO_NUM,
    .pin_d1 = Y3_GPIO_NUM,
    .pin_d0 = Y2_GPIO_NUM,
    .pin_vsync = VSYNC_GPIO_NUM,
    .pin_href = HREF_GPIO_NUM,
    .pin_pclk = PCLK_GPIO_NUM,

    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, 
    .frame_size = FRAMESIZE_QVGA,    

    .jpeg_quality = 12, 
    .fb_count = 1,       
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

/* Function definitions ------------------------------------------------------- */
bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) ;
void handleAIStream();

/**
* @brief      Arduino setup function
*/
void setup()
{
    Serial.begin(115200);
    while (!Serial);
    
    // --- START ESP32 INDEPENDENT WI-FI ROUTER ---
    Serial.println("\nStarting ESP32 Access Point...");
    WiFi.mode(WIFI_AP);
    WiFi.softAP("RobotVision", "password123"); 
    delay(500); 

    Serial.println("\n=================================");
    Serial.println("       NETWORK IS LIVE!          ");
    Serial.println("=================================");
    Serial.println("1. Click the Wi-Fi icon on your computer/phone.");
    Serial.println("2. Connect to network: RobotVision");
    Serial.println("3. Enter password: password123");
    Serial.print("4. Open Chrome and type exact IP: ");
    Serial.println(WiFi.softAPIP()); 
    Serial.println("=================================\n");

    server.on("/", handleAIStream);
    server.begin();

    Serial.println("Edge Impulse Inferencing Demo");
    if (ei_camera_init() == false) {
        ei_printf("Failed to initialize Camera!\r\n");
    }
    else {
        ei_printf("Camera initialized\r\n");
    }
}

/**
* @brief      Constantly listen for Web Server clients
*/
void loop() {
  server.handleClient(); 
}

/**
* @brief      Get data, run inferencing, and serve HTML
*/
void handleAIStream()
{
    if (ei_sleep(5) != EI_IMPULSE_OK) {
        return;
    }

    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);

    if(snapshot_buf == nullptr) {
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false) {
        ei_printf("Failed to capture image\r\n");
        free(snapshot_buf);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = { 0 };

    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

    // --- BUILD THE HTML WEBSITE ---
    String html = "<!DOCTYPE html><html><head><meta http-equiv='refresh' content='1'></head>";
    html += "<body style='background:#111; color:white; font-family:sans-serif; text-align:center; padding-top:20px;'>";
    html += "<h2>Robotic Sorting Vision Feed</h2>";
    html += "<div style='position:relative; display:inline-block;'>";
    
    // Embed the raw camera image
    html += "<img src='data:image/jpeg;base64," + global_base64 + "' style='width:320px; height:240px; border:2px solid #555;'>";

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    // Loop through AI results and draw dynamic CSS boxes over the image
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) continue;

        // Scale the 96x96 AI math up to the 320x240 image size
        float scaleX = 320.0 / 96.0;
        float scaleY = 240.0 / 96.0;
        int w = bb.width * scaleX;
        int h = bb.height * scaleY;
        int x = (bb.x * scaleX) - (w / 2); 
        int y = (bb.y * scaleY) - (h / 2);

        html += "<div style='position:absolute; border:3px solid #00FF00; ";
        html += "left:" + String(x) + "px; top:" + String(y) + "px; ";
        html += "width:" + String(w) + "px; height:" + String(h) + "px;'>";
        html += "<span style='background:#00FF00; color:black; font-weight:bold; font-size:12px; position:absolute; top:-18px; left:-3px; padding:2px 5px;'>";
        html += String(bb.label) + " (" + String(bb.value * 100, 0) + "%)</span></div>";
    }
#endif

    html += "</div></body></html>";
    
    // Serve the generated website to the browser
    server.send(200, "text/html", html);

    // Free memory to prevent crashes
    free(snapshot_buf);
}

/**
 * @brief   Setup image sensor & start streaming
 */
bool ei_camera_init(void) {
    if (is_initialised) return true;

    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
      Serial.printf("Camera init failed with error 0x%x\n", err);
      return false;
    }

    sensor_t * s = esp_camera_sensor_get();
    if (s->id.PID == OV3660_PID) {
      s->set_vflip(s, 1); 
      s->set_brightness(s, 1); 
      s->set_saturation(s, 0); 
    }

    s->set_vflip(s, 1);
    s->set_hmirror(s, 1);

    is_initialised = true;
    return true;
}

/**
 * @brief      Stop streaming of sensor data
 */
void ei_camera_deinit(void) {
    esp_err_t err = esp_camera_deinit();
    if (err != ESP_OK)
    {
        ei_printf("Camera deinit failed\n");
        return;
    }
    is_initialised = false;
    return;
}

/**
 * @brief      Capture, rescale and crop image
 */
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf) {
    bool do_resize = false;

    if (!is_initialised) {
        ei_printf("ERR: Camera is not initialized\r\n");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb) {
        ei_printf("Camera capture failed\n");
        return false;
    }

   bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);
   
   // HIJACK THE IMAGE FOR THE WEB SERVER
   global_base64 = base64::encode(fb->buf, fb->len);
   
   esp_camera_fb_return(fb);

   if(!converted){
       ei_printf("Conversion failed\n");
       return false;
   }

    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS)
        || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS)) {
        do_resize = true;
    }

    if (do_resize) {
        ei::image::processing::crop_and_interpolate_rgb888(
        out_buf,
        EI_CAMERA_RAW_FRAME_BUFFER_COLS,
        EI_CAMERA_RAW_FRAME_BUFFER_ROWS,
        out_buf,
        img_width,
        img_height);
    }
    return true;
}

static int ei_camera_get_data(size_t offset, size_t length, float *out_ptr)
{
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0) {
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix + 2] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix];
        out_ptr_ix++;
        pixel_ix+=3;
        pixels_left--;
    }
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif
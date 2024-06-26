/* Includes ---------------------------------------------------------------- */
#include <iot_tracking_inferencing.h>
#include "edge-impulse-sdk/dsp/image/image.hpp"
#include "esp_camera.h"
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

/*Define  module and pins*/
#define CAMERA_MODEL_AI_THINKER // Has PSRAM

#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27

#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

#define BUILT_IN_LED 4

/* Constant defines -------------------------------------------------------- */
#define EI_CAMERA_RAW_FRAME_BUFFER_COLS 320
#define EI_CAMERA_RAW_FRAME_BUFFER_ROWS 240
#define EI_CAMERA_FRAME_BYTE_SIZE 3

/* Private variables ------------------------------------------------------- */
static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
static bool is_initialised = false;
uint8_t *snapshot_buf; // points to the output of the capture

WebServer server(80);

StaticJsonDocument<500> jdoc;
char buffer[500];

const char *ssid = "broker-rapsi-Salle-IoT";
const char *password = "pi-corte";

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

    // XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
    .frame_size = FRAMESIZE_QVGA,   // QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, // 0-63 lower number means higher quality
    .fb_count = 1,      // if more than one, i2s runs in continuous mode. Use only with JPEG
    .fb_location = CAMERA_FB_IN_PSRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

/* Function definitions ------------------------------------------------------- */
/*ML Functions*/

bool ei_camera_init(void);
void ei_camera_deinit(void);
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf);

/*Server Functions*/

void wifi_connection(void);
void start_server(void);
void espToAPI(void);
void create_json(const char *tag, int xValue, int yValue, int width, int height, float precison);
void setPosition(void);

/*Helper Functions*/

void fade(int delay_time);
void blink(int times, int delay_time);

/**
 * @brief      Arduino setup function
 */
void setup()
{
    Serial.begin(115200);
    pinMode(BUILT_IN_LED, OUTPUT);
    analogWrite(BUILT_IN_LED, 5);
    analogWrite(BUILT_IN_LED, 0);
    wifi_connection();
    start_server();
    ei_sleep(5000);

    if (!ei_camera_init())
        ei_printf("Failed to initialize Camera!\r\n");
    else
        ei_printf("Camera initialized\r\n");

    blink(2, 100);
    ei_printf("\nStarting continious inference in 2 seconds...\n");

    ei_sleep(2000);
}

/**
 * @brief      Get data and run inferencing
 *
 * @param[in]  debug  Get debug info if true
 */
void loop()
{
    // permet de gérer les requêtes du serveur
    server.handleClient();
    // instead of wait_ms, we'll wait on the signal, this allows threads to cancel us...
    if (ei_sleep(5) != EI_IMPULSE_OK)
    {
        return;
    }

    snapshot_buf = (uint8_t *)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);

    // check if allocation was successful
    if (snapshot_buf == nullptr)
    {
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf) == false)
    {
        ei_printf("Failed to capture image\r\n");
        free(snapshot_buf);
        return;
    }

    // Run the classifier
    ei_impulse_result_t result = {0};

    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK)
    {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        return;
    }

#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    bool bb_found = result.bounding_boxes[0].value > 0;
    for (size_t ix = 0; ix < result.bounding_boxes_count; ix++)
    {
        auto bb = result.bounding_boxes[ix];
        if (bb.value == 0)
        {
            continue;
        }
        create_json(bb.label, bb.x, bb.y, bb.width, bb.height, bb.value);
        ei_printf("    %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\n", bb.label, bb.value, bb.x, bb.y, bb.width, bb.height);
    }
    if (!bb_found)
    {
        create_json("No object", 0, 0, 0, 0, 100.00);
        ei_printf("    No objects found\n");
    }
#else
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
        ei_printf("    %s: %.5f\n", result.classification[ix].label,
                  result.classification[ix].value);
    }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

    free(snapshot_buf);
}

//---------------------------------- Helper functions--------------------------------------------

/**
 * @brief      Fait clignoter la LED
 *
 * @param[in]  times       nombre de clignotements
 * @param[in]  delay_time  durée d'un clignotement
 */
void blink(int times, int delay_time)
{
    for (int i = 0; i < 5; i++)
    {
        analogWrite(BUILT_IN_LED, 7);
        delay(delay_time);
        analogWrite(BUILT_IN_LED, 0);
        delay(delay_time);
    }
}

/**
 * @brief      Fait un fade de la LED
 *
 * @param[in]  delay_time  The delay time
 */
void fade(int delay_time)
{
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 7; j++)
        {
            analogWrite(BUILT_IN_LED, j);
            delay(delay_time);
        }
        for (int j = 7; j > 0; j--)
        {
            analogWrite(BUILT_IN_LED, j);
            delay(delay_time);
        }
    }
}

//---------------------------------- Server functions--------------------------------------------

/**
 * @brief      Connecte le device au wifi avec les identifiants donnés.
 *            la LED oscille tant que la connexion n'est pas établie
 *              puis clignote 2 fois pour indiquer que la connexion est établie
 *
 */
void wifi_connection(void)
{
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
        fade(500);
    Serial.println("Connected to WiFi");
    blink(2, 300);
}

/**
 * @brief      Demmarre le server et met en place les routes de l'api
 *              /MLData pour envoyer les données du model a l'api
 *             /set pour recevoir les données de position du serveur de calcul
 *            et les envoyer au servo
 *
 *         La LED clignote 3 fois pour indiquer que le serveur est démarré
 */
void start_server(void)
{
    server.on("/MLData", HTTP_GET, espToAPI);
    server.on("/set", HTTP_GET, setPosition);
    server.begin();
    Serial.print("Connected to wifi. My address:");
    IPAddress myAddress = WiFi.localIP();
    Serial.println(myAddress);
    blink(3, 300);
}

/**
 * @brief      Creer un json avec les valeurs du model
 *
 * @param[in]  tag       The tag
 * @param[in]  xValue    The x value
 * @param[in]  yValue    The y value
 * @param[in]  width     The width
 * @param[in]  height    The height
 * @param[in]  precison  The precison
 */
void create_json(const char *tag, int xValue, int yValue, int width, int height, float precison)
{
    jdoc["type"] = tag;
    jdoc["x"] = xValue;
    jdoc["y"] = yValue;
    jdoc["width"] = width;
    jdoc["height"] = height;
    jdoc["precision"] = precison;

    serializeJson(jdoc, buffer);
}

/**
 * @brief      Recupère les valeurs x et y de la requête /set apres les
 *             calculs de position du Serveur de Calcul et les envoies au servo
 *
 */
void setPosition(void)
{
    if (server.hasArg("x") && server.hasArg("y"))
    {
        int x = server.arg("x").toInt();
        int y = server.arg("y").toInt();
        Serial.println("Setting servo position");
        Serial.printf("/%d,%d", x, y);
        server.send(200, "text/plain", "Received x: " + String(x) + ", y: " + String(y));
    }
    else
        server.send(400, "text/plain", "Missing parameters");
}

/**
 * @brief    Envoies les données du model sur l'api
 *          sous forme de json
 *
 */
void espToAPI(void)
{
    Serial.println("Get data from ML model");
    Serial.println("Sending data to API");
    server.send(200, "application/json", buffer);
}

//---------------------------------- Camera functions--------------------------------------------

/**
 * @brief   Setup image sensor & start streaming
 *
 * @retval  false if initialisation failed
 */
bool ei_camera_init(void)
{
    if (is_initialised)
        return true;
    // initialize the camera
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }

    sensor_t *s = esp_camera_sensor_get();
    // initial sensors are flipped vertically and colors are a bit saturated
    if (s->id.PID == OV3660_PID)
    {
        s->set_vflip(s, 1);      // flip it back
        s->set_brightness(s, 1); // up the brightness just a bit
        s->set_saturation(s, 0); // lower the saturation
    }
    is_initialised = true;
    return true;
}

/**
 * @brief      Stop streaming of sensor data
 */
void ei_camera_deinit(void)
{
    // deinitialize the camera
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
 *
 * @param[in]  img_width     width of output image
 * @param[in]  img_height    height of output image
 * @param[in]  out_buf       pointer to store output image, NULL may be used
 *                           if ei_camera_frame_buffer is to be used for capture and resize/cropping.
 *
 * @retval     false if not initialised, image captured, rescaled or cropped failed
 *
 */
bool ei_camera_capture(uint32_t img_width, uint32_t img_height, uint8_t *out_buf)
{
    bool do_resize = false;

    if (!is_initialised)
    {
        ei_printf("ERR: Camera is not initialized\r\n");
        return false;
    }

    camera_fb_t *fb = esp_camera_fb_get();

    if (!fb)
    {
        ei_printf("Camera capture failed\n");
        return false;
    }

    bool converted = fmt2rgb888(fb->buf, fb->len, PIXFORMAT_JPEG, snapshot_buf);

    esp_camera_fb_return(fb);

    if (!converted)
    {
        ei_printf("Conversion failed\n");
        return false;
    }

    if ((img_width != EI_CAMERA_RAW_FRAME_BUFFER_COLS) || (img_height != EI_CAMERA_RAW_FRAME_BUFFER_ROWS))
    {
        do_resize = true;
    }

    if (do_resize)
    {
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
    // we already have a RGB888 buffer, so recalculate offset into pixel index
    size_t pixel_ix = offset * 3;
    size_t pixels_left = length;
    size_t out_ptr_ix = 0;

    while (pixels_left != 0)
    {
        out_ptr[out_ptr_ix] = (snapshot_buf[pixel_ix] << 16) + (snapshot_buf[pixel_ix + 1] << 8) + snapshot_buf[pixel_ix + 2];

        // go to the next pixel
        out_ptr_ix++;
        pixel_ix += 3;
        pixels_left--;
    }
    // and done!
    return 0;
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_CAMERA
#error "Invalid model for current sensor"
#endif

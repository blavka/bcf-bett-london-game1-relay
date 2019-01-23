#include <application.h>

#define NUMBER 8

// LED instance
bc_led_t led;

// Button instance
bc_button_t button;
bool radio_pairing_mode = false;
int count = 0;
bc_module_relay_t relay;

void radio_pairing_stop(void)
{
    if (!radio_pairing_mode)
    {
        return;
    }
    radio_pairing_mode = false;
    bc_radio_pairing_mode_stop();
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    bc_log_debug("Radio pairing mode stop");
}

void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param)
{
    (void) self;
    (void) event_param;

    if (event == BC_BUTTON_EVENT_PRESS)
    {
    }
    else if (event == BC_BUTTON_EVENT_HOLD)
    {
        if (radio_pairing_mode)
        {
            radio_pairing_stop();
        }
        else{
            radio_pairing_mode = true;
            bc_radio_pairing_mode_start();
            bc_led_set_mode(&led, BC_LED_MODE_BLINK_FAST);
            bc_log_debug("Radio pairing mode start");
        }
    }
}

void radio_event_handler(bc_radio_event_t event, void *event_param)
{
    (void) event_param;

    if (event == BC_RADIO_EVENT_ATTACH)
    {
        bc_log_debug("Radio attach %llx", bc_radio_get_event_id());
        bc_led_pulse(&led, 1000);
        radio_pairing_stop();
    }
    else if (event == BC_RADIO_EVENT_ATTACH_FAILURE)
    {
        bc_log_error("Radio attach");
        bc_led_pulse(&led, 5000);
    }
}

void bc_radio_pub_on_event_count(uint64_t *id, uint8_t event_id, uint16_t *event_count)
{
    (void) id;
    (void) event_count;

    if (event_id == BC_RADIO_PUB_EVENT_PUSH_BUTTON)
    {
        bc_log_debug("Click count=%d", ++count);

        bc_led_pulse(&led, 30);

        bc_scheduler_plan_from_now(0, 800);
    }
}

void application_init(void)
{
    bc_log_init(BC_LOG_LEVEL_DUMP, BC_LOG_TIMESTAMP_ABS);

    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);

    bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    bc_button_set_event_handler(&button, button_event_handler, NULL);

    bc_module_relay_init(&relay, BC_MODULE_RELAY_I2C_ADDRESS_DEFAULT);
    bc_module_relay_set_state(&relay, false);

    bc_radio_init(BC_RADIO_MODE_GATEWAY);
    bc_radio_set_event_handler(radio_event_handler, NULL);

    bc_led_pulse(&led, 2000);
}

void application_task(void)
{
    bc_log_debug("Timer count=%d", count);

    if (count == NUMBER)
    {
        bc_module_relay_set_state(&relay, true);
        bc_log_debug("Ok");
    }
    else
    {
        bc_module_relay_set_state(&relay, false);
        bc_log_debug("Bad");
    }

    count = 0;
}

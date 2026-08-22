#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/device.h>
#include <zephyr/init.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/activity_state_changed.h>
#include <zmk/keymap.h>

#define NUMLOCK_LAYER 2

static const struct gpio_dt_spec numlock_led =
    GPIO_DT_SPEC_GET(DT_NODELABEL(num_lock_led), gpios);

static void numlock_led_apply(void) {
    if (!device_is_ready(numlock_led.port)) {
        return;
    }
    bool active = zmk_keymap_layer_active(NUMLOCK_LAYER);
    gpio_pin_set_dt(&numlock_led, active ? 1 : 0);
}

static int numlock_led_listener(const zmk_event_t *eh) {
    numlock_led_apply();
    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(numlock_led, numlock_led_listener);
ZMK_SUBSCRIPTION(numlock_led, zmk_layer_state_changed);
ZMK_SUBSCRIPTION(numlock_led, zmk_activity_state_changed);

static int numlock_led_init(void) {
    numlock_led_apply();
    return 0;
}

SYS_INIT(numlock_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);

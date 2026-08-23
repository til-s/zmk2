/*
 * NumLock LED をホストのHIDインジケーターではなく、
 * ZMK側のnumlock_layer(layer 2)のアクティブ状態から直接制御する。
 *
 * これにより、電源off/onでレイヤーがdefault_layerにリセットされたとき、
 * ホスト側のNumLock状態に関わらずLEDも必ず消灯状態になる。
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

#include <zmk/event_manager.h>
#include <zmk/events/layer_state_changed.h>

LOG_MODULE_REGISTER(numlock_layer_led, CONFIG_ZMK_LOG_LEVEL);

/* keymap.c の numlock_layer のインデックス (0始まり: default=0, fn=1, numlock=2) */
#define NUMLOCK_LAYER 2

/* overlay の num_lock_led ノード (leds { num_lock_led: num_lock_led { ... } }) を直接参照 */
static const struct gpio_dt_spec num_lock_led =
    GPIO_DT_SPEC_GET(DT_NODELABEL(num_lock_led), gpios);

static int numlock_layer_led_init(void) {
    if (!device_is_ready(num_lock_led.port)) {
        LOG_ERR("num_lock_led GPIO port not ready");
        return -ENODEV;
    }

    /* 起動時は必ず消灯状態から開始する (layerはdefaultにリセットされているため) */
    return gpio_pin_configure_dt(&num_lock_led, GPIO_OUTPUT_INACTIVE);
}

static int numlock_layer_led_listener(const zmk_event_t *eh) {
    const struct zmk_layer_state_changed *ev = as_zmk_layer_state_changed(eh);

    if (ev == NULL || ev->layer != NUMLOCK_LAYER) {
        return ZMK_EV_EVENT_BUBBLE;
    }

    gpio_pin_set_dt(&num_lock_led, ev->state ? 1 : 0);

    return ZMK_EV_EVENT_BUBBLE;
}

ZMK_LISTENER(numlock_layer_led, numlock_layer_led_listener);
ZMK_SUBSCRIPTION(numlock_layer_led, zmk_layer_state_changed);

SYS_INIT(numlock_layer_led_init, APPLICATION, CONFIG_APPLICATION_INIT_PRIORITY);


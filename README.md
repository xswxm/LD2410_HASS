# LD2410
24GHz雷达LD2410接入Home Assistant，基于ESP方案，支持ESP8266和ESP32
代码示例中，使用了LD2410作为传感器来感应控制LED灯带的开关。


## 安装

* 将 ld2410.h 文件放到 esphome文件夹下

* 新建一个esphome工程，复制以下代码，并修改WiFi名称和密码为你自己的，编译后烧录即可
```yaml   
esphome:
  name: mirror
  platform: ESP8266
  board: nodemcuv2
  includes:
    - ld2410.h

# Enable logging
logger:
  level: VERBOSE #makes uart stream available in esphome logstream
  baud_rate: 0 #disable logging over uart

# Enable Home Assistant API
api:

ota:
  password: "94e11883e96a1ddef93e9dea0ccac109"

wifi:
  ssid: "你的WiFi名称"
  password: "你的WiFi密码"

  # Enable fallback hotspot (captive portal) in case wifi connection fails
  ap:
    ssid: "Mirror Fallback Hotspot"
    password: "gNBCbGo5EbeQ"

captive_portal:

binary_sensor:
  - platform: gpio
    pin: D5
    name: "Radar"
    device_class: motion

light:
  - platform: neopixelbus
    type: GRB
    variant: ws2812
    pin: D4
    num_leds: 110
    name: "LED Strip"
    effects:
     - pulse:
     - random:
     - strobe:
     - flicker:
     - addressable_rainbow:
     - addressable_color_wipe:
     - addressable_scan:
     - addressable_twinkle:
     - addressable_random_twinkle:
     - addressable_fireworks:
     - addressable_flicker:

text_sensor:
  - platform: wifi_info
    ip_address:
      name: IP Address
      id: ip
    ssid:
      name: Connected SSID
      id: ssid

uart:
  id: uart_bus
  data_bits: 8
  stop_bits: 1
  tx_pin: GPIO01
  rx_pin: GPIO03
  baud_rate: 256000

sensor:
  - platform: wifi_signal
    name: "WiFi Signal"
    update_interval: 60s
  - platform: uptime
    name: Uptime

  - platform: custom
    lambda: |-
      auto espradar_sensor = new LD2410Sensor(id(uart_bus));
      App.register_component(espradar_sensor);
      return {
        espradar_sensor->target_sensor, 
        espradar_sensor->distance_sensor,
        espradar_sensor->energy_sensor,
        espradar_sensor->distance_standby_sensor,
        espradar_sensor->energy_standby_sensor,
        espradar_sensor->range_sensor
      };
    sensors:
      - name: "Target"
        unit_of_measurement: ""
        accuracy_decimals: 0
      - name: "Distance"
        unit_of_measurement: "cm"
        accuracy_decimals: 0
        icon: "mdi:signal-distance-variant"
      - name: "Energy"
        unit_of_measurement: "%"
        accuracy_decimals: 0
      - name: "Distance (Standby)"
        unit_of_measurement: "cm"
        accuracy_decimals: 0
        icon: "mdi:signal-distance-variant"
      - name: "Energy (Standby)"
        unit_of_measurement: "%"
        accuracy_decimals: 0
      - name: "Range"
        unit_of_measurement: "cm"
        accuracy_decimals: 0
        icon: "mdi:signal-distance-variant"
```
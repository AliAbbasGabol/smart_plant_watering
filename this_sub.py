import paho.mqtt.client as mqtt

MQTT_ADDRESS = '192.168.43.21'
MQTT_USER = 'neo'
MQTT_PASSWORD = 'eglabs'
MQTT_TOPIC_TEMP = 'temperature'
MQTT_TOPIC_HUMD = 'humidity'
MQTT_TOPIC_MOIST = 'moisture'
MQTT_TOPIC_RAINDROP = 'raindrop'  # New raindrop topic

def on_connect(client, userdata, flags, rc):
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC_TEMP)
    client.subscribe(MQTT_TOPIC_HUMD)
    client.subscribe(MQTT_TOPIC_MOIST)
    client.subscribe(MQTT_TOPIC_RAINDROP)  # Subscribe to raindrop topic

def on_message(client, userdata, msg):
    if msg.topic == MQTT_TOPIC_TEMP:
        print("Temperature: " + str(msg.payload) + " C")
    elif msg.topic == MQTT_TOPIC_HUMD:
        print("Humidity: " + str(msg.payload) + " %")
    elif msg.topic == MQTT_TOPIC_MOIST:
        print("Soil Moisture: " + str(msg.payload))
    elif msg.topic == MQTT_TOPIC_RAINDROP:
        print("Raindrop Sensor: " + str(msg.payload))  # Display raindrop sensor data
    else:
        print("Unknown topic: " + msg.topic + " " + str(msg.payload))

def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()

if __name__ == '__main__':
    print('MQTT to InfluxDB bridge')
    main()


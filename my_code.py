import paho.mqtt.client as mqtt
from firebase import firebase

# Update Firebase connection
firebase = firebase.FirebaseApplication('https://smart-irrigation-system-7e2e7-default-rtdb.firebaseio.com/', None)

# Updated MQTT configuration
MQTT_ADDRESS = '192.168.43.21'
MQTT_USER = 'neo'
MQTT_PASSWORD = 'eglabs'
MQTT_TOPIC_TEMP = 'temperature'
MQTT_TOPIC_HUMD = 'humidity'
MQTT_TOPIC_MOIST = 'moisture'
MQTT_TOPIC_RAINDROP = 'raindrop'

# Global variables for temperature, humidity, moisture, and raindrop
val_temp = ''
val_humi = ''
val_moist = ''
val_raindrop = ''

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(MQTT_TOPIC_TEMP)
    client.subscribe(MQTT_TOPIC_HUMD)
    client.subscribe(MQTT_TOPIC_MOIST)
    client.subscribe(MQTT_TOPIC_RAINDROP)

def on_message(client, userdata, message):
    global val_temp, val_humi, val_moist, val_raindrop
    
    payload = str(message.payload, 'UTF-8').strip()
    
    if message.topic == MQTT_TOPIC_TEMP:
        print("Temperature update: " + payload + " C")
        val_temp = payload
    elif message.topic == MQTT_TOPIC_HUMD:
        print("Humidity update: " + payload + " %")
        val_humi = payload
    elif message.topic == MQTT_TOPIC_MOIST:
        print("Moisture update: " + payload)
        val_moist = payload
    elif message.topic == MQTT_TOPIC_RAINDROP:
        print("Raindrop sensor update: " + payload)
        val_raindrop = payload
    
    # Only post to Firebase if all required data is available
    if val_temp and val_humi and val_moist and val_raindrop:
        data = {
            "Temperature": val_temp,
            "Humidity": val_humi,
            "Moisture": val_moist,
            "Raindrop": val_raindrop
        }
        firebase.post('/sensor/dht', data)
        
        # Reset values after posting to Firebase
        val_temp, val_humi, val_moist, val_raindrop = '', '', '', ''

def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message
    
    mqtt_client.connect(MQTT_ADDRESS, 1883, 60)
    mqtt_client.loop_forever()

if __name__ == '__main__':
    print('MQTT to Firebase bridge')
    main()

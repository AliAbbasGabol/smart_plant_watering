import paho.mqtt.client as mqtt

# MQTT broker settings
broker_address = "192.168.43.21"  # Replace with your Raspberry Pi's IP address
topic = "esp32/data"

def on_connect(client, userdata, flags, rc):
    print("Connected to MQTT broker")
    client.subscribe(topic)

def on_message(client, userdata, message):
    print("Received message:", message.payload.decode())  # Decode the payload

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(broker_address)
client.loop_forever()

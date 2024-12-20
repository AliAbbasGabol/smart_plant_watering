import firebase_admin
from firebase_admin import db, credentials
import mysql.connector
import paho.mqtt.client as mqtt

# Firebase configuration
service_account_key_path = '/home/smart_irrigation/Desktop/smart-a6984-firebase-adminsdk-fo30m-43570c922a.json'
database_url = 'https://smart-a6984-default-rtdb.firebaseio.com/'
cred = credentials.Certificate(service_account_key_path)
firebase_admin.initialize_app(cred, {'databaseURL': database_url})

# MySQL configuration
mydb = mysql.connector.connect(
    host="localhost",
    user="myuser",
    password="Smart1234",
    database="smart_irrigation"
)

# MQTT configuration
mqtt_broker = "192.168.43.21"
mqtt_client = mqtt.Client("RaspberryPi")
mqtt_client.username_pw_set("neo", "eglabs")
mqtt_client.connect(mqtt_broker)

def get_plant_names():
    ref = db.reference('plants')
    snapshot = ref.get()

    if snapshot:
        for key, value in snapshot.items():
            plant_name = value.get('name')
            print(f"Key: {key}, Name: {plant_name}")
            fetch_plant_details(plant_name)
    else:
        print("No data found.")

def fetch_plant_details(plant_name):
    cursor = mydb.cursor(dictionary=True)
    query = "SELECT Common_Name, Water_Needs, Plant_Constant FROM Smart_Plants WHERE Common_Name LIKE %s"
    cursor.execute(query, (f"%{plant_name}%",))
    result = cursor.fetchone()
    
    if result:
        print(f"Plant: {result['Common_Name']}, Water Needs: {result['Water_Needs']}, Plant Constant: {result['Plant_Constant']}")
        send_via_mqtt(result['Common_Name'], result['Water_Needs'], result['Plant_Constant'])
    else:
        print(f"No match found for plant name: {plant_name}")

def send_via_mqtt(common_name, water_needs, plant_constant):
    mqtt_client.publish("plant/common_name", common_name)
    mqtt_client.publish("plant/water_needs", water_needs)
    mqtt_client.publish("plant/plant_constant", plant_constant)
    print("Data sent via MQTT")

if __name__ == "__main__":
    print("Retrieving plant names from Firebase:")
    get_plant_names()

import firebase_admin
from firebase_admin import db, credentials

# Replace this with the path to your service account key JSON file
service_account_key_path = '/home/smart_irrigation/Desktop/smart-a6984-firebase-adminsdk-fo30m-43570c922a.json'
database_url = 'https://smart-a6984-default-rtdb.firebaseio.com/'

# Initialize the Firebase app using the service account key JSON file
cred = credentials.Certificate(service_account_key_path)
firebase_admin.initialize_app(cred, {
    'databaseURL': database_url
})

def get_plant_names():
    # Reference to the "plants" node in your Firebase database
    ref = db.reference('plants')
    # Get the data at this database reference
    snapshot = ref.get()

    if snapshot:
        for key, value in snapshot.items():
            plant_name = value.get('name')
            print(f"Key: {key}, Name: {plant_name}")
    else:
        print("No data found.")

if __name__ == "__main__":
    # Retrieve and print plant names from the Firebase database
    print("Retrieving plant names from Firebase:")
    get_plant_names()

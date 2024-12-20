import mariadb
import time
from datetime import datetime

# Connection details
db_config = {
    'user': 'myuser',
    'password': 'Smart1234',
    'host': '192.168.43.21',
    'database': 'mydb'
}

# Function to connect to the database
def connect_to_db(config):
    try:
        connection = mariadb.connect(**config)
        return connection
    except mariadb.Error as e:
        print(f"Error connecting to database: {e}")
        return None

# Function to insert and delete data in the loop
def handle_data():
    # Connect to the database
    db_connection = connect_to_db(db_config)
    if not db_connection:
        return

    try:
        # Create a cursor
        with db_connection.cursor() as cursor:
            while True:
                # Get current time in seconds
                current_time = int(time.time())

                # Convert the current time to datetime format for the timestamp column
                datetime_format = datetime.fromtimestamp(current_time).strftime('%Y-%m-%d %H:%M:%S')

                # Insert a new data point with the current time
                insert_query = "INSERT INTO data_table (timestamp, value) VALUES (?, ?)"
                cursor.execute(insert_query, (datetime_format, current_time))

                # Delete data older than 60 seconds
                threshold_time = current_time - 60
                threshold_datetime = datetime.fromtimestamp(threshold_time).strftime('%Y-%m-%d %H:%M:%S')
                delete_query = "DELETE FROM data_table WHERE timestamp < ?"
                cursor.execute(delete_query, (threshold_datetime,))

                # Commit the transaction
                db_connection.commit()

                # Wait for one second
                time.sleep(1)

    except mariadb.Error as e:
        print(f"Database error: {e}")

    finally:
        # Ensure the connection is closed
        db_connection.close()

# Run the data handling function
handle_data()

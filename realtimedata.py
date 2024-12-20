import mariadb
import time

# Connection details
db_config = {
    'user': 'myuser',
    'password': 'Smart1234',
    'host': '192.168.43.21',
    'database': 'mydb'
}

# Connect to the database using a context manager
with mariadb.connect(**db_config) as db_connection:
    # Create a cursor using a context manager
    with db_connection.cursor() as cursor:
        while True:
            # Get current time in seconds
            current_time = int(time.time())
            
            # Insert a new data point with the current time
            insert_query = "INSERT INTO data_table (timestamp, value) VALUES (?, ?)"
            cursor.execute(insert_query, (current_time, current_time))
            
            # Delete data older than 60 seconds
            delete_query = "DELETE FROM data_table WHERE timestamp < ?"
            threshold_time = current_time - 60
            cursor.execute(delete_query, (threshold_time,))
            
            # Commit the transaction
            db_connection.commit()
            
            # Wait for one second
            time.sleep(1)

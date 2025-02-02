import requests
import sys
from concurrent.futures import ThreadPoolExecutor
from collections import Counter
import time
import random

# Define a timeout value (in seconds)
REQUEST_TIMEOUT = 5
RUN_TIME = 30  # Time in seconds for GET and POST requests to run continuously

def send_request(line, stats):
    try:
        line = line.strip()
        if not line or line.startswith('#'):
            return  # Skip empty lines and comments

        # Parse the request
        parts = line.split(' ', 2)
        method = parts[0].upper()
        url = parts[1]
        data = parts[2] if len(parts) > 2 else None

        # Send the request
        if method == 'GET':
            response = requests.get(url, timeout=REQUEST_TIMEOUT)
        elif method == 'POST':
            response = requests.post(url, data=data, timeout=REQUEST_TIMEOUT)
        elif method == 'DELETE':
            response = requests.delete(url, timeout=REQUEST_TIMEOUT)
        else:
            stats['unsupported'] += 1
            print(f"[UNSUPPORTED] {method} {url}")
            return

        # Log the response
        if 200 <= response.status_code < 300:
            stats['success'] += 1
            print(f"[SUCCESS] {method} {url} - {response.status_code} {response.reason}")
        elif response.status_code >= 300:
            # Treat responses with status codes >= 300 as not failed
            print(f"[INFO] {method} {url} - {response.status_code} {response.reason} (non-failure status code)")
        else:
            stats['failed'] += 1
            print(f"[FAILED] {method} {url} - {response.status_code} {response.reason}")
    except requests.exceptions.Timeout:
        stats['failed'] += 1
        print(f"[TIMEOUT] {method} {url} - Request timed out")
    except Exception as e:
        stats['failed'] += 1
        print(f"[ERROR] {method} {url} - {str(e)}")

def send_requests_randomly(file_path, max_workers=10):
    stats = Counter(success=0, failed=0, unsupported=0, total=0)

    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()
            stats['total'] = len([line for line in lines if line.strip() and not line.startswith('#')])

        # Separate GET/POST and DELETE requests
        get_post_lines = [line for line in lines if line.strip() and not line.startswith('#') and line.split(' ')[0].upper() in ['GET', 'POST']]
        delete_lines = [line for line in lines if line.strip() and not line.startswith('#') and line.split(' ')[0].upper() == 'DELETE']

        # Start time to run the GET and POST requests randomly
        start_time = time.time()
        print(f"Running GET and POST requests randomly for {RUN_TIME} seconds...")

        # Send random GET and POST requests for the specified time (RUN_TIME)
        while time.time() - start_time < RUN_TIME:
            line = random.choice(get_post_lines)
            send_request(line, stats)
            time.sleep(random.uniform(0.1, 1))  # Random delay between requests (0.1s to 1s)

        # Wait for 1 second before sending DELETE requests
        print("\nWaiting for 1 second before sending DELETE requests...")
        time.sleep(1)

        # Process DELETE requests concurrently
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            executor.map(lambda line: send_request(line, stats), delete_lines)

        # Display summary
        print("\n=== Request Summary ===")
        print(f"Total Requests: {stats['total']}")
        print(f"Successful: {stats['success']}")
        print(f"Failed: {stats['failed']}")
        print(f"Unsupported Methods: {stats['unsupported']}")
    except FileNotFoundError:
        print(f"File not found: {file_path}")
    except Exception as e:
        print(f"An error occurred: {e}")

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python http_requester_with_timeout.py <file_path>")
        sys.exit(1)

    file_path = sys.argv[1]
    send_requests_randomly(file_path)

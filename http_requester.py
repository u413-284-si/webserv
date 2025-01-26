import requests
import sys
from concurrent.futures import ThreadPoolExecutor
from collections import Counter

# Define a timeout value (in seconds)
REQUEST_TIMEOUT = 5

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
            print(f"[SUCCESS] {method} {url} - {response.status_code}")
        else:
            stats['failed'] += 1
            print(f"[FAILED] {method} {url} - {response.status_code}")
    except requests.exceptions.Timeout:
        stats['failed'] += 1
        print(f"[TIMEOUT] {method} {url} - Request timed out")
    except Exception as e:
        stats['failed'] += 1
        print(f"[ERROR] {method} {url} - {str(e)}")

def send_requests_concurrently(file_path, max_workers=10):
    stats = Counter(success=0, failed=0, unsupported=0, total=0)

    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()
            stats['total'] = len([line for line in lines if line.strip() and not line.startswith('#')])

        # Process requests concurrently
        with ThreadPoolExecutor(max_workers=max_workers) as executor:
            executor.map(lambda line: send_request(line, stats), lines)

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
    send_requests_concurrently(file_path)

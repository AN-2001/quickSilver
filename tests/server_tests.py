import random
import socket
import threading
import json
import string

HOST = "127.0.0.1"
PORT = 8080

THREADS = 10
JOBS_PER_THREAD = 10


def random_label(length=8):
    return "".join(random.choices(string.ascii_lowercase, k=length))

def random_job():
    num_vertices = random.randint( 1, 100 )
    num_edges = random.randint( 1, 100 )

    adjacency = [[] for _ in range(num_vertices)]

    for _ in range(num_edges):
        u = random.randrange(num_vertices)
        v = random.randrange(num_vertices)

        adjacency[u].append(v)

    return {
        "jobType": "compute",
        "algorithm": random.choice(["BFS", "DFS"]),
        "graph": {
            "numVertices": num_vertices,
            "edges": adjacency,
            "labels": [
                random_label()
                for _ in range(num_vertices)
            ]
        },
        "input": [
            random.randrange(num_vertices)
        ]
    }


def worker(tid):
    for i in range(JOBS_PER_THREAD):
        job = json.dumps(random_job()).encode()

        try:
            with socket.create_connection((HOST, PORT)) as s:
                s.sendall(job)
                s.shutdown( socket.SHUT_WR )

                # Read until the server closes the connection.
                response = b""
                while True:
                    chunk = s.recv(4096)
                    if not chunk:
                        break
                    response += chunk

                responseDict = json.loads(response.decode("utf-8"))
                assert responseDict[ 'status' ] == 'ok'

        except Exception as e:
            print(f"[Thread {tid}] Job {i}: {e}")


def main():
    threads = []

    for tid in range(THREADS):
        t = threading.Thread(target=worker, args=(tid,))
        t.start()
        threads.append(t)

    for t in threads:
        t.join()

    metrics = { 'jobType': "metrics" }
    job = json.dumps(metrics).encode()
    responseDict = None

    with socket.create_connection((HOST, PORT)) as s:
        s.sendall(job)
        s.shutdown( socket.SHUT_WR )

        response = b""
        while True:
            chunk = s.recv( 4096 )
            if not chunk:
                break
            response += chunk

        responseDict = json.loads(response.decode("utf-8"))
    assert responseDict[ 'jobCount' ] == 100 

if __name__ == "__main__":
    main()

import random
import socket
import threading
import json

HOST = "127.0.0.1"
PORT = 8080

THREADS = 10
JOBS_PER_THREAD = 10


def random_graph():
    n = random.randint(5, 100)

    edges = set()

    # Make connected
    for i in range(1, n):
        edges.add((random.randint(0, i - 1), i))

    while len(edges) < random.randint(n, n * 3):
        u = random.randrange(n)
        v = random.randrange(n)
        if u != v:
            edges.add(tuple(sorted((u, v))))

    return {
        "numVertices": n,
        "edges": [list(e) for e in edges],
        "labels": [f"v{i}" for i in range(n)]
    }


def random_job():
    graph = random_graph()

    return {
        "jobType": "compute",
        "algorithm": random.choice(["BFS", "DFS"]),
        "graph": graph,
        "input": [random.randrange(graph["numVertices"])]
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

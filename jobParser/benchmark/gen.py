import random
import string
import json

def random_label(length=8):
    return "".join(random.choices(string.ascii_lowercase, k=length))

def generate_graph():
    num_vertices = random.randint(1000, 4000)
    num_edges = random.randint(1000, 4000)

    edges = set()

    while len(edges) < num_edges:
        u = random.randrange(num_vertices)
        v = random.randrange(num_vertices)

        if u == v:
            continue

        if u > v:
            u, v = v, u

        edges.add((u, v))

    return {
        "jobType": "compute",
        "algorithm": random.choice(["BFS", "DFS"]),
        "graph": {
            "numVertices": num_vertices,
            "edges": [list(e) for e in edges],
            "labels": [
                random_label()
                for _ in range(num_vertices)
            ]
        },
        "input": [
            random.randrange(num_vertices)
        ]
    }


def main():
    graph = generate_graph()
    print( json.dumps( graph ) )

if __name__ == '__main__':
    main()

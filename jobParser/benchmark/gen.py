import random
import string
import json

def random_label(length=8):
    return "".join(random.choices(string.ascii_lowercase, k=length))

def generate_graph():
    num_vertices = random.randint( 1000, 2000 )
    num_edges = random.randint( 1000, 2000 )

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

def main():
    graph = generate_graph()
    print( json.dumps( graph ) )

if __name__ == '__main__':
    main()

/* Generating random weighted undirected graphs */

#include<bits/stdc++.h>
using namespace std;

#define U first.first
#define V first.second
#define W_UV second
typedef pair<pair<int, int>, int> edge_type;

// approx size of the testcase (upto +-1)
const int maxn = 20;
const int r = 0;

int main(int argc, char **argv) {
	ofstream out;
	out.open(argv[1]);

	int undir = stoi(argv[2]);
	
	// Generating random values every time 
	srand(time(NULL));
	
	// #vertices, #edges 
	int n, m;

	// Finding random n 
	for (n = 0; n == 0;)
		n = rand() % maxn;
	// Finding random feasible m 
	int maxm = n * (n - 1) / 2;
	m = maxm ? rand() % maxm : 0;
	cout << "    n: " << n << " m: " << m << endl; 
	
	// Printing first line
	out << n << ' ' << m << ' ' << r << ' ' <<  undir << endl;
	
	// Adding edges 
	set<edge_type> edges;
	int edges_added = 0;
	while (edges_added < m) {
		int u = rand() % n;
		int v = rand() % n;
		int w_uv = 1 + rand() % n;
		if (!undir && u < v) swap(u, v);          /* to ensure undirected edges */
		
		edge_type edge = {{u, v}, w_uv};
		if (u == v || edges.find(edge) != edges.end())      /* dealing with previously found edge */
			continue;

		edges.insert(edge); ++edges_added;        /* inserting the edge */
	}
	
	// printing edges 
	for (auto edge : edges)
		out << edge.U << ' ' << edge.V << ' ' << edge.W_UV << endl;
	
	out.close();
	return(0);
}
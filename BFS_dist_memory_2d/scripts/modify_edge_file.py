import sys
import re
import random

def create_simplified_file(input_filename, output_filename, reverse_map_filename):
    '''
        Creates a simplified graph file
        Assume that lines starting with hashtag(#) are comments
        Creates a mapping between original vertices numbering and new vertex numbering
        Output format:

        n m r undirected
        u_1 v_1 uv_1
        ...
        u_m v_m uv_m

        Here
        n               :   Number of unique nodes in the graph
        m               :   Number of edges in the graph
        r               :   Source vertex

        Also, makes a reverse mapping file to get original indices later on
        Format:
        n
        u_1 v_1
        ...
        u_n v_n

        Here
        n               :   Number of nodes

        Our file maps v_i to u_i

        All spaces are from tabs
    '''
    n = 0
    m = 0
    undirected = 1
    root = -1
    node_map = {}
    delimiters = [",","\t","\s"]
    regexPattern = "|".join(delimiters)
    with open(input_filename, "r") as fi:
        #lines = fi.readlines()
        #for line in lines:
        #    if(line[0]=="#"):
        #        continue
        #    tokens = list(filter(lambda x: len(x)>0, re.split(regexPattern,line)))
        #    u = tokens[0]
        #    v = tokens[1]
        #    if u not in node_map:
        #        node_map[u] = n
        #        n = n + 1
        #    if v not in node_map:
        #        node_map[v] = n
        #        n = n + 1
        #    m += 1
        
        while True:
            line = fi.readline()
            if not line:
                break
            if(line[0]=="#"):
                continue
            tokens = list(filter(lambda x: len(x)>0, re.split(regexPattern,line)))
            u = tokens[0]
            v = tokens[1]
            if root==-1:
                root = u
            if u not in node_map:
                node_map[u] = n
                n = n + 1
            if v not in node_map:
                node_map[v] = n
                n = n + 1
            m += 1
    order = list(range(n))
    random.shuffle(order)
    node_map_reverse = {}
    for i,k in enumerate(node_map.keys()):
        node_map[k] = order[i]
        node_map_reverse[order[i]] = int(k)

    with open(reverse_map_filename, "w") as fo:
        fo.write("{:d}\n".format(n))
        for i in range(n):
            fo.write("{:d}\t{:d}\n".format(i,node_map_reverse[i]))

    with open(input_filename, "r") as fi:
        with open(output_filename, "w") as fo:
            fo.write("{:d}\t{:d}\t{:d}\t{:d}\n".format(n,m,node_map[str(root)],undirected))
            #lines = fi.readlines()
            #for line in lines:
            #    if(line[0]=="#"):
            #        continue
            #    tokens = list(filter(lambda x: len(x)>0, re.split(regexPattern,line)))
            #    u = node_map[tokens[0]]
            #    v = node_map[tokens[1]]
            #    weight = 1
            #    fo.write("{:d}\t{:d}\t{:d}\n".format(u,v,weight))
    
            while True:
                line = fi.readline()
                if not line:
                    break
                if(line[0]=="#"):
                    continue
                tokens = list(filter(lambda x: len(x)>0, re.split(regexPattern,line)))
                u = node_map[tokens[0]]
                v = node_map[tokens[1]]
                weight = 1
                fo.write("{:d}\t{:d}\t{:d}\n".format(u,v,weight))
 
 


if __name__=="__main__":
    #print("Starting script")
    if not(len(sys.argv)==5):
        print("Need 4 inputs")
        print("> Original file")
        print("> New file")
        print("> Reverse mapping file")
        print("> Undirected?(1 if yes and 0 otherwise)")
        exit(-1)
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    reverse_map_file = sys.argv[3]
    create_simplified_file(input_file, output_file, reverse_map_file)
    
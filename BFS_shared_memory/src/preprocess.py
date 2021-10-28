import sys
import re

def create_simplified_file(input_filename, output_filename, undirected):
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
    '''
    n = 0
    m = 0
    root = 0
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
            if u not in node_map:
                node_map[u] = n
                n = n + 1
            if v not in node_map:
                node_map[v] = n
                n = n + 1
            m += 1


    with open(input_filename, "r") as fi:
        with open(output_filename, "w") as fo:
            fo.write("{:d}\t{:d}\t{:d}\t{:d}\n".format(n,m,root,undirected))
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
    print("Starting script")
    if not(len(sys.argv)==4):
        print("Need 3 inputs")
        print("> Original file")
        print("> New file")
        print("> Undirected Flag")
        exit(0)
    input_file = sys.argv[1]
    output_file = sys.argv[2]
    undir = int(sys.argv[3])
    create_simplified_file(input_file, output_file, undir)
    

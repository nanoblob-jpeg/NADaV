# Network Alignment Data Visualization (NADaV)

This offers a quick way to see how two networks are aligned which has been made specifically to work with SANA (https://github.com/nmamano/SANA/tree/SANA2/src)
#### Requires the boost graph library which can be found: https://www.boost.org/doc/libs/1_74_0/libs/graph/doc/index.html
#### Uses the freetype library, glad, and glfw
### It expects 4 files, two of which have default values:
* two graphs, g and g2, where the g is bigger than g2, every node in g2 is aligned to a node in g, and either in edge list format or graphWin format.
* an align file, defaulted to sana.align, which has two nodes on each line separated by a space. The node on the left being from g2 and the node on the right being the aligned node in g.
* an edge aligned file, defaulted to sana.ccs-el, where each line has two sets of parenthesized values which each have two values separated by a comma. Each value left of the comma is the node from g2 while each value right of the comma is the node from g.
* You can see examples of these files in the example data folder

### Compiling
If you want to recompile this program for yourself, use the following gcc command or it's equivalent (this is a 32 bit program)
```
g++ -std=c++17 glad.o main.cpp Shader.cpp image.cpp -o NADV.exe -lglfw3 -lfreetype -I. -IPath/to/boost/library -I./freetype -L. -O2
```

### Using the Program
```
./NADV.exe -g nameOfGraphOne.gw -g2 nameOfGraphTwo.gw
```
or
```
./NADV.exe -g nameOfGraphOne.el -g2 nameOfGraphTwo.el
```
or any permutation thereof.

To view an example network alignment visualization, run the following command:
```
./NADV.exe -g ./exampleData/SPombe.el -g2 ./exampleData/RNorvegicus.el -a ./exampleData/sana.align -e ./exampleData/sana.ccs-el
```
There also additional arguments that can be specified:
* -i int : set's number of iterations for fruchterman reingold algorithm which spreads the points of the graph
* -a name.align : sets the align file for when it is not named sana.align or is not in same directory as this file
* -e name.ccs-el : sets the ccs-el file if not name sana.ccs-el or is not in same directory as this file
* -va float float : sets the color of aligned vertexes with {red, blue} from 0 to 255
* -vu float float : sets the color of unaligned vertexes with {red, blue} from 0 to 255
* -eu1 float float : sets the color of unaligned edges in graph g with {red, blue} from 0 to 255
* -eu2 float float : sets the color of unaligned edges in graph g2 with {red, blue} from 0 to 255
* -ea float float : sets the color of aligned edges with {red, blue} from 0 to 255

### Controls within NADaV
To move around the graph used the WASD keys. To zoom in use Q and to zoom out use E. There are buttons on the right which toggle which edges you see. The labels next to these buttons, so far, correlate to what the default colors are. You can select a node to isolate by left or right clicking on it. You can select multiple vertices by continuing to left click other vertices. If you left click a vertex again, it will deselect it. If you want to isolate a vertex when you have multiple vertexes selected, right click on the vertex. If you want to deselect all vertices, use the clear selection button in the top right.

### Console Input within NADaV
NADaV has a naive test input box in the lower left. There are a few things that you can do with this. You can turn induced subgraphing on/off. Induced subgraphing only happens when you have one or more nodes selected. You can also select specific nodes and specify a file which contains a list of nodes. Finally, you can clear selection by typing clear. A list of commands and usage appears below.
* file path/to/file : selects nodes specified in the file. File format follows with each line separated by commas: (g or g2 specifying which graph these nodes are a part of),(name or num specifying how you are specifying the nodes: ie by name or number), node, node, etc.
* induced (on/off) : turns induced subgraphing on or off
* nodeName : selects node with node name nodeName
* clear : clears selection

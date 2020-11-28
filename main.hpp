#define GLFW_DLL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>
#include <set>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/random_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topology.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/progress.hpp>
#include <boost/shared_ptr.hpp>

#include "Shader.hpp"
#include "stb_image.h"

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::no_property> UndirectedGraph;
typedef boost::rectangle_topology<> topology_type;
typedef topology_type::point_type point_type;
typedef std::vector<point_type> PositionVec;
typedef boost::iterator_property_map<PositionVec::iterator,
		boost::property_map<UndirectedGraph, boost::vertex_index_t>::type>
		PositionMap;

class progress_cooling : public boost::linear_cooling<double>
{
    typedef linear_cooling< double > inherited;

public:
    explicit progress_cooling(std::size_t iterations) : inherited(iterations)
    {
        display.reset(new boost::progress_display(iterations + 1, std::cerr));
    }

    double operator()()
    {
        ++(*display);
        return inherited::operator()();
    }

private:
    boost::shared_ptr< boost::progress_display > display;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, int key, int scancode, int action, int mode);
void charCallback(GLFWwindow *window, unsigned int codepoint);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void createBoostGraph_gw(UndirectedGraph &g, std::map<std::string, int> &name_to_num);
void createBoostGraph_el(UndirectedGraph &g, std::map<std::string, int> &name_to_num);
void readAlignedNodes(
	std::map<int, bool> &alignedNodes, 
	std::map<std::string, int> &name_to_num,
	std::map<std::string, int> &name_to_num_two);
void fruchtermanReingold(UndirectedGraph &g, 
	PositionMap &position, 
	std::vector<float> &unalignedVerticesList, 
	std::vector<float> &alignedVerticesList, 
	std::map<int, bool> &alignedNodes);
void createEdgeList_gw(std::vector<std::vector<int>> &graphTwoEdges, 
	std::map<std::string, int> &name_to_num_two, 
	std::map<int, std::string> &num_to_name);
void createEdgeList_el(std::vector<std::vector<int>> &graphTwoEdges,
	std::map<std::string, int> &name_to_num_two,
	std::map<int, std::string> &num_to_name);
void readAlignedEdges(
	UndirectedGraph &g, 
	std::vector<std::vector<int>> &graphTwoEdges, 
	PositionMap &position,
	std::vector<float> &alignedEdges, 
	std::vector<float> &alignedEdgeColor, 
	std::map<std::string, int> &name_to_num, 
	std::map<std::string, int> &name_to_num_two);
void getUnalignedEdgesInGraph(
	UndirectedGraph &g, 
	PositionMap &position, 
	std::vector<float> &unalignedEdgesGraphOne, 
	std::vector<float> &unalignedEdgeColorGraphOne);
void getUnaligendEdgesInEdgeGraph(
	std::vector<std::vector<int>> &graphTwoEdges, 
	PositionMap &position, 
	std::vector<float> &unalignedEdges, 
	std::vector<float> &unalignedEdgeColor, 
	std::map<std::string, int> &name_to_num,
	std::map<int, std::string> &num_to_name);
void setVertexColors(
	std::vector<float> &unalignedVertices, 
	std::vector<float> &alignedVertices, 
	std::vector<float> &unalignedVertexColor, 
	std::vector<float> &alignedVertexColor);
void printHelp();
void checkGraphSizeOrder();
unsigned int createVAO(std::vector<float> &list, int numElementsPerObject, std::vector<float> &color, int numElementsPerColor);
void verifyCommandLineInput(int argc, char *argv[]);
void registerMouseClick(bool leftClick);
std::vector<unsigned int> createGUIVAO(float topOffset, float bottomOffset, float leftOffset, float rightOffset);
void setGUIData(std::vector<unsigned int> input, float topOffset, float bottomOffset, float leftOffset, float rightOffset);
std::vector<unsigned int> createGuiBox(float xOffsetLeft, float xOffsetRight, float yOffsetTop, float yOffsetBottom, glm::vec3 color);
void setGUIBoxData(std::vector<unsigned int> input, float xOffsetLeft, float xOffsetRight, float yOffsetTop, float yOffsetBottom, glm::vec3 color);
int findSelectedVertex();
void deleteRemovedEdges(int vertex, std::set<std::pair<int, int>> &edges);
std::vector<unsigned int> createSelectedVertexVAO();
void setSelectedVertexVAOData(std::vector<unsigned int> &arr, std::set<std::pair<int, int>> &edges, glm::vec2 color);
void addSelectedVertex(int vertex);
void loadCharacters();
void RenderText(Shader &shader, std::string text, float x, float y, float scale, glm::vec3 color, unsigned int VAO, unsigned int VBO);
std::vector<unsigned int> createTextVAO();
void isolateEdgesFromFile();
void addSelectedInducedVertex(int vertex);
void parseTerminalInput();
void clearSelection();
void setSelectionData();
void setSelectedVertexPositionVAOData(std::vector<unsigned int> &arr);
unsigned int loadTexture(std::string path, bool alpha = false);
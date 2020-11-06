/*
TODO: 
- allow for script to find the files by commandline arguments
- code to make sure that the graphs are sorted by size
- add exception handling, expecially for the files

Details:
The graph created is 0 indexed but the edge list data is 1 indexed. I keep these two in their respective
formats and jsut subtract 1 whenever I am accessing graph data from edge list data.

*/
#define GLFW_DLL
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <sstream>

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


float deltaTime = 0.0f;
float lastFrame = 0.0f;
float positionDivider = 200.0f;
int iterations = 100;
glm::vec4 topologyBounds(-1600, -1200, 1600, 1200);

bool displayAlignedNodes = true;
bool displayUnalignedNodes = true;
bool displayAlignedEdges = true;
bool displayUnalignedEdgesGraphOne = true;
bool displayUnalignedEdgesGraphTwo = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window, int key, int scancode, int action, int mode);
unsigned int createVAO(std::vector<float> &list, int numElementsPerObject, std::vector<float> &color, int numElementsPerColor);

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); 
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

int main(){
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(800, 600, "SANA", NULL, NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetKeyCallback(window, processInput);
	if(!gladLoadGLLoader((GLADloadproc)(glfwGetProcAddress))){
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	//creating the shader program
	Shader nodeShaderProgram;
	nodeShaderProgram.compile("node.vert", "node.frag", "node.geo");
	Shader edgeShaderProgram;
	edgeShaderProgram.compile("edge.vert", "edge.frag", "edge.geo");

	//create graph
	std::ifstream fstream("SPombe.gw");
	std::map<std::string, int> name_to_num;
	std::string line;
	std::getline(fstream, line);
	std::getline(fstream, line);
	std::getline(fstream, line);
	std::getline(fstream, line);
	std::getline(fstream, line);
	int num_of_vertices = std::stoi(line);
	int num = num_of_vertices;
	std::cout << num_of_vertices << '\n';
	UndirectedGraph g(num);
	while(num--){
		std::getline(fstream, line);
		name_to_num[line.substr(2, line.length() - 4)] = num_of_vertices - num;
	}
	std::getline(fstream, line);
	num = std::stoi(line);
	while(num--){
		std::getline(fstream, line);
		std::stringstream ss;
		ss << line;
		int a, b;
		ss >> a >> b;
		boost::add_edge(a-1, b-1, g);
	}
	fstream.close();

	std::map<std::string, std::string> mapping;
  	std::map<int, bool> alignedNodes;
    fstream.open("sana.align");
    while(std::getline(fstream, line)){
    	std::stringstream ss;
    	ss << line;
    	std::string a, b;
    	ss >> a >> b;
    	mapping[a] = b;
    	alignedNodes[name_to_num[b]-1] = true;
    }
    fstream.close();

	
	PositionVec position_vec(num_vertices(g));
	PositionMap position(position_vec.begin(), get(boost::vertex_index, g));
	boost::minstd_rand gen;
	topology_type topo(gen, topologyBounds.x, topologyBounds.y, topologyBounds.z, topologyBounds.w);
	boost::random_graph_layout(g, position, topo);
	
	fruchterman_reingold_force_directed_layout(
		g, position, topo, cooling(progress_cooling(iterations)));
	boost::graph_traits< UndirectedGraph >::vertex_iterator vi, vi_end;
    
	std::vector<float> unalignedVerticesList;
	std::vector<float> alignedVerticesList;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
    {
    	if(alignedNodes[*vi]){
    	    alignedVerticesList.push_back((float)position[*vi][0]/positionDivider);
    	    alignedVerticesList.push_back((float)position[*vi][1]/positionDivider);
    	}else{
    		unalignedVerticesList.push_back((float)position[*vi][0]/positionDivider);
    		unalignedVerticesList.push_back((float)position[*vi][1]/positionDivider);
    	}
    }

    
    std::vector<std::vector<int>> graphTwoEdges;
    std::map<std::string, int> name_to_num_two;
    std::map<int, std::string> num_to_name;
    fstream.open("RNorvegicus.gw");
    std::getline(fstream, line);
    std::getline(fstream, line);
    std::getline(fstream, line);
    std::getline(fstream, line);
    std::getline(fstream, line);
    int num_of_vertices_2 = std::stoi(line);
    graphTwoEdges.resize(num_of_vertices_2 + 1, std::vector<int>());
    num = num_of_vertices_2;
    while(num--){
    	std::getline(fstream, line);
    	name_to_num_two[line.substr(2, line.length() - 4)] = num_of_vertices_2 - num;
    	num_to_name[num_of_vertices_2 - num] = line.substr(2, line.length() - 4);
    }
   	std::getline(fstream, line);
   	num = std::stoi(line);
   	while(num--){
   		std::getline(fstream, line);
   		std::stringstream ss;
   		ss << line;
   		int a, b;
   		ss >> a >> b;
   		graphTwoEdges[a].push_back(b);
   		graphTwoEdges[b].push_back(a);
   	}
    fstream.close();


    std::vector<float> unalignedVertexColor;
    std::vector<float> alignedVertexColor;
    for(int i{}; i < unalignedVerticesList.size()/2; ++i){
    	unalignedVertexColor.push_back(0.5f);
    	unalignedVertexColor.push_back(0.0f);
    }
    for(int i{}; i < alignedVerticesList.size()/2; ++i){
    	alignedVertexColor.push_back(0.0f);
    	alignedVertexColor.push_back(0.5f);
    }


    
    std::vector<float> alignedEdges;
	std::vector<float> alignedEdgeColor;
    fstream.open("sana.ccs-el");
    while(std::getline(fstream, line)){
    	std::stringstream ss;
    	ss << line;
    	std::string a, b, c, d;
    	ss >> a >> b;
    	a = a.substr(1, a.length() - 2);
    	b = b.substr(1, b.length() - 2);
    	auto it = std::find(a.begin(), a.end(), ',');
    	auto it2 = std::find(b.begin(), b.end(), ',');
    	c = a.substr(std::distance(a.begin(), it) + 1);
    	d = b.substr(std::distance(b.begin(), it2) + 1);
    	a = a.substr(0, std::distance(a.begin(), it));
    	b = b.substr(0, std::distance(b.begin(), it2));

    	alignedEdges.push_back(position[name_to_num[c]-1][0]/positionDivider);
    	alignedEdges.push_back(position[name_to_num[c]-1][1]/positionDivider);
    	alignedEdges.push_back(position[name_to_num[d]-1][0]/positionDivider);
    	alignedEdges.push_back(position[name_to_num[d]-1][1]/positionDivider);

    	alignedEdgeColor.push_back(0.8f);
    	alignedEdgeColor.push_back(0.8f);

    	boost::remove_edge(name_to_num[c]-1, name_to_num[d]-1, g);
    	int ia, ib;
    	ia = name_to_num_two[a];
    	ib = name_to_num_two[b];
    	graphTwoEdges[ia].erase(graphTwoEdges[ia].begin() + std::distance(graphTwoEdges[ia].begin(), std::find(graphTwoEdges[ia].begin(), graphTwoEdges[ia].end(), ib)));
    	graphTwoEdges[ib].erase(graphTwoEdges[ib].begin() + std::distance(graphTwoEdges[ib].begin(), std::find(graphTwoEdges[ib].begin(), graphTwoEdges[ib].end(), ia)));
    }
    fstream.close();

	std::vector<float> unalignedEdgesGraphOne;
	std::vector<float> unalignedEdgeColorGraphOne;
	auto ei = boost::edges(g);
	for(auto eit = ei.first; eit != ei.second; ++eit){
		int a, b;
		a = boost::source(*eit, g);
		b = boost::target(*eit, g);
		unalignedEdgesGraphOne.push_back(position[a][0]/positionDivider);
		unalignedEdgesGraphOne.push_back(position[a][1]/positionDivider);
		unalignedEdgesGraphOne.push_back(position[b][0]/positionDivider);
		unalignedEdgesGraphOne.push_back(position[b][1]/positionDivider);
		
		unalignedEdgeColorGraphOne.push_back(0.5f);
		unalignedEdgeColorGraphOne.push_back(0.0f);
	}

	std::vector<float> unalignedEdgesGraphTwo;
	std::vector<float> unalignedEdgeColorGraphTwo;
	for(int i{1}; i < graphTwoEdges.size(); ++i){
		if(graphTwoEdges[i].size() > 0){
			for(int j{}; j < graphTwoEdges[i].size(); ++j){
				unalignedEdgesGraphTwo.push_back(position[name_to_num[mapping[num_to_name[i]]]-1][0]/positionDivider);
				unalignedEdgesGraphTwo.push_back(position[name_to_num[mapping[num_to_name[i]]]-1][1]/positionDivider);
				unalignedEdgesGraphTwo.push_back(position[name_to_num[mapping[num_to_name[graphTwoEdges[i][j]]]]-1][0]/positionDivider);
				unalignedEdgesGraphTwo.push_back(position[name_to_num[mapping[num_to_name[graphTwoEdges[i][j]]]]-1][1]/positionDivider);
				unalignedEdgeColorGraphTwo.push_back(0.0f);
				unalignedEdgeColorGraphTwo.push_back(0.5f);
			}
		}
	}
	unsigned int unalignedVerticesVAO = createVAO(unalignedVerticesList, 2, unalignedVertexColor, 2);
	unsigned int alignedVerticesVAO = createVAO(alignedVerticesList, 2, alignedVertexColor, 2);
	unsigned int alignedEdgeVAO = createVAO(alignedEdges, 4, alignedEdgeColor, 2);
	unsigned int unalignedEdgesGraphOneVAO = createVAO(unalignedEdgesGraphOne, 4, unalignedEdgeColorGraphOne, 2);
	unsigned int unalignedEdgesGraphTwoVAO = createVAO(unalignedEdgesGraphTwo, 4, unalignedEdgeColorGraphTwo, 2);

	std::cout << glGetError() << std::endl;

	//set up camera
	glm::mat4 projection = glm::mat4(1.0f);
    //projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)(800.0/600.0), 0.1f, 100.0f);


	//render loop
	while(!glfwWindowShouldClose(window)){
		float currentFrame = glfwGetTime();
    	deltaTime = currentFrame - lastFrame;
    	lastFrame = currentFrame;
		glfwPollEvents();
		glm::mat4 view = glm::mat4(1.0f);
		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

    	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    	glClear(GL_COLOR_BUFFER_BIT);
    	//render here
    	
    	nodeShaderProgram.use();
    	nodeShaderProgram.setMat4("proj", projection);
    	nodeShaderProgram.setMat4("view", view);
    	if(displayUnalignedNodes){
			glBindVertexArray(unalignedVerticesVAO);
			glDrawArrays(GL_POINTS, 0, unalignedVerticesList.size()/2);
		}
		if(displayAlignedNodes){
			glBindVertexArray(alignedVerticesVAO);
			glDrawArrays(GL_POINTS, 0, alignedVerticesList.size()/2);
		}

    	edgeShaderProgram.use();
    	edgeShaderProgram.setMat4("proj", projection);
    	edgeShaderProgram.setMat4("view", view);
    	if(displayAlignedEdges){
			glBindVertexArray(alignedEdgeVAO);
    		glDrawArrays(GL_POINTS, 0, alignedEdges.size() / 4);
    	}
    	if(displayUnalignedEdgesGraphOne){
    		glBindVertexArray(unalignedEdgesGraphOneVAO);
    		glDrawArrays(GL_POINTS, 0, unalignedEdgesGraphOne.size() / 4);
    	}
    	if(displayUnalignedEdgesGraphTwo){
    		glBindVertexArray(unalignedEdgesGraphTwoVAO);
    		glDrawArrays(GL_POINTS, 0, unalignedEdgesGraphTwo.size() / 4);
    	}

    	glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window, int key, int scancode, int action, int mode){
	if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS){
		glfwSetWindowShouldClose(window, true);
	}
	if (glfwGetKey(window, GLFW_KEY_T) && action == GLFW_PRESS)
		std::cout << glGetError() << std::endl;
	const float cameraSpeed = 50.0f; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        cameraPos += cameraSpeed * cameraUp * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_S) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        cameraPos -= cameraSpeed * cameraUp * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_A) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaTime * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) && (action == GLFW_PRESS || action == GLFW_REPEAT))
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * deltaTime * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_E) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    	cameraPos += cameraSpeed * cameraFront * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_Q) && (action == GLFW_PRESS || action == GLFW_REPEAT))
    	cameraPos -= cameraSpeed * cameraFront * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_0) && action == GLFW_PRESS && action != GLFW_REPEAT)
    	displayAlignedNodes = !displayAlignedNodes;
    if (glfwGetKey(window, GLFW_KEY_9) && action == GLFW_PRESS && action != GLFW_REPEAT)
    	displayUnalignedNodes = !displayUnalignedNodes;
    if (glfwGetKey(window, GLFW_KEY_1) && action == GLFW_PRESS && action != GLFW_REPEAT)
    	displayUnalignedEdgesGraphOne = !displayUnalignedEdgesGraphOne;
    if (glfwGetKey(window, GLFW_KEY_2) && action == GLFW_PRESS && action != GLFW_REPEAT)
    	displayUnalignedEdgesGraphTwo = !displayUnalignedEdgesGraphTwo;
    if (glfwGetKey(window, GLFW_KEY_3) && action == GLFW_PRESS && action != GLFW_REPEAT)
    	displayAlignedEdges = !displayAlignedEdges;
}

unsigned int createVAO(std::vector<float> &list, int numElementsPerObject, std::vector<float> &color, int numElementsPerColor){
	unsigned int VAO, VBO, BVBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * list.size(), &list[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, numElementsPerObject, GL_FLOAT, GL_FALSE, numElementsPerObject * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &BVBO);
	glBindBuffer(GL_ARRAY_BUFFER, BVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * color.size(), &color[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, numElementsPerColor, GL_FLOAT, GL_FALSE, numElementsPerColor * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	return VAO;
}
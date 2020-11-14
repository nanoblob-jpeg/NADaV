/*
TODO: 
- later, move to java and integrate with cytoscope

Details:
Both graphs created are 0 indexed
vertexes when selecting are in terms of the boost graph numbering

*/
#include "main.hpp"

float deltaTime = 0.0f;
float lastFrame = 0.0f;
float positionDivider = 200.0f;
int iterations = 100;
int WIDTH = 800;
int HEIGHT = 600;
glm::vec2 mouse_position(0.0, 0.0);
glm::vec4 topologyBounds(-1600, -1200, 1600, 1200);

bool displayAlignedNodes = true;
bool displayUnalignedNodes = true;
bool displayAlignedEdges = true;
bool displayUnalignedEdgesGraphOne = true;
bool displayUnalignedEdgesGraphTwo = true;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f); 
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, glm::vec3(0.0f, 1.0f, 0.0f)));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

glm::mat4 projection = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);

std::vector<unsigned int> guiData;
std::vector<unsigned int> clearSelectionGuiData;
std::vector<unsigned int> box1;
std::vector<unsigned int> box2;
std::vector<unsigned int> box3;

PositionMap position;
UndirectedGraph g;
std::vector<std::vector<int>> graphTwoEdges;
std::vector<std::vector<int>> alignedEdgeList;
std::map<int, int> int_map_one_to_two;
std::map<int, int> int_map_two_to_one;
bool didSelectVertices = false;
std::set<int> selectedVertices;
std::set<std::pair<int, int>> selectedVertexEdgesAligned;
std::set<std::pair<int, int>> selectedVertexEdgeUnalignedOne;
std::set<std::pair<int, int>> selectedVertexEdgeUnalignedTwo;
std::vector<unsigned int> selectedVertexEdgesAlignedVAO;
std::vector<unsigned int> selectedVertexEdgeUnalignedOneVAO;
std::vector<unsigned int> selectedVertexEdgeUnalignedTwoVAO;

std::string graphOne;
std::string graphTwo;
std::string align = "sana.align";
std::string edgeAlign = "sana.ccs-el";

std::pair<float, float> vertexColorAligned{0.8f, 0.8f};
std::pair<float, float> vertexColorUnaligned{0.5f, 0.0f};
std::pair<float, float> EdgeOneColorUnaligned{0.5f, 0.0f};
std::pair<float, float> EdgeTwoColorUnaligned{0.0f, 0.5f};
std::pair<float, float> EdgeColorAligned{0.8f, 0.8f};

int main(int argc, char *argv[]){
	stbi_set_flip_vertically_on_load(true);
	verifyCommandLineInput(argc, argv);

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "SANA", NULL, NULL);
	if(window == NULL){
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
	Shader staticTextureProgram;
	staticTextureProgram.compile("staticTexture.vert", "staticTexture.frag", nullptr);
	Shader staticProgram;
	staticProgram.compile("static.vert", "static.frag", nullptr);

	//create graph
	std::map<std::string, int> name_to_num;
	if(graphOne.length() <= 3){
		std::cout << "Invalid graph file\n";
		exit(1);
	}
	if(graphOne.substr(graphOne.length() - 3).compare(".gw") == 0)
		createBoostGraph_gw(g, name_to_num);
	else if(graphOne.substr(graphOne.length() - 3).compare(".el") == 0)
		createBoostGraph_el(g, name_to_num);
	else{
		std::cout << "Invalid graph format\n";
		exit(1);
	}

	std::map<std::string, int> name_to_num_two;
    std::map<int, std::string> num_to_name;
    if(graphTwo.length() <= 3){
    	std::cout << "Invalid graph file\n";
    	exit(1);
    }
    if(graphTwo.substr(graphTwo.length() - 3).compare(".gw") == 0)
		createEdgeList_gw(graphTwoEdges, name_to_num_two, num_to_name);
	else if(graphTwo.substr(graphTwo.length() - 3).compare(".el") == 0)
		createEdgeList_el(graphTwoEdges, name_to_num_two, num_to_name);
	else{
		std::cout << "Invalid graph format\n";
		exit(1);
	}

  	std::map<int, bool> alignedNodes;
    readAlignedNodes(alignedNodes, name_to_num, name_to_num_two);

    std::vector<float> unalignedVerticesList;
	std::vector<float> alignedVerticesList;
	PositionVec position_vec(num_vertices(g));
	position = PositionMap(position_vec.begin(), get(boost::vertex_index, g));
	fruchtermanReingold(g, position, unalignedVerticesList, alignedVerticesList, alignedNodes);

    std::vector<float> unalignedVertexColor;
    std::vector<float> alignedVertexColor;
    setVertexColors(unalignedVerticesList, alignedVerticesList, unalignedVertexColor, alignedVertexColor);

    std::vector<float> alignedEdges;
	std::vector<float> alignedEdgeColor;
	readAlignedEdges(g, graphTwoEdges, position, alignedEdges, alignedEdgeColor, name_to_num, name_to_num_two);

	std::vector<float> unalignedEdgesGraphOne;
	std::vector<float> unalignedEdgeColorGraphOne;
	getUnalignedEdgesInGraph(g, position, unalignedEdgesGraphOne, unalignedEdgeColorGraphOne);

	std::vector<float> unalignedEdgesGraphTwo;
	std::vector<float> unalignedEdgeColorGraphTwo;
	getUnaligendEdgesInEdgeGraph(graphTwoEdges, position, unalignedEdgesGraphTwo, unalignedEdgeColorGraphTwo, name_to_num, num_to_name);
	
	unsigned int unalignedVerticesVAO = createVAO(unalignedVerticesList, 2, unalignedVertexColor, 2);
	unsigned int alignedVerticesVAO = createVAO(alignedVerticesList, 2, alignedVertexColor, 2);
	unsigned int alignedEdgeVAO = createVAO(alignedEdges, 4, alignedEdgeColor, 2);
	unsigned int unalignedEdgesGraphOneVAO = createVAO(unalignedEdgesGraphOne, 4, unalignedEdgeColorGraphOne, 2);
	unsigned int unalignedEdgesGraphTwoVAO = createVAO(unalignedEdgesGraphTwo, 4, unalignedEdgeColorGraphTwo, 2);

	//gui stuff
	unsigned int texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int guiw, guih, nrChannels;
	unsigned char *data = stbi_load("gui.png", &guiw, &guih, &nrChannels, 0);
	if (data)
	{
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, guiw, guih, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	    glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
	    std::cout << "Could not find gui.png texture" << std::endl;
	    exit(1);
	}
	stbi_image_free(data);

	unsigned int texture2;
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	data = stbi_load("clearSelectionButton.png", &guiw, &guih, &nrChannels, 0);
	if (data)
	{
	    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, guiw, guih, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	    glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
	    std::cout << "Could not find gui.png texture" << std::endl;
	    exit(1);
	}
	stbi_image_free(data);

	guiData = createGUIVAO(0.0f, 200.0f, 200.0f, 0.0f);
	clearSelectionGuiData = createGUIVAO(210.0f, 300.0f, 200.0f, 0.0f);
	box1 = createGuiBox(175, 150, 25, 50);
	box2 = createGuiBox(175, 150, 88, 113);
	box3 = createGuiBox(175, 150, 150, 175);

	selectedVertexEdgesAlignedVAO = createSelectedVertexVAO();
	selectedVertexEdgeUnalignedOneVAO = createSelectedVertexVAO();
	selectedVertexEdgeUnalignedTwoVAO = createSelectedVertexVAO();

    //projection = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -10.0f, 10.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)(800.0/600.0), 0.1f, 100.0f);

	//render loop
	while(!glfwWindowShouldClose(window)){
		float currentFrame = glfwGetTime();
    	deltaTime = currentFrame - lastFrame;
    	lastFrame = currentFrame;
		glfwPollEvents();
		view = glm::mat4(1.0f);
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
    	if(!didSelectVertices){
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
    	}else{
    		if(displayAlignedEdges){
    			glBindVertexArray(selectedVertexEdgesAlignedVAO[0]);
    			glDrawArrays(GL_POINTS, 0, selectedVertexEdgesAligned.size());
    		}
    		if(displayUnalignedEdgesGraphOne){
    			glBindVertexArray(selectedVertexEdgeUnalignedOneVAO[0]);
    			glDrawArrays(GL_POINTS, 0, selectedVertexEdgeUnalignedOne.size());
    		}
    		if(displayUnalignedEdgesGraphTwo){
    			glBindVertexArray(selectedVertexEdgeUnalignedTwoVAO[0]);
    			glDrawArrays(GL_POINTS, 0, selectedVertexEdgeUnalignedTwo.size());
    		}
    	}

    	glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
    	staticTextureProgram.use();
    	glBindVertexArray(guiData[0]);
    	glDrawArrays(GL_TRIANGLES, 0, 6);
    	if(didSelectVertices){
    		glBindTexture(GL_TEXTURE_2D, texture2);
    		glBindVertexArray(clearSelectionGuiData[0]);
    		glDrawArrays(GL_TRIANGLES, 0, 6);
    	}

    	staticProgram.use();
    	if(displayAlignedEdges){
			glBindVertexArray(box3[0]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
    	}
    	if(displayUnalignedEdgesGraphOne){
    		glBindVertexArray(box1[0]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
    	}
    	if(displayUnalignedEdgesGraphTwo){
    		glBindVertexArray(box2[0]);
			glDrawArrays(GL_TRIANGLES, 0, 6);
    	}

    	glfwSwapBuffers(window);
	}
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	glViewport(0, 0, width, height);
	glfwGetWindowSize(window, &WIDTH, &HEIGHT);
	guiData = createGUIVAO(0.0f, 200.0f, 200.0f, 0.0f);
	clearSelectionGuiData = createGUIVAO(210.0f, 300.0f, 200.0f, 0.0f);
	setGUIBoxData(box1,175, 150, 25, 50);
	setGUIBoxData(box2,175, 150, 88, 113);
	setGUIBoxData(box3,175, 150, 150, 175);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	mouse_position.x = xpos;
	mouse_position.y = ypos;
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
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
	if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
    	registerMouseClick(true);
    }
    if(button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE){
    	registerMouseClick(false);
    }
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

void createBoostGraph_gw(UndirectedGraph &g, std::map<std::string, int> &name_to_num){
	try{
		std::ifstream fstream(graphOne.c_str());
		std::string line;
		std::getline(fstream, line);
		std::getline(fstream, line);
		std::getline(fstream, line);
		std::getline(fstream, line);
		std::getline(fstream, line);
		int num_of_vertices = std::stoi(line);
		alignedEdgeList.resize(num_of_vertices);
		int num = num_of_vertices;
		while(num--){
			boost::add_vertex(g);
			std::getline(fstream, line);
			name_to_num[line.substr(2, line.length() - 4)] = num_of_vertices - num - 1;
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
	}catch(...){
		std::cout << "Error reading graph one .gw file format\n";
		exit(1);
	}
}

void createBoostGraph_el(UndirectedGraph &g, std::map<std::string, int> &name_to_num){
	try{
		std::ifstream fstream(graphOne.c_str());
		std::string line;
		int counter{0};
		std::vector<int> edge_holder;
		while(std::getline(fstream, line)){
			std::stringstream ss;
			ss << line;
			std::string a, b;
			ss >> a >> b;
			if(name_to_num.count(a) != 1){
				name_to_num[a] = counter++;
				boost::add_vertex(g);
			}
			if(name_to_num.count(b) != 1){
				name_to_num[b] = counter++;
				boost::add_vertex(g);
			}
			edge_holder.push_back(name_to_num[a]);
			edge_holder.push_back(name_to_num[b]);
		}
		fstream.close();
		alignedEdgeList.resize(counter);
		for(int i{}; i < edge_holder.size(); i += 2){
			boost::add_edge(edge_holder[i], edge_holder[i+1], g);
		}
	}catch(...){
		std::cout << "Error reading graph one .el file format\n";
		exit(1);
	}
}

void readAlignedNodes(std::map<int, bool> &alignedNodes, std::map<std::string, int> &name_to_num, std::map<std::string, int> &name_to_num_two){
	try{
		std::ifstream fstream(align.c_str());
		std::string line;
	    while(std::getline(fstream, line)){
	    	std::stringstream ss;
	    	ss << line;
	    	std::string a, b;
	    	ss >> a >> b;
	    	int_map_one_to_two[name_to_num[b]] = name_to_num_two[a];
	    	int_map_two_to_one[name_to_num_two[a]] = name_to_num[b];
	    	alignedNodes[name_to_num[b]] = true;
	    }
	    fstream.close();
	}catch(...){
		std::cout << "Error reading align file\n";
		exit(1);
	}
}

void fruchtermanReingold(UndirectedGraph &g, 
	PositionMap &position, 
	std::vector<float> &unalignedVerticesList, 
	std::vector<float> &alignedVerticesList, 
	std::map<int, bool> &alignedNodes){
	boost::minstd_rand gen;
	topology_type topo(gen, topologyBounds.x, topologyBounds.y, topologyBounds.z, topologyBounds.w);
	boost::random_graph_layout(g, position, topo);
	fruchterman_reingold_force_directed_layout(
		g, position, topo, cooling(progress_cooling(iterations)));
	boost::graph_traits< UndirectedGraph >::vertex_iterator vi, vi_end;
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
}

void createEdgeList_gw(std::vector<std::vector<int>> &graphTwoEdges, 
	std::map<std::string, int> &name_to_num_two, 
	std::map<int, std::string> &num_to_name){
	try{
		std::ifstream fstream(graphTwo.c_str());
		std::string line;
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    int num_of_vertices_2 = std::stoi(line);
	    graphTwoEdges.resize(num_of_vertices_2, std::vector<int>());
	    int num = num_of_vertices_2;
	    while(num--){
	    	std::getline(fstream, line);
	    	name_to_num_two[line.substr(2, line.length() - 4)] = num_of_vertices_2 - num - 1;
	    	num_to_name[num_of_vertices_2 - num - 1] = line.substr(2, line.length() - 4);
	    }
	   	std::getline(fstream, line);
	   	num = std::stoi(line);
	   	while(num--){
	   		std::getline(fstream, line);
	   		std::stringstream ss;
	   		ss << line;
	   		int a, b;
	   		ss >> a >> b;
	   		graphTwoEdges[a-1].push_back(b-1);
	   		graphTwoEdges[b-1].push_back(a-1);
	   	}
	    fstream.close();
	}catch(...){
		std::cout << "Error reading graph two .gw file format\n";
		exit(1);
	}
}

void createEdgeList_el(std::vector<std::vector<int>> &graphTwoEdges,
	std::map<std::string, int> &name_to_num_two,
	std::map<int, std::string> &num_to_name){
	try{
		std::vector<int> edge_holder;
		std::ifstream fstream(graphTwo.c_str());
		std::string line;
		int counter{};
		while(std::getline(fstream, line)){
			std::stringstream ss;
			ss << line;
			std::string a, b;
			ss >> a >> b;
			if(name_to_num_two.count(a) != 1){
				name_to_num_two[a] = counter++;
				num_to_name[counter-1] = a;
			}
			if(name_to_num_two.count(b) != 1){
				name_to_num_two[b] = counter++;
				num_to_name[counter-1] = b;
			}
			edge_holder.push_back(name_to_num_two[a]);
			edge_holder.push_back(name_to_num_two[b]);
		}
		fstream.close();
		graphTwoEdges.resize(counter, std::vector<int>());
		for(int i{}; i < edge_holder.size(); i += 2){
			graphTwoEdges[edge_holder[i]].push_back(edge_holder[i+1]);
			graphTwoEdges[edge_holder[i+1]].push_back(edge_holder[i]);
		}
	}catch(...){
		std::cout << "Error reading graph two .el file format\n";
		exit(1);
	}
}

void readAlignedEdges(UndirectedGraph &g, 
	std::vector<std::vector<int>> &graphTwoEdges, 
	PositionMap &position,
	std::vector<float> &alignedEdges, 
	std::vector<float> &alignedEdgeColor, 
	std::map<std::string, int> &name_to_num, 
	std::map<std::string, int> &name_to_num_two){
	try{
		std::ifstream fstream(edgeAlign.c_str());
		std::string line;
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
	
			alignedEdgeList[name_to_num[c]].push_back(name_to_num[d]);
			alignedEdgeList[name_to_num[d]].push_back(name_to_num[c]);
	    	alignedEdges.push_back(position[name_to_num[c]][0]/positionDivider);
	    	alignedEdges.push_back(position[name_to_num[c]][1]/positionDivider);
	    	alignedEdges.push_back(position[name_to_num[d]][0]/positionDivider);
	    	alignedEdges.push_back(position[name_to_num[d]][1]/positionDivider);
	
	    	alignedEdgeColor.push_back(EdgeColorAligned.first);
	    	alignedEdgeColor.push_back(EdgeColorAligned.second);
	
	    	boost::remove_edge(name_to_num[c], name_to_num[d], g);
	    	int ia, ib;
	    	ia = name_to_num_two[a];
	    	ib = name_to_num_two[b];
	    	graphTwoEdges[ia].erase(graphTwoEdges[ia].begin() + std::distance(graphTwoEdges[ia].begin(), 
	    		std::find(graphTwoEdges[ia].begin(), graphTwoEdges[ia].end(), ib)));
	    	graphTwoEdges[ib].erase(graphTwoEdges[ib].begin() + std::distance(graphTwoEdges[ib].begin(), 
	    		std::find(graphTwoEdges[ib].begin(), graphTwoEdges[ib].end(), ia)));
	    }
	    fstream.close();
	}catch(...){
		std::cout << "Error reading edge align file\n";
		exit(1);
	}
}
void getUnalignedEdgesInGraph(UndirectedGraph &g, PositionMap &position, std::vector<float> &unalignedEdgesGraphOne, std::vector<float> &unalignedEdgeColorGraphOne){
	auto ei = boost::edges(g);
	for(auto eit = ei.first; eit != ei.second; ++eit){
		int a, b;
		a = boost::source(*eit, g);
		b = boost::target(*eit, g);
		unalignedEdgesGraphOne.push_back(position[a][0]/positionDivider);
		unalignedEdgesGraphOne.push_back(position[a][1]/positionDivider);
		unalignedEdgesGraphOne.push_back(position[b][0]/positionDivider);
		unalignedEdgesGraphOne.push_back(position[b][1]/positionDivider);
		
		unalignedEdgeColorGraphOne.push_back(EdgeOneColorUnaligned.first);
		unalignedEdgeColorGraphOne.push_back(EdgeOneColorUnaligned.second);
	}
}

void getUnaligendEdgesInEdgeGraph(std::vector<std::vector<int>> &graphTwoEdges, 
	PositionMap &position, 
	std::vector<float> &unalignedEdges, 
	std::vector<float> &unalignedEdgeColor, 
	std::map<std::string, int> &name_to_num,
	std::map<int, std::string> &num_to_name){
	for(int i{}; i < graphTwoEdges.size(); ++i){
		if(graphTwoEdges[i].size() > 0){
			for(int j{}; j < graphTwoEdges[i].size(); ++j){
				unalignedEdges.push_back(position[int_map_two_to_one[i]][0]/positionDivider);
				unalignedEdges.push_back(position[int_map_two_to_one[i]][1]/positionDivider);
				unalignedEdges.push_back(position[int_map_two_to_one[graphTwoEdges[i][j]]][0]/positionDivider);
				unalignedEdges.push_back(position[int_map_two_to_one[graphTwoEdges[i][j]]][1]/positionDivider);
				unalignedEdgeColor.push_back(EdgeTwoColorUnaligned.first);
				unalignedEdgeColor.push_back(EdgeTwoColorUnaligned.second);
			}
		}
	}
}

void setVertexColors(std::vector<float> &unalignedVertices, std::vector<float> &alignedVertices, std::vector<float> &unalignedVertexColor, std::vector<float> &alignedVertexColor){
	for(int i{}; i < unalignedVertices.size()/2; ++i){
    	unalignedVertexColor.push_back(vertexColorUnaligned.first);
    	unalignedVertexColor.push_back(vertexColorUnaligned.second);
    }
    for(int i{}; i < alignedVertices.size()/2; ++i){
    	alignedVertexColor.push_back(vertexColorAligned.first);
    	alignedVertexColor.push_back(vertexColorAligned.second);
    }
}

void printHelp(){
	std::cout << 
"./visualize -g name -g2 name_2 [-a name.align] [-e name.ccs-el] [-i int] [-vu float float] [-va float float] [-eu1 float float] [-eu2 float float] [-ea float float]\n";
	std::cout <<
"color is specified using red, blue\n";
}

int getGraphSizegw(std::string graph){
	try{
		std::ifstream fstream(graph.c_str());
		std::string line;
		std::getline(fstream, line);
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    std::getline(fstream, line);
	    int a = std::stoi(line);
	    fstream.close();
	    return a;
	}catch(...){
		std::cout << "Graph file could not be read\n";
		exit(1);
	}
}

int getGraphSizeel(std::string graph){
	try{
		std::ifstream fstream(graph.c_str());
		std::string line;
		std::set<std::string> counter;
		while(std::getline(fstream, line)){
			std::stringstream ss;
			ss << line;
			std::string a, b;
			ss >> a >> b;
			if(counter.count(a) != 1){
				counter.insert(a);
			}
			if(counter.count(b) != 1){
				counter.insert(b);
			}
		}
		return counter.size();
	}catch(...){
		std::cout << "Graph file could not be read\n";
		exit(1);
	}
}

void verifyCommandLineInput(int argc, char *argv[]){
	for(int i{1}; i < argc; ++i){
		if(argv[i][0] != '-'){
			printHelp();
			exit(1);
		}else{
			if(strcmp(argv[i], "-g") == 0){
				if(i+1 == argc){
					printHelp();
					exit(1);
				}
				graphOne = argv[++i];
			}else if(strcmp(argv[i], "-g2") == 0){
				if(i+1 == argc){
					printHelp();
					exit(1);
				}
				graphTwo = argv[++i];
			}else if(strcmp(argv[i], "-a") == 0){
				if(i+1 == argc){
					printHelp();
					exit(1);
				}
				align = argv[++i];
			}else if(strcmp(argv[i], "-e") == 0){
				if(i+1 == argc){
					printHelp();
					exit(1);
				}
				edgeAlign = argv[++i];
			}else if(strcmp(argv[i], "-i") == 0){
				if(i+1 == argc){
					printHelp();
					exit(1);
				}
				iterations = std::stoi(argv[++i]);
			}
			else if(strcmp(argv[i], "-va") == 0){
				if(i+2 == argc){
					printHelp();
					exit(1);
				}
				vertexColorAligned.first = std::stof(argv[++i])/255.0f;
				vertexColorAligned.second = std::stof(argv[++i])/255.0f;
			}else if(strcmp(argv[i], "-vu") == 0){
				if(i+2 == argc){
					printHelp();
					exit(1);
				}
				vertexColorUnaligned.first = std::stof(argv[++i])/255.0f;
				vertexColorUnaligned.second = std::stof(argv[++i])/255.0f;
			}else if(strcmp(argv[i], "-eu1") == 0){
				if(i+2 == argc){
					printHelp();
					exit(1);
				}
				EdgeOneColorUnaligned.first = std::stof(argv[++i])/255.0f;
				EdgeOneColorUnaligned.second = std::stof(argv[++i])/255.0f;
			}else if(strcmp(argv[i], "-eu2") == 0){
				if(i+2 == argc){
					printHelp();
					exit(1);
				}
				EdgeTwoColorUnaligned.first = std::stof(argv[++i])/255.0f;
				EdgeTwoColorUnaligned.second = std::stof(argv[++i])/255.0f;
			}else if(strcmp(argv[i], "-ea") == 0){
				if(i+2 == argc){
					printHelp();
					exit(1);
				}
				EdgeColorAligned.first = std::stof(argv[++i])/255.0f;
				EdgeColorAligned.second = std::stof(argv[++i])/255.0f;
			}else{
				printHelp();
				exit(1);
			}
			
		}
	}
	if(graphOne == "" || graphTwo == "" || align == "" || edgeAlign == ""){
		printHelp();
		exit(1);
	}
	int a, b;
	if(graphOne.substr(graphOne.length() - 3).compare(".gw") == 0)
		a = getGraphSizegw(graphOne);
	if(graphTwo.substr(graphTwo.length() - 3).compare(".gw") == 0)
		b = getGraphSizegw(graphTwo);
	if(graphOne.substr(graphOne.length() - 3).compare(".el") == 0)
		a = getGraphSizeel(graphOne);
	if(graphTwo.substr(graphTwo.length() - 3).compare(".el") == 0)
		b = getGraphSizeel(graphTwo);
	if(a < b){
		std::cout << "Please switch order of graph";
		exit(1);
	}
}

void registerMouseClick(bool leftClick){
	if(leftClick){
		if(mouse_position.x >= 700 && mouse_position.y <= 100){
			if(mouse_position.x >= WIDTH - 180/2 && mouse_position.x <= WIDTH - 145/2){
				if(mouse_position.y <= 55/2 && mouse_position.y >= 20/2)
					displayUnalignedEdgesGraphOne = !displayUnalignedEdgesGraphOne;
				else if(mouse_position.y <= 118/2  && mouse_position.y >= 83/2)
					displayUnalignedEdgesGraphTwo = !displayUnalignedEdgesGraphTwo;
				else if(mouse_position.y <= 180/2 && mouse_position.y >= 145/2)
					displayAlignedEdges = !displayAlignedEdges;
			}
		}else if(didSelectVertices && mouse_position.x >= 700 && mouse_position.y <= 150){
			didSelectVertices = false;
			selectedVertices.clear();
			selectedVertexEdgesAligned.clear();
			selectedVertexEdgeUnalignedOne.clear();
			selectedVertexEdgeUnalignedTwo.clear();
		}else{
			int vertex = findSelectedVertex();
			if(vertex == -1)
				return;
			if(didSelectVertices){
				if(selectedVertices.count(vertex) == 1){
					selectedVertices.erase(vertex);
					if(selectedVertices.empty()){
						selectedVertexEdgesAligned.clear();
						selectedVertexEdgeUnalignedOne.clear();
						selectedVertexEdgeUnalignedTwo.clear();
						didSelectVertices = false;
					}else{
						deleteRemovedEdges(vertex, selectedVertexEdgesAligned);
						deleteRemovedEdges(vertex, selectedVertexEdgeUnalignedOne);
						deleteRemovedEdges(vertex, selectedVertexEdgeUnalignedTwo);
					}
				}else{
					addSelectedVertex(vertex);
				}
			}else{
				//only add edges that are {selected vertex, other vertex}
				//don't need to also add the inverse but do need to check if
				//inverse is in there to not duplicate edges and need to check
				//this when deleting edges too
				didSelectVertices = true;
				addSelectedVertex(vertex);
			}
			if(didSelectVertices){
				//create the VAO stuff here
				setSelectedVertexVAOData(selectedVertexEdgesAlignedVAO, selectedVertexEdgesAligned, glm::vec2(EdgeColorAligned.first, EdgeColorAligned.second));
				setSelectedVertexVAOData(selectedVertexEdgeUnalignedOneVAO, selectedVertexEdgeUnalignedOne, glm::vec2(EdgeOneColorUnaligned.first, EdgeOneColorUnaligned.second));
				setSelectedVertexVAOData(selectedVertexEdgeUnalignedTwoVAO, selectedVertexEdgeUnalignedTwo, glm::vec2(EdgeTwoColorUnaligned.first, EdgeTwoColorUnaligned.second));
			}
		}
	}
}

std::vector<unsigned int> createGUIVAO(float topOffset, float bottomOffset, float leftOffset, float rightOffset){
	//all offsets from top right corner
	unsigned int VAO, VBO, BVBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &BVBO);

	setGUIData({VAO, VBO, BVBO}, topOffset, bottomOffset, leftOffset, rightOffset);
	return {VAO, VBO, BVBO};
}

void setGUIData(std::vector<unsigned int> input, float topOffset, float bottomOffset, float leftOffset, float rightOffset){
	unsigned int VAO = input[0];
	unsigned int VBO = input[1];
	unsigned int BVBO = input[2];

	std::vector<float> coords{
		1.0f - leftOffset/WIDTH, 1.0f - topOffset/HEIGHT,
		1.0f - leftOffset/WIDTH, 1.0f - bottomOffset/HEIGHT, 
		1.0f - rightOffset/WIDTH, 1.0f - topOffset/HEIGHT,
		1.0f - rightOffset/WIDTH, 1.0f - topOffset/HEIGHT,
		1.0f - rightOffset/WIDTH, 1.0f - bottomOffset/HEIGHT, 
		1.0f - leftOffset/WIDTH, 1.0f - bottomOffset/HEIGHT
	};
	std::vector<float> colors{
		0.827f, 0.827f, 0.827f,0.0f, 1.0f,
		0.827f, 0.827f, 0.827f,0.0f, 0.0f,
		0.827f, 0.827f, 0.827f,1.0f, 1.0f,
		0.827f, 0.827f, 0.827f,1.0f, 1.0f,
		0.827f, 0.827f, 0.827f,1.0, 0.0f,
		0.827f, 0.827f, 0.827f,0.0f, 0.0f
	};

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * coords.size(), &coords[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, BVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colors.size(), &colors[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
}

std::vector<unsigned int> createGuiBox(float xOffsetLeft, float xOffsetRight, float yOffsetTop, float yOffsetBottom){
	/*
	the x offset and y offset is from the offset from top right corner
	*/
	
	unsigned VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	setGUIBoxData({VAO, VBO}, xOffsetLeft, xOffsetRight, yOffsetTop, yOffsetBottom);
	return {VAO, VBO};
}

void setGUIBoxData(std::vector<unsigned int> input, float xOffsetLeft, float xOffsetRight, float yOffsetTop, float yOffsetBottom){
	std::vector<float> data{
		1.0f - xOffsetLeft/WIDTH, 1.0f - yOffsetTop/HEIGHT, 0.2f, 0.6f, 1.0f,
		1.0f - xOffsetLeft/WIDTH, 1.0f - yOffsetBottom/HEIGHT,  0.2f, 0.6f, 1.0f,
		1.0f - xOffsetRight/WIDTH, 1.0f - yOffsetTop/HEIGHT, 0.2f, 0.6f, 1.0f,
		1.0f - xOffsetRight/WIDTH, 1.0f - yOffsetTop/HEIGHT, 0.2f, 0.6f, 1.0f,
		1.0f - xOffsetRight/WIDTH, 1.0f - yOffsetBottom/HEIGHT,  0.2f, 0.6f, 1.0f,
		1.0f - xOffsetLeft/WIDTH, 1.0f - yOffsetBottom/HEIGHT,  0.2f, 0.6f, 1.0f
	};
	unsigned int VAO = input[0];
	unsigned int VBO = input[1];
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * data.size(), &data[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
}

int findSelectedVertex(){
	float mouse_x = (float)(mouse_position.x - WIDTH/2) / (WIDTH/2);
	float mouse_y = -(float)(mouse_position.y - HEIGHT/2) / (HEIGHT/2);
	for(int i{}; i < num_vertices(g); ++i){
		glm::vec4 temp = glm::vec4(0.0f,0.0f,0.0f,1.0f);
		temp.x = (float)position[i][0]/positionDivider;
		temp.y = (float)position[i][1]/positionDivider;
		glm::vec4 finalPos =  projection * view * temp;
		finalPos.x /= finalPos.w;
		finalPos.y /= finalPos.w;
		if(abs(finalPos.x - mouse_x) <= 0.025 && abs(finalPos.y - mouse_y) <= 0.025){
			return i;
		}
	}
	return -1;
}

void deleteRemovedEdges(int vertex, std::set<std::pair<int, int>> &edges){
	for(auto it = edges.begin(); it != edges.end();){
		if((*it).first == vertex){
			if(selectedVertices.count((*it).second) != 1){
				edges.erase(it++);
			}else{
				++it;
			}
		}else if((*it).second == vertex){
			if(selectedVertices.count((*it).first) != 1){
				edges.erase(it++);
			}else{
				++it;
			}
		}else{
			++it;
		}
	}
}

std::vector<unsigned int> createSelectedVertexVAO(){
	unsigned int VAO, VBO, BVBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &BVBO);
	return {VAO, VBO, BVBO};
}

void setSelectedVertexVAOData(std::vector<unsigned int> &arr, std::set<std::pair<int, int>> &edges, glm::vec2 color){
	unsigned int VAO = arr[0];
	unsigned int VBO = arr[1];
	unsigned int BVBO = arr[2];

	std::vector<float> colorData;
	for(int i{}; i < edges.size(); ++i){
		colorData.push_back(color.x);
		colorData.push_back(color.y);
	}
	std::vector<float> positionData;
	for(auto it = edges.begin(); it != edges.end(); ++it){
		positionData.push_back((float)position[(*it).first][0]/positionDivider);
		positionData.push_back((float)position[(*it).first][1]/positionDivider);
		positionData.push_back((float)position[(*it).second][0]/positionDivider);
		positionData.push_back((float)position[(*it).second][1]/positionDivider);
	}
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * positionData.size(), &positionData[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, BVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * colorData.size(), &colorData[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));
	glEnableVertexAttribArray(1);
}

void addSelectedVertex(int vertex){
	selectedVertices.insert(vertex);
	for(int i{}; i < alignedEdgeList[vertex].size(); ++i){
		if(selectedVertexEdgesAligned.count({alignedEdgeList[vertex][i], vertex}) != 1){
			selectedVertexEdgesAligned.insert({vertex, alignedEdgeList[vertex][i]});
		}
	}
	auto ei = boost::edges(g);
	for(auto eit = ei.first; eit != ei.second; ++eit){
		int a, b;
		a = boost::source(*eit, g);
		b = boost::target(*eit, g);
		if(a == vertex){
			if(selectedVertexEdgeUnalignedOne.count({b, vertex}) != 1){
				selectedVertexEdgeUnalignedOne.insert({vertex, b});
			}
		}else if(b == vertex){
			if(selectedVertexEdgeUnalignedOne.count({a, vertex}) != 1){
				selectedVertexEdgeUnalignedOne.insert({vertex, a});
			}
		}
	}
	if(int_map_one_to_two.count(vertex) != 0){
		for(int i{}; i < graphTwoEdges[int_map_one_to_two[vertex]].size(); ++i){
			if(selectedVertexEdgeUnalignedTwo.count({int_map_two_to_one[graphTwoEdges[int_map_one_to_two[vertex]][i]], vertex}) != 1){
				selectedVertexEdgeUnalignedTwo.insert({vertex, int_map_two_to_one[graphTwoEdges[int_map_one_to_two[vertex]][i]]});
			}
		}
	}
}
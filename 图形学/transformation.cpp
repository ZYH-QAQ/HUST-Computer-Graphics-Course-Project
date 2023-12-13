//////////////////////////////////////////////////////////////////////////////
//
//  Sphere.cpp
//  1. 球体的绘制（求出球面上所有的点）
//  2. 三角面片的构造
//  3. 利用统一变量进行数据传递
//////////////////////////////////////////////////////////////////////////////

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vmath.h>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
//窗口大小参数
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float aspact = (float)4.0 / (float)3.0;

//旋转参数
static GLfloat xRot = 20.0f;
static GLfloat yRot = 20.0f;

//光源颜色、位置参数
GLfloat lightColor[3] = {2.0f, 2.0f, 2.0f};//白光

//句柄参数
GLuint vertex_array_object; // == VAO句柄
GLuint vertex_buffer_object; // == VBO句柄
GLuint element_buffer_object;//==EBO句柄
GLuint lightVAO;
GLuint lightVBO;
int shader_program;//着色器程序句柄
int light_shader_program;
GLuint texture_buffer_object_sun;	// 太阳纹理对象句柄
GLuint texture_buffer_object_earth;	// 地球纹理对象句柄
GLuint texture_buffer_object_moon;	// 月球纹理对象句柄

//球的数据参数
std::vector<float> sphereVertices;
std::vector<int> sphereIndices;
const int Y_SEGMENTS = 20;
const int X_SEGMENTS = 20;
const float Radio = 2.0;
const GLfloat  PI = 3.14159265358979323846f;

void loadTexture(GLuint& texture_buffer_object, const char* filename) 
{
	glGenTextures(1, &texture_buffer_object);
	glBindTexture(GL_TEXTURE_2D, texture_buffer_object);

	// 指定纹理的参数
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	int width, height, nrchannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(filename, &width, &height, &nrchannels, 0);

	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture: " << filename << std::endl;
	}

	glBindTexture(GL_TEXTURE_2D, 0);
	stbi_image_free(data);
}
// 创建纹理对象并加载所有纹理
void loadAllTextures() {
	loadTexture(texture_buffer_object_sun, "textures/sun.jpg");
	loadTexture(texture_buffer_object_earth, "textures/earth.jpg");
	loadTexture(texture_buffer_object_moon, "textures/moon.jpg");
}
void initial(void)
{
	//进行球体顶点和三角面片的计算
	// 生成球的顶点
	for (int y = 0; y <= Y_SEGMENTS; y++)
	{
		for (int x = 0; x <= X_SEGMENTS; x++)
		{
			float xSegment = (float)x / (float)X_SEGMENTS;
			float ySegment = (float)y / (float)Y_SEGMENTS;
			float xPos = std::cos(xSegment * Radio * PI) * std::sin(ySegment * PI);
			float yPos = std::cos(ySegment * PI);
			float zPos = std::sin(xSegment * Radio * PI) * std::sin(ySegment * PI);


			sphereVertices.push_back(xPos);
			sphereVertices.push_back(yPos);
			sphereVertices.push_back(zPos);
			//法线与pos完全相同
			sphereVertices.push_back(xPos);
			sphereVertices.push_back(yPos);
			sphereVertices.push_back(zPos);
			//纹理
			sphereVertices.push_back(xSegment);
			sphereVertices.push_back(1-ySegment);
		}
	}

	// 生成球的顶点
	for (int i = 0; i < Y_SEGMENTS; i++)
	{
		for (int j = 0; j < X_SEGMENTS; j++)
		{

			sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);

			sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
			sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
			sphereIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
		}
	}
	// 创建纹理对象并加载纹理
	loadAllTextures();
	// 球
	glGenVertexArrays(1, &vertex_array_object);
	glGenBuffers(1, &vertex_buffer_object);
	//生成并绑定球体的VAO和VBO
	glBindVertexArray(vertex_array_object);

	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
	// 将顶点数据绑定至当前默认的缓冲中    !!!!顶点数据修改TODO
	glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &element_buffer_object);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_object);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

	// 设置顶点属性指针
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0); //两个in vec3，步长为6
	glEnableVertexAttribArray(0);
	// vNormal属性指针
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);
	// 纹理属性指针
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);
	// 解绑VAO和VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//接下来是光源的VAO和VBO（TODO）
	glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    // We only need to bind to the VBO (to link it with glVertexAttribPointer), no need to fill it; the VBO's data already contains all we need.
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    // Set the vertex attributes (only position data for the lamp))
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0); // Note that we skip over the normal vectors
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);


	// 顶点着色器和片段着色器源码
	const char *vertex_shader_source =
		"#version 330 core\n"
		"layout (location = 0) in vec3 vPos;\n"           // 位置变量的属性位置值为0
		"layout (location = 1) in vec3 vNormal;\n"		//法线
		"layout (location = 2) in vec2 vTexture;\n"
		"out vec4 vColor;\n"           // 位置变量的属性位置值为0
		"out vec2 myTexture;\n"							// 输出2维纹理向量
		"out vec3 Normal;\n"
		"out vec3 FragPos;\n"
		"uniform mat4 transform;\n"
		"uniform vec4 color;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = projection * transform * vec4(vPos, 1.0f);\n"
		"	 FragPos = vec3(transform * vec4(vPos, 1.0f));\n"
		"    vColor = color;\n"
		"	 Normal = mat3(transpose(inverse(transform))) * vNormal;\n"	
		"    myTexture = vTexture;\n"
		"}\n\0";
	const char *fragment_shader_source =
		"#version 330 core\n"
		"in vec4 vColor;\n"                   // 输出的颜色向量
		"in vec3 Normal;\n"
		"in vec3 FragPos;\n"
		"in vec2 myTexture;\n"		// 输入的纹理向量
		"out vec4 FragColor;\n"                   // 输出的颜色向量
		"uniform vec3 lightPos;\n"
		"uniform vec3 lightColor;\n"
		"uniform sampler2D tex;\n" 
		"void main()\n"
		"{\n"
		"    float ambientStrength = 0.1f;\n"
		"    vec3 ambient = ambientStrength * lightColor;\n"
		"    vec3 norm = normalize(Normal);\n"
		"    vec3 lightDir = normalize(lightPos - FragPos);\n"
		"    float diff = max(dot(norm, lightDir), 0.0);\n"
		"    vec3 diffuse = diff * lightColor;\n"
		"    vec3 result = (ambient + diffuse) * vec3(texture(tex, myTexture));\n"
		"    FragColor = vec4(result, 1.0f);\n"
		"    \n"
		"    \n"

		"}\n\0";


	// 生成并编译着色器
	// 顶点着色器
	int success;
	char info_log[512];
	int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	// 检查着色器是否成功编译，如果编译失败，打印错误信息
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// 片段着色器
	int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	// 检查着色器是否成功编译，如果编译失败，打印错误信息
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// 链接顶点和片段着色器至一个着色器程序
	shader_program = glCreateProgram();
	glAttachShader(shader_program, vertex_shader);
	glAttachShader(shader_program, fragment_shader);
	glLinkProgram(shader_program);
	// 检查着色器是否成功链接，如果链接失败，打印错误信息
	glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shader_program, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	}

	// 删除着色器
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);

	//光源着色器
	vertex_shader_source =
		"#version 330 core\n"
		"layout (location = 0) in vec3 vPos;\n"           // 位置变量的属性位置值为0
		"layout (location = 1) in vec3 vNormal;\n"		//法线
		"layout (location = 2) in vec2 vTexture;\n"
		"out vec4 vColor;\n"           // 位置变量的属性位置值为0
		"out vec2 myTexture;\n"							// 输出2维纹理向量
		"out vec3 Normal;\n"
		"out vec3 FragPos;\n"
		"uniform mat4 transform;\n"
		"uniform vec4 color;\n"
		"uniform mat4 projection;\n"
		"void main()\n"
		"{\n"
		"    gl_Position = projection * transform * vec4(vPos, 1.0f);\n"
		"	 FragPos = vec3(transform * vec4(vPos, 1.0f));\n"
		"    vColor = color;\n"
		"	 Normal = mat3(transpose(inverse(transform))) * vNormal;\n"	
		"    myTexture = vTexture;\n"
		"}\n\0";
	fragment_shader_source =
		"#version 330 core\n"
		"in vec4 vColor;\n"                   // 输出的颜色向量
		"in vec3 Normal;\n"
		"in vec3 FragPos;\n"
		"in vec2 myTexture;\n"		// 输入的纹理向量
		"out vec4 FragColor;\n"                   // 输出的颜色向量
		"uniform vec3 lightPos;\n"
		"uniform vec3 lightColor;\n"
		"uniform sampler2D tex;\n" 
		"void main()\n"
		"{\n"
		"    float ambientStrength = 1.0f;\n"
		"    vec3 ambient = ambientStrength * lightColor;\n"
		"    vec3 result = ambient * vec3(texture(tex, myTexture));\n"
		"    FragColor = vec4(result, 1.0f);\n"
		"    \n"
		"    \n"

		"}\n\0";
	// 生成并编译着色器
	// 顶点着色器
	success = 0;
	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
	glCompileShader(vertex_shader);
	// 检查着色器是否成功编译，如果编译失败，打印错误信息
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertex_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// 片段着色器
	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
	glCompileShader(fragment_shader);
	// 检查着色器是否成功编译，如果编译失败，打印错误信息
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragment_shader, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << info_log << std::endl;
	}
	// 链接顶点和片段着色器至一个着色器程序
	light_shader_program = glCreateProgram();
	glAttachShader(light_shader_program, vertex_shader);
	glAttachShader(light_shader_program, fragment_shader);
	glLinkProgram(light_shader_program);
	// 检查着色器是否成功链接，如果链接失败，打印错误信息
	glGetProgramiv(light_shader_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(light_shader_program, 512, NULL, info_log);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << info_log << std::endl;
	}

	// 删除着色器
	glDeleteShader(vertex_shader);
	glDeleteShader(fragment_shader);


	//glUseProgram(shader_program);

	//设定点线面的属性
	glPointSize(15);//设置点的大小
	glLineWidth(5);//设置线宽

	//启动剔除操作
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	//开启深度测试
	glEnable(GL_DEPTH_TEST);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	switch (key)
	{
	case GLFW_KEY_ESCAPE:
		glfwSetWindowShouldClose(window, GL_TRUE);
		break;
	case GLFW_KEY_UP:
		xRot -= 5.0f;
		break;
	case GLFW_KEY_DOWN:
		xRot += 5.0f;
		break;
	case GLFW_KEY_LEFT:
		yRot -= 5.0f;
		break;
	case GLFW_KEY_RIGHT:
		yRot += 5.0f;
		break;
	case GLFW_KEY_1:
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		break;
	case GLFW_KEY_2:
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		break;
	case GLFW_KEY_3:
		glEnable(GL_CULL_FACE);    //打开背面剔除
		glCullFace(GL_BACK);          //剔除多边形的背面
		break;
	case GLFW_KEY_4:
		glDisable(GL_CULL_FACE);     //关闭背面剔除
		break;
	default:
		break;
	}
}

void Draw(void)
{

	glUseProgram(light_shader_program);
	// 清空颜色缓冲和深度缓冲区
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//处理图形的旋转
	vmath::mat4 projectionMatrix = vmath::perspective(60, aspact, 1.0f, 500.0f);//填写projection矩阵
	unsigned int projectionLoc = glGetUniformLocation(light_shader_program, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMatrix);
	vmath::mat4 trans = vmath::translate(0.0f, 0.0f, -5.0f);//公转
	unsigned int transformLoc = glGetUniformLocation(light_shader_program, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans * vmath::rotate(xRot * 0.4f, vmath::vec3(0.0, 1.0, 0.0)));//自转最后计算
	//添加光照（TODO）
	GLfloat lightPos[3] = {0.0f, 0.0f, -5.0f};//与trans保持一致
	GLint lightPosLoc = glGetUniformLocation(light_shader_program, "lightPos");
	glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);
	GLint lightColorLoc = glGetUniformLocation(light_shader_program, "lightColor");
	glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);
	//处理图形的颜色
	glUniform1i(glGetUniformLocation(light_shader_program, "tex"), 0);  // 太阳纹理单元为0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture_buffer_object_sun);
	GLfloat vColor[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
	unsigned int colorLoc = glGetUniformLocation(light_shader_program, "color");
	glUniform4fv(colorLoc, 1, vColor);

	// 绘制第一个红色的球
	glBindVertexArray(vertex_array_object);                                    // 绑定VAO
	glDrawElements(GL_TRIANGLES, X_SEGMENTS*Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);         



	//对第二个shader，重复
	glUseProgram(shader_program);
	transformLoc = glGetUniformLocation(shader_program, "transform");
	//添加光照（TODO）
	lightPosLoc = glGetUniformLocation(shader_program, "lightPos");
	glUniform3f(lightPosLoc, lightPos[0], lightPos[1], lightPos[2]);
	lightColorLoc = glGetUniformLocation(shader_program, "lightColor");
	glUniform3f(lightColorLoc, lightColor[0], lightColor[1], lightColor[2]);
	//绘制第二个黑色的球
	projectionMatrix = vmath::perspective(60, aspact, 1.0f, 500.0f);//填写projection矩阵
	projectionLoc = glGetUniformLocation(shader_program, "projection");
	glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, projectionMatrix);
	xRot += (float)0.5f;
	vmath::mat4 trans2 = vmath::translate(0.0f, 0.0f, -5.0f)* vmath::rotate(xRot, vmath::vec3(0.0, 1.0, 0.0))*vmath::translate(3.0f, 0.0f, 0.0f)*vmath::scale(0.3f);//公转
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans2 * vmath::rotate(xRot * 3.0f, vmath::vec3(0.0, 1.0, 0.0)));//自转最后计算

	glUniform1i(glGetUniformLocation(shader_program, "tex"), 1);  // 地球纹理单元为1
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texture_buffer_object_earth);
	GLfloat vColor2[4] = { 0.0f, 0.9f, 0.9f, 1.0f };
	glUniform4fv(colorLoc, 1, vColor2);
	glDrawElements(GL_TRIANGLES, X_SEGMENTS*Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

	vmath::mat4 trans3 = trans2 * vmath::rotate(30.0f, vmath::vec3(0.0, 0.0, 1.0)) *vmath::rotate(xRot * 12.36f, vmath::vec3(0.0, 1.0, 0.0))* vmath::translate(1.5f, 0.0f, 0.0f) * vmath::scale(0.2f); //月球公转周期为地球的13.36倍
	glUniform1i(glGetUniformLocation(shader_program, "tex"), 2);  // 月球纹理单元为2
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texture_buffer_object_moon);
    GLfloat vColor3[4] = { 0.6f, 0.6f, 0.7f, 1.0f };  // 月亮的颜色
    glUniformMatrix4fv(transformLoc, 1, GL_FALSE, trans3);//月球潮汐锁定，旋转系内不用单独处理自转
    glUniform4fv(colorLoc, 1, vColor3);
    glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);

	
	glBindVertexArray(0);                                                      // 解除绑定

}

void reshaper(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	if (height == 0)
	{
		aspact = (float)width;
	}
	else
	{
		aspact = (float)width / (float)height;
	}

}

int main()
{
	glfwInit(); // 初始化GLFW

	// OpenGL版本为3.3，主次版本号均设为3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	// 使用核心模式(无需向后兼容性)
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// 创建窗口(宽、高、窗口名称)
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Sphere", NULL, NULL);

	if (window == NULL)
	{
		std::cout << "Failed to Create OpenGL Context" << std::endl;
		glfwTerminate();
		return -1;
	}

	// 将窗口的上下文设置为当前线程的主上下文
	glfwMakeContextCurrent(window);

	// 初始化GLAD，加载OpenGL函数指针地址的函数
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	initial();//初始化

	//窗口大小改变时调用reshaper函数
	glfwSetFramebufferSizeCallback(window, reshaper);

	//窗口中有键盘操作时调用key_callback函数
	glfwSetKeyCallback(window, key_callback);

	
	while (!glfwWindowShouldClose(window))
	{
		Draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// 解绑和删除VAO和VBO
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteVertexArrays(1, &vertex_array_object);
	glDeleteBuffers(1, &vertex_buffer_object);

	glfwDestroyWindow(window);

	glfwTerminate();
	return 0;
}

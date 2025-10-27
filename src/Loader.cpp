#include "Loader.h"


// constructor generates the shader on the fly
// ------------------------------------------------------------------------
Loader::Loader(float vertices[], size_t vertexCount, unsigned int indices[], size_t indexCount) {

    // create VBO, VAO, EBO
    vbo = makeVBO();
    vao = makeVAO();
    ebo = makeEBO();

    // Bind VAO
    glBindVertexArray(vao);

    // Copy vertices array in a vertex buffer for openGL to use
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(float), vertices, GL_STATIC_DRAW);

    // Copy index array in a element buffer for openGL to use
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    // Then set the vertex attributes pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    // uncomment this call to draw in wireframe polygons.
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

};

// destructor 
Loader::~Loader() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &ebo);
    std::cout << "Loader destructed, VBO, VAO, EBO deleted." << std::endl;
};

// getter for VBO, VAO, EBO
// ------------------------------------------------------------------------ 
unsigned int Loader::getVBO() const { return vbo; };
unsigned int Loader::getVAO() const { return vao; };
unsigned int Loader::getEBO() const { return ebo; };

// create VBO, VAO, EBO
// ------------------------------------------------------------------------
unsigned int Loader::makeVBO() {
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    return VBO;
};

unsigned int Loader::makeVAO() {
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    return VAO;
};

unsigned int Loader::makeEBO() {
    unsigned int EBO;
    glGenBuffers(1, &EBO);
    return EBO;
};



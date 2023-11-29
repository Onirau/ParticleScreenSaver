#include "Application.h"
#include "fpsCounter.h"

// OpenGL implementation of https://gpfault.net/posts/webgl2-particles.txt.html
// original was made by nice byte

const char* updateVertexShaderSource = R"(
    #version 330 core
    precision mediump float;

    /* Number of seconds (possibly fractional) that has passed since the last
       update step. */
    uniform float u_TimeDelta;
    uniform float u_TotalTime;

    /* A texture with just 2 channels (red and green), filled with random values.
       This is needed to assign a random direction to newly born particles. */
    uniform sampler2D u_RgNoise;

    /* This is the gravity vector. It's a force that affects all particles all the
       time.*/
    uniform vec2 u_Gravity;

    /* This is the point from which all newborn particles start their movement. */
    uniform vec2 u_Origin;

    uniform vec2 u_screenSize;

    /* Theta is the angle between the vector (1, 0) and a newborn particle's
       velocity vector. By setting u_MinTheta and u_MaxTheta, we can restrict it
       to be in a certain range to achieve a directed "cone" of particles.
       To emit particles in all directions, set these to -PI and PI. */
    uniform float u_MinTheta;
    uniform float u_MaxTheta;

    /* The min and max values of the (scalar!) speed assigned to a newborn
       particle.*/
    uniform float u_MinSpeed;
    uniform float u_MaxSpeed;


    /* Inputs. These reflect the state of a single particle before the update. */

    /* Where the particle is. */
    in vec2 i_Position;

    /* Age of the particle in seconds. */
    in float i_Age;

    /* How long this particle is supposed to live. */
    in float i_Life;

    /* Which direction it is moving, and how fast. */ 
    in vec2 i_Velocity;


    /* Outputs. These mirror the inputs. These values will be captured
       into our transform feedback buffer! */
    out vec2 v_Position;
    out float v_Age;
    out float v_Life;
    out vec2 v_Velocity;


    void main() {
      if (i_Age >= i_Life) {
        ivec2 noise_coord = ivec2(gl_VertexID % 512, gl_VertexID / 512);
        vec2 rand = texelFetch(u_RgNoise, noise_coord, 0).rg;
        float theta = u_MinTheta + rand.r*(u_MaxTheta - u_MinTheta);

        float x = cos(theta);
        float y = sin(theta);

        /* Return the particle to origin. */
        v_Position = (u_Origin)/u_screenSize;

        /* It's new, so age must be set accordingly.*/
        v_Age = 0.0;
        v_Life = i_Life;

        /* Generate final velocity vector. We use the second random value here
           to randomize speed. */
        v_Velocity =
          vec2(x, y) * (u_MinSpeed + rand.g * (u_MaxSpeed - u_MinSpeed));

      } else {
        /* Update parameters according to our simple rules.*/
        v_Position = (i_Position/u_screenSize) + i_Velocity * u_TimeDelta;
        v_Age = i_Age + u_TimeDelta;
        v_Life = i_Life;
        v_Velocity = i_Velocity + u_Gravity * u_TimeDelta;
      }
    }
)";

const char* updateFragmentShaderSource = R"(
    #version 330 core
    precision mediump float;

    out vec4 o_FragColor;

    void main() {
      o_FragColor = vec4(1.0);
    }
)";

const char* renderVertexShaderSource = R"(
    #version 330 core
    precision mediump float;

    in vec2 i_Position;
    in float i_Age;
    in float i_Life;
    in vec2 i_Velocity;

    void main() {
      gl_PointSize = 1.0;
      gl_Position = vec4(i_Position, 0.0, 1.0);
    }
)";

const char* renderFragmentShaderSource = R"(
    #version 330 core
    precision mediump float;

    out vec4 o_FragColor;

    void main() {
      o_FragColor = vec4(1.0);
    }
)";

// Function to generate random RGB data
vector<uint8_t> randomRGData(int size_x, int size_y) {
    vector<uint8_t> data;
    for (int i = 0; i < size_x * size_y; ++i) {
        data.push_back(static_cast<uint8_t>(rand() % 256));
        data.push_back(static_cast<uint8_t>(rand() % 256));
    }
    return data;
}

// Function to initialize particle data
vector<Particle> initialParticleData(int num_parts, float min_age, float max_age, IntVector2 windowDimensions) {
    vector<Particle> data;
    for (int i = 0; i < num_parts; ++i) {
        float life = min_age + static_cast<float>(rand()) / RAND_MAX * (max_age - min_age);
        float rX = (-windowDimensions.x) + static_cast<float>(rand()) / RAND_MAX * (windowDimensions.x - (-windowDimensions.x));
        float rY = (-windowDimensions.y) + static_cast<float>(rand()) / RAND_MAX * (windowDimensions.y - (-windowDimensions.y));

        Particle particle = {
            rX, // px
            rY, // py
            0.0, // vx
            0.0, // vy
            life + 1.0, // age
            life // life
        };

        data.push_back(particle);
    }
    return data;
}

size_t getArraySize(const char* array[]) {
    size_t size = 0;
    while (array[size] != nullptr) {
        ++size;
    }
    return size;
}

GLuint createProgram(initializer_list<Shader> shaders, const char* transform_varyings[] = nullptr) {
    vector<GLuint> shaderList;

    for (const Shader& shader : shaders) {
        GLuint shaderID = glCreateShader(static_cast<GLenum>(shader.type));
        glShaderSource(shaderID, 1, &shader.source, nullptr);
        glCompileShader(shaderID);

        const char* shaderName = "Unknown";

        if (shader.name != nullptr) {
            shaderName = shader.name;
        }

        GLint shaderStatus;
        glGetShaderiv(shaderID, GL_COMPILE_STATUS, &shaderStatus);
        if (shaderStatus != GL_TRUE) {
            GLint logLength;
            glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);
            if (logLength > 0) {
                GLchar* log = new GLchar[logLength];
                glGetShaderInfoLog(shaderID, logLength, nullptr, log);
                cerr << "[" << shaderName << "] Shader compilation failed:\n" << log << endl;
                delete[] log;
            }
            else {
                cerr << "[" << shaderName << "] Shader compilation failed." << endl;
            }
        }
        else {
            cout << "[" << shaderName << "] Shader compilation successful." << endl;
            shaderList.push_back(shaderID);
        }
    }

    GLuint shaderProgram = glCreateProgram();

    for (GLuint shader : shaderList) {
        glAttachShader(shaderProgram, shader);
    }

    if (transform_varyings != nullptr) {
        GLsizei transformSize = getArraySize(transform_varyings);

        glTransformFeedbackVaryings(shaderProgram, transformSize, transform_varyings, GL_INTERLEAVED_ATTRIBS);
        cout << "Transform Feedback Set." << endl;
    }
    else {
        cout << "No Transform Feedback Set." << endl;
    }

    glLinkProgram(shaderProgram);
    // Check shader program linking status
    GLint linkStatus;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        GLint logLength;
        glGetProgramiv(shaderProgram, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
            GLchar* log = new GLchar[logLength];
            glGetProgramInfoLog(shaderProgram, logLength, nullptr, log);
            cerr << "Shader program linking failed:\n" << log << endl;
            delete[] log;
        }
        else {
            cerr << "Shader program linking failed." << endl;
        }
    }
    else {
        cout << "Shader program linking successful." << endl;
    }

    for (GLuint shader : shaderList) {
        glDeleteShader(shader);
    }

    shaderList.clear();

    return shaderProgram;
}

void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
    cerr << "OpenGL Debug Message:" << endl;
    cerr << "  Source: " << source << endl;
    cerr << "  Type: ";

    switch (type) {
    case GL_DEBUG_TYPE_ERROR:
        cout << "ERROR";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        cout << "DEPRECATED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        cout << "UNDEFINED_BEHAVIOR";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        cout << "PORTABILITY";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        cout << "PERFORMANCE";
        break;
    case GL_DEBUG_TYPE_OTHER:
        cout << "OTHER";
        break;
    }

    cerr << endl;
    cerr << "  ID: " << id << endl;
    cerr << "  Severity: ";

    switch (severity) {
    case GL_DEBUG_SEVERITY_LOW:
        cout << "LOW";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        cout << "MEDIUM";
        break;
    case GL_DEBUG_SEVERITY_HIGH:
        cout << "HIGH";
        break;
    }

    cerr << endl;
    cerr << "  Message: " << message << endl;
}

void Application::createWindow()
{
    if (!glfwInit()) {
        cerr << "Failed to init GLFW!" << endl;
        return;
    }

    // Set Hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);  // Set the number of samples for anti-aliasing
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);  // Enable transparent

    // Create Window
    _window = glfwCreateWindow(windowDimensions.x, windowDimensions.y, title, NULL, NULL);
}

void setupBufferVAO(GLuint vao, GLuint* buffer, AttributeLocation* attributes) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, *buffer);

    int offset = 0;
    size_t attributeIndex = 0;

    while (attributes[attributeIndex].location != static_cast<GLuint>(-1)) {
        const AttributeLocation& attribute = attributes[attributeIndex];

        glEnableVertexAttribArray(attribute.location);
        
        // HOW glVertexAttribPointer
        // Set up the vertex attribute pointer
        glVertexAttribPointer(
            attribute.location,         // Attribute location
            attribute.num_components,   // Number of components per attribute
            attribute.type,             // Data type of each component
            GL_FALSE,                   // Whether to normalize the data
            attribute.stride,           // Byte offset between attributes
            reinterpret_cast<void*>(offset)  // Offset of the first component
        );


        // Check for OpenGL errors after each glVertexAttribPointer call
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            std::cerr << "OpenGL Error in glVertexAttribPointer: " << error << std::endl;
            // Print attribute information for debugging
            std::cout << "Attribute " << attributeIndex << ": "
                << "Location=" << attribute.location
                << ", Components=" << attribute.num_components
                << ", Type=" << attribute.type
                << ", Stride=" << attribute.stride
                << ", Offset=" << offset << std::endl;
        }
        
        offset += attribute.num_components * sizeof(attribute.type);

        ++attributeIndex;
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Application::setupBuffers() {
    GLsizei stride = sizeof(Particle);

    AttributeLocation update_attrib_locations[] = {
        { glGetAttribLocation(_updateProgram, "i_Position"), 2, stride, GL_FLOAT},
        { glGetAttribLocation(_updateProgram, "i_Age"), 1, stride, GL_FLOAT},
        { glGetAttribLocation(_updateProgram, "i_Life"), 1, stride, GL_FLOAT},
        { glGetAttribLocation(_updateProgram, "i_Velocity"), 2, stride, GL_FLOAT},
        { -1, 0, 0, 0}
    };

    AttributeLocation render_attrib_locations[] = {
        { glGetAttribLocation(_renderProgram, "i_Position"), 2, stride, GL_FLOAT},
        { -1, 0, 0, 0}
    };

    setupBufferVAO(_particleVAO[0], &_particleBuffers[0], update_attrib_locations);
    setupBufferVAO(_particleVAO[1], &_particleBuffers[1], update_attrib_locations);
    setupBufferVAO(_particleVAO[2], &_particleBuffers[0], render_attrib_locations);
    setupBufferVAO(_particleVAO[3], &_particleBuffers[1], render_attrib_locations);
}

void Application::genBuffers() {
    glCreateVertexArrays(1, &_particleVAO[0]);
    glCreateVertexArrays(1, &_particleVAO[1]);
    glCreateVertexArrays(1, &_particleVAO[2]);
    glCreateVertexArrays(1, &_particleVAO[3]);
    glCreateBuffers(1, &_particleBuffers[0]);
    glCreateBuffers(1, &_particleBuffers[1]);
}

void Application::compileShaders() {
    const char* transformVaryings[] = {"v_Position", "v_Age", "v_Life", "v_Velocity", nullptr};

    _updateProgram = createProgram(
        {
            {"particle-update-vert", ShaderType::Vertex, updateVertexShaderSource},
            {"passthru-frag-shader", ShaderType::Fragment, updateFragmentShaderSource}
        },
        transformVaryings
    );

    _renderProgram = createProgram(
        {
            {"particle-render-vert", ShaderType::Vertex, renderVertexShaderSource},
            {"particle-render-frag", ShaderType::Fragment, renderFragmentShaderSource}
        },
        nullptr
    );
}

void Application::_update(double tt, double dt)
{
    // Main (RENDER)
    glUseProgram(_updateProgram);

    glUniform1f(glGetUniformLocation(_updateProgram, "u_TimeDelta"), dt);
    glUniform1f(glGetUniformLocation(_updateProgram, "u_TotalTime"), tt);
    glUniform2f(glGetUniformLocation(_updateProgram, "u_Gravity"), gravity[0], gravity[1]);
    glUniform2f(glGetUniformLocation(_updateProgram, "u_Origin"), origin[0], origin[1]);
    glUniform2f(glGetUniformLocation(_updateProgram, "u_screenSize"), windowDimensions.x, windowDimensions.y);
    glUniform1f(glGetUniformLocation(_updateProgram, "u_MinTheta"), theta[0]);
    glUniform1f(glGetUniformLocation(_updateProgram, "u_MaxTheta"), theta[1]);
    glUniform1f(glGetUniformLocation(_updateProgram, "u_MinSpeed"), speed[0]);
    glUniform1f(glGetUniformLocation(_updateProgram, "u_MaxSpeed"), speed[1]);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _noiseTexture);
    glUniform1i(glGetUniformLocation(_updateProgram, "u_RgNoise"), 0);

    // bind read
    glBindVertexArray(_particleVAO[_read]); // wrong?

    // bind write
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _particleBuffers[_write]);
    glEnable(GL_RASTERIZER_DISCARD);

    glBeginTransformFeedback(GL_POINTS);
    glDrawArrays(GL_POINTS, 0, numParticles);
    glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);
    glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

    glBindVertexArray(_particleVAO[_read + 2]);
    glUseProgram(_renderProgram);
    glDrawArrays(GL_POINTS, 0, numParticles);

    // Swap Read/Write Buffers
    int temp = _read;
    _read = _write;
    _write = temp;

    //cout << "read val: " << _read << endl;


    // Set FPS Counter
    double DisplayDelta = _applicationCurrentTime - _applicationLastDisplayUpdate;

    if (DisplayDelta >= 1.0f) {
        string newWindowTitle = string(title) + " [FPS: " + to_string(static_cast<int>(_applicationFrameCount + 0.5f)) + "]" + "[ UP-TIME: " + to_string(static_cast<int>(tt)) + "]" + "[ PARTICLE-COUNT: " + to_string(numParticles)+"]";
        _applicationFrameCount = 0;

        glfwSetWindowTitle(_window, newWindowTitle.c_str());

        _applicationLastDisplayUpdate = _applicationCurrentTime;
    }
    else {
        _applicationFrameCount++;
    }
}

void Application::run() {
    if (!_window) return;

    // Compile Shaders
    cout << "Compiling Shaders!" << endl;
    compileShaders();
    cout << "Compiled Shaders!" << endl;

    // Populate
    genBuffers();    
    
    vector<Particle> particles = initialParticleData(numParticles, minAge, maxAge, windowDimensions);

    size_t dataSize = particles.size() * sizeof(Particle);

    glBindBuffer(GL_ARRAY_BUFFER, _particleBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, dataSize, particles.data(), GL_STREAM_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, _particleBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, dataSize, particles.data(), GL_STREAM_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // Gen Buffers
    cout << "Creating Buffers!" << endl;
    setupBuffers();
    cout << "Created Buffers!" << endl;

    // Create random noise texture
    glCreateTextures(GL_TEXTURE_2D, 1, &_noiseTexture);
    glBindTexture(GL_TEXTURE_2D, _noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 512, 512, 0, GL_RG, GL_UNSIGNED_BYTE, randomRGData(512, 512).data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    while (!glfwWindowShouldClose(_window)) {
        _applicationCurrentTime = glfwGetTime();

        //glViewport(0, 0, windowDimensions.x, windowDimensions.y);
        glClear(GL_COLOR_BUFFER_BIT);

        double tT = _applicationCurrentTime - _applicationStartTime;
        double dT = _applicationCurrentTime - _applicationLastUpdate;

        _update(tT, dT);

        glfwSwapBuffers(_window);
        glfwPollEvents();

        _applicationLastUpdate = _applicationCurrentTime;
    }

    glfwDestroyWindow(_window);
    glfwTerminate();
}

Application::Application(const char* _title, int _numParticles, float _minAge, float _maxAge, IntVector2 _windowDimensions) {
    title = _title;
    numParticles = _numParticles;
    windowDimensions = _windowDimensions;
    minAge = _minAge;
    maxAge = _maxAge;

    // Load GLFW
    createWindow();
    glfwGetFramebufferSize(_window, &windowDimensions.x, &windowDimensions.y);

    if (!_window) {
        cerr << "Failed to construct window!" << endl;
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(_window);

    // Load OpenGL using Glad
    if (!gladLoadGL()) {
        cerr << "Failed to init OpenGL!" << endl;
        glfwDestroyWindow(_window);
        glfwTerminate();
        return;
    }

    glfwSwapInterval(1);

    _applicationStartTime = glfwGetTime();

    cout << "Window Constructed!" << endl;
    cout << "( OpenGL Version " << GLVersion.major << "." << GLVersion.minor << ")" << endl;

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debugCallback, nullptr);
}

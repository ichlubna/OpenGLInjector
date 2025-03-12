class GLInjection
{
    public:
    void init(int windowWidth, int windowHeight)
    {
        width = windowWidth;
        height = windowHeight; 
        GLint drawFbo = 0, readFbo = 0;
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFbo);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFbo);

        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenTextures(1, &fboTexture);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)  
            std::cerr << "Framebuffer creation failed" << std::endl;

        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
        glCompileShader(vertexShader); 
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "Holo vertex shader:" << std::endl;
            std::cerr << infoLog;
        }

        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "Injected fragment shader:" << std::endl;
            std::cerr << infoLog;
        }

        shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);
        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "Injected shader program:";
            std::cerr <<  infoLog;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glGenVertexArrays(1, &emptyVAO);
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFbo); 
        glBindFramebuffer(GL_READ_FRAMEBUFFER, drawFbo); 
    }
    void render()
    {
        GLint originalFbo = 0;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &originalFbo);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, originalFbo); 
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
        glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, originalFbo); 
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glUseProgram(shaderProgram);
        glBindVertexArray(emptyVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        
        glBindFramebuffer(GL_FRAMEBUFFER, originalFbo);
    }

    private:
    GLuint fbo;
    GLuint fboTexture;
    GLuint shaderProgram;
    GLuint emptyVAO;
    int width;
    int height;
    const char *vertexShaderSource = R""""(
        #version 330 core
        out vec2 uv;

        void main()
        {
            vec2 triangle[3] = vec2[3](vec2(-1,-1), vec2(3,-1), vec2(-1, 3));
            gl_Position = vec4(triangle[gl_VertexID], 0, 1);
            uv = 0.5f * gl_Position.xy + vec2(0.5f);
        }
        )"""";
    const char *fragmentShaderSource =  R""""(
        #version 330 core
        uniform sampler2D fboTexture;
        in vec2 uv;
        out vec4 fragColor;

        void main(void)
        {
            vec4 color = texture(fboTexture, uv);
            color.r = 0;
            fragColor = color;
        }
        )"""";

};

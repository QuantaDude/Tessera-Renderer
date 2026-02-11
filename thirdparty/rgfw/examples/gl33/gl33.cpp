#define RGFW_OPENGL
//#define RGFW_USE_XDL // feel free to remove this line if you don't want to use XDL (-lX11 -lXrandr -lGLX will be required)
#define RGFW_ALLOC_DROPFILES
//#define RGFW_IMPLEMENTATION
#define RGFW_PRINT_ERRORS
#define RGFW_DEBUG
#define GL_SILENCE_DEPRECATION
extern "C"{
#include <RGFW.h>
}
#ifndef __EMSCRIPTEN__
#define RGL_LOAD_IMPLEMENTATION
#include "rglLoad.h"
#else
#include <GLES3/gl3.h>
#endif

#define MULTILINE_STR(...) #__VA_ARGS__

#include <stdbool.h>

// settings
const int SCR_WIDTH = 800;
const int SCR_HEIGHT = 600;

#ifndef __EMSCRIPTEN__
const char *vertexShaderSource = MULTILINE_STR(

\x23version 330 core\n
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
);

const char *fragmentShaderSource = MULTILINE_STR(
\x23version 330 core\n
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);
}
);
#else
   const char *vertexShaderSource = MULTILINE_STR(
      attribute vec3 aPos;
      void main() {
         gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
      }
   );

  const char *fragmentShaderSource = MULTILINE_STR(
      void main() {
        gl_FragColor = vec4(1.0, 0.5, 0.2, 1.0);
      }
    );
#endif


int main(void) {
    RGFW_glHints* hints = RGFW_getGlobalHints_OpenGL();
    hints->major = 3;
    hints->minor = 3;
    RGFW_setGlobalHints_OpenGL(hints);

	RGFW_window* window = RGFW_createWindow("LearnOpenGL", SCR_WIDTH, SCR_HEIGHT, SCR_WIDTH, SCR_HEIGHT, RGFW_windowAllowDND | RGFW_windowCenter | RGFW_windowScaleToMonitor | RGFW_windowOpenGL);
    if (window == NULL)
    {
        printf("Failed to create RGFW window\n");
        return -1;
    }
    RGFW_window_setExitKey(window, RGFW_escape);
    RGFW_window_makeCurrentContext_OpenGL(window);

    #ifndef RGFW_WASM
    if (RGL_loadGL3((RGLloadfunc)RGFW_getProcAddress_OpenGL)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }
    #endif

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
    }
    // link shaders
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);


    // uncomment this call to draw in wireframe polygons.
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
        RGFW_event event;

        while (RGFW_window_checkEvent(window, &event)) {
            if (event.type == RGFW_quit) {
                break;
            }
        }

		// render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // draw our first triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized
        //glDrawArrays(GL_TRIANGLES, 0, 6);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind it every time

        RGFW_window_swapBuffers_OpenGL(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);

    RGFW_window_close(window);
    return 0;
}
// #include <cstdlib>
// #include <format>
// #include <glm/gtc/type_ptr.hpp>
// #include <glm/mat4x4.hpp> // glm::mat4
// #include <glm/vec3.hpp>   // glm::vec3
// #include <glm/vec4.hpp>   // glm::vec4
// #include <print>
// #include <unordered_map>
// #include <utility>
// //
// #define LOGF_INFO(fmt, ...)                                                                        \
// logMessage(LogLevel::Info, std::format(fmt, __VA_ARGS__), __FILE__, __LINE__)
// #define LOGF_WARN(fmt, ...)                                                                        \
// logMessage(LogLevel::Warning, std::format(fmt, __VA_ARGS__), __FILE__, __LINE__)
//
// #define LOGF_ERROR(fmt, ...)                                                                       \
// logMessage(LogLevel::Error, std::format(fmt, __VA_ARGS__), __FILE__, __LINE__)
//
// #define LOG_INFO(msg) logMessage(LogLevel::Info, msg, __FILE__, __LINE__)
// #define LOG_WARN(msg) logMessage(LogLevel::Warning, msg, __FILE__, __LINE__)
//
// #define LOG_ERROR(msg) logMessage(LogLevel::Error, msg, __FILE__, __LINE__)
//
// enum class LogLevel { Info, Warning, Error };
//
// inline void logMessage(LogLevel level, std::string_view message, const char* file, int line) {
//     const char* levelStr = level == LogLevel::Info      ? "INFO"
//     : level == LogLevel::Warning ? "WARN"
//     : "ERROR";
//
//     std::println(stderr, "[{}] {} ({}:{})", levelStr, message, file, line);
// }
//
// #ifdef NDEBUG
//
// #  define ASSERT(cond) ((void)0)
//
// #else
//
// #  include <cstdlib>
//
// #  define ASSERT(cond)                                                                             \
// do {                                                                                           \
//     if (!(cond)) {                                                                               \
//         logMessage(LogLevel::Error, "ASSERT FAILED: " #cond, __FILE__, __LINE__);                  \
//         std::abort();                                                                              \
//     }                                                                                            \
// } while (0)
//
// #endif
//
// #include <cstdlib>
//
// #define ASSERT_ALWAYS(cond)                                                                        \
// do {                                                                                             \
//     if (!(cond)) {                                                                                 \
//         logMessage(LogLevel::Error, "ASSERT_ALWAYS FAILED: " #cond, __FILE__, __LINE__);             \
//         std::abort();                                                                                \
//     }                                                                                              \
// } while (0)
//
// #define CHECK(cond, fmt, ...)                                                                      \
// do {                                                                                             \
//     if (!(cond)) {                                                                                 \
//         LOGF_WARN("CHECK FAILED: {}: " fmt, #cond, ##__VA_ARGS__);                                   \
//     }                                                                                              \
// } while (0)
//
// #define RGFW_OPENGL
// // #undec> RGFW_X11
// #define RGFW_WAYLAND
// // #define RGFW_USE_XDL //  XDL (-lX11 -lXrandr -lGLX will be required)
// #define RGFW_ALLOC_DROPFILES
// //#define RGFW_IMPLEMENTATION
// #define RGFW_PRINT_ERRORS
// // #define RGFW_DEBUG
// #define GL_SILENCE_DEPRECATION
// #define RGFW_PRINTF(...) ((void)0)
//
// extern "C" {
//     #include "RGFW.h"
// }
// #ifdef RGFW_MACOS
// #  include <OpenGL/gl.h>
// #else
// #  include <GL/gl.h>
// #endif
// #ifndef __EMSCRIPTEN__
// #  define RGL_LOAD_IMPLEMENTATION
// #  include "rglLoad.h"
// #else
// #  include <GLES3/gl3.h>
// #endif
//
// void APIENTRY glDebugCallback(GLenum        source,
//                               GLenum        type,
//                               GLuint        id,
//                               GLenum        severity,
//                               GLsizei       length,
//                               const GLchar* message,
//                               const void*   userParam) {
//     std::string_view src = source == GL_DEBUG_SOURCE_API               ? "API"
//     : source == GL_DEBUG_SOURCE_SHADER_COMPILER ? "SHADER"
//     : source == GL_DEBUG_SOURCE_WINDOW_SYSTEM   ? "WINDOW"
//     : source == GL_DEBUG_SOURCE_THIRD_PARTY     ? "3RD PARTY"
//     : source == GL_DEBUG_SOURCE_APPLICATION     ? "APP"
//     : "OTHER";
//
//     std::string_view msg(message);
//
//     switch (severity) {
//         case GL_DEBUG_SEVERITY_HIGH:
//             // Something is seriously wrong — UB or crash likely
//             logMessage(LogLevel::Error,
//                        std::string("GL HIGH : ") + std::string(src) + " : " + std::string(msg),
//                        __FILE__,
//                        __LINE__);
//             // std::abort();
//
//         case GL_DEBUG_SEVERITY_MEDIUM:
//             logMessage(LogLevel::Error,
//                        std::string("GL MEDIUM : ") + std::string(src) + " : " + std::string(msg),
//                        __FILE__,
//                        __LINE__);
//             break;
//
//         case GL_DEBUG_SEVERITY_LOW:
//             logMessage(LogLevel::Warning,
//                        std::string("GL LOW : ") + std::string(src) + " : " + std::string(msg),
//                        __FILE__,
//                        __LINE__);
//             break;
//
//         case GL_DEBUG_SEVERITY_NOTIFICATION:
//             // Optional — often spammy
//             logMessage(LogLevel::Info, msg, __FILE__, __LINE__);
//             break;
//     }
//                               }
//                               void enableOpenGLDebug() {
//                                   glEnable(GL_DEBUG_OUTPUT);
//                                   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
//
//                                   glDebugMessageCallback(glDebugCallback, nullptr);
//
//                                   //  filter out notifications
//                                   // glDebugMessageControl(GL_DONT_CARE,  GL_DONT_CARE,
//                                   //                       GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr,
//                                   //                       GL_FALSE);
//                               }
//
//                               constexpr const char* RGFWDebugToString(RGFW_debugType type) {
//                                   switch (type) {
//                                       case RGFW_typeError:
//                                           return "Error";
//                                       case RGFW_typeWarning:
//                                           return "Warning";
//                                       case RGFW_typeInfo:
//                                           return "Info";
//                                       default:
//                                           return "UnknownDebugType";
//                                   }
//                               }
//
//
//                               constexpr const char* RGFWErrCodeToString(RGFW_errorCode err) {
//                                   switch (err) {
//                                       case RGFW_noError:
//                                           return "NoError";
//                                       case RGFW_errOutOfMemory:
//                                           return "OutOfMemory";
//                                       case RGFW_errOpenGLContext:
//                                           return "OpenGLContext";
//                                       case RGFW_errEGLContext:
//                                           return "EGLContext";
//                                       case RGFW_errWayland:
//                                           return "Wayland";
//                                       case RGFW_errX11:
//                                           return "X11";
//                                       case RGFW_errDirectXContext:
//                                           return "DirectXContext";
//                                       case RGFW_errIOKit:
//                                           return "IOKit";
//                                       case RGFW_errClipboard:
//                                           return "Clipboard";
//                                       case RGFW_errFailedFuncLoad:
//                                           return "FailedFuncLoad";
//                                       case RGFW_errBuffer:
//                                           return "Buffer";
//                                       case RGFW_errEventQueue:
//                                           return "EventQueue";
//                                       case RGFW_infoWindow:
//                                           return "InfoWindow";
//                                       case RGFW_infoBuffer:
//                                           return "InfoBuffer";
//                                       case RGFW_infoGlobal:
//                                           return "InfoGlobal";
//                                       case RGFW_infoOpenGL:
//                                           return "InfoOpenGL";
//
//                                       case RGFW_warningWayland:
//                                           return "WarningWayland";
//                                       case RGFW_warningOpenGL:
//                                           return "WarningOpenGL";
//
//                                       default:
//                                           return "UnknownErrorCode";
//                                   }
//                               }
//
//                               bool isFatalRGFWError(RGFW_errorCode err) {
//                                   switch (err) {
//                                       case RGFW_errOutOfMemory:
//                                       case RGFW_errOpenGLContext:
//                                       case RGFW_errEGLContext:
//                                       case RGFW_errDirectXContext:
//                                       case RGFW_errFailedFuncLoad:
//                                           return true;
//                                       default:
//                                           return false;
//                                   }
//                               }
//
//                               void engineRGFWDebugCallback(RGFW_debugType type, RGFW_errorCode err, const char* msg) {
//                                   switch (type) {
//                                       case RGFW_typeInfo:
//                                           LOGF_INFO("RGFW [{}]: {}", RGFWErrCodeToString(err), msg);
//                                           break;
//
//                                       case RGFW_typeWarning:
//                                           LOGF_WARN("RGFW [{}]: {}", RGFWErrCodeToString(err), msg);
//                                           break;
//
//                                       case RGFW_typeError:
//                                           LOGF_ERROR("RGFW [{}]: {}", RGFWErrCodeToString(err), msg);
//                                           #ifndef NDEBUG
//                                           // In debug builds, RGFW errors are invariants
//                                           if (isFatalRGFWError(err))
//                                               ASSERT(false && "Fatal RGFW error");
//                                       #endif
//                                       break;
//
//                                       default:
//                                           LOGF_WARN("RGFW: Unknown debug type {} (err={}): {}",
//                                                     static_cast<int>(type),
//                                                     static_cast<int>(err),
//                                                     msg);
//                                           break;
//                                   }
//                               }
//
//                               #define MULTILINE_STR(...) #__VA_ARGS__
//                               #define GLSL(code) R"GLSL(code)GLSL"
//                               #define CONCAT_IMPL(x, y) x##y
//                               #define CONCAT(x, y) CONCAT_IMPL(x, y)
//
//                               #define defer(code) auto CONCAT(_defer_, __LINE__) = ScopeExit([&]() { code; })
//                               template <typename F>
//                               struct ScopeExit {
//                                   F f;
//                                   ScopeExit(F&& f) : f(std::forward<F>(f)) {}
//                                   ~ScopeExit() { f(); }
//                               };
//
//                               #include <fstream>
//                               #include <sstream>
//                               #include <stdbool.h>
//                               #include <stdexcept>
//                               #include <string>
//                               #include <variant>
//                               #include <vector>
//
//                               enum class ShaderStage { Vertex, Fragment, Geometry, Compute, TessControl, TessEval };
//
//                               struct FileSource {
//                                   std::string path;
//                               };
//
//                               struct StringSource {
//                                   std::string code;
//                               };
//
//                               struct EmbeddedSource {
//                                   const char* code;
//                               };
//
//                               struct VertexAttribute {
//                                   GLuint    location;
//                                   GLint     size;
//                                   GLenum    type;
//                                   GLboolean normalized;
//                                   GLsizei   stride;
//                                   size_t    offset;
//                               };
//
//                               struct BufferData {
//                                   GLenum                 target;
//                                   std::vector<std::byte> bytes;
//                               };
//
//                               struct MeshSpec {
//                                   std::vector<BufferData>      buffers;
//                                   std::vector<VertexAttribute> attributes;
//                                   GLsizei                      indexCount = 0;
//                               };
//
//                               struct Mesh {
//                                   GLuint  vao        = 0;
//                                   GLuint  vbo        = 0;
//                                   GLuint  ebo        = 0;
//                                   GLsizei indexCount = 0;
//
//                                   Mesh() = default;
//
//                                   Mesh(const Mesh&)            = delete;
//                                   Mesh& operator=(const Mesh&) = delete;
//
//                                   Mesh(Mesh&& other) noexcept { *this = std::move(other); }
//
//                                   Mesh& operator=(Mesh&& other) noexcept {
//                                       destroy();
//                                       vao        = other.vao;
//                                       vbo        = other.vbo;
//                                       ebo        = other.ebo;
//                                       indexCount = other.indexCount;
//
//                                       other.vao = other.vbo = other.ebo = 0;
//                                       other.indexCount                  = 0;
//                                       return *this;
//                                   }
//
//                                   void draw() const {
//                                       ASSERT_ALWAYS(vao != 0);
//                                       glBindVertexArray(vao);
//                                       glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
//                                   }
//
//                                   void setDebugName(const char* name) {
//                                       glObjectLabel(GL_VERTEX_ARRAY, vao, -1, name);
//                                       glObjectLabel(GL_BUFFER, vbo, -1, "VBO");
//                                       glObjectLabel(GL_BUFFER, ebo, -1, "EBO");
//                                   }
//                                   ~Mesh() { destroy(); }
//
//                                   void destroy() {
//                                       if (vao)
//                                           glDeleteVertexArrays(1, &vao);
//                                       if (vbo)
//                                           glDeleteBuffers(1, &vbo);
//                                       if (ebo)
//                                           glDeleteBuffers(1, &ebo);
//
//                                       vao = vbo = ebo = 0;
//                                       indexCount      = 0;
//                                   }
//                               };
//                               struct Program {
//                                   GLuint id = 0;
//
//                                   Program() = default;
//                                   explicit Program(GLuint id) : id(id) {}
//
//                                   Program(const Program&)            = delete;
//                                   Program& operator=(const Program&) = delete;
//
//                                   Program(Program&& other) noexcept { *this = std::move(other); }
//
//                                   Program& operator=(Program&& other) noexcept {
//                                       destroy();
//                                       id       = other.id;
//                                       other.id = 0;
//                                       return *this;
//                                   }
//
//                                   ~Program() { destroy(); }
//
//                                   void destroy() {
//                                       if (id) {
//                                           glDeleteProgram(id);
//                                           id = 0;
//                                       }
//                                   }
//                                   void use() const { glUseProgram(id); }
//
//                                   void setDebugName(const char* name) { glObjectLabel(GL_PROGRAM, id, -1, name); }
//                                   // ---- uniform setters ----
//                                   void set(const char* name, int v) const { glUniform1i(uniformLocation(name), v); }
//
//                                   void set(const char* name, float v) const { glUniform1f(uniformLocation(name), v); }
//
//                                   void setVec4(const char* name, const float* v) const {
//                                       glUniform4fv(uniformLocation(name), 1, v);
//                                   }
//
//                                   void setMat4(const char* name, const float* m) const {
//                                       glUniformMatrix4fv(uniformLocation(name), 1, GL_FALSE, m);
//                                   }
//
//                                   mutable std::unordered_map<std::string, GLint> uniformCache;
//
//                                   GLint uniformLocation(const char* name) const {
//                                       auto it = uniformCache.find(name);
//                                       if (it != uniformCache.end())
//                                           return it->second;
//
//                                       GLint loc = glGetUniformLocation(id, name);
//
//                                       CHECK(loc != -1, "Uniform not found ({})", name);
//                                       uniformCache[name] = loc;
//                                       return loc;
//                                   }
//                               };
//
//                               struct MeshPipe {
//                                   MeshSpec spec;
//
//                                   MeshPipe addVBO(const void* data, size_t size) const {
//                                       MeshPipe next = *this;
//                                       next.spec.buffers.push_back(
//                                           {GL_ARRAY_BUFFER,
//                                               std::vector<std::byte>(static_cast<const std::byte*>(data),
//                                                                      static_cast<const std::byte*>(data) + size)});
//                                       return next;
//                                   }
//
//                                   MeshPipe addEBO(const void* data, size_t size, GLsizei indexCount) const {
//                                       MeshPipe next = *this;
//                                       next.spec.buffers.push_back(
//                                           {GL_ELEMENT_ARRAY_BUFFER,
//                                               std::vector<std::byte>(static_cast<const std::byte*>(data),
//                                                                      static_cast<const std::byte*>(data) + size)});
//                                       next.spec.indexCount = indexCount;
//                                       return next;
//                                   }
//
//                                   MeshPipe attrib(GLuint    location,
//                                                   GLint     size,
//                                                   GLenum    type,
//                                                   GLboolean normalized,
//                                                   GLsizei   stride,
//                                                   size_t    offset) const {
//                                                       MeshPipe next = *this;
//                                                       next.spec.attributes.push_back({location, size, type, normalized, stride, offset});
//                                                       return next;
//                                                   }
//                               };
//
//                               Mesh buildMesh(const MeshSpec& spec) {
//                                   Mesh mesh{};
//                                   glGenVertexArrays(1, &mesh.vao);
//                                   glBindVertexArray(mesh.vao);
//
//                                   for (const auto& buf : spec.buffers) {
//                                       GLuint id;
//                                       glGenBuffers(1, &id);
//                                       glBindBuffer(buf.target, id);
//                                       glBufferData(buf.target, buf.bytes.size(), buf.bytes.data(), GL_STATIC_DRAW);
//
//                                       if (buf.target == GL_ARRAY_BUFFER)
//                                           mesh.vbo = id;
//                                       else if (buf.target == GL_ELEMENT_ARRAY_BUFFER)
//                                           mesh.ebo = id;
//                                   }
//
//                                   for (const auto& attr : spec.attributes) {
//                                       glVertexAttribPointer(attr.location,
//                                                             attr.size,
//                                                             attr.type,
//                                                             attr.normalized,
//                                                             attr.stride,
//                                                             reinterpret_cast<void*>(attr.offset));
//                                       glEnableVertexAttribArray(attr.location);
//                                   }
//
//                                   glBindVertexArray(0);
//
//                                   mesh.indexCount = spec.indexCount;
//                                   return mesh;
//                               }
//
//                               using ShaderSource = std::variant<FileSource, StringSource, EmbeddedSource>;
//
//                               struct ShaderSpec {
//                                   ShaderStage  stage;
//                                   ShaderSource source;
//                               };
//
//                               using ShaderPipeline = std::vector<ShaderSpec>;
//
//                               constexpr GLenum toGLenum(ShaderStage stage) {
//                                   switch (stage) {
//                                       case ShaderStage::Vertex:
//                                           return GL_VERTEX_SHADER;
//                                       case ShaderStage::Fragment:
//                                           return GL_FRAGMENT_SHADER;
//                                       case ShaderStage::Geometry:
//                                           return GL_GEOMETRY_SHADER;
//                                       case ShaderStage::Compute:
//                                           return GL_COMPUTE_SHADER;
//                                       case ShaderStage::TessControl:
//                                           return GL_TESS_CONTROL_SHADER;
//                                       case ShaderStage::TessEval:
//                                           return GL_TESS_EVALUATION_SHADER;
//                                   }
//                                   throw std::logic_error("Unhandled ShaderStage");
//                               }
//
//                               static std::string readFile(const std::string& path) {
//                                   std::ifstream file(path);
//                                   if (!file)
//                                       throw std::runtime_error("Failed to open shader file: " + path);
//
//                                   std::stringstream buffer;
//                                   buffer << file.rdbuf();
//                                   return buffer.str();
//                               }
//
//                               static GLuint compileShader(GLenum type, const std::string& source) {
//                                   GLuint      shader = glCreateShader(type);
//                                   const char* src    = source.c_str();
//
//                                   glShaderSource(shader, 1, &src, nullptr);
//                                   glCompileShader(shader);
//
//                                   GLint ok = 0;
//                                   glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
//
//                                   if (!ok) {
//                                       GLint len = 0;
//                                       glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
//
//                                       std::string log(len, '\0');
//                                       glGetShaderInfoLog(shader, len, nullptr, log.data());
//
//                                       glDeleteShader(shader);
//                                       throw std::runtime_error("Shader compile error:\n" + log);
//                                   }
//
//                                   return shader;
//                               }
//                               struct ShaderModule {
//                                   GLuint      id = 0;
//                                   ShaderStage stage;
//
//                                   ShaderModule(GLuint id, ShaderStage stage) : id(id), stage(stage) {}
//
//                                   ShaderModule(const ShaderModule&)            = delete;
//                                   ShaderModule& operator=(const ShaderModule&) = delete;
//
//                                   ShaderModule(ShaderModule&& other) noexcept : id(other.id), stage(other.stage) { other.id = 0; }
//
//                                   ~ShaderModule() {
//                                       if (id)
//                                           glDeleteShader(id);
//                                   }
//                               };
//
//                               template <typename>
//                               inline constexpr bool always_false = false;
//                               struct ShaderLoader {
//                                   static ShaderModule load(ShaderStage stage, const ShaderSource& source) {
//                                       GLuint shader = std::visit(
//                                           [glStage = toGLenum(stage)](auto&& src) -> GLuint {
//                                               using T = std::decay_t<decltype(src)>;
//
//                                               if constexpr (std::is_same_v<T, FileSource>)
//                                                   return compileShader(glStage, readFile(src.path));
//
//                                               else if constexpr (std::is_same_v<T, StringSource>)
//                                                   return compileShader(glStage, src.code);
//
//                                               else if constexpr (std::is_same_v<T, EmbeddedSource>)
//                                                   return compileShader(glStage, src.code);
//
//                                               else
//                                                   static_assert(always_false<T>, "Unhandled ShaderSource");
//                                           },
//                                           source);
//
//                                       return ShaderModule{shader, stage};
//                                   }
//                               };
//                               struct ProgramPipe {
//                                   ShaderPipeline pipeline;
//
//                                   ProgramPipe add(ShaderStage stage, ShaderSource source) const {
//                                       ProgramPipe next = *this;
//                                       next.pipeline.push_back({stage, std::move(source)});
//                                       return next;
//                                   }
//
//                                   Program build() const {
//                                       if (pipeline.empty())
//                                           throw std::runtime_error("ProgramPipe: empty pipeline");
//
//                                       GLuint              program = glCreateProgram();
//                                       std::vector<GLuint> shaders;
//                                       shaders.reserve(pipeline.size());
//
//                                       // compile + attach
//                                       for (const auto& spec : pipeline) {
//                                           ShaderModule mod = ShaderLoader::load(spec.stage, spec.source);
//                                           glAttachShader(program, mod.id);
//                                           shaders.push_back(mod.id);
//                                       }
//
//                                       glLinkProgram(program);
//
//                                       GLint success = 0;
//                                       glGetProgramiv(program, GL_LINK_STATUS, &success);
//                                       if (!success) {
//                                           GLint len = 0;
//                                           glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
//                                           std::string log(len, '\0');
//                                           glGetProgramInfoLog(program, len, nullptr, log.data());
//
//                                           for (GLuint s : shaders)
//                                               glDeleteShader(s);
//
//                                           glDeleteProgram(program);
//                                           throw std::runtime_error("Program link error:\n" + log);
//                                       }
//
//                                       // shaders no longer needed
//                                       // foc> (GLuint s : shaders) {
//                                       //   glDetachShader(program, s);
//                                       //   glDeleteShader(s);
//                                       // }
//                                       //
//                                       return Program{program}; // RAII object
//                                   }
//                               };
//                               using UniformValue = std::variant<int, float, glm::vec4, glm::mat4>;
//
//                               struct TextureBinding {
//                                   GLuint texture;
//                                   GLenum target;
//                                   GLuint unit;
//                               };
//                               struct CachedUniform {
//                                   GLint        location = -1;
//                                   UniformValue value;
//                               };
//
//                               struct Material {
//                                   Program*                                       program; // non-owning reference
//                                   std::unordered_map<std::string, CachedUniform> uniforms;
//                                   std::vector<TextureBinding>                    textures;
//
//                                   void set(const std::string& name, UniformValue value) { uniforms[name].value = std::move(value); }
//
//                                   void resolveUniforms() {
//                                       ASSERT_ALWAYS(program);
//
//                                       for (auto& [name, u] : uniforms) {
//                                           u.location = program->uniformLocation(name.c_str());
//
//                                           #ifndef NDEBUG
//                                           if (u.location == -1) {
//                                               LOGF_WARN("Uniform '{}' not found in program {}", name, program->id);
//                                           }
//                                           #endif
//                                       }
//                                   }
//
//                                   void bind() const {
//                                       ASSERT_ALWAYS(program);
//                                       program->use();
//
//                                       for (const auto& [_, u] : uniforms) {
//                                           if (u.location == -1)
//                                               continue;
//
//                                           std::visit(
//                                               [&](auto&& v) {
//                                                   using T = std::decay_t<decltype(v)>;
//
//                                                   if constexpr (std::is_same_v<T, int>)
//                                                       glUniform1i(u.location, v);
//
//                                                   else if constexpr (std::is_same_v<T, float>)
//                                                       glUniform1f(u.location, v);
//
//                                                   else if constexpr (std::is_same_v<T, glm::vec4>)
//                                                       glUniform4fv(u.location, 1, glm::value_ptr(v));
//
//                                                   else if constexpr (std::is_same_v<T, glm::mat4>)
//                                                       glUniformMatrix4fv(u.location, 1, GL_FALSE, glm::value_ptr(v));
//                                               },
//                                               u.value);
//                                       }
//
//                                       for (const auto& t : textures) {
//                                           glActiveTexture(GL_TEXTURE0 + t.unit);
//                                           glBindTexture(t.target, t.texture);
//                                       }
//                                   }
//                               };
//
//                               struct Renderable {
//                                   const Mesh*     mesh     = nullptr;
//                                   const Material* material = nullptr;
//                                   glm::mat4       transform;
//
//                                   void draw() const {
//                                       ASSERT(mesh);
//                                       ASSERT(material);
//
//                                       material->bind();
//
//                                       // per-object uniforms
//                                       material->program->setMat4("u_Model", glm::value_ptr(transform));
//
//                                       mesh->draw();
//                                   }
//                               };
//
//                               struct Camera {
//                                   glm::vec3 position{0, 0, 3};
//
//                                   float yaw   = -90.0f; // degrees
//                                   float pitch = 0.0f;
//
//                                   float fov    = 60.0f; // degrees
//                                   float aspect = 16.0f / 9.0f;
//                                   float nearZ  = 0.1f;
//                                   float farZ   = 1000.0f;
//
//                                   glm::mat4 view;
//                                   glm::mat4 proj;
//                                   glm::mat4 viewProj;
//
//                                   void updateMatrices() {
//                                       glm::vec3 forward{cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
//                                           sin(glm::radians(pitch)),
//                                           sin(glm::radians(yaw)) * cos(glm::radians(pitch))};
//
//                                           glm::vec3 target = position + normalize(forward);
//                                           view             = glm::lookAt(position, target, glm::vec3{0, 1, 0});
//                                           proj             = glm::perspective(glm::radians(fov), aspect, nearZ, farZ);
//                                           viewProj         = proj * view;
//                                   }
//                                   glm::vec3 forward() const {
//                                       return glm::normalize(glm::vec3{cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
//                                           sin(glm::radians(pitch)),
//                                                             sin(glm::radians(yaw)) * cos(glm::radians(pitch))});
//                                   }
//
//                                   glm::vec3 right() const { return glm::normalize(glm::cross(forward(), glm::vec3{0, 1, 0})); }
//                               };
//
//                               struct FrameUniform {
//                                   GLuint buffer  = 0;
//                                   GLuint binding = 0;
//                                   size_t size    = 0;
//
//                                   FrameUniform() = default;
//                                   FrameUniform(GLuint buffer, GLuint binding, size_t size) :
//                                   buffer(buffer), binding(binding), size(size) {}
//
//                                   FrameUniform(const FrameUniform&)            = delete;
//                                   FrameUniform& operator=(const FrameUniform&) = delete;
//
//                                   FrameUniform(FrameUniform&& other) noexcept { *this = std::move(other); }
//
//                                   FrameUniform& operator=(FrameUniform&& other) noexcept {
//                                       destroy();
//                                       buffer       = other.buffer;
//                                       binding      = other.binding;
//                                       size         = other.size;
//                                       other.buffer = 0;
//                                       return *this;
//                                   }
//
//                                   void update(const void* data, size_t bytes, size_t offset = 0) const {
//                                       ASSERT_ALWAYS(buffer);
//                                       ASSERT_ALWAYS(offset + bytes <= size);
//                                       glBindBuffer(GL_UNIFORM_BUFFER, buffer);
//                                       glBufferSubData(GL_UNIFORM_BUFFER, offset, bytes, data);
//                                   }
//
//                                   void destroy() {
//                                       if (buffer)
//                                           glDeleteBuffers(1, &buffer);
//                                       buffer = 0;
//                                   }
//
//                                   ~FrameUniform() { destroy(); }
//                               };
//                               struct FrameUniformSpec {
//                                   GLuint binding = 0;
//                                   size_t size    = 0;
//                               };
//                               struct FrameUniformPipe {
//                                   FrameUniformSpec spec;
//
//                                   FrameUniformPipe binding(GLuint b) const {
//                                       FrameUniformPipe next = *this;
//                                       next.spec.binding     = b;
//                                       return next;
//                                   }
//
//                                   FrameUniformPipe size(size_t s) const {
//                                       FrameUniformPipe next = *this;
//                                       next.spec.size        = s;
//                                       return next;
//                                   }
//
//                                   FrameUniform build() const {
//                                       ASSERT_ALWAYS(spec.size > 0);
//
//                                       GLuint ubo = 0;
//                                       glGenBuffers(1, &ubo);
//                                       glBindBuffer(GL_UNIFORM_BUFFER, ubo);
//                                       glBufferData(GL_UNIFORM_BUFFER, spec.size, nullptr, GL_DYNAMIC_DRAW);
//                                       glBindBufferBase(GL_UNIFORM_BUFFER, spec.binding, ubo);
//
//                                       return FrameUniform{ubo, spec.binding, spec.size};
//                                   }
//                               };
//                               struct RenderPassSpec {
//                                   Camera*                  camera = nullptr;
//                                   FrameUniform             frameUniform;
//                                   std::vector<Renderable*> objects;
//                               };
//
//                               struct RenderPass {
//                                   Camera*      camera = nullptr;
//                                   FrameUniform frameUniform;
//
//                                   std::vector<Renderable*> objects;
//
//                                   void render() {
//                                       ASSERT(camera);
//
//                                       camera->updateMatrices();
//                                       frameUniform.update(glm::value_ptr(camera->viewProj), sizeof(glm::mat4));
//
//                                       for (Renderable* r : objects)
//                                           r->draw();
//                                   }
//                               };
//                               struct RenderPassPipe {
//                                   RenderPassSpec spec;
//
//                                   RenderPassPipe()                            = default;
//                                   RenderPassPipe(RenderPassPipe&&)            = default;
//                                   RenderPassPipe& operator=(RenderPassPipe&&) = default;
//
//                                   RenderPassPipe(const RenderPassPipe&)            = delete;
//                                   RenderPassPipe& operator=(const RenderPassPipe&) = delete;
//
//                                   RenderPassPipe&& camera(Camera* cam) && {
//                                       spec.camera = cam;
//                                       return std::move(*this);
//                                   }
//
//                                   RenderPassPipe&& frameUniform(FrameUniform&& fu) && {
//                                       spec.frameUniform = std::move(fu);
//                                       return std::move(*this);
//                                   }
//
//                                   RenderPassPipe&& add(Renderable& r) && {
//                                       spec.objects.push_back(&r);
//                                       return std::move(*this);
//                                   }
//
//                                   RenderPassPipe&& add(std::span<Renderable> rs) && {
//                                       for (auto& r : rs)
//                                           spec.objects.push_back(&r);
//                                       return std::move(*this);
//                                   }
//
//                                   RenderPass build() && {
//                                       ASSERT_ALWAYS(spec.camera);
//                                       ASSERT_ALWAYS(spec.frameUniform.buffer);
//
//                                       return RenderPass{.camera       = spec.camera,
//                                           .frameUniform = std::move(spec.frameUniform),
//                                           .objects      = std::move(spec.objects)};
//                                   }
//                               };
//
//                               // settings
//                               const int SCR_WIDTH  = 1920;
//                               const int SCR_HEIGHT = 1080;
//
//                               constexpr const char* vertexShaderSource = R"GLSL(
// #version 430 core
// layout(std140, binding = 0) uniform Camera {
//   mat4 u_ViewProj;
// };
// layout (location = 0) in vec3 a_Position;
//
// uniform mat4 u_Model;    // object transform (per-draw)
//
// void main() {
//     gl_Position = u_ViewProj * u_Model * vec4(a_Position, 1.0);
// }
// )GLSL";
//
// // const char *fragmentShaderSource = MULTILINE_STR(
// // \x23versioc> 330 core\n out vec4 FragColor;
// //   void main() { FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f); });
// constexpr const char* fragmentShaderSource = R"GLSL(
// #version 430 core
//
// out vec4 FragColor;
//
// uniform vec4 u_Color; // material color
//
// void main() {
//     FragColor = u_Color;
// }
// )GLSL";
// static Camera*        gCamera              = nullptr;
// void                  mousePosCallback(RGFW_window* w, int x, int y, float dx, float dy) {
//     if (!gCamera)
//         return;
//
//     constexpr float sensitivity = 0.1f;
//
//     gCamera->yaw += dx * sensitivity;
//     gCamera->pitch -= dy * sensitivity;
//
//     gCamera->pitch = glm::clamp(gCamera->pitch, -89.0f, 89.0f);
// }
//
// int main(void) {
//     // --- OpenGL hints ---
//     RGFW_setDebugCallback(engineRGFWDebugCallback);
//
//     RGFW_glHints* hints   = RGFW_getGlobalHints_OpenGL();
//     hints->major          = 4;
//     hints->minor          = 3;
//     RGFW_setGlobalHints_OpenGL(hints);
//     // --- Window creation ---
//     RGFW_window* window = RGFW_createWindow("Tessera Renderer",
//                                             SCR_WIDTH,
//                                             SCR_HEIGHT,
//                                             SCR_WIDTH,
//                                             SCR_HEIGHT,
//                                             RGFW_windowAllowDND | RGFW_windowCenter |
//                                             RGFW_windowScaleToMonitor | RGFW_windowOpenGL |  RGFW_windowHideMouse);
//     RGFW_window_captureRawMouse(window, RGFW_TRUE);
//     RGFW_window_setRawMouseMode(window, RGFW_TRUE);
//     ASSERT_ALWAYS(window && "Failed to create RGFW window");
//
//     defer(RGFW_window_close(window));
//
//     RGFW_window_setExitKey(window, RGFW_escape);
//     RGFW_window_makeCurrentContext_OpenGL(window);
//     // window->_winArgs |= RGFW_HOLD_MOUSE;
//     // RGFW_captureCursor(window);
//     #ifndef RGFW_WASM
//
//     ASSERT_ALWAYS(!RGL_loadGL3((RGLloadfunc)RGFW_getProcAddress_OpenGL) &&
//     "Failed to initialize OpenGL loader");
//     #endif
//     #ifndef NDEBUG
//     enableOpenGLDebug();
//     #endif
//
//     std::vector<Renderable> renderQueue;
//     Camera                  camera;
//     gCamera = &camera;
//
//     // RGFW_mousePosCallbackSrc = mousePosCallback;
//
//     RGFW_setMousePosCallback(mousePosCallback);
//
//     Program shaderProgram = ProgramPipe{}
//     .add(ShaderStage::Vertex, EmbeddedSource{vertexShaderSource})
//     .add(ShaderStage::Fragment, EmbeddedSource{fragmentShaderSource})
//     .build();
//     // optional
//     defer(shaderProgram.destroy());
//
//
//     Material mat;
//     mat.program = &shaderProgram;
//     mat.set("u_Color", glm::vec4{1, 0.9f, 0.2f, 1});
//     mat.resolveUniforms();
//
//     // --- Mesh data ---
//     float vertices[] = {0.5f, 0.5f, 0.0f, 0.5f, -0.5f, 0.0f, -0.5f, -0.5f, 0.0f, -0.5f, 0.5f, 0.0f};
//
//     unsigned int indices[] = {0, 1, 3, 1, 2, 3};
//
//     Mesh quad = buildMesh(MeshPipe{}
//     .addVBO(vertices, sizeof(vertices))
//     .addEBO(indices, sizeof(indices), 6)
//     .attrib(0, // location
//             3, // vec3
//             GL_FLOAT,
//             GL_FALSE,
//             3 * sizeof(float),
//             0)
//     .spec);
//     // optional
//     defer(quad.destroy());
//
//     renderQueue.push_back({.mesh = &quad, .material = &mat, .transform = glm::identity<glm::mat4>()});
//
//     FrameUniform cameraViewUniform = FrameUniformPipe{}.binding(0).size(sizeof(glm::mat4)).build();
//
//     RenderPass pass = RenderPassPipe{}
//     .camera(&camera)
//     .frameUniform(std::move(cameraViewUniform))
//     .add(renderQueue)
//     .build();
//
//     float sensitivity = 0.5f;
//     float speed       = .5f;
//     //  --- Render loop ---
//     while (RGFW_window_shouldClose(window) == RGFW_FALSE) {
//         RGFW_event event;
//         while (RGFW_window_checkEvent(window, &event)) {
//             if (event.type == RGFW_quit)
//                 break;
//         }
//
//         glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
//         // ic> (event.type == RGFW_mousePosChanged) {
//         //   float dx = event.mouse.vecX;
//         //   float dy = event.mouse.vecY;
//         //
//         //   dx *= sensitivity;
//         //   dy *= sensitivity;
//         //
//         //   camera.yaw += dx;
//         //   camera.pitch -= dy; // invert Y for natural look
//         //
//         //   LOGF_INFO("camera pos {} {}", camera.position.x, camera.position.y);
//         //   camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);
//         // }
//         if (RGFW_isKeyDown(RGFW_w))
//             camera.position += camera.forward() * speed;
//
//         if (RGFW_isKeyDown(RGFW_s))
//             camera.position -= camera.forward() * speed;
//
//         if (RGFW_isKeyDown(RGFW_a))
//             camera.position -= camera.right() * speed;
//
//         if (RGFW_isKeyDown(RGFW_d))
//             camera.position += camera.right() * speed;
//
//         if (RGFW_isKeyDown(RGFW_space))
//             camera.position.y += speed;
//
//         if (RGFW_isKeyDown(RGFW_z))
//             camera.position.y -= speed;
//
//         // LOGF_INFO("camera pos {} {}", camera.position.x, camera.position.y);
//         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//         pass.render();
//
//         RGFW_window_swapBuffers_OpenGL(window);
//     }
//
//     return 0;
// }

#include "platform/MacOS/MacOSWindow.h"

#include "core/base.h"
#include "events/window_events.h"
#include "events/event_bus.h"
#include <spdlog/spdlog.h>

namespace slim
{
  MacOSWindow::MacOSWindow(std::string_view title, uint16_t width, uint16_t height, bool vsync)
  {
    m_properties = WindowProperties(title, width, height, vsync);
    init();
  }

  MacOSWindow::~MacOSWindow()
  {
    destroy();
  }

  bool MacOSWindow::shouldClose()
  {
    return glfwWindowShouldClose(m_window);
  }

  void MacOSWindow::update()
  {
    glfwPollEvents();
    glfwSwapBuffers(m_window);
  }

  void* MacOSWindow::getNative() const
  {
    return m_window;
  }

  WindowProperties MacOSWindow::getProperties() const
  {
    return m_properties;
  }

  glm::vec2 MacOSWindow::getDimensions() const
  {
    return glm::vec2{m_properties.width, m_properties.height};
  }

  void MacOSWindow::setWidth(float width)
  {
    m_properties.width = width;
    glViewport(0, 0, m_properties.width, m_properties.height);
  }
 
  void MacOSWindow::setHeight(float height)
  {
    m_properties.height = height;
    glViewport(0, 0, m_properties.width, m_properties.height);
  }

  bool MacOSWindow::getVsync() const
  {
    return m_properties.vsync;
  }

  void MacOSWindow::setVsync(bool value)
  {
    m_properties.vsync = value;
    glfwSwapInterval(m_properties.vsync ? 1 : 0);
  }

  void MacOSWindow::init()
  {
    glfwSetErrorCallback(glfwErrorCallback);

    if (!glfwInit())
      SLIM_ASSERT(false, "Failed to initialize GLFW")

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #ifdef SLIM_PLATFORM_MACOS
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    m_window = glfwCreateWindow(m_properties.width, m_properties.height, m_properties.title.c_str(), NULL, NULL);

    if (!m_window)
      SLIM_ASSERT(false, "Failed to create GLFW window")

    glfwSetWindowUserPointer(m_window, this);
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(m_properties.vsync ? 1 : 0);
    glfwSetFramebufferSizeCallback(m_window, glfwFramebufferSizeCallback);
    glfwSetWindowCloseCallback(m_window, glfwWindowCloseCallback);

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
      SLIM_ASSERT(false, "Failed to initialize OpenGL context")

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(m_window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    spdlog::info("Loaded OpenGL {}.{}", GLAD_VERSION_MAJOR(version), GLAD_VERSION_MINOR(version));
  }

  void MacOSWindow::destroy()
  {
    glfwDestroyWindow(m_window);
    glfwTerminate();
  }

  void MacOSWindow::glfwErrorCallback(int error, const char* description)
  {
    spdlog::error("GLFW Error {}: {}", error, description);
  }

  void MacOSWindow::glfwFramebufferSizeCallback(GLFWwindow* window, int width, int height)
  {
    MacOSWindow abstractWindow = *(MacOSWindow*)glfwGetWindowUserPointer(window);
    glm::vec2 size{width, height};

    // TODO: Create new method "setDimensions" and call it with "size" to prevent calling setWidth + setHeight and therefore glViewport twice
    //       Also update m_properties.
    abstractWindow.setWidth(width);
    abstractWindow.setHeight(height);

    WindowResizeEvent e{ &abstractWindow, size };
    EventBus::post(e);
  }

  void MacOSWindow::glfwWindowCloseCallback(GLFWwindow* window)
  {
    MacOSWindow abstractWindow = *(MacOSWindow*)glfwGetWindowUserPointer(window);
    WindowCloseEvent e{ &abstractWindow };
    EventBus::post(e);
  }
}
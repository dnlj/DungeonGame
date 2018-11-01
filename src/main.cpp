// Windows
#include <Windows.h>

// STD
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>

// glLoadGen
#include <glloadgen/gl_core_4_5.hpp>

// GLFW
#include <GLFW/glfw3.h>

// SOIL
#include <SOIL.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Meta
#include <Meta/TypeSet/TypeSet.hpp>

// ImGui
#include <imgui.h>

// Engine
#include <Engine/Engine.hpp>
#include <Engine/Debug/Debug.hpp>
#include <Engine/SystemBase.hpp>
#include <Engine/TextureManager.hpp>
#include <Engine/Utility/Utility.hpp>
#include <Engine/ECS/World.hpp>
#include <Engine/EngineInstance.hpp>
#include <Engine/Camera.hpp>
#include <Engine/ResourceManager.hpp>

// Game
#include <Game/Common.hpp>
#include <Game/SpriteSystem.hpp>
#include <Game/CameraTrackingSystem.hpp>
#include <Game/CharacterSpellSystem.hpp>
#include <Game/CharacterSpellComponent.hpp>
#include <Game/CharacterMovementSystem.hpp>
#include <Game/MapRenderSystem.hpp>
#include <Game/MapSystem.hpp>
#include <Game/CharacterMovementComponent.hpp>
#include <Game/SpriteComponent.hpp>
#include <Game/InputComponent.hpp>
#include <Game/PhysicsComponent.hpp>
#include <Game/imgui_impl_glfw_gl3.hpp>


namespace {
	constexpr int OPENGL_VERSION_MAJOR = 4;
	constexpr int OPENGL_VERSION_MINOR = 5;

	void editorUI();

	void initializeOpenGL() {
		auto loaded = ogl_LoadFunctions();

		if (loaded == ogl_LOAD_FAILED) {
			ENGINE_ERROR("[glLoadGen] initialization failed.");
		}

		auto failed = loaded - ogl_LOAD_SUCCEEDED;
		if (failed > 0) {
			ENGINE_ERROR("[glLoadGen] Failed to load " << failed << " functions.");
		}


		if (!ogl_IsVersionGEQ(OPENGL_VERSION_MAJOR, OPENGL_VERSION_MINOR)) {
			ENGINE_ERROR("[glLoadGen] OpenGL version " << OPENGL_VERSION_MAJOR << "." << OPENGL_VERSION_MINOR << " is not available.");
		}
	}

	GLFWwindow* createWindow() {
		// GLFW hints
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, OPENGL_VERSION_MAJOR);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, OPENGL_VERSION_MINOR);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		#if defined(DEBUG)
			glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		#endif
		
		glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
		glfwWindowHint(GLFW_DECORATED, GL_TRUE);
		glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

		glfwWindowHint(GLFW_RED_BITS, 8);
		glfwWindowHint(GLFW_GREEN_BITS, 8);
		glfwWindowHint(GLFW_BLUE_BITS, 8);
		glfwWindowHint(GLFW_ALPHA_BITS, 8);
		glfwWindowHint(GLFW_DEPTH_BITS, 32);

		// Create a window
		constexpr int width = 1280;
		constexpr int height = 720;
		auto window = glfwCreateWindow(width, height, "Window Title", nullptr, nullptr);

		if (!window) {
			ENGINE_ERROR("[GLFW] Failed to create window.");
		}

		{ // Position the window
			auto mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
			glfwSetWindowPos(window, mode->width/2 - width/2, mode->height/2 - height/2);
		}

		return window;
	}

	void initImGui(GLFWwindow* window) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGui_ImplGlfwGL3_Init(window, false);
		ImGui::StyleColorsDark();
	}

	void doUI() {
		static bool showWindow = true;

		ImGui_ImplGlfwGL3_NewFrame();

		ImGui::ShowDemoWindow(&showWindow);
		editorUI();

		ImGui::Render();
		ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());
	}

	b2Body* createPhysicsCircle(Engine::ECS::Entity ent, Game::PhysicsSystem& physSys, b2Vec2 position = b2Vec2_zero) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = position;

		b2Body* body = physSys.createBody(ent, bodyDef);

		b2CircleShape shape;
		shape.m_radius = 1.0f/8;

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
		body->SetLinearDamping(10.0f);
		body->SetFixedRotation(true);

		return body;
	}

	b2Body* createPhysicsSquare(Engine::ECS::Entity ent, Game::PhysicsSystem& physSys, b2Vec2 position = b2Vec2_zero) {
		b2BodyDef bodyDef;
		bodyDef.type = b2_dynamicBody;
		bodyDef.position = position;

		b2Body* body = physSys.createBody(ent, bodyDef);

		b2PolygonShape shape;
		shape.SetAsBox(1.0f/8, 1.0f/8);

		b2FixtureDef fixtureDef;
		fixtureDef.shape = &shape;
		fixtureDef.density = 1.0f;

		body->CreateFixture(&fixtureDef);
		body->SetLinearDamping(10.0f);
		body->SetFixedRotation(true);

		return body;
	}

	void framebufferCallback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);

		auto& camera = static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->camera;
		camera.setAsOrtho(width, height, 1.0f / 250.0f);
	}
}

namespace {
	void editorUI() {
		bool open = true;
		ImGui::Begin("Editor UI", &open, ImGuiWindowFlags_MenuBar);

		ImGui::Text("test");

		ImGui::End();
	}
}

void run() {
	// GLFW error callback
	glfwSetErrorCallback([](int error, const char* desc) {
		ENGINE_ERROR("[GLFW] " << desc);
	});

	// Initialize GLFW
	if (!glfwInit()) {
		ENGINE_ERROR("[GLFW] Failed to initialize.");
	}

	// Create a window
	GLFWwindow* window = createWindow();
	glfwMakeContextCurrent(window);
	
	// Enable vsync
	glfwSwapInterval(1);

	// Initialize OpenGL functions
	initializeOpenGL();

	// OpenGL debug message
	#if defined(DEBUG_GRAPHICS)
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(Engine::Debug::GL::debugMessageCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	#endif

	glEnable(GL_FRAMEBUFFER_SRGB);

	// UI
	initImGui(window);

	// Engine stuff
	Engine::EngineInstance engine;
	Game::World world;

	{
		auto& physSys = world.getSystem<Game::PhysicsSystem>();
		world.getSystem<Game::SpriteSystem>().setup(engine);
		world.getSystem<Game::CameraTrackingSystem>().setup(engine.camera);
		world.getSystem<Game::CharacterSpellSystem>().setup(engine);
		world.getSystem<Game::MapSystem>().setup(engine);
		
		// Player
		auto player = world.createEntity();
		world.addComponent<Game::SpriteComponent>(player).texture = engine.textureManager.get("../assets/player.png");
		world.addComponent<Game::PhysicsComponent>(player).body = createPhysicsCircle(player, physSys);
		world.addComponent<Game::CharacterMovementComponent>(player);
		world.addComponent<Game::CharacterSpellComponent>(player);
		world.addComponent<Game::InputComponent>(player).inputManager = &engine.inputManager;

		world.getSystem<Game::CameraTrackingSystem>().focus = player;
	}

	// Binds
	engine.inputManager.bindkey(17, "MoveUp");
	engine.inputManager.bindkey(31, "MoveDown");
	engine.inputManager.bindkey(30, "MoveLeft");
	engine.inputManager.bindkey(32, "MoveRight");
	engine.inputManager.bindkey(57, "Spell_1");
	engine.inputManager.bindMouseButton(0, "edit_place");
	engine.inputManager.bindMouseButton(1, "edit_remove");

	// Callbacks
	glfwSetWindowUserPointer(window, &engine);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
			glfwSetWindowShouldClose(window, true);
		}

		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.keyCallback(scancode, action);

		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	});

	glfwSetCursorPosCallback(window, [](GLFWwindow* window, double x, double y) {
		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.mouseCallback(x, y);
	});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods){
		static_cast<Engine::EngineInstance*>(glfwGetWindowUserPointer(window))->inputManager.mouseCallback(button, action);
		ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	});

	// Framebuffer callback
	glfwSetFramebufferSizeCallback(window, framebufferCallback);
	{
		int w;
		int h;
		glfwGetFramebufferSize(window, &w, &h);
		framebufferCallback(window, w, h);
	}

	// ImGui callbacks
	glfwSetScrollCallback(window, ImGui_ImplGlfw_ScrollCallback);
	glfwSetCharCallback(window, ImGui_ImplGlfw_CharCallback);

	// Main loop
	auto startTime = std::chrono::high_resolution_clock::now();
	auto lastUpdate = startTime;
	while (!glfwWindowShouldClose(window)) {
		// Get the elapsed time in seconds
		auto diff = std::chrono::high_resolution_clock::now() - startTime;
		startTime = std::chrono::high_resolution_clock::now();
		auto dt = std::chrono::duration_cast<
			std::chrono::duration<
				float,
				std::chrono::seconds::period
			>
		>(diff).count();
		
		// Update frame time
		if ((std::chrono::high_resolution_clock::now() - lastUpdate) > std::chrono::seconds{1}) {
			glfwSetWindowTitle(window, std::to_string(dt).c_str());
			lastUpdate = std::chrono::high_resolution_clock::now();
		}

		// Rendering
		glClearColor(0.2176f, 0.2176f, 0.2176f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// ECS
		world.run(dt);
		engine.inputManager.update();

		// Physics debug
		#if defined (DEBUG_PHYSICS)
			world.getSystem<Game::PhysicsSystem>().getDebugDraw().draw(engine.camera.getProjection(), engine.camera.getView());
		#endif

		//std::this_thread::sleep_for(std::chrono::milliseconds{70});

		// Draw UI
		doUI();

		// GLFW
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// UI cleanup
	ImGui_ImplGlfwGL3_Shutdown();
	ImGui::DestroyContext();

	// GLFW cleanup
	glfwDestroyWindow(window);
}


int main(int argc, char* argv[]) {
	std::atexit([](){
		glfwTerminate();
	});

	{ // Position the console
		auto window = GetConsoleWindow();
		SetWindowPos(window, HWND_TOP, 0, 0, 1000, 500, 0);
	}

	run();

	std::cout << "Done." << std::endl;
	return EXIT_SUCCESS;
}

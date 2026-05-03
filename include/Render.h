#ifndef RENDER_H
#define RENDER_H

// GLAD must be included before GLFW — it loads the actual OpenGL function pointers.
// If you include GLFW first, it pulls in its own GL header and conflicts with GLAD.
#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "Types.h"   // for BodyData and vec3d

#include <vector>
#include <string>

// Represents the horizontal gap between two consecutive ground contacts.
// Used by drawBounceLabels to position distance text on the ground.
struct BounceSegment {
    float xStart;  // world x of launch / previous bounce
    float xEnd;    // world x of this bounce landing
};

// ─────────────────────────────────────────────
//  Renderer
//  Wraps all OpenGL and GLFW calls so that the
//  rest of the engine never has to touch GL directly.
//
//  Typical usage each frame:
//      renderer.beginFrame();
//      renderer.drawGround();
//      renderer.drawBall(body, radius);
//      renderer.drawTrail(trail);        // optional
//      renderer.endFrame();
// ─────────────────────────────────────────────
class Renderer
{
public:

    // ── Construction / destruction ─────────────
    // windowWidth / windowHeight : pixel size of the OS window
    // title                      : text shown in the title bar
    Renderer(int windowWidth, int windowHeight, const std::string& title);
    ~Renderer();

    GLFWwindow* getWindow() const { return m_window; }

    // Window state
    // Returns true while the window is open and the user has not pressed ESC / clicked X
    bool isRunning() const;

    // World-space coordinate mapping
    // The physics engine works in metres starting at (0,0).
    // OpenGL's normalised device coordinates (NDC) go from -1 to +1.
    // We define a "world view" rectangle so physics units map naturally.
    //
    //   worldWidth  : how many metres fit across the full window width
    //   worldHeight : how many metres fit across the full window height
    //   originX     : where world x=0 sits on screen (0.0 = left edge, 0.5 = centre)
    //   originY     : where world y=0 sits on screen (0.0 = bottom edge)
    //
    // Example: setWorldView(100, 50, 0.05f, 0.1f)
    //   -> 100 m wide, 50 m tall, launch point near bottom-left.
    void setWorldView(float worldWidth, float worldHeight,
        float originX = 0.05f, float originY = 0.1f);

    // ── Per-frame calls ────────────────────────

    // Clear the screen to the background colour. Call this first each frame.
    void beginFrame();

    // Draw a horizontal ground line across the bottom of the world view.
    // colour: RGB floats in [0,1], e.g. {0.4f, 0.8f, 0.3f} for green
    void drawGround(float r = 0.35f, float g = 0.75f, float b = 0.25f);

    // Draw the projectile ball at its current physics position.
    // body   : the BodyData whose position.x / position.y are read
    // radius : visual radius in world metres (e.g. 0.5f)
    // r,g,b  : fill colour
    void drawBall(const BodyData& body, float radius = 0.5f,
        float r = 0.9f, float g = 0.6f, float b = 0.1f);

    // Draw the trajectory trail as a polyline.
    // Pass a std::vector of past positions recorded each timestep.
    // Fades from opaque at the oldest point to bright at the newest.
    void drawTrail(const std::vector<vec3d>& trail,
        float r = 0.3f, float g = 0.7f, float b = 1.0f);

    // Draw a simple cannon at the launch origin (decorative).
    void drawCannon(float worldX, float worldY, float angle_deg);

    // Draw a small HUD box showing live angle and speed.
    void drawHUD(float angle_deg, float speed);

    // Draw a stats box in the top-right showing max height and horizontal distance.
    void drawStatsBox(float maxHeight, float totalHorizDist);

    // Draw horizontal bounce-gap distances as numbers midway between each bounce,
    // slightly below the ground line. Only shown when C is toggled on.
    void drawBounceLabels(const std::vector<BounceSegment>& segs);

    // Draws a single bitmap character at pixel position (px, py).
    void drawChar(char c, float px, float py,
        float r, float g, float b, float scale = 1.0f);

    // Draws a string of bitmap characters at pixel position (px, py).
    void drawText(const std::string& s, float px, float py,
        float r, float g, float b, float scale = 1.0f);

    // Converts a world-space position (metres) to OpenGL NDC [-1, +1].
    // This is the core coordinate transform used by every draw call.
    void worldToNDC(float wx, float wy, float& ndcX, float& ndcY) const;


    // Swap front/back buffers and poll OS events. Call this last each frame.
    void endFrame();

private:

    // ── Internal helpers ───────────────────────

    
    // Draws a filled circle centred at NDC position (cx, cy).
    // segments: how many triangles approximate the circle — 32 is smooth enough.
    void drawCircleNDC(float cx, float cy, float radiusNDC,
        float r, float g, float b, int segments = 32);

    // Draws a line from NDC (x1,y1) to (x2,y2).
    void drawLineNDC(float x1, float y1, float x2, float y2,
        float r, float g, float b, float alpha = 1.0f);

    // Converts pixel coordinates (top-left origin) to OpenGL NDC [-1, +1].
    void pixelToNDC(float px, float py, float& ndcX, float& ndcY) const;

    // Draws a filled rectangle in pixel coordinates (top-left origin, y grows down).
    void drawRectPx(float x, float y, float w, float h,
        float r, float g, float b, float a = 1.0f);

    // Draws a line using pixel coordinates.
    void drawLinePx(float x1, float y1, float x2, float y2,
        float r, float g, float b, float a = 1.0f);

    
    // ── Members ────────────────────────────────
    GLFWwindow* m_window = nullptr;

    int   m_windowW = 800;
    int   m_windowH = 600;

    // World-view parameters (set by setWorldView)
    float m_worldW = 100.0f;   // metres across full width
    float m_worldH = 50.0f;   // metres across full height
    float m_originX = 0.05f;  // fractional x position of world origin on screen
    float m_originY = 0.10f;  // fractional y position of world origin on screen
};

#endif // RENDER_H
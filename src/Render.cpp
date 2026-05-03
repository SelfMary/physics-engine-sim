#include "Render.h"
#include <cmath>
#include <iostream>
#include <sstream>
#include <iomanip>

// ─────────────────────────────────────────────────────────────────────────────
//  Window resize callback — keeps the viewport filling the whole window
// ─────────────────────────────────────────────────────────────────────────────
static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────
Renderer::Renderer(int windowWidth, int windowHeight, const std::string& title)
    : m_windowW(windowWidth), m_windowH(windowHeight)
{
    if (!glfwInit()) {
        std::cerr << "[Render] GLFW init failed\n";
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);

    m_window = glfwCreateWindow(windowWidth, windowHeight, title.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "[Render] Window creation failed\n";
        glfwTerminate();
        return;
    }

    glfwMakeContextCurrent(m_window);
    glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "[Render] GLAD init failed\n";
        return;
    }

    glViewport(0, 0, windowWidth, windowHeight);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::cout << "[Render] OpenGL " << glGetString(GL_VERSION) << "\n";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Destructor
// ─────────────────────────────────────────────────────────────────────────────
Renderer::~Renderer()
{
    if (m_window) glfwDestroyWindow(m_window);
    glfwTerminate();
}

bool Renderer::isRunning() const
{
    return m_window && !glfwWindowShouldClose(m_window);
}

void Renderer::setWorldView(float worldWidth, float worldHeight,
    float originX, float originY)
{
    m_worldW = worldWidth;
    m_worldH = worldHeight;
    m_originX = originX;
    m_originY = originY;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Coordinate helpers
// ─────────────────────────────────────────────────────────────────────────────

// World metres → OpenGL NDC [-1, +1]
void Renderer::worldToNDC(float wx, float wy, float& ndcX, float& ndcY) const
{
    float fx = (wx / m_worldW) + m_originX;
    float fy = (wy / m_worldH) + m_originY;
    ndcX = fx * 2.0f - 1.0f;
    ndcY = fy * 2.0f - 1.0f;
}

// Pixel coords (top-left origin) → OpenGL NDC
void Renderer::pixelToNDC(float px, float py, float& ndcX, float& ndcY) const
{
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    ndcX = (px / w) * 2.0f - 1.0f;
    ndcY = -((py / h) * 2.0f - 1.0f);   // flip y: pixels go down, NDC goes up
}

// ─────────────────────────────────────────────────────────────────────────────
//  beginFrame — update stored size and clear screen
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::beginFrame()
{
    glfwGetFramebufferSize(m_window, &m_windowW, &m_windowH);
    glClearColor(0.08f, 0.10f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawGround
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawGround(float r, float g, float b)
{
    float x1, y0, x2, dummy;
    worldToNDC(0.0f, 0.0f, x1, y0);
    worldToNDC(m_worldW, 0.0f, x2, dummy);
    glLineWidth(2.0f);
    drawLineNDC(x1, y0, x2, y0, r, g, b, 1.0f);
    glLineWidth(1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawBall
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawBall(const BodyData& body, float radius,
    float r, float g, float b)
{
    float cx, cy;
    worldToNDC(body.position.x, body.position.y, cx, cy);
    float radiusNDC = (radius / m_worldW) * 2.0f;
    drawCircleNDC(cx, cy, radiusNDC, r, g, b);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawTrail
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawTrail(const std::vector<vec3d>& trail,
    float r, float g, float b)
{
    if (trail.size() < 2) return;
    int n = (int)trail.size();
    for (int i = 1; i < n; i++) {
        float x1, y1, x2, y2;
        worldToNDC(trail[i - 1].x, trail[i - 1].y, x1, y1);
        worldToNDC(trail[i].x, trail[i].y, x2, y2);
        drawLineNDC(x1, y1, x2, y2, r, g, b, 0.85f);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawCannon
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawCannon(float worldX, float worldY, float angle_deg)
{
    float cx, cy;
    worldToNDC(worldX, worldY, cx, cy);

    float baseRadius = (3.6f / m_worldW) * 2.0f;
    drawCircleNDC(cx, cy, baseRadius, 0.25f, 0.25f, 0.28f);

    float angle_rad = angle_deg * 3.14159265f / 180.0f;
    float barrelLen = (4.5f / m_worldW) * 2.0f;

    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    float aspect = (float)w / (float)h;

    float tipX = cx + cosf(angle_rad) * barrelLen;
    float tipY = cy + sinf(angle_rad) * barrelLen * aspect;

    glLineWidth(10.0f);
    drawLineNDC(cx, cy, tipX, tipY, 0.35f, 0.35f, 0.38f, 1.0f);
    glLineWidth(1.0f);
}

// ─────────────────────────────────────────────────────────────────────────────
//  endFrame
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::endFrame()
{
    glfwSwapBuffers(m_window);
    glfwPollEvents();
    if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(m_window, true);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawRectPx  (private)
//  Filled rectangle in pixel coordinates (top-left origin, y grows down).
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawRectPx(float x, float y, float w, float h,
    float r, float g, float b, float a)
{
    float x0n, y0n, x1n, y1n;
    pixelToNDC(x, y, x0n, y0n);   // top-left
    pixelToNDC(x + w, y + h, x1n, y1n);   // bottom-right

    glColor4f(r, g, b, a);
    glBegin(GL_QUADS);
    glVertex2f(x0n, y0n);
    glVertex2f(x1n, y0n);
    glVertex2f(x1n, y1n);
    glVertex2f(x0n, y1n);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawLinePx  (private)
//  Line using pixel coordinates.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawLinePx(float x1, float y1, float x2, float y2,
    float r, float g, float b, float a)
{
    float nx1, ny1, nx2, ny2;
    pixelToNDC(x1, y1, nx1, ny1);
    pixelToNDC(x2, y2, nx2, ny2);
    drawLineNDC(nx1, ny1, nx2, ny2, r, g, b, a);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawCircleNDC  (private)
//  Filled circle using a triangle fan. Corrects for aspect ratio.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawCircleNDC(float cx, float cy, float radiusNDC,
    float r, float g, float b, int segments)
{
    int w, h;
    glfwGetFramebufferSize(m_window, &w, &h);
    float aspect = (float)w / (float)h;

    glColor3f(r, g, b);
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * 3.14159265f * i / (float)segments;
        glVertex2f(cx + cosf(angle) * radiusNDC / aspect,
            cy + sinf(angle) * radiusNDC);
    }
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawLineNDC  (private)
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawLineNDC(float x1, float y1, float x2, float y2,
    float r, float g, float b, float a)
{
    glColor4f(r, g, b, a);
    glBegin(GL_LINES);
    glVertex2f(x1, y1);
    glVertex2f(x2, y2);
    glEnd();
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawHUD
//  Small info box drawn in pixel space below the cannon showing live angle
//  and speed values.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawHUD(float angle_deg, float speed)
{
    const float BOX_W = 130.0f;
    const float BOX_H = 44.0f;
    const float MARGIN = 12.0f;
    const float PADDING = 8.0f;
    const float SCALE = 1.8f;  // text scale

    // Anchor bottom-left, just above the bottom edge
    float bx = MARGIN;
    float by = (float)m_windowH - BOX_H - MARGIN;

    // Background
    drawRectPx(bx, by, BOX_W, BOX_H, 0.08f, 0.11f, 0.16f, 0.85f);

    // Border
    drawLinePx(bx, by, bx + BOX_W, by, 0.2f, 0.5f, 0.9f, 0.6f);
    drawLinePx(bx, by + BOX_H, bx + BOX_W, by + BOX_H, 0.2f, 0.5f, 0.9f, 0.6f);
    drawLinePx(bx, by, bx, by + BOX_H, 0.2f, 0.5f, 0.9f, 0.6f);
    drawLinePx(bx + BOX_W, by, bx + BOX_W, by + BOX_H, 0.2f, 0.5f, 0.9f, 0.6f);

    // Format values as strings (1 decimal place, manual int cast for clean look)
    auto fmtInt = [](float v) -> std::string {
        int iv = (int)roundf(v);
        return std::to_string(iv);
        };

    std::string angleStr = "ANG " + fmtInt(angle_deg) + " DEG";
    std::string speedStr = "SPD " + fmtInt(speed) + " M/S";

    float textX = bx + PADDING;
    drawText(angleStr, textX, by + PADDING, 0.4f, 0.75f, 1.0f, SCALE);
    drawText(speedStr, textX, by + PADDING + 10.0f * SCALE, 0.9f, 0.6f, 0.2f, SCALE);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawStatsBox
//  Top-right box showing max height and total horizontal distance.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawStatsBox(float maxHeight, float totalHorizDist)
{
    const float BOX_W = 220.0f;
    const float BOX_H = 58.0f;
    const float MARGIN = 12.0f;
    const float PADDING = 8.0f;
    const float SCALE = 2.4f;

    // Anchor top-right
    float bx = (float)m_windowW - BOX_W - MARGIN;
    float by = MARGIN;

    // Background
    drawRectPx(bx, by, BOX_W, BOX_H, 0.08f, 0.11f, 0.16f, 0.85f);

    // Border
    drawLinePx(bx, by, bx + BOX_W, by, 0.2f, 0.5f, 0.9f, 0.6f);
    drawLinePx(bx, by + BOX_H, bx + BOX_W, by + BOX_H, 0.2f, 0.5f, 0.9f, 0.6f);
    drawLinePx(bx, by, bx, by + BOX_H, 0.2f, 0.5f, 0.9f, 0.6f);
    drawLinePx(bx + BOX_W, by, bx + BOX_W, by + BOX_H, 0.2f, 0.5f, 0.9f, 0.6f);

    // Format to 1 decimal place
    auto fmt1 = [](float v) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << v;
        return ss.str();
        };

    std::string heightStr = "MAX H " + fmt1(maxHeight) + " M";
    std::string distStr = "DIST  " + fmt1(totalHorizDist) + " M";

    float textX = bx + PADDING;
    drawText(heightStr, textX, by + PADDING, 0.4f, 1.0f, 0.6f, SCALE);
    drawText(distStr, textX, by + PADDING + 13.0f * SCALE, 1.0f, 0.7f, 0.2f, SCALE);
}

// ─────────────────────────────────────────────────────────────────────────────
//  drawBounceLabels
//  For each bounce segment, draws the horizontal distance as a number
//  centred on the ground midway between the two bounce points, slightly
//  below the ground line.
// ─────────────────────────────────────────────────────────────────────────────
void Renderer::drawBounceLabels(const std::vector<BounceSegment>& segs)
{
    const float SCALE = 2.4f;
    const float BELOW_GROUND = 18.0f; // pixels below ground line

    // Find where y=0 sits in pixel space so we can place labels just below it
    float gndNDCX, gndNDCY;
    worldToNDC(0.0f, 0.0f, gndNDCX, gndNDCY);

    // NDC y → pixel y (invert: NDC +1 = top, pixel 0 = top)
    int fw, fh;
    glfwGetFramebufferSize(m_window, &fw, &fh);
    float gndPxY = ((1.0f - gndNDCY) / 2.0f) * (float)fh + BELOW_GROUND;

    auto fmt1 = [](float v) -> std::string {
        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1) << v;
        return ss.str();
        };

    int count = 0;
    for (const auto& seg : segs)
    {
        if (count++ >= 3) break;


        // ... rest unchanged

        float segLen = seg.xEnd - seg.xStart;
        float midWorldX = seg.xStart + segLen * 0.5f;

        // World x → NDC x → pixel x
        float ndcX, ndcY;
        worldToNDC(midWorldX, 0.0f, ndcX, ndcY);
        float pxX = ((ndcX + 1.0f) / 2.0f) * (float)fw;

        std::string label = fmt1(segLen) + "M";

        // Centre the label: each char is ~4px wide at scale 1, SCALE px per unit
        float charW = 4.0f * SCALE;
        float labelW = (float)label.size() * (charW + 1.0f);
        float labelX = pxX - labelW * 0.5f;

        drawText(label, labelX, gndPxY, 0.9f, 0.85f, 0.3f, SCALE);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Bitmap font
// ─────────────────────────────────────────────────────────────────────────────
struct FontSeg { float x0, y0, x1, y1; };

static const FontSeg* getCharSegs(char c, int& count)
{
    static const FontSeg N0[] = { {1,0,3,0},{3,0,3,6},{3,6,1,6},{1,6,1,0} };
    static const FontSeg N1[] = { {2,0,2,6},{1,6,3,6} };
    static const FontSeg N2[] = { {1,0,3,0},{3,0,3,3},{3,3,1,3},{1,3,1,6},{1,6,3,6} };
    static const FontSeg N3[] = { {1,0,3,0},{3,0,3,6},{1,6,3,6},{1,3,3,3} };
    static const FontSeg N4[] = { {1,0,1,3},{1,3,3,3},{3,0,3,6} };
    static const FontSeg N5[] = { {3,0,1,0},{1,0,1,3},{1,3,3,3},{3,3,3,6},{3,6,1,6} };
    static const FontSeg N6[] = { {3,0,1,0},{1,0,1,6},{1,6,3,6},{3,6,3,3},{3,3,1,3} };
    static const FontSeg N7[] = { {1,0,3,0},{3,0,2,6} };
    static const FontSeg N8[] = { {1,0,3,0},{3,0,3,6},{3,6,1,6},{1,6,1,0},{1,3,3,3} };
    static const FontSeg N9[] = { {1,6,3,6},{3,6,3,0},{3,0,1,0},{1,0,1,3},{1,3,3,3} };
    static const FontSeg DOT[] = { {2,5,2,6},{0,0,0,0} };
    static const FontSeg SPC[] = { {0,0,0,0} };
    static const FontSeg A[] = { {1,6,2,0},{2,0,3,6},{1,3,3,3} };
    static const FontSeg B[] = { {1,0,1,6},{1,0,3,1},{3,1,3,3},{1,3,3,3},{3,3,3,5},{3,5,1,6} };
    static const FontSeg C[] = { {3,0,1,0},{1,0,1,6},{1,6,3,6} };
    static const FontSeg D[] = { {1,0,1,6},{1,0,3,2},{3,2,3,4},{3,4,1,6} };
    static const FontSeg E[] = { {3,0,1,0},{1,0,1,6},{1,6,3,6},{1,3,2,3} };
    static const FontSeg F[] = { {3,0,1,0},{1,0,1,6},{1,3,2,3} };
    static const FontSeg G[] = { {3,0,1,0},{1,0,1,6},{1,6,3,6},{3,6,3,3},{2,3,3,3} };
    static const FontSeg H[] = { {1,0,1,6},{3,0,3,6},{1,3,3,3} };
    static const FontSeg I[] = { {1,0,3,0},{2,0,2,6},{1,6,3,6} };
    static const FontSeg K[] = { {1,0,1,6},{1,3,3,0},{1,3,3,6} };
    static const FontSeg L[] = { {1,0,1,6},{1,6,3,6} };
    static const FontSeg M[] = { {1,6,1,0},{1,0,2,3},{2,3,3,0},{3,0,3,6} };
    static const FontSeg NN[] = { {1,6,1,0},{1,0,3,6},{3,6,3,0} };
    static const FontSeg O[] = { {1,0,3,0},{3,0,3,6},{3,6,1,6},{1,6,1,0} };
    static const FontSeg P[] = { {1,0,1,6},{1,0,3,0},{3,0,3,3},{3,3,1,3} };
    static const FontSeg R[] = { {1,0,1,6},{1,0,3,0},{3,0,3,3},{3,3,1,3},{1,3,3,6} };
    static const FontSeg S[] = { {3,0,1,0},{1,0,1,3},{1,3,3,3},{3,3,3,6},{3,6,1,6} };
    static const FontSeg T[] = { {1,0,3,0},{2,0,2,6} };
    static const FontSeg U[] = { {1,0,1,6},{1,6,3,6},{3,6,3,0} };
    static const FontSeg W[] = { {1,0,1,6},{1,6,2,3},{2,3,3,6},{3,6,3,0} };
    static const FontSeg X[] = { {1,0,3,6},{3,0,1,6} };
    static const FontSeg Y[] = { {1,0,2,3},{3,0,2,3},{2,3,2,6} };

    switch (c) {
    case '0': count = 4; return N0;
    case '1': count = 2; return N1;
    case '2': count = 5; return N2;
    case '3': count = 4; return N3;
    case '4': count = 3; return N4;
    case '5': count = 5; return N5;
    case '6': count = 5; return N6;
    case '7': count = 2; return N7;
    case '8': count = 5; return N8;
    case '9': count = 5; return N9;
    case '.': count = 1; return DOT;
    case ' ': count = 0; return SPC;
    case 'A': count = 3; return A;
    case 'B': count = 6; return B;
    case 'C': count = 3; return C;
    case 'D': count = 4; return D;
    case 'E': count = 4; return E;
    case 'F': count = 3; return F;
    case 'G': count = 5; return G;
    case 'H': count = 3; return H;
    case 'I': count = 3; return I;
    case 'K': count = 3; return K;
    case 'L': count = 2; return L;
    case 'M': count = 4; return M;
    case 'N': count = 3; return NN;
    case 'O': count = 4; return O;
    case 'P': count = 4; return P;
    case 'R': count = 5; return R;
    case 'S': count = 5; return S;
    case 'T': count = 2; return T;
    case 'U': count = 3; return U;
    case 'W': count = 4; return W;
    case 'X': count = 2; return X;
    case 'Y': count = 3; return Y;
    default:  count = 0; return SPC;
    }
}

void Renderer::drawChar(char c, float px, float py,
    float r, float g, float b, float scale)
{
    if (c >= 'a' && c <= 'z') c = c - 'a' + 'A';
    int count = 0;
    const FontSeg* segs = getCharSegs(c, count);
    for (int i = 0; i < count; i++)
        drawLinePx(px + segs[i].x0 * scale, py + segs[i].y0 * scale,
            px + segs[i].x1 * scale, py + segs[i].y1 * scale,
            r, g, b, 1.0f);
}

void Renderer::drawText(const std::string& s, float px, float py,
    float r, float g, float b, float scale)
{
    float charW = 5.0f * scale;
    for (size_t i = 0; i < s.size(); i++)
        drawChar(s[i], px + (float)i * charW, py, r, g, b, scale);
}
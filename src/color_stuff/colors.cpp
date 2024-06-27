#include <vector>
#include <cmath>
#include <tuple>

// Function to generate n distinct RGB colors
std::vector<std::tuple<int, int, int, int>> generateColors(int n) {
    auto HSVtoRGB = [](float h, float s, float v, int &r, int &g, int &b) {
        float p, q, t, f;
        int i;

        h = fmod(h, 360.0f) / 60.0f;
        i = static_cast<int>(h);
        f = h - i;
        p = v * (1.0f - s);
        q = v * (1.0f - s * f);
        t = v * (1.0f - s * (1.0f - f));

        switch (i) {
            case 0:
                r = static_cast<int>(v * 255.0f);
                g = static_cast<int>(t * 255.0f);
                b = static_cast<int>(p * 255.0f);
                break;
            case 1:
                r = static_cast<int>(q * 255.0f);
                g = static_cast<int>(v * 255.0f);
                b = static_cast<int>(p * 255.0f);
                break;
            case 2:
                r = static_cast<int>(p * 255.0f);
                g = static_cast<int>(v * 255.0f);
                b = static_cast<int>(t * 255.0f);
                break;
            case 3:
                r = static_cast<int>(p * 255.0f);
                g = static_cast<int>(q * 255.0f);
                b = static_cast<int>(v * 255.0f);
                break;
            case 4:
                r = static_cast<int>(t * 255.0f);
                g = static_cast<int>(p * 255.0f);
                b = static_cast<int>(v * 255.0f);
                break;
            default:
                r = static_cast<int>(v * 255.0f);
                g = static_cast<int>(p * 255.0f);
                b = static_cast<int>(q * 255.0f);
                break;
        }
    };

    std::vector<std::tuple<int, int, int, int>> colors(n);
    float increment = 360.0f / n;

    for (int i = 0; i < n; ++i) {
        float h = i * increment;
        float s = 1.0f;  // Full saturation
        float v = 1.0f;  // Full value

        int r, g, b;
        HSVtoRGB(h, s, v, r, g, b);
        colors[i] = std::make_tuple(r, g, b, 255);
    }

    return colors;
}
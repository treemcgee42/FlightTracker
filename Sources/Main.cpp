
#include <assert.h>
#include <cmath>
#include <iostream>
#include <mutex>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <fmt/base.h>
#include <fmt/format.h>

namespace janet {
#include "janet.h"
}
namespace rl {
#include "raylib.h"
}

#include "Layout.hpp"

using Latitude = double;
using Longitude = double;

// struct GeoCoord {
//     double longitude;
//     double latitude;
// };

// struct FlightData {
//     std::string callSign;
//     GeoCoord position;
// };

// struct GeoBb {
//     GeoCoord min;
//     GeoCoord max;
// };

struct WindowPos {
    uint32_t x;
    uint32_t y;
};

struct WindowRegion {
    WindowPos min;
    WindowPos max;

    uint32_t
    width() {
        return this->max.x - this->min.x;
    }

    uint32_t
    height() {
        return this->max.y - this->min.y;
    }
};

enum class Direction {
    ABOVE,
    BELOW,
};

void
drawLabeledPoint( int pointX, int pointY, float pointRadius, const char * text, rl::Color color, Direction direction ) {
    rl::DrawCircle( pointX, pointY, pointRadius, color);

    int offsetX;
    int offsetY;
    if( direction == Direction::BELOW ) {
        offsetX = 0;
        offsetY = 4 * pointRadius;
    } else if( direction == Direction::ABOVE ) {
        offsetX = 0;
        offsetY = -4 * pointRadius;
    } else {
        assert( false );
    }

    rl::DrawText( text, pointX + offsetX, pointY + offsetY, 12, color );
}

// class Radar2d {
// public:
//     // struct Config {
//     //     rl::Color borderColor;

//     //     static Config
//     //     fromJanetTable( const janet::JanetTable * janetTable ) {
//     //         const janet::Janet color = janet::janet_table_get(
//     //             janetTable, ":border-color" );
//     //         if( color.type != janet::JANET_NIL ) {
                
//     //         }
//     //         return Config {
                
//     //         };
//     //     }
//     // };

//     GeoBb bb;
//     std::vector< FlightData > flights{};

//     void
//     draw( WindowRegion region ) {
//         rl::DrawRectangleLinesEx(
//             { static_cast< float >( region.min.x ),
//               static_cast< float >( region.min.y ),
//               static_cast< float >( region.max.x - region.min.x ),
//               static_cast< float >( region.max.y - region.min.y ) },
//             3,
//             rl::BLUE );

//         // The padding within `region` before we draw the actual box.
//         const uint32_t bbPaddingX = 50;
//         const uint32_t bbPaddingY = 50;
//         const uint32_t bbMinX = region.min.x + bbPaddingX;
//         const uint32_t bbMinY = region.min.y + bbPaddingY;
//         const uint32_t bbMaxX = region.max.x - bbPaddingX;
//         const uint32_t bbMaxY = region.max.y - bbPaddingY;
//         rl::DrawRectangleLinesEx(
//             { static_cast< float >( bbMinX ),
//               static_cast< float >( bbMinY ),
//               static_cast< float >( bbMaxX - bbMinX ),
//               static_cast< float >( bbMaxY - bbMinY ) },
//             3,
//             rl::RED );

//         drawLabeledPoint( bbMinX, bbMinY, 5, "bbMin", rl::RED, Direction::ABOVE );
//     }
// };

class Repl {
public:
    Repl() {
        _thread = std::thread([this] {
            while( true ) {
                // If there is previous REPL input to handle, don't accept anything
                // else and don't display the prompt.
                if( _nextLine ) {
                    continue;
                }
                std::cout << "REPL> ";
                std::cout.flush();
                std::string line;
                if( std::getline(std::cin, line) ) {
                    std::lock_guard<std::mutex> lock(_mutex);
                    _nextLine = line;
                }
            }
        });
        _thread.detach();  // Let it run in background
    }

    // Be sure to call `readyForNextInput()` when you've finished processing the
    // polled input.
    std::optional<std::string>
    poll() {
        std::lock_guard<std::mutex> lock(_mutex);
        if ( _nextLine ) {
            return _nextLine;
        }
        return std::nullopt;
    }

    // Allow the REPl to accept new inputs.
    void
    readyForNextInput() {
        _nextLine.reset();
    }

private:
    std::thread _thread;
    std::mutex _mutex;
    std::optional<std::string> _nextLine;
};

class EmbeddedJanet {
public:
    EmbeddedJanet() {
        janet::janet_init();
        _env = janet::janet_core_env(NULL);
    }

    ~EmbeddedJanet() {
        janet::janet_deinit();
    }

    void
    eval( const std::string & input ) {
        janet::janet_dostring( _env, input.c_str(), "repl", NULL );
    }

    janet::JanetTable * _env;
};

int
mainMain() {
    rl::SetConfigFlags(rl::FLAG_WINDOW_HIGHDPI);
    rl::InitWindow( 10, 10, "FlightTracker" );

    // You can call these BEFORE InitWindow
    int monitor = rl::GetCurrentMonitor();  // usually 0
    int screenWidth = rl::GetMonitorWidth(monitor);
    int screenHeight = rl::GetMonitorHeight(monitor);
    const int windowWidth = screenWidth * 0.9;
    const int windowHeight = screenHeight * 0.9;

    rl::SetWindowSize( windowWidth, windowHeight );
    rl::SetWindowPosition( ( screenWidth - windowWidth ) / 2,
                           ( screenHeight - windowHeight ) / 2);

    fmt::print( "I think I have {}x{} pixels to work with. "
                "I'll use {}x{}.\n",
                screenWidth, screenHeight,
                windowWidth, windowHeight );

    // Radar2d radar{};
    // radar.bb = { { -71.245840, 42.183094 },
    //              { -70.777170, 42.529427 } };

    Repl repl{};
    EmbeddedJanet janet{};

    while( !rl::WindowShouldClose() ) {
        if ( const auto & line = repl.poll() ) {
            janet.eval( *line );
            repl.readyForNextInput();
        }

        rl::BeginDrawing();
        rl::ClearBackground( rl::RAYWHITE );

        // const uint32_t radarPaddingX = 50;
        // const uint32_t radarPaddingY = 50;
        // const WindowRegion radarRegion = { { radarPaddingX, radarPaddingY },
        //                                    { windowWidth - radarPaddingX,
        //                                      windowHeight - radarPaddingY } };
        // radar.draw( radarRegion );

        // rl::DrawText("Congrats! You created your first window!", 190, 200, 20, rl::LIGHTGRAY);
        rl::EndDrawing();
    }
    rl::CloseWindow();
    return 0;
}

extern int testRadar();
extern int testScrollingText();
extern int testVStack();

int
main() {
    // return mainMain();
    return testRadar();
    // return testScrollingText();
    // return testVStack();
}

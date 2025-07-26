
#include <fmt/base.h>
#include <fmt/format.h>
#include <fstream>

#include "FlightData.hpp"

const char* ws = " \t\n\r\f\v";

// trim from end of string (right)
inline std::string& rtrim(std::string& s, const char* t = ws)
{
    s.erase(s.find_last_not_of(t) + 1);
    return s;
}

// trim from beginning of string (left)
inline std::string& ltrim(std::string& s, const char* t = ws)
{
    s.erase(0, s.find_first_not_of(t));
    return s;
}

// trim from both ends of string (right then left)
inline std::string& trim(std::string& s, const char* t = ws)
{
    return ltrim(rtrim(s, t), t);
}

std::string
format_as( const GeoCoord & geoCoord ) {
    assert( geoCoord.longitude && geoCoord.latitude );
    return fmt::format( "GeoCoord{{ {}, {} }}",
                        geoCoord.longitude, geoCoord.latitude);
}

std::string
format_as( const FlightData & flightData ) {
    assert( flightData.callSign.size() );
    return fmt::format( "FlightData{{ {}, {} }}",
                        flightData.callSign, flightData.position );
}

Vector2
GeoBb::relativePosition( GeoCoord coord ) {
    Vector2 pos;
    pos.xIs( ( coord.longitude - min.longitude ) /
             ( max.longitude - min.longitude ) );
    pos.yIs( ( coord.latitude - min.latitude ) /
             ( max.latitude - min.latitude ) );
    return pos;
}

FlightData
OpenSky::parseState( const nlohmann::json & state ) {
    FlightData flightData;

    flightData.callSign =
        state.is_null() ? "???" : state[ 1 ].get< std::string >();
    flightData.callSign = trim( flightData.callSign );
    if( flightData.callSign == "" ) {
        flightData.callSign = "???";
    }

    flightData.position.longitude =
        state[ 5 ].is_null() ? 0 : state[ 5 ].get< float >();
    flightData.position.latitude =
        state[ 6 ].is_null() ? 0 : state[ 6 ].get< float >();

    return flightData;
}

void
Radar::drawFlight( Vector2 radarAt, Vector2 mousePos, FlightData flightData ) {
    const Vector2 relPos = _geoBb.relativePosition( flightData.position );
    Vector2 at = radarAt;
    at.xInc( size().width() * relPos.x() );
    at.yInc( size().height() * relPos.y() );
    const double radius = mousePos.distanceTo( at ) > 5 ? 3 : 6;
    rl::DrawCircleV( at.toRlVector2(), radius, rl::RED );
}

void
Radar::draw( const DrawContext & ctx ) {
    rl::DrawRectangleLinesEx( ctx.at.toRlRectangle( size() ), 2, rl::RED );
    for( const FlightData & flightData : _flightData ) {
        drawFlight( ctx.at, ctx.mousePos, flightData );
    }
}

int
testRadar() {
    int windowWidth = 800;
    int windowHeight = 600;
    rl::SetConfigFlags( rl::FLAG_WINDOW_RESIZABLE );
    rl::InitWindow( windowWidth, windowHeight, "Radar Demo" );

    std::ifstream f( "../sample_data.json" );
    nlohmann::json data = nlohmann::json::parse( f );

    LayoutManager layoutManager;

    RectangleV2 root{ layoutManager, rl::BLANK };
    root.xLayoutMut()->paddingIs( 20 );
    root.yLayoutMut()->paddingIs( 20 );

    VStackV2 vstack{ layoutManager };
    vstack.xLayoutMut()->sizeSpecIs( SizeSpec::grow() );
    vstack.yLayoutMut()->sizeSpecIs( SizeSpec::grow() );
    vstack.yLayoutMut()->childGapIs( 5 );
    root.addChild( &vstack );

    RectangleV2 modeLine{ layoutManager, rl::BLUE };
    modeLine.xLayoutMut()->sizeSpecIs( SizeSpec::growAcrossAxis() );
    modeLine.yLayoutMut()->sizeSpecIs( SizeSpec::absolute( 20 ) );
    vstack.addChild( &modeLine );

    Radar radar{ layoutManager };
    radar.xLayoutMut()->sizeSpecIs( SizeSpec::growAcrossAxis() );
    radar.yLayoutMut()->sizeSpecIs( SizeSpec::grow() );
    radar.geoBbIs( { { -71.245840, 42.183094 },
                     { -70.777170, 42.529427 } } );
    vstack.addChild( &radar );

    for( const auto & state : data[ "states" ] ) {
        const auto flightData = OpenSky::parseState( state );
        fmt::print( "{}\n", flightData );
        radar.flightDataPush( flightData );
    }

    while( !rl::WindowShouldClose() ) {
        windowWidth = rl::GetScreenWidth();
        windowHeight = rl::GetScreenHeight();
        const float deltaTime = rl::GetFrameTime();
        const Vector2 mousePos = Vector2::fromRlVector2( rl::GetMousePosition() );

        root.xLayoutMut()->sizeSpecIs( SizeSpec::absolute( windowWidth ) );
        root.yLayoutMut()->sizeSpecIs( SizeSpec::absolute( windowHeight ) );
        root.computeLayout();

        rl::BeginDrawing();
        rl::ClearBackground( rl::RAYWHITE );

        DrawContext drawCtx;
        drawCtx.at = { 0, 0 };
        drawCtx.deltaTime = deltaTime;
        drawCtx.mousePos = mousePos;
        root.draw( drawCtx );

        rl::EndDrawing();
    }

    rl::CloseWindow();
    return 0;
}

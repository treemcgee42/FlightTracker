#pragma once

#include <string>

#include <nlohmann/json.hpp>

#include "Layout.hpp"
#include "SizeTypes.hpp"

struct GeoCoord {
    double longitude;
    double latitude;
};
std::string format_as( const GeoCoord & geoCoord );

struct GeoBb {
    GeoCoord min;
    GeoCoord max;

    Vector2 relativePosition( GeoCoord coord );
};

struct FlightData {
    std::string callSign;
    GeoCoord position;
};
std::string format_as( const FlightData & flightData );

class OpenSky {
public:
    static FlightData parseState( const nlohmann::json & state );
};

class Radar: public ComponentV2 {
public:
    Radar( LayoutManager & layoutManager ): ComponentV2( layoutManager ) {};

    void flightDataPush( const FlightData & flightData ) {
        _flightData.push_back( flightData );
    }
    void geoBbIs( const GeoBb & val ) { _geoBb = val; }

    void drawFlight( Vector2 radarAt, Vector2 mousePos, FlightData flightData );
    void draw( const DrawContext & ctx ) override;

private:
    std::vector< FlightData > _flightData;
    GeoBb _geoBb;
};

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

class Radar: public Component {
public:
    Radar();

    void flightDataPush( const FlightData & flightData ) {
        _flightData.push_back( flightData );
    }
    void geoBbIs( const GeoBb & val ) { _geoBb = val; }

    ComponentSize size() const override;
    void drawFlight( Vector2 radarAt, FlightData flightData );
    void draw( Vector2 at, double deltaTime ) override;

private:
    std::vector< FlightData > _flightData;
    GeoBb _geoBb;
};

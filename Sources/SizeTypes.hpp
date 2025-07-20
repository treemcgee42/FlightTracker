#pragma once

namespace rl {
#include <raylib.h>
}

struct Vector2 {
    double _x;
    double _y;

    constexpr Vector2(): _x( 0 ), _y( 0 ) {}
    constexpr Vector2( double x, double y ): _x( x ), _y( y ) {}

    constexpr double x() const { return _x; }
    void xIs( double newX ) { _x = newX; }
    void xInc( double inc ) { _x += inc; }
    constexpr double width() const { return _x; }
    void widthIs( double newWidth ) { xIs( newWidth ); }
    void widthInc( double inc ) { xInc( inc ); }
    constexpr double y() const { return _y; }
    void yIs( double newY ) { _y = newY; }
    void yInc( double inc ) { _y += inc; }
    constexpr double height() const { return _y; }
    void heightIs( double newHeight ) { yIs( newHeight ); }
    void heightInc( double inc ) { yInc( inc ); }

    constexpr rl::Vector2 toRlVector2() const {
        return { static_cast< float >( _x ), static_cast< float >( _y ) };
    }
    static constexpr Vector2 fromRlVector2( rl::Vector2 v ) {
        return Vector2( v.x, v.y );
    }

    constexpr rl::Rectangle toRlRectangle( const Vector2 & widthAndHeight ) {
        return {
            static_cast< float >( _x ),
            static_cast< float >( _y ),
            static_cast< float >( widthAndHeight.width() ),
            static_cast< float >( widthAndHeight.height() )
        };
    }

    constexpr Vector2 operator+( const Vector2 & other ) const {
        return Vector2( x() + other.x(), y() + other.y() );
    }
    Vector2 & operator+=( const Vector2 & other ) {
        _x += other.x();
        _y += other.y();
        return *this;
    }
};

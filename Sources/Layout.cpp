namespace rl {
#include <raylib.h>
}

#include "Layout.hpp"

Text::Text( const std::string & content ):
    _content( content ) {
    _size = ComponentSize::fromRlVector2(
        rl::MeasureTextEx( _font, _content.c_str(), _fontSize, 1 ) );
}

void
Text::draw( Vector2 at ) {
    rl::DrawTextEx( _font, _content.c_str(), at.toRlVector2(), _fontSize, 1,
                    _textColor );
}

ComponentSize
Text::size() const {
    return _size;
}

VStack::VStack():
    _components() {
}

void
VStack::addComponent( std::unique_ptr< Component > c ) {
    _components.push_back( std::move( c ) );
}

ComponentSize
VStack::size() const {
    ComponentSize s;
    for( const auto & component : _components ) {
        s.widthIs( std::max( s.width(), component->size().width() ) );
        s.heightInc( component->size().y() );
    }
    s.heightInc( _spacing * ( _components.size() - 1 ) );
    return s;
}

void
VStack::draw( Vector2 at ) {
    Vector2 componentPos = at;
    for( auto & component : _components ) {
        component->draw( componentPos );
        componentPos.yInc( component->size().y() + _spacing );
    }
}

HStack::HStack():
    _components() {
}

void
HStack::addComponent( std::unique_ptr< Component > c ) {
    _components.push_back( std::move( c ) );
}

ComponentSize
HStack::size() const {
    ComponentSize s;
    for( const auto & component : _components ) {
        s.widthInc( component->size().width() );
        s.heightIs( std::max( s.height(), component->size().height() ) );
    }
    s.widthInc( _spacing * ( _components.size() - 1 ) );
    return s;
}

void
HStack::draw( Vector2 at ) {
    Vector2 componentPos = at;
    for( auto & component : _components ) {
        component->draw( componentPos );
        componentPos.xInc( component->size().x() + _spacing );
    }
}

int
testVStack() {
    rl::InitWindow(800, 600, "VStack Demo");

    HStack hstack;
    hstack.spacingIs( 10 );

    auto vstack1 = std::make_unique< VStack >();
    vstack1->addComponent( std::make_unique< Text >( "line 1" ) );
    vstack1->addComponent( std::make_unique< Text >( "line 2" ) );
    vstack1->addComponent( std::make_unique< Text >( "line 3" ) );

    auto vstack2 = std::make_unique< VStack >();
    vstack2->spacingIs( 5 );
    vstack2->addComponent( std::make_unique< Text >( "line 4" ) );
    vstack2->addComponent( std::make_unique< Text >( "line 5" ) );
    vstack2->addComponent( std::make_unique< Text >( "line 6" ) );

    hstack.addComponent( std::move( vstack1 ) );
    hstack.addComponent( std::move( vstack2 ) );

    while( !rl::WindowShouldClose() ) {
        rl::BeginDrawing();
        rl::ClearBackground( rl::RAYWHITE );

        hstack.draw( { 20, 20 } );

        rl::EndDrawing();
    }

    rl::CloseWindow();
    return 0;
}

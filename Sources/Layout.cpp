#include <assert.h>

#include <fmt/format.h>
namespace rl {
#include <raylib.h>
}

#include "Layout.hpp"

const Layout &
LayoutHandle::getLayoutConst() const {
    return _manager->getLayoutConst( this );
}

Layout &
LayoutHandle::getLayoutMut() {
    return _manager->getLayoutMut( this );
}

void
Layout::baseSizePass() {
    if( _sizeSpec.isAbsolute() ) {
        sizeIs( _sizeSpec.getAbsolute() );
    } else if( _sizeSpec.isGrow() ) {
        sizeIs( 0 );
    } else {
        assert( false );
    }

    for( LayoutHandle & child : childrenMut() ) {
        child.getLayoutMut().baseSizePass();
    }
}

// growing must happen top-down
void
Layout::growSizePass() {
    if( children().size() == 0 ) {
        return;
    }

    int numGrowChildren = 0;
    double availableSpace =
        size() - ( 2 * padding() ) - ( ( children().size() - 1 ) * childGap() );
    for( const LayoutHandle & child : children() ) {
        const Layout & childLayout = child.getLayoutConst();
        if( childLayout.sizeSpec().isGrow() ) {
            assert( childLayout.size() == 0 );
            numGrowChildren += 1;
        }
        availableSpace -= childLayout.size();
    }

    const double growSize =
        std::max( availableSpace / static_cast< double >( numGrowChildren ), 0.0 );
    for( LayoutHandle & child : childrenMut() ) {
        Layout & childLayout = child.getLayoutMut();
        if( childLayout.sizeSpec().isGrow() ) {
            childLayout.sizeIs( growSize );
        }
        childLayout.growSizePass();
    }
}

void
Layout::computeLayout() {
    baseSizePass();
    growSizePass();
}



// void
// RectangleV2::draw( Vector2 at, double deltaTime ) {
//     rl::DrawRectangleV( at.toRlVector2(), size().toRlVector2(), rl::BLANK );
// }













VSpace::VSpace( double amount ): _amount( amount ) {}

ComponentSize
VSpace::size() const {
    return ComponentSize( 1, _amount );
}

void
VSpace::draw( Vector2 at, double deltaTime ) {
    rl::DrawRectangleV( at.toRlVector2(), size().toRlVector2(), rl::BLANK );
}

Text::Text( const std::string & content ):
    _content( content ) {
    _size = ComponentSize::fromRlVector2(
        rl::MeasureTextEx( _font, _content.c_str(), _fontSize, _textSpacing ) );
}

Text::Text( const std::string & content, double fontSize ):
    Text( content ) {
    _fontSize = fontSize;
}

void
Text::draw( Vector2 at, double deltaTime ) {
    rl::DrawTextEx( _font, _content.c_str(), at.toRlVector2(), _fontSize,
                    _textSpacing, _textColor );
}

ComponentSize
Text::size() const {
    return _size;
}

void
CircularScrollOffset::update( double deltaTime ) {
    assert( _initialized );
    if( _scrollRegionSize < _contentSize ) {
        _offset = fmod( _offset + _scrollSpeed * deltaTime,
                        _contentSize + _paddingSize );
    }
}

ScrollingText::ScrollingText( const std::string & content, double width,
                              double scrollSpeed )
    : Text( content ), _width( width ), _scrollSpeed( scrollSpeed ) {
    const char * padding = "    ";
    _text = fmt::format( "{}{}", content, padding, content, padding );

    const rl::Vector2 contentSize = rl::MeasureTextEx(
        _font, content.c_str(), _fontSize, _textSpacing );
    _textHeight = contentSize.y;
    const rl::Vector2 textWithPaddingSize = rl::MeasureTextEx(
        _font, _text.c_str(), _fontSize, _textSpacing );
    const float paddingSize = textWithPaddingSize.x - contentSize.x;

    _offsetHelper = CircularScrollOffset( contentSize.x, paddingSize, width,
                                          _scrollSpeed );

    _textTexture = rl::LoadRenderTexture(
        static_cast< int >( 2 * contentSize.x + paddingSize ),
        static_cast< int >( contentSize.y ) );
    rl::BeginTextureMode( _textTexture );
    rl::ClearBackground( rl::BLANK );
    rl::DrawTextEx( _font, _text.c_str(), { 0, 0 }, _fontSize, 1, rl::BLACK );
    rl::DrawTextEx( _font, _text.c_str(), { textWithPaddingSize.x, 0 }, _fontSize,
                    1, rl::BLACK );
    rl::EndTextureMode();
}

ScrollingText::~ScrollingText() {
    rl::UnloadRenderTexture( _textTexture );
}

ComponentSize
ScrollingText::size() const {
    return ComponentSize( _width, _textHeight );
}

void
ScrollingText::draw( Vector2 at, double deltaTime ) {
    _offsetHelper.update( deltaTime );
    DrawTextureRec( _textTexture.texture,
                    { static_cast< float >( _offsetHelper.offset() ), 0,
                      static_cast< float >( _width ),
                      -1.f * static_cast< float >( _textHeight ) },
                    at.toRlVector2(),
                    rl::WHITE );
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
VStack::draw( Vector2 at, double deltaTime ) {
    Vector2 componentPos = at;
    for( auto & component : _components ) {
        component->draw( componentPos, deltaTime );
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
HStack::draw( Vector2 at, double deltaTime ) {
    Vector2 componentPos = at;
    for( auto & component : _components ) {
        component->draw( componentPos, deltaTime );
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

        const float deltaTime = rl::GetFrameTime();
        hstack.draw( { 20, 20 }, deltaTime );

        rl::EndDrawing();
    }

    rl::CloseWindow();
    return 0;
}

int
testScrollingText() {
    rl::InitWindow(800, 200, "Scrolling Text Demo");

    VStack vstack;
    vstack.addComponent( std::make_unique< Text >( "Title", 20 ) );
    vstack.addComponent( std::make_unique< VSpace >( 10 ) );
    vstack.addComponent( std::make_unique< ScrollingText >(
        "This is some scrolling text. The first line of scrolling text.",
        100, 25 ) );
    vstack.addComponent( std::make_unique< ScrollingText >(
        "This is some more scrolling text (the second line of scrolling text).",
        100, 50 ) );

    while( !rl::WindowShouldClose() ) {
        rl::BeginDrawing();
        rl::ClearBackground( rl::RAYWHITE );

        const float deltaTime = rl::GetFrameTime();
        vstack.draw( { 20, 20 }, deltaTime );

        rl::EndDrawing();
    }

    rl::CloseWindow();
    return 0;
}

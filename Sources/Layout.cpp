#include <assert.h>

#include <fmt/format.h>
namespace rl {
#include <raylib.h>
}

#include "Layout.hpp"

void
RectangleV2::draw( const DrawContext & ctx ) {
    rl::DrawRectangleV( ctx.at.toRlVector2(), size().toRlVector2(), _fillColor );

    double xOffset = xLayoutConst()->padding();
    double yOffset = yLayoutConst()->padding();
    for( ComponentV2 * child : _children ) {
        auto childCtx = ctx;
        childCtx.at = { ctx.at.x() + xOffset, ctx.at.y() + yOffset };
        child->draw( childCtx );
        xOffset += child->xLayoutConst()->size() + xLayoutConst()->childGap();
        yOffset += child->yLayoutConst()->size() + yLayoutConst()->childGap();
    }
}

void
VStackV2::draw( const DrawContext & ctx ) {
    const double xOffset = xLayoutConst()->padding();
    double yOffset = yLayoutConst()->padding();
    for( ComponentV2 * child : _children ) {
        auto childCtx = ctx;
        childCtx.at = { ctx.at.x() + xOffset, ctx.at.y() + yOffset };
        child->draw( childCtx );
        yOffset += child->yLayoutConst()->size() + yLayoutConst()->childGap();
    }
}

int
testRectangleV2() {
    int windowWidth = 800;
    int windowHeight = 600;
    rl::SetConfigFlags( rl::FLAG_WINDOW_RESIZABLE );
    rl::InitWindow( windowWidth, windowHeight, "RectangleV2" );

    Dile::LayoutManager layoutManager;

    RectangleV2 root{ layoutManager, rl::BLANK };
    root.xLayoutMut()->paddingIs( 40 );
    root.yLayoutMut()->paddingIs( 100 );

    RectangleV2 child{ layoutManager, rl::BLUE };
    child.xLayoutMut()->sizeSpecIs( Dile::SizeSpec::grow() );
    child.yLayoutMut()->sizeSpecIs( Dile::SizeSpec::grow() );
    root.addChild( &child );

    while( !rl::WindowShouldClose() ) {
        windowWidth = rl::GetScreenWidth();
        windowHeight = rl::GetScreenHeight();
        const float deltaTime = rl::GetFrameTime();

        root.xLayoutMut()->sizeSpecIs( Dile::SizeSpec::absolute( windowWidth ) );
        root.yLayoutMut()->sizeSpecIs( Dile::SizeSpec::absolute( windowHeight ) );
        root.computeLayout();

        rl::BeginDrawing();
        rl::ClearBackground( rl::RAYWHITE );

        DrawContext drawCtx;
        drawCtx.at = { 0, 0 };
        drawCtx.deltaTime = deltaTime;
        root.draw( drawCtx );

        rl::EndDrawing();
    }

    rl::CloseWindow();
    return 0;
}











VSpace::VSpace( double amount ): _amount( amount ) {}

ComponentSize
VSpace::size() const {
    return ComponentSize( 1, _amount );
}

void
VSpace::draw( Vector2 at, double deltaTime ) {
    rl::DrawRectangleV( at.toRlVector2(), size().toRlVector2(), rl::BLANK );
}

Text::Text( Dile::LayoutManager & layoutManager,
            const std::string & content,
            const rl::Font & font,
            double fontSize,
            double textSpacing,
            const rl::Color & textColor ):
    ComponentV2( layoutManager ),
    _content( content ),
    _font( font ),
    _fontSize( fontSize ),
    _textSpacing( textSpacing ),
    _textColor( textColor ) {
    _size = Vector2::fromRlVector2(
        rl::MeasureTextEx( _font, _content.c_str(), _fontSize, _textSpacing ) );
}

void
Text::draw( const DrawContext & ctx ) {
    rl::DrawTextEx( _font, _content.c_str(), ctx.at.toRlVector2(), _fontSize,
                    _textSpacing, _textColor );
}

void
CircularScrollOffset::update( double deltaTime ) {
    assert( _initialized );
    if( _scrollRegionSize < _contentSize ) {
        _offset = fmod( _offset + _scrollSpeed * deltaTime,
                        _contentSize + _paddingSize );
    }
}

ScrollingText::ScrollingText( Dile::LayoutManager & layoutManager,
                              const std::string & content,
                              const rl::Font & font,
                              double fontSize,
                              double textSpacing,
                              const rl::Color & textColor,
                              double scrollSpeed ):
    Text( layoutManager, content, font, fontSize, textSpacing, textColor ),
    _scrollSpeed( scrollSpeed ) {
    const char * padding = "    ";
    _text = fmt::format( "{}{}", content, padding, content, padding );

    const rl::Vector2 contentSize = rl::MeasureTextEx(
        _font, content.c_str(), _fontSize, _textSpacing );
    _textHeight = contentSize.y;
    const rl::Vector2 textWithPaddingSize = rl::MeasureTextEx(
        _font, _text.c_str(), _fontSize, _textSpacing );
    const float paddingSize = textWithPaddingSize.x - contentSize.x;

    _offsetHelper = CircularScrollOffset(
        contentSize.x, paddingSize, xLayoutConst()->size(), _scrollSpeed );

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

void
ScrollingText::draw( const DrawContext & ctx ) {
    if( std::abs( xLayoutConst()->size() -
                  _offsetHelper.scrollRegionSize() ) > 0.01 ) {
        fmt::print( "scroll region size: {}\n", xLayoutConst()->size() );
        _offsetHelper.scrollRegionSizeIs( xLayoutConst()->size() );
    }
    _offsetHelper.update( ctx.deltaTime );
    DrawTextureRec( _textTexture.texture,
                    { static_cast< float >( _offsetHelper.offset() ), 0,
                      static_cast< float >(
                          std::min( _offsetHelper.scrollRegionSize(),
                                    _offsetHelper.contentSize() ) ),
                      -1.f * static_cast< float >( _textHeight ) },
                    ctx.at.toRlVector2(),
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

// int
// testVStack() {
//     rl::InitWindow(800, 600, "VStack Demo");

//     HStack hstack;
//     hstack.spacingIs( 10 );

//     auto vstack1 = std::make_unique< VStack >();
//     vstack1->addComponent( std::make_unique< Text >( "line 1" ) );
//     vstack1->addComponent( std::make_unique< Text >( "line 2" ) );
//     vstack1->addComponent( std::make_unique< Text >( "line 3" ) );

//     auto vstack2 = std::make_unique< VStack >();
//     vstack2->spacingIs( 5 );
//     vstack2->addComponent( std::make_unique< Text >( "line 4" ) );
//     vstack2->addComponent( std::make_unique< Text >( "line 5" ) );
//     vstack2->addComponent( std::make_unique< Text >( "line 6" ) );

//     hstack.addComponent( std::move( vstack1 ) );
//     hstack.addComponent( std::move( vstack2 ) );

//     while( !rl::WindowShouldClose() ) {
//         rl::BeginDrawing();
//         rl::ClearBackground( rl::RAYWHITE );

//         const float deltaTime = rl::GetFrameTime();
//         hstack.draw( { 20, 20 }, deltaTime );

//         rl::EndDrawing();
//     }

//     rl::CloseWindow();
//     return 0;
// }

// int
// testScrollingText() {
//     rl::InitWindow(800, 200, "Scrolling Text Demo");

//     VStack vstack;
//     vstack.addComponent( std::make_unique< Text >( "Title", 20 ) );
//     vstack.addComponent( std::make_unique< VSpace >( 10 ) );
//     vstack.addComponent( std::make_unique< ScrollingText >(
//         "This is some scrolling text. The first line of scrolling text.",
//         100, 25 ) );
//     vstack.addComponent( std::make_unique< ScrollingText >(
//         "This is some more scrolling text (the second line of scrolling text).",
//         100, 50 ) );

//     while( !rl::WindowShouldClose() ) {
//         rl::BeginDrawing();
//         rl::ClearBackground( rl::RAYWHITE );

//         const float deltaTime = rl::GetFrameTime();
//         vstack.draw( { 20, 20 }, deltaTime );

//         rl::EndDrawing();
//     }

//     rl::CloseWindow();
//     return 0;
// }

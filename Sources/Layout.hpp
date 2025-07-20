#pragma once

#include <vector>

#include "SizeTypes.hpp"

using ComponentSize = Vector2;

class Component {
public:
    virtual ~Component() = default;

    virtual ComponentSize size() const = 0;
    virtual void draw( Vector2 at, double deltaTime ) = 0;
};

class VSpace: public Component {
public:
    VSpace( double amount );

    ComponentSize size() const override;
    void draw( Vector2 at, double deltaTime ) override;
private:
    double _amount;
};

class Text: public Component {
public:
    Text( const std::string & content );
    Text( const std::string & content, double fontSize );

    ComponentSize size() const override;
    void draw( Vector2 at, double deltaTime ) override;

protected:
    std::string _content;

    rl::Font _font = rl::GetFontDefault();
    double _fontSize = 12;
    double _textSpacing = 1;
    rl::Color _textColor = rl::BLACK;

    ComponentSize _size = ComponentSize( 0, 0 );
};

class CircularScrollOffset {
public:
    CircularScrollOffset():
        _initialized( false ),
        _contentSize( 0 ),
        _paddingSize( 0 ),
        _scrollRegionSize( 0 ),
        _scrollSpeed( 0 ),
        _offset( 0 ) {}
    CircularScrollOffset( double contentSize,
                          double paddingSize,
                          double scrollRegionSize,
                          double scrollSpeed ):
        _initialized( true ),
        _contentSize( contentSize ),
        _paddingSize( paddingSize ),
        _scrollRegionSize( scrollRegionSize ),
        _scrollSpeed( scrollSpeed ),
        _offset( 0 ) {}

    constexpr double offset() { return _offset; }
    void update( double deltaTime );

private:
    bool _initialized;
    double _contentSize;
    double _paddingSize;
    double _scrollRegionSize;
    double _scrollSpeed;
    double _offset;
};

class ScrollingText: public Text {
public:
    ScrollingText( const std::string & content, double width, double scrollSpeed );
    ~ScrollingText();

    ComponentSize size() const override;
    void draw( Vector2 at, double deltaTime ) override;

private:
    double _width;
    double _scrollSpeed;

    std::string _text;
    rl::RenderTexture2D _textTexture;
    CircularScrollOffset _offsetHelper;
    double _textHeight;
};

class VStack: public Component {
public:
    VStack();

    void addComponent( std::unique_ptr< Component > c );
    void spacingIs( double val ) { _spacing = val; }

    ComponentSize size() const override;
    void draw( Vector2 at, double deltaTime ) override;

private:
    std::vector< std::unique_ptr< Component > > _components;
    double _spacing = 0;
};

class HStack: public Component {
public:
    HStack();

    void addComponent( std::unique_ptr< Component > c );
    void spacingIs( double val ) { _spacing = val; }

    ComponentSize size() const override;
    void draw( Vector2 at, double deltaTime ) override;

private:
    std::vector< std::unique_ptr< Component > > _components;
    double _spacing = 0;
};

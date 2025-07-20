#pragma once

#include <vector>

#include "SizeTypes.hpp"

using ComponentSize = Vector2;

class SizeSpec {
public:
    constexpr static SizeSpec fit() { return SizeSpec( FitTag {} ); }
    constexpr static SizeSpec grow() { return SizeSpec( GrowTag {} ); }
    constexpr static SizeSpec absolute( double val ) { return SizeSpec( val ); }

    bool isFit() const { return std::holds_alternative< FitTag >( _variant ); }
    bool isGrow() const { return std::holds_alternative< GrowTag >( _variant ); }
    bool isAbsolute() const {
        return std::holds_alternative< Absolute >( _variant );
    }
    double getAbsolute() const { return std::get< Absolute >( _variant ).val; }

private:
    struct FitTag {};
    struct GrowTag {};
    struct Absolute {
        double val;
    };

    constexpr SizeSpec( FitTag fitTag ): _variant( fitTag ) {}
    constexpr SizeSpec( GrowTag growTag ): _variant( growTag ) {}
    constexpr SizeSpec( double absolute ): _variant( Absolute { absolute } ) {}

    std::variant< FitTag, GrowTag, Absolute > _variant;
};

class Layout1d {
public:
    void sizeSpecIs( SizeSpec val ) { _sizeSpec = val; }
    constexpr bool isGrow() const { return _sizeSpec.isGrow(); }
    constexpr bool isAbsolute() const { return _sizeSpec.isAbsolute(); }

    constexpr double size() { return _size; }
    void sizeIs( double val ) { _size = val; }

    void paddingIs( double val ) { _padding = val; }

    Layout1d child( int idx ) const { return _children[ idx ]; }
    void parentIs( Layout1d * val ) { _parent = val; }
    void addChild( const Layout1d & child ) { _children.push_back( child ); }

    void baseSizePass();
    void growSizePass();
    void computeLayout() {
        baseSizePass();
        growSizePass();
    }

private:
    SizeSpec _sizeSpec = SizeSpec::absolute( 0 );
    double _padding = 0;
    double _childGap = 0;

    double _size = 0;

    Layout1d * _parent;
    std::vector< Layout1d > _children;
};

class Component {
public:
    virtual ~Component() = default;

    // Adjust sizes for child components that need to grow to the bounds of their
    // parent (this) container.
    virtual void growSizePass() {}
    // Adjust size for this component if it should shrink to fit the bounds of its
    // child components.
    virtual void fitSizePass() {}
    virtual ComponentSize size() const = 0;
    virtual void draw( Vector2 at, double deltaTime ) = 0;
    // void sizeSpecIs( SizeSpec sizeSpec ) { _sizeSpec = sizeSpec; }

    // ComponentSize _size;
    // SizeSpec _sizeSpec;

    Component * parent;
    std::vector< std::unique_ptr< Component > > _children;
};

// class Rectangle: public Component {
// public:
//     Rectangle() {}

//     ComponentSize size() const { return _size; }
//     void draw( Vector2 at, double deltaTime );
// };

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

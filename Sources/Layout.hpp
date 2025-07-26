#pragma once

#include <assert.h>
#include <vector>

#include "SizeTypes.hpp"

using ComponentSize = Vector2;

class SizeSpec {
public:
    constexpr static SizeSpec fit() { return SizeSpec( FitTag {} ); }
    constexpr static SizeSpec shrinkAcrossAxis() {
        return SizeSpec( ShrinkAcrossAxis {} );
    }
    constexpr static SizeSpec grow() { return SizeSpec( GrowTag {} ); }
    constexpr static SizeSpec growAcrossAxis() {
        return SizeSpec( GrowAcrossAxis {} );
    }
    constexpr static SizeSpec absolute( double val ) { return SizeSpec( val ); }

    bool isFit() const { return std::holds_alternative< FitTag >( _variant ); }
    bool isShrinkAcrossAxis() const {
        return std::holds_alternative< ShrinkAcrossAxis >( _variant );
    }
    bool isGrow() const { return std::holds_alternative< GrowTag >( _variant ); }
    bool isGrowAcrossAxis() const {
        return std::holds_alternative< GrowAcrossAxis >( _variant );
    }
    bool isAbsolute() const {
        return std::holds_alternative< Absolute >( _variant );
    }
    double getAbsolute() const { return std::get< Absolute >( _variant ).val; }

private:
    struct FitTag {};
    struct ShrinkAcrossAxis {};
    struct GrowTag {};
    struct GrowAcrossAxis {};
    struct Absolute {
        double val;
    };

    constexpr SizeSpec( FitTag fitTag ): _variant( fitTag ) {}
    constexpr SizeSpec( ShrinkAcrossAxis tag ): _variant( tag ) {}
    constexpr SizeSpec( GrowTag growTag ): _variant( growTag ) {}
    constexpr SizeSpec( GrowAcrossAxis tag ): _variant( tag ) {}
    constexpr SizeSpec( double absolute ): _variant( Absolute { absolute } ) {}

    std::variant< FitTag, ShrinkAcrossAxis, GrowTag, GrowAcrossAxis, Absolute > _variant;
};

class Layout;
class LayoutManager;

struct LayoutHandle {
public:
    LayoutHandle( LayoutManager * manager, int index ):
        _manager( manager ), _index( index ) {}

    int index() const { return _index; }
    bool valid() const { return _manager != nullptr; }

    const Layout * getLayoutConst() const;
    const Layout * operator->() const { return getLayoutConst(); }
    Layout * getLayoutMut();
    Layout * operator->() { return getLayoutMut(); }

private:
    LayoutManager * _manager;
    int _index;
    // int generation;
};

class Layout {
public:
    constexpr const SizeSpec & sizeSpec() const { return _sizeSpec; }
    constexpr double size() const { return _size; }
    constexpr double padding() const { return _padding; }
    constexpr double childGap() const { return _childGap; }
    const std::vector< LayoutHandle > & children() const { return _children; }
    std::vector< LayoutHandle > & childrenMut() { return _children; }
    LayoutHandle child( int idx ) const { return _children[ idx ]; }

    void paddingIs( double val ) { _padding = val; }
    void childGapIs( double val ) { _childGap = val; }
    void sizeIs( double val ) { _size = val; }
    void sizeSpecIs( SizeSpec val ) { _sizeSpec = val; }
    void parentIs( const LayoutHandle & val ) {
        assert( val.valid() );
        _parent = val;
    }
    void addChild( const LayoutHandle & child ) { _children.push_back( child ); }

    void computeLayout();

private:
    LayoutManager * _manager;
    SizeSpec _sizeSpec = SizeSpec::absolute( 0 );
    double _padding = 0;
    double _childGap = 0;

    double _size = 0;

    std::optional< LayoutHandle > _parent;
    std::vector< LayoutHandle > _children;

    void baseSizePass();
    void growAcrossAxis();
    void growAlongAxis();
    void growSizePass();
    void fitSizePass();
};

class LayoutManager {
public:
    LayoutHandle createLayout();

    const Layout * getLayoutConst( const LayoutHandle * handle ) const {
        return &( _layouts.at( handle->index() ) );
    }
    Layout * getLayoutMut( const LayoutHandle * handle ) {
        return &( _layouts.at( handle->index() ) );
    }

    void computeLayout( const LayoutHandle & handle );

private:
    void baseSizePass( const LayoutHandle & handle );
    void growSizePass( const LayoutHandle & handle );

    std::vector< Layout > _layouts;
};



struct DrawContext {
    Vector2 at;
    double deltaTime;
    Vector2 mousePos;
};

class ComponentV2 {
public:
    ComponentV2( LayoutManager & layoutManager ):
        _xLayout( layoutManager.createLayout() ),
        _yLayout( layoutManager.createLayout() ) {}
    virtual ~ComponentV2() = default;

    virtual void handleXLayoutChange() {}
    void xLayoutSizeSpecIs( SizeSpec val ) {
        _xLayout->sizeSpecIs( val );
        handleXLayoutChange();
    }
    LayoutHandle & xLayoutMut() { return _xLayout; }
    LayoutHandle & yLayoutMut() { return _yLayout; }

    const LayoutHandle & xLayoutConst() const { return _xLayout; }
    const LayoutHandle & yLayoutConst() const { return _yLayout; }
    ComponentSize size() const {
        return ComponentSize( xLayoutConst()->size(), yLayoutConst()->size() );
    }

    void addChild( ComponentV2 * child ) {
        _children.push_back( child );
        child->_parent = this;

        child->xLayoutMut()->parentIs( xLayoutMut() );
        xLayoutMut()->addChild( child->xLayoutMut() );
        child->yLayoutMut()->parentIs( yLayoutMut() );
        yLayoutMut()->addChild( child->yLayoutMut() );
    };

    void computeLayout() {
        _xLayout->computeLayout();
        _yLayout->computeLayout();
    }

    virtual void draw( const DrawContext & ctx ) = 0;

protected:
    ComponentV2 * _parent;
    std::vector< ComponentV2 * > _children;

private:
    LayoutHandle _xLayout;
    LayoutHandle _yLayout;
};

class RectangleV2: public ComponentV2 {
public:
    RectangleV2( LayoutManager & layoutManager, rl::Color fillColor ):
        ComponentV2( layoutManager ),
        _fillColor( fillColor ) {}

    void draw( const DrawContext & ctx ) override;

private:
    rl::Color _fillColor;
};

class VStackV2: public ComponentV2 {
public:
    VStackV2( LayoutManager & layoutManager ):
        ComponentV2( layoutManager ) {}

    void draw( const DrawContext & ctx ) override;
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

class Text: public ComponentV2 {
public:
    Text( LayoutManager & layoutManager,
          const std::string & content,
          const rl::Font & font,
          double fontSize,
          double textSpacing,
          const rl::Color & textColor );

    void draw( const DrawContext & ctx ) override;

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

    double contentSize() const { return _contentSize; }
    double scrollRegionSize() const { return _scrollRegionSize; }
    double offset() const { return _offset; }

    void scrollRegionSizeIs( double val ) {
        _scrollRegionSize = val;
        _offset = 0;
    }

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
    ScrollingText( LayoutManager & layoutManager,
                   const std::string & content,
                   const rl::Font & font,
                   double fontSize,
                   double textSpacing,
                   const rl::Color & textColor,
                   double scrollSpeed );
    ~ScrollingText();

    void draw( const DrawContext & ctx ) override;

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

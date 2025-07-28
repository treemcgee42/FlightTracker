// Copyright (C) 2025 by Runi Malladi

#pragma once

#include <assert.h>
#include <optional>
#include <variant>
#include <vector>

namespace Dile {

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

} // namespace Dile

// Copyright (C) 2025 by Runi Malladi

#pragma once

#include <assert.h>
#include <optional>
#include <variant>
#include <vector>

namespace Dile {

class SizeSpec {
public:
    // --- Factory functions
    [[nodiscard]] constexpr static SizeSpec fit() noexcept {
        return SizeSpec( FitTag {} );
    }
    [[nodiscard]] constexpr static SizeSpec shrinkAcrossAxis() noexcept {
        return SizeSpec( ShrinkAcrossAxis {} );
    }
    [[nodiscard]] constexpr static SizeSpec grow() noexcept {
        return SizeSpec( GrowTag {} );
    }
    [[nodiscard]] constexpr static SizeSpec growAcrossAxis() noexcept {
        return SizeSpec( GrowAcrossAxis {} );
    }
    [[nodiscard]] constexpr static SizeSpec absolute( double val ) noexcept {
        return SizeSpec( val );
    }

    [[nodiscard]] constexpr bool isFit() const noexcept {
        return std::holds_alternative< FitTag >( _variant );
    }
    [[nodiscard]] constexpr bool isShrinkAcrossAxis() const noexcept {
        return std::holds_alternative< ShrinkAcrossAxis >( _variant );
    }
    [[nodiscard]] constexpr bool isGrow() const noexcept {
        return std::holds_alternative< GrowTag >( _variant );
    }
    [[nodiscard]] constexpr bool isGrowAcrossAxis() const noexcept {
        return std::holds_alternative< GrowAcrossAxis >( _variant );
    }
    [[nodiscard]] constexpr std::optional< double > isAbsolute() const noexcept {
        if( std::holds_alternative< Absolute >( _variant ) ) {
            return std::get< Absolute >( _variant ).val;
        }
        return std::nullopt;
    }

private:
    struct FitTag {};
    struct ShrinkAcrossAxis {};
    struct GrowTag {};
    struct GrowAcrossAxis {};
    struct Absolute {
        double val;
    };

    constexpr explicit SizeSpec( FitTag fitTag ) noexcept: _variant( fitTag ) {}
    constexpr explicit SizeSpec( ShrinkAcrossAxis tag ) noexcept: _variant( tag ) {}
    constexpr explicit SizeSpec( GrowTag growTag ) noexcept: _variant( growTag ) {}
    constexpr explicit SizeSpec( GrowAcrossAxis tag ) noexcept: _variant( tag ) {}
    constexpr explicit SizeSpec( double absolute ) noexcept:
        _variant( Absolute { absolute } ) {}

    std::variant< FitTag, ShrinkAcrossAxis, GrowTag, GrowAcrossAxis, Absolute > _variant;
};

class Layout;
class LayoutManager;

struct LayoutHandle {
public:
    constexpr LayoutHandle( LayoutManager * manager, int index ) noexcept:
        _manager( manager ), _index( index ) {}

    [[nodiscard]] constexpr int index() const noexcept {
        return _index;
    }
    [[nodiscard]] constexpr bool valid() const noexcept {
        return _manager != nullptr;
    }

    [[nodiscard]] const Layout * getLayoutConst() const;
    [[nodiscard]] const Layout * operator->() const { return getLayoutConst(); }
    [[nodiscard]] Layout * getLayoutMut();
    [[nodiscard]] Layout * operator->() { return getLayoutMut(); }

private:
    LayoutManager * _manager;
    int _index;
    // int generation;
};

class Layout {
public:
    [[nodiscard]] constexpr const SizeSpec & sizeSpec() const noexcept {
        return _sizeSpec;
    }
    [[nodiscard]] constexpr double size() const noexcept { return _size; }
    [[nodiscard]] constexpr double padding() const noexcept { return _padding; }
    [[nodiscard]] constexpr double childGap() const noexcept { return _childGap; }
    [[nodiscard]] constexpr const std::vector< LayoutHandle > &
    children() const noexcept {
        return _children;
    }
    [[nodiscard]] constexpr std::vector< LayoutHandle > & childrenMut() noexcept {
        return _children;
    }
    [[nodiscard]] constexpr LayoutHandle child( int idx ) const noexcept {
        return _children[ idx ];
    }

    constexpr void paddingIs( double val ) noexcept { _padding = val; }
    constexpr void childGapIs( double val ) noexcept { _childGap = val; }
    constexpr void sizeIs( double val ) noexcept { _size = val; }
    constexpr void sizeSpecIs( SizeSpec val ) noexcept { _sizeSpec = val; }
    constexpr void parentIs( const LayoutHandle & val ) noexcept {
        assert( val.valid() );
        _parent = val;
    }
    constexpr void addChild( const LayoutHandle & child ) noexcept {
        _children.push_back( child );
    }

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
    LayoutHandle createLayout() noexcept;

    [[nodiscard]] constexpr const Layout *
    getLayoutConst( const LayoutHandle * handle ) const {
        return &( _layouts.at( handle->index() ) );
    }
    [[nodiscard]] constexpr Layout *
    getLayoutMut( const LayoutHandle * handle ) {
        return &( _layouts.at( handle->index() ) );
    }

    void computeLayout( const LayoutHandle & handle );

private:
    void baseSizePass( const LayoutHandle & handle );
    void growSizePass( const LayoutHandle & handle );

    std::vector< Layout > _layouts;
};

} // namespace Dile

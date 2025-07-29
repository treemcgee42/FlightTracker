// Copyright (C) 2025 by Runi Malladi

#include <spdlog/spdlog.h>

#include "Dile.hpp"

static std::shared_ptr< spdlog::logger > logger = spdlog::default_logger();

namespace Dile {

const Layout *
LayoutHandle::getLayoutConst() const {
    assert( valid() );
    return _manager->getLayoutConst( this );
}

Layout *
LayoutHandle::getLayoutMut() {
    assert( valid() );
    return _manager->getLayoutMut( this );
}

void
Layout::baseSizePass() {
    logger->trace( "" );
    if( const auto absoluteSize = _sizeSpec.isAbsolute();
        absoluteSize ) {
        sizeIs( *absoluteSize );
    } else if( _sizeSpec.isGrow() ||
               _sizeSpec.isGrowAcrossAxis() ||
               _sizeSpec.isFit() ||
               _sizeSpec.isShrinkAcrossAxis() ) {
        sizeIs( 0 );
    } else {
        assert( false );
    }

    for( LayoutHandle & child : childrenMut() ) {
        child->baseSizePass();
    }
}

// growing must happen top-down
void
Layout::growAcrossAxis() {
    logger->trace( "" );
    const double growSize = size() - 2 * padding();
    for( LayoutHandle & child : childrenMut() ) {
        if( child->sizeSpec().isGrowAcrossAxis() ) {
            child->sizeIs( growSize );
        }
    }
}

void
Layout::growAlongAxis() {
    logger->trace( "" );
    int numGrowChildren = 0;
    double availableSpace =
        size() - ( 2 * padding() ) - ( ( children().size() - 1 ) * childGap() );
    for( const LayoutHandle & child : children() ) {
        if( child->sizeSpec().isGrow() ) {
            assert( child->size() == 0 );
            numGrowChildren += 1;
        }
        availableSpace -= child->size();
    }

    const double growSize =
        std::max( availableSpace / static_cast< double >( numGrowChildren ), 0.0 );
    for( LayoutHandle & child : childrenMut() ) {
        if( child->sizeSpec().isGrow() ) {
            child->sizeIs( growSize );
        }
    }
}

void
Layout::growSizePass() {
    logger->trace( "" );
    if( children().size() == 0 ) {
        return;
    }

    growAcrossAxis();
    growAlongAxis();

   for( LayoutHandle & child : childrenMut() ) {
       child->growSizePass();
   }
}

void
Layout::fitSizePass() {
    logger->trace( "" );
    // The size of fit layout depends on the sizes of its children. So this is bottom
    // to top.
    for( LayoutHandle & child : childrenMut() ) {
        child->fitSizePass();
    }

    if( _sizeSpec.isFit() ) {
        if( children().size() == 0 ) {
            sizeIs( 0 );
            return;
        }

        double size = 2 * padding() + ( children().size() - 1 ) * childGap();
        for( const LayoutHandle & child : children() ) {
            size += child->size();
        }
        sizeIs( size );
    } else if( _sizeSpec.isShrinkAcrossAxis() ) {
        if( children().size() == 0 ) {
            sizeIs( 0 );
            return;
        }

        double maxChildSize = 0;
        for( const LayoutHandle & child : children() ) {
            maxChildSize = std::max( maxChildSize, child->size() );
        }
        sizeIs( 2 * padding() + maxChildSize );
    }
}

void
Layout::computeLayout() {
    logger->debug( "" );
    baseSizePass();
    fitSizePass();
    growSizePass();
}

LayoutHandle
LayoutManager::createLayout() noexcept  {
    logger->info( "allocating index {}", _layouts.size() );
    _layouts.push_back( Layout() );
    return LayoutHandle( this, _layouts.size() - 1 );
}

}


#include <doctest/doctest.h>

#include "Layout.hpp"

TEST_CASE("expanding") {
    const double rootSize = 80;
    double padding;
    SUBCASE( "" ) { padding = 0; }
    SUBCASE( "" ) { padding = 10; }
    CAPTURE( padding );
    double childGap;
    SUBCASE( "" ) { childGap = 0; }
    SUBCASE( "" ) { childGap = 1; }
    CAPTURE( childGap );

    LayoutManager layoutManager;

    const LayoutHandle mainLayoutHandle = layoutManager.createLayout();
    {
        Layout & mainLayout = layoutManager.getLayoutMut( mainLayoutHandle );
        mainLayout.sizeSpecIs( SizeSpec::absolute( rootSize ) );
        mainLayout.paddingIs( padding );
        mainLayout.childGapIs( childGap );
    }

    const LayoutHandle child0Handle = layoutManager.createLayout();
    {
        Layout & child0 = layoutManager.getLayoutMut( child0Handle );
        child0.parentIs( mainLayoutHandle );
        child0.sizeSpecIs( SizeSpec::grow() );
    }
    layoutManager.getLayoutMut( mainLayoutHandle ).addChild( child0Handle );

    layoutManager.computeLayout( mainLayoutHandle );
    REQUIRE( doctest::Approx( rootSize ) ==
             layoutManager.getLayoutConst( mainLayoutHandle ).size() );

    SUBCASE( "single child" ) {
        layoutManager.computeLayout( mainLayoutHandle );
        const double childSize = layoutManager.getLayoutConst( child0Handle ).size();
        const double mainSize =
            layoutManager.getLayoutConst( mainLayoutHandle ).size();
        CHECK( doctest::Approx( childSize ) ==
               ( mainSize - 2 * padding ) );
    }
    SUBCASE( "absolute child" ) {
        const double child1Size = 10;
        const LayoutHandle child1Handle = layoutManager.createLayout();
        {
            Layout & child1 = layoutManager.getLayoutMut( child1Handle );
            child1.parentIs( mainLayoutHandle );
            child1.sizeSpecIs( SizeSpec::absolute( child1Size ) );
            Layout & mainLayout = layoutManager.getLayoutMut( mainLayoutHandle );
            mainLayout.addChild( child1Handle );
        }

        layoutManager.computeLayout( mainLayoutHandle );
        const Layout & mainLayout = layoutManager.getLayoutConst( mainLayoutHandle );
        const Layout & child0Layout = layoutManager.getLayoutConst( child0Handle );
        const Layout & child1Layout = layoutManager.getLayoutConst( child1Handle );
        CHECK( child0Layout.size() ==
               doctest::Approx( rootSize - child1Size - 2 * padding - childGap ) );
        CHECK( child1Layout.size() == doctest::Approx( child1Size ) );
    }
    SUBCASE( "multiple expanding children" ) {
        const int numChildren = 3;
        std::vector< LayoutHandle > childHandles = { child0Handle };
        for( int i=0; i<( numChildren - 1 ); ++i ) {
            const LayoutHandle childHandle = layoutManager.createLayout();
            childHandles.push_back( childHandle );
            Layout & child = layoutManager.getLayoutMut( childHandle );
            child.parentIs( mainLayoutHandle );
            child.sizeSpecIs( SizeSpec::grow() );
            Layout & mainLayout = layoutManager.getLayoutMut( mainLayoutHandle );
            mainLayout.addChild( childHandle );
        }

        layoutManager.computeLayout( mainLayoutHandle );
        const double expectedChildSize =
            ( rootSize - 2 * padding - ( numChildren - 1 ) * childGap ) /
            static_cast< double >( numChildren );
        for( const LayoutHandle & childHandle : childHandles ) {
            const Layout & child = layoutManager.getLayoutConst( childHandle );
            CHECK( child.size() == doctest::Approx( expectedChildSize ) );
        }
    }
}
